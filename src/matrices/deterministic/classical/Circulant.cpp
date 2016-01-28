/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

template<typename T> 
void Circulant( Matrix<T>& A, const vector<T>& a )
{
    DEBUG_ONLY(CSE cse("Circulant"))
    const Int n = a.size();
    A.Resize( n, n );
    // NOTE: gcc (Ubuntu 5.2.1-22ubuntu2) 5.2.1 20151010 segfaults here
    //       if the return type of the lambda is not manually specified.
    auto circFill = [&]( Int i, Int j ) -> T { return a[Mod(i-j,n)]; };
    IndexDependentFill( A, function<T(Int,Int)>(circFill) );
}

template<typename T>
void Circulant( AbstractDistMatrix<T>& A, const vector<T>& a )
{
    DEBUG_ONLY(CSE cse("Circulant"))
    const Int n = a.size();
    A.Resize( n, n );
    auto circFill = [&]( Int i, Int j ) -> T { return a[Mod(i-j,n)]; };
    IndexDependentFill( A, function<T(Int,Int)>(circFill) );
}

#define PROTO(T) \
  template void Circulant( Matrix<T>& A, const vector<T>& a ); \
  template void Circulant( AbstractDistMatrix<T>& A, const vector<T>& a );

#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGINT
#define EL_ENABLE_BIGFLOAT
#include "El/macros/Instantiate.h"

} // namespace El