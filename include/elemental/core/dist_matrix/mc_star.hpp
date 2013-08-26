/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_CORE_DISTMATRIX_MC_STAR_DECL_HPP
#define ELEM_CORE_DISTMATRIX_MC_STAR_DECL_HPP

namespace elem {

// Partial specialization to A[MC,* ].
//
// The rows of these distributed matrices will be replicated on all 
// processes (*), and the columns will be distributed like "Matrix Columns" 
// (MC). Thus the columns will be distributed among columns of the process
// grid.
template<typename T>
class DistMatrix<T,MC,STAR> : public AbstractDistMatrix<T>
{
public:
    // Create a 0 x 0 distributed matrix
    DistMatrix( const elem::Grid& g=DefaultGrid() );

    // Create a height x width distributed matrix
    DistMatrix
    ( Int height, Int width, const elem::Grid& g=DefaultGrid() );

    // Create a height x width distributed matrix with specified alignments
    DistMatrix
    ( Int height, Int width, Int colAlignment, const elem::Grid& g );

    // Create a height x width distributed matrix with specified alignments
    // and leading dimension
    DistMatrix
    ( Int height, Int width, 
      Int colAlignment, Int ldim, const elem::Grid& g );

    // View a constant distributed matrix's buffer
    DistMatrix
    ( Int height, Int width, Int colAlignment, 
      const T* buffer, Int ldim, const elem::Grid& g );

    // View a mutable distributed matrix's buffer
    DistMatrix
    ( Int height, Int width, Int colAlignment,
      T* buffer, Int ldim, const elem::Grid& g );

    // Create a copy of distributed matrix A
    DistMatrix( const DistMatrix<T,MC,STAR>& A );
    template<Distribution U,Distribution V>
    DistMatrix( const DistMatrix<T,U,V>& A );

    // Move constructor
    DistMatrix( DistMatrix<T,MC,STAR>&& A );

    ~DistMatrix();

    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,MC,MR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,MC,STAR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,STAR,MR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,MD,STAR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,STAR,MD>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,MR,MC>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,MR,STAR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,STAR,MC>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,VC,STAR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,STAR,VC>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,VR,STAR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,STAR,VR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,STAR,STAR>& A );
    const DistMatrix<T,MC,STAR>& operator=( const DistMatrix<T,CIRC,CIRC>& A );

    DistMatrix<T,MC,STAR>& operator=( DistMatrix<T,MC,STAR>&& A );

    //------------------------------------------------------------------------//
    // Overrides of AbstractDistMatrix                                        //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    virtual Int ColStride() const;
    virtual Int RowStride() const;
    virtual Int ColRank() const;
    virtual Int RowRank() const;
    virtual elem::DistData DistData() const;

    //
    // Collective routines
    //

    virtual T Get( Int i, Int j ) const;
    virtual void Set( Int i, Int j, T alpha );
    virtual void Update( Int i, Int j, T alpha );

    virtual void ResizeTo( Int height, Int width );
    virtual void ResizeTo( Int height, Int width, Int ldim );

    // Distribution alignment
    virtual void AlignWith( const elem::DistData& data );
    virtual void AlignWith( const AbstractDistMatrix<T>& A );
    virtual void AlignColsWith( const elem::DistData& data );
    virtual void AlignColsWith( const AbstractDistMatrix<T>& A );

    //
    // Though the following routines are meant for complex data, all but two
    // logically applies to real data.
    //

    virtual void SetRealPart( Int i, Int j, BASE(T) u );
    // Only valid for complex datatypes
    virtual void SetImagPart( Int i, Int j, BASE(T) u );
    virtual void UpdateRealPart( Int i, Int j, BASE(T) u );
    // Only valid for complex datatypes
    virtual void UpdateImagPart( Int i, Int j, BASE(T) u );

    //------------------------------------------------------------------------//
    // Routines specific to [MC,* ] distribution                              //
    //------------------------------------------------------------------------//

    //
    // Collective routines
    //

    void GetDiagonal( DistMatrix<T,MC,STAR>& d, Int offset=0 ) const;
    void GetDiagonal( DistMatrix<T,STAR,MC>& d, Int offset=0 ) const;
    void SetDiagonal( const DistMatrix<T,MC,STAR>& d, Int offset=0 );
    void SetDiagonal( const DistMatrix<T,STAR,MC>& d, Int offset=0 );

    void AlignWithDiagonal( const elem::DistData& data, Int offset=0 );
    void AlignWithDiagonal( const AbstractDistMatrix<T>& A, Int offset=0 );
    bool AlignedWithDiagonal
    ( const elem::DistData& data, Int offset=0 ) const;
    bool AlignedWithDiagonal
    ( const AbstractDistMatrix<T>& A, Int offset=0 ) const;

    // (Immutable) view of a distributed matrix's buffer
    void Attach
    ( Int height, Int width, Int colAlignment,
      T* buffer, Int ldim, const elem::Grid& grid );
    void LockedAttach
    ( Int height, Int width, Int colAlignment,
      const T* buffer, Int ldim, const elem::Grid& grid );

    // AllReduce sum over process row
    void SumOverRow();

    //
    // Though the following routines are meant for complex data, all but two
    // logically applies to real data.
    //

    void GetRealPartOfDiagonal
    ( DistMatrix<BASE(T),MC,STAR>& d, Int offset=0 ) const;
    void GetImagPartOfDiagonal
    ( DistMatrix<BASE(T),MC,STAR>& d, Int offset=0 ) const;
    void GetRealPartOfDiagonal
    ( DistMatrix<BASE(T),STAR,MC>& d, Int offset=0 ) const;
    void GetImagPartOfDiagonal
    ( DistMatrix<BASE(T),STAR,MC>& d, Int offset=0 ) const;
    void SetRealPartOfDiagonal
    ( const DistMatrix<BASE(T),MC,STAR>& d, Int offset=0 );
    // Only valid for complex datatypes
    void SetImagPartOfDiagonal
    ( const DistMatrix<BASE(T),MC,STAR>& d, Int offset=0 );
    void SetRealPartOfDiagonal
    ( const DistMatrix<BASE(T),STAR,MC>& d, Int offset=0 );
    // Only valid for complex datatypes
    void SetImagPartOfDiagonal
    ( const DistMatrix<BASE(T),STAR,MC>& d, Int offset=0 );

private:
#ifndef SWIG
    template<typename S,Distribution U,Distribution V>
    friend class DistMatrix;
#endif // ifndef SWIG
};

} // namespace elem

#endif // ifndef ELEM_CORE_DISTMATRIX_MC_STAR_DECL_HPP
