/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
// NOTE: It is possible to simply include "elemental.hpp" instead
#include "elemental-lite.hpp"
#include "elemental/lapack-like/Cholesky.hpp"
#include "elemental/matrices/Uniform.hpp"
using namespace std;
using namespace elem;

// Typedef our real and complex types to 'Real' and 'C' for convenience
typedef double Real;
typedef Complex<Real> C;

int
main( int argc, char* argv[] )
{
    Initialize( argc, argv );

    try 
    {
        const Int n = Input("--size","size of HPSD matrix",100);
        const bool print = Input("--print","print matrices?",false);
        ProcessInput();
        PrintInputReport();

        const Grid& g = DefaultGrid();
        auto L = Uniform<C>( g, n, n );
        MakeTrapezoidal( LOWER, L, -1 );
        auto A = Zeros<C>( g, n, n );
        Herk( LOWER, NORMAL, C(1), L, C(0), A );
        if( print )
            Print( A, "A" );

        // Replace A with its Cholesky factor
        HPSDCholesky( LOWER, A );
        if( print )
        {
            MakeTriangular( LOWER, A );
            Print( A, "chol(A)" );
        }
    }
    catch( exception& e ) { ReportException(e); }

    Finalize();
    return 0;
}
