/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
// NOTE: It is possible to simply include "elemental.hpp" instead
#include "elemental-lite.hpp"
#include "elemental/blas-like/level1/Axpy.hpp"
#include "elemental/blas-like/level1/Scale.hpp"
#include "elemental/lapack-like/Norm/Frobenius.hpp"
#include "elemental/lapack-like/Norm/EntrywiseOne.hpp"
#include "elemental/lapack-like/Norm/Zero.hpp"
#include "elemental/convex/SingularValueSoftThreshold.hpp"
#include "elemental/matrices/Uniform.hpp"
#include <set>
using namespace elem;

// NOTE: Loosely based on the algorithm given in 

// Corrupt a percentage of the entries with uniform samples from the unit ball
template<typename F>
int Corrupt( DistMatrix<F>& A, double percentCorrupt )
{
#ifndef RELEASE
    PushCallStack("Corrupt");
#endif
    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int localSize = localHeight*localWidth;
    int numLocalCorrupt = (percentCorrupt/100.)*localSize;
    int numLocalCollisions = 0;
    std::set<int> localIndices;
    for( int k=0; k<numLocalCorrupt; ++k )
    {
        const int localIndex = rand() % localSize;
        if( localIndices.count(localIndex) != 0 )
        {
            ++numLocalCollisions;
            continue;
        } 
        const int iLocal = localIndex % localHeight;
        const int jLocal = localIndex / localHeight;
        const F perturb = SampleUnitBall<F>();
        A.SetLocal( iLocal, jLocal, A.GetLocal(iLocal,jLocal)+perturb );
    }
    
    int numCorrupt;
    numLocalCorrupt -= numLocalCollisions;
    mpi::AllReduce
    ( &numLocalCorrupt, &numCorrupt, 1, mpi::SUM, A.Grid().VCComm() );
#ifndef RELEASE
    PopCallStack();
#endif
    return numCorrupt;
}

template<typename F>
void RPCA_ADMM
( const DistMatrix<F>& M, DistMatrix<F>& L, DistMatrix<F>& S, 
  typename Base<F>::type beta, 
  typename Base<F>::type tau, 
  typename Base<F>::type tol, 
  int maxIts,
  bool print )
{
    const int m = M.Height();
    const int n = M.Width();
    const int commRank = mpi::CommRank( M.Grid().Comm() );

    DistMatrix<F> E( M.Grid() ), Y( M.Grid() );
    Zeros( m, n, Y );

    const double frobM = FrobeniusNorm( M );
    if( commRank == 0 )
        std::cout << "|| M ||_F = " << frobM << std::endl;

    int numIts = 0;
    while( true )
    {
        // ST_{tau/beta}(M - L + Y/beta)
        S = M;
        Axpy( -1., L, S );
        Axpy( 1./beta, Y, S );
        SoftThreshold( S, tau/beta );
        const int numNonzeros = ZeroNorm( S );

        // SVT_{1/beta}(M - S + Y/beta)
        L = M;
        Axpy( -1., S, L );
        Axpy( 1./beta, Y, L );
        const int rank = SingularValueSoftThreshold( L, 1./beta );
      
        // E := M - (L + S)
        E = M;    
        Axpy( -1., L, E );
        Axpy( -1., S, E );
        const double frobE = FrobeniusNorm( E );

        if( frobE/frobM <= tol )            
        {
            if( commRank == 0 )
                std::cout << "Converged after " << numIts << " iterations "
                          << " with rank=" << rank 
                          << ", numNonzeros=" << numNonzeros << " and "
                          << "|| E ||_F / || M ||_F = " << frobE/frobM
                          << std::endl;
            break;
        }
        else if( numIts >= maxIts )
        {
            if( commRank == 0 )
                std::cout << "Aborting after " << maxIts << " iterations"
                          << std::endl;
            break;
        }
        else
        {
            if( commRank == 0 )
                std::cout << numIts << ": || E ||_F / || M ||_F = " 
                          << frobE/frobM << ", rank=" << rank 
                          << ", numNonzeros=" << numNonzeros << std::endl;
        }
        
        // Y := Y + beta E
        Axpy( beta, E, Y );
        ++numIts;
    }
}

int 
main( int argc, char* argv[] )
{
    Initialize( argc, argv );
    mpi::Comm comm = mpi::COMM_WORLD;
    const int commRank = mpi::CommRank( comm );

    try
    {
        const int m = Input("--height","height of matrix",100);
        const int n = Input("--width","width of matrix",100);
        const int rank = Input("--rank","rank of structured matrix",10);
        const double percentCorrupt = 
            Input("--percentCorrupt","percentage of corrupted entries",10.);
        const double tau = Input("--tau","sparse weighting factor",0.1);
        const double beta = Input("--beta","step size",1.);
        const int maxIts = Input("--maxIts","maximum iterations",1000);
        const double tol = Input("--tol","tolerance",1.e-6);
        const bool print = Input("--print","print matrices",false);
        ProcessInput();
        PrintInputReport();

        DistMatrix<double> LTrue;
        {
            DistMatrix<double> U, V;
            Uniform( m, rank, U );
            Uniform( n, rank, V );
            Zeros( m, n, LTrue );
            Gemm( NORMAL, ADJOINT, 1./std::max(m,n), U, V, 0., LTrue );
        }
        const double frobLTrue = FrobeniusNorm( LTrue );
        if( commRank == 0 )
            std::cout << "|| L ||_F = " << frobLTrue << std::endl;
        if( print )
            LTrue.Print("True L");

        DistMatrix<double> STrue;
        Zeros( m, n, STrue );
        const int numCorrupt = Corrupt( STrue, percentCorrupt );
        const double frobSTrue = FrobeniusNorm( STrue );
        if( commRank == 0 )
            std::cout << "number of corrupted entries: " << numCorrupt << "\n"
                      << "|| S ||_F = " << frobSTrue << std::endl;
        if( print )
            STrue.Print("True S");

        // M = LTrue + STrue
        DistMatrix<double> M( LTrue );
        Axpy( 1., STrue, M );

        DistMatrix<double> L, S;
        Zeros( m, n, L );
        Zeros( m, n, S ); 
        RPCA_ADMM( M, L, S, beta, tau, tol, maxIts, print );

        if( print )
        {
            L.Print("L");
            S.Print("S"); 
        }
        Axpy( -1., LTrue, L );
        Axpy( -1., STrue, S );
        const double frobLDiff = FrobeniusNorm( L );
        const double frobSDiff = FrobeniusNorm( S );
        if( commRank == 0 )
            std::cout << "|| L - LTrue ||_F / || LTrue ||_F = " 
                      << frobLDiff/frobLTrue << "\n"
                      << "|| S - STrue ||_F / || STrue ||_F = " 
                      << frobSDiff/frobSTrue << "\n"
                      << std::endl;
        if( print )
        {
            L.Print("L - LTrue");
            S.Print("S - STrue");
        }
    }
    catch( ArgException& e )
    {
        // There is nothing to do
    }
    catch( std::exception& e )
    {
        std::ostringstream os;
        os << "Process " << commRank << " caught error message:\n" << e.what()
           << std::endl;
        std::cerr << os.str();
#ifndef RELEASE
        DumpCallStack();
#endif
    }

    Finalize();
    return 0;
}
