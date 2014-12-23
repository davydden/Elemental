/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {
namespace adjoint {

template<typename T,Dist U,Dist V>
void PartialColAllGather
( const DistMatrix<T,U,V           >& A, 
        DistMatrix<T,V,Partial<U>()>& B )
{
    DEBUG_ONLY(CallStackEntry cse("adjoint::PartialColAllGather"))
    transpose::PartialColAllGather( A, B, true );
}

#define PROTO_DIST(T,U,V) \
  template void PartialColAllGather \
  ( const DistMatrix<T,U,V           >& A, \
          DistMatrix<T,V,Partial<U>()>& B );

#define PROTO(T) \
  PROTO_DIST(T,CIRC,CIRC) \
  PROTO_DIST(T,MC,  MR  ) \
  PROTO_DIST(T,MC,  STAR) \
  PROTO_DIST(T,MD,  STAR) \
  PROTO_DIST(T,MR,  MC  ) \
  PROTO_DIST(T,MR,  STAR) \
  PROTO_DIST(T,STAR,MC  ) \
  PROTO_DIST(T,STAR,MD  ) \
  PROTO_DIST(T,STAR,MR  ) \
  PROTO_DIST(T,STAR,STAR) \
  PROTO_DIST(T,STAR,VC  ) \
  PROTO_DIST(T,STAR,VR  ) \
  PROTO_DIST(T,VC,  STAR) \
  PROTO_DIST(T,VR,  STAR)

#include "El/macros/Instantiate.h"

} // namespace adjoint
} // namespace El