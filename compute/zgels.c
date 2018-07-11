/**
 *
 * @file zgels.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgels wrappers
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 1.0.0
 * @author Jakub Kurzak
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2010-11-15
 * @precisions normal z -> s d c
 *
 */
#include "control/common.h"
#include <stdlib.h>

/**
 *
 * @ingroup CHAMELEON_Complex64_t
 *
 *  CHAMELEON_zgels - solves overdetermined or underdetermined linear systems involving an M-by-N
 *  matrix A using the QR or the LQ factorization of A.  It is assumed that A has full rank.
 *  The following options are provided:
 *
 *  # trans = ChamNoTrans and M >= N: find the least squares solution of an overdetermined
 *    system, i.e., solve the least squares problem: minimize || B - A*X ||.
 *
 *  # trans = ChamNoTrans and M < N:  find the minimum norm solution of an underdetermined
 *    system A * X = B.
 *
 *  Several right hand side vectors B and solution vectors X can be handled in a single call;
 *  they are stored as the columns of the M-by-NRHS right hand side matrix B and the N-by-NRHS
 *  solution matrix X.
 *
 *******************************************************************************
 *
 * @param[in] trans
 *          Intended usage:
 *          = ChamNoTrans:   the linear system involves A;
 *          = ChamConjTrans: the linear system involves A**H.
 *          Currently only ChamNoTrans is supported.
 *
 * @param[in] M
 *          The number of rows of the matrix A. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrix A. N >= 0.
 *
 * @param[in] NRHS
 *          The number of right hand sides, i.e., the number of columns of the matrices B and X.
 *          NRHS >= 0.
 *
 * @param[in,out] A
 *          On entry, the M-by-N matrix A.
 *          On exit,
 *          if M >= N, A is overwritten by details of its QR factorization as returned by
 *                     CHAMELEON_zgeqrf;
 *          if M < N, A is overwritten by details of its LQ factorization as returned by
 *                      CHAMELEON_zgelqf.
 *
 * @param[in] LDA
 *          The leading dimension of the array A. LDA >= max(1,M).
 *
 * @param[out] descT
 *          On exit, auxiliary factorization data.
 *
 * @param[in,out] B
 *          On entry, the M-by-NRHS matrix B of right hand side vectors, stored columnwise;
 *          On exit, if return value = 0, B is overwritten by the solution vectors, stored
 *          columnwise:
 *          if M >= N, rows 1 to N of B contain the least squares solution vectors; the residual
 *          sum of squares for the solution in each column is given by the sum of squares of the
 *          modulus of elements N+1 to M in that column;
 *          if M < N, rows 1 to N of B contain the minimum norm solution vectors;
 *
 * @param[in] LDB
 *          The leading dimension of the array B. LDB >= MAX(1,M,N).
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zgels_Tile
 * @sa CHAMELEON_zgels_Tile_Async
 * @sa CHAMELEON_cgels
 * @sa CHAMELEON_dgels
 * @sa CHAMELEON_sgels
 *
 */
int CHAMELEON_zgels( cham_trans_t trans, int M, int N, int NRHS,
                 CHAMELEON_Complex64_t *A, int LDA,
                 CHAM_desc_t *descT,
                 CHAMELEON_Complex64_t *B, int LDB )
{
    int i, j;
    int NB;
    int status;
    CHAM_context_t *morse;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    CHAM_desc_t descAl, descAt;
    CHAM_desc_t descBl, descBt;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zgels", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }

    /* Check input arguments */
    if (trans != ChamNoTrans) {
        morse_error("CHAMELEON_zgels", "only ChamNoTrans supported");
        return CHAMELEON_ERR_NOT_SUPPORTED;
    }
    if (M < 0) {
        morse_error("CHAMELEON_zgels", "illegal value of M");
        return -2;
    }
    if (N < 0) {
        morse_error("CHAMELEON_zgels", "illegal value of N");
        return -3;
    }
    if (NRHS < 0) {
        morse_error("CHAMELEON_zgels", "illegal value of NRHS");
        return -4;
    }
    if (LDA < chameleon_max(1, M)) {
        morse_error("CHAMELEON_zgels", "illegal value of LDA");
        return -6;
    }
    if (LDB < chameleon_max(1, chameleon_max(M, N))) {
        morse_error("CHAMELEON_zgels", "illegal value of LDB");
        return -9;
    }
    /* Quick return */
    if (chameleon_min(M, chameleon_min(N, NRHS)) == 0) {
        for (i = 0; i < chameleon_max(M, N); i++)
            for (j = 0; j < NRHS; j++)
                B[j*LDB+i] = 0.0;
        return CHAMELEON_SUCCESS;
    }

    /* Tune NB & IB depending on M, N & NRHS; Set NBNB */
    status = morse_tune(CHAMELEON_FUNC_ZGELS, M, N, NRHS);
    if (status != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zgels", "morse_tune() failed");
        return status;
    }

    /* Set NT */
    NB = CHAMELEON_NB;

    morse_sequence_create( morse, &sequence );

    /* Submit the matrix conversion */
    if ( M >= N ) {
        morse_zlap2tile( morse, &descAl, &descAt, ChamDescInout, ChamUpperLower,
                         A, NB, NB, LDA, N, M, N, sequence, &request );
        morse_zlap2tile( morse, &descBl, &descBt, ChamDescInout, ChamUpperLower,
                         B, NB, NB, LDB, NRHS, M, NRHS, sequence, &request );
    } else {
        /* Submit the matrix conversion */
        morse_zlap2tile( morse, &descAl, &descAt, ChamDescInout, ChamUpperLower,
                         A, NB, NB, LDA, N, M, N, sequence, &request );
        morse_zlap2tile( morse, &descBl, &descBt, ChamDescInout, ChamUpperLower,
                         B, NB, NB, LDB, NRHS, N, NRHS, sequence, &request );
    }

    /* Call the tile interface */
    CHAMELEON_zgels_Tile_Async( ChamNoTrans, &descAt, descT, &descBt, sequence, &request );

    /* Submit the matrix conversion back */
    morse_ztile2lap( morse, &descAl, &descAt,
                     ChamDescInout, ChamUpperLower, sequence, &request );
    morse_ztile2lap( morse, &descBl, &descBt,
                     ChamDescInout, ChamUpperLower, sequence, &request );
    CHAMELEON_Desc_Flush( descT, sequence );

    morse_sequence_wait( morse, sequence );

    /* Cleanup the temporary data */
    morse_ztile2lap_cleanup( morse, &descAl, &descAt );
    morse_ztile2lap_cleanup( morse, &descBl, &descBt );

    status = sequence->status;
    morse_sequence_destroy( morse, sequence );
    return status;
}

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t_Tile
 *
 *  CHAMELEON_zgels_Tile - Solves overdetermined or underdetermined linear system of equations
 *  using the tile QR or the tile LQ factorization.
 *  Tile equivalent of CHAMELEON_zgels().
 *  Operates on matrices stored by tiles.
 *  All matrices are passed through descriptors.
 *  All dimensions are taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in] trans
 *          Intended usage:
 *          = ChamNoTrans:   the linear system involves A;
 *          = ChamConjTrans: the linear system involves A**H.
 *          Currently only ChamNoTrans is supported.
 *
 * @param[in,out] A
 *          On entry, the M-by-N matrix A.
 *          On exit,
 *          if M >= N, A is overwritten by details of its QR factorization as returned by
 *                     CHAMELEON_zgeqrf;
 *          if M < N, A is overwritten by details of its LQ factorization as returned by
 *                      CHAMELEON_zgelqf.
 *
 * @param[out] T
 *          On exit, auxiliary factorization data.
 *
 * @param[in,out] B
 *          On entry, the M-by-NRHS matrix B of right hand side vectors, stored columnwise;
 *          On exit, if return value = 0, B is overwritten by the solution vectors, stored
 *          columnwise:
 *          if M >= N, rows 1 to N of B contain the least squares solution vectors; the residual
 *          sum of squares for the solution in each column is given by the sum of squares of the
 *          modulus of elements N+1 to M in that column;
 *          if M < N, rows 1 to N of B contain the minimum norm solution vectors;
 *
 *******************************************************************************
 *
 * @return
 *          \return CHAMELEON_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zgels
 * @sa CHAMELEON_zgels_Tile_Async
 * @sa CHAMELEON_cgels_Tile
 * @sa CHAMELEON_dgels_Tile
 * @sa CHAMELEON_sgels_Tile
 *
 */
int CHAMELEON_zgels_Tile( cham_trans_t trans, CHAM_desc_t *A,
                      CHAM_desc_t *T, CHAM_desc_t *B )
{
    CHAM_context_t *morse;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zgels_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create( morse, &sequence );

    CHAMELEON_zgels_Tile_Async( trans, A, T, B, sequence, &request );

    CHAMELEON_Desc_Flush( A, sequence );
    CHAMELEON_Desc_Flush( T, sequence );
    CHAMELEON_Desc_Flush( B, sequence );

    morse_sequence_wait( morse, sequence );
    status = sequence->status;
    morse_sequence_destroy( morse, sequence );
    return status;
}

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t_Tile_Async
 *
 *  CHAMELEON_zgels_Tile_Async - Solves overdetermined or underdetermined linear
 *  system of equations using the tile QR or the tile LQ factorization.
 *  Non-blocking equivalent of CHAMELEON_zgels_Tile().
 *  May return before the computation is finished.
 *  Allows for pipelining of operations at runtime.
 *
 *******************************************************************************
 *
 * @param[in] sequence
 *          Identifies the sequence of function calls that this call belongs to
 *          (for completion checks and exception handling purposes).
 *
 * @param[out] request
 *          Identifies this function call (for exception handling purposes).
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zgels
 * @sa CHAMELEON_zgels_Tile
 * @sa CHAMELEON_cgels_Tile_Async
 * @sa CHAMELEON_dgels_Tile_Async
 * @sa CHAMELEON_sgels_Tile_Async
 *
 */
int CHAMELEON_zgels_Tile_Async( cham_trans_t trans, CHAM_desc_t *A,
                            CHAM_desc_t *T, CHAM_desc_t *B,
                            RUNTIME_sequence_t *sequence, RUNTIME_request_t *request )
{
    CHAM_desc_t *subA;
    CHAM_desc_t *subB;
    CHAM_context_t *morse;
    CHAM_desc_t D, *Dptr = NULL;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zgels_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("CHAMELEON_zgels_Tile", "NULL sequence");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("CHAMELEON_zgels_Tile", "NULL request");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    /* Check sequence status */
    if (sequence->status == CHAMELEON_SUCCESS) {
        request->status = CHAMELEON_SUCCESS;
    }
    else {
        return morse_request_fail(sequence, request, CHAMELEON_ERR_SEQUENCE_FLUSHED);
    }

    /* Check descriptors for correctness */
    if (morse_desc_check(A) != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zgels_Tile", "invalid first descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(T) != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zgels_Tile", "invalid second descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(B) != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zgels_Tile", "invalid third descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (A->nb != A->mb || B->nb != B->mb) {
        morse_error("CHAMELEON_zgels_Tile", "only square tiles supported");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (trans != ChamNoTrans) {
        morse_error("CHAMELEON_zgels_Tile", "only ChamNoTrans supported");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_NOT_SUPPORTED);
    }
    /* Quick return  - currently NOT equivalent to LAPACK's:
     if (chameleon_min(M, chameleon_min(N, NRHS)) == 0) {
     for (i = 0; i < chameleon_max(M, N); i++)
     for (j = 0; j < NRHS; j++)
     B[j*LDB+i] = 0.0;
     return CHAMELEON_SUCCESS;
     }
     */
    if (A->m >= A->n) {

#if defined(CHAMELEON_COPY_DIAG)
        {
            int n = chameleon_min(A->mt, A->nt) * A->nb;
            morse_zdesc_alloc(D, A->mb, A->nb, A->m, n, 0, 0, A->m, n, );
            Dptr = &D;
        }
#endif
        if (morse->householder == ChamFlatHouseholder) {

            morse_pzgeqrf( A, T, Dptr, sequence, request );

            morse_pzunmqr( ChamLeft, ChamConjTrans, A, B, T, Dptr, sequence, request );
        }
        else {
            morse_pzgeqrfrh( A, T, Dptr, CHAMELEON_RHBLK, sequence, request );

            morse_pzunmqrrh( ChamLeft, ChamConjTrans, A, B, T, Dptr, CHAMELEON_RHBLK, sequence, request );
        }
        subB = morse_desc_submatrix(B, 0, 0, A->n, B->n);
        subA = morse_desc_submatrix(A, 0, 0, A->n, A->n);
        morse_pztrsm( ChamLeft, ChamUpper, ChamNoTrans, ChamNonUnit, 1.0, subA, subB, sequence, request );

    }
    else {
        /* subB = morse_desc_submatrix(B, A->m, 0, A->n-A->m, B->n);
         morse_pzlaset( ChamUpperLower, 0., 0., subB, sequence, request );
         free(subB); */
#if defined(CHAMELEON_COPY_DIAG)
        {
            int m = chameleon_min(A->mt, A->nt) * A->mb;
            morse_zdesc_alloc(D, A->mb, A->nb, m, A->n, 0, 0, m, A->n, );
            Dptr = &D;
        }
#endif
        if (morse->householder == ChamFlatHouseholder) {
            morse_pzgelqf( A, T, Dptr, sequence, request );
        }
        else {
            morse_pzgelqfrh( A, T, Dptr, CHAMELEON_RHBLK, sequence, request );
        }
        subB = morse_desc_submatrix(B, 0, 0, A->m, B->n);
        subA = morse_desc_submatrix(A, 0, 0, A->m, A->m);
        morse_pztrsm( ChamLeft, ChamLower, ChamNoTrans, ChamNonUnit, 1.0, subA, subB, sequence, request );

        if (morse->householder == ChamFlatHouseholder) {
            morse_pzunmlq( ChamLeft, ChamConjTrans, A, B, T, Dptr, sequence, request );
        }
        else {
            morse_pzunmlqrh( ChamLeft, ChamConjTrans, A, B, T, Dptr, CHAMELEON_RHBLK, sequence, request );
        }
    }

    free(subA);
    free(subB);

    if (Dptr != NULL) {
        CHAMELEON_Desc_Flush( A, sequence );
        CHAMELEON_Desc_Flush( T, sequence );
        CHAMELEON_Desc_Flush( B, sequence );
        CHAMELEON_Desc_Flush( Dptr, sequence );
        morse_sequence_wait( morse, sequence );
        morse_desc_mat_free( Dptr );
    }
    (void)D;
    return CHAMELEON_SUCCESS;
}
