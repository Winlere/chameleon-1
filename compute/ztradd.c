/**
 *
 * @file ztradd.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon ztradd wrappers
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 1.0.0
 * @author Mathieu Faverge
 * @date 2011-11-03
 * @precisions normal z -> s d c
 *
 */
#include "control/common.h"

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t
 *
 *  CHAMELEON_ztradd - Performs a matrix addition similarly to the pztradd()
 *  function from the PBLAS library:
 *
 *    \f[ C = \alpha op( A ) + \beta B \f],
 *
 *  where op( X ) is one of
 *
 *    op( X ) = X  or op( X ) = X' or op( X ) = conjg( X' )
 *
 *  alpha and beta are scalars, and A, and B are two trapezoidal matrices, with
 *  op( A ) and B two m by n matrices.
 *
 *******************************************************************************
 *
 * @param[in] uplo
 *          Specifies the shape of A and B matrices:
 *          = ChamUpperLower: A and B are general matrices.
 *          = ChamUpper: op(A) and B are upper trapezoidal matrices.
 *          = ChamLower: op(A) and B are lower trapezoidal matrices.
 *
 * @param[in] trans
 *          Specifies whether the matrix A is transposed, not transposed or
 *          conjugate transposed:
 *          = ChamNoTrans:   A is not transposed;
 *          = ChamTrans:     A is transposed;
 *          = ChamConjTrans: A is conjugate transposed.
 *
 * @param[in] M
 *          M specifies the number of rows of the matrix op( A ) and of the matrix B. M >= 0.
 *
 * @param[in] N
 *          N specifies the number of columns of the matrix op( A ) and of the matrix B. N >= 0.
 *
 * @param[in] alpha
 *          alpha specifies the scalar alpha
 *
 * @param[in] A
 *          A is a LDA-by-ka matrix, where ka is N when trans = ChamNoTrans,
 *          and is M otherwise.
 *
 * @param[in] LDA
 *          The leading dimension of the array A. LDA >= max(1,K), where K is M
 *          when trans = ChamNoTrans, and is N when otherwise.
 *
 * @param[in] beta
 *          beta specifies the scalar beta
 *
 * @param[in,out] B
 *          B is a LDB-by-N matrix.
 *
 * @param[in] LDB
 *          The leading dimension of the array B. LDB >= max(1,M).
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_ztradd_Tile
 * @sa CHAMELEON_ctradd
 * @sa CHAMELEON_dtradd
 * @sa CHAMELEON_stradd
 *
 */
int CHAMELEON_ztradd( cham_uplo_t uplo, cham_trans_t trans, int M, int N,
                  CHAMELEON_Complex64_t alpha, CHAMELEON_Complex64_t *A, int LDA,
                  CHAMELEON_Complex64_t beta,  CHAMELEON_Complex64_t *B, int LDB )
{
    int NB;
    int Am, An;
    int status;
    CHAM_desc_t descAl, descAt;
    CHAM_desc_t descBl, descBt;
    CHAM_context_t *morse;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_ztradd", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }

    /* Check input arguments */
    if ((uplo != ChamUpperLower) && (uplo != ChamUpper) && (uplo != ChamLower)) {
        morse_error("CHAMELEON_ztradd", "illegal value of uplo");
        return -1;
    }
    if ((trans < ChamNoTrans) || (trans > ChamConjTrans)) {
        morse_error("CHAMELEON_ztradd", "illegal value of trans");
        return -2;
    }
    if ( trans == ChamNoTrans ) {
        Am = M; An = N;
    } else {
        Am = N; An = M;
    }
    if (M < 0) {
        morse_error("CHAMELEON_ztradd", "illegal value of M");
        return -3;
    }
    if (N < 0) {
        morse_error("CHAMELEON_ztradd", "illegal value of N");
        return -4;
    }
    if (LDA < chameleon_max(1, Am)) {
        morse_error("CHAMELEON_ztradd", "illegal value of LDA");
        return -7;
    }
    if (LDB < chameleon_max(1, M)) {
        morse_error("CHAMELEON_ztradd", "illegal value of LDB");
        return -10;
    }

    /* Quick return */
    if (M == 0 || N == 0 ||
        ((alpha == (CHAMELEON_Complex64_t)0.0) && beta == (CHAMELEON_Complex64_t)1.0))
        return CHAMELEON_SUCCESS;

    /* Tune NB depending on M, N & NRHS; Set NBNBSIZE */
    status = morse_tune(CHAMELEON_FUNC_ZGEMM, M, N, 0);
    if (status != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_ztradd", "morse_tune() failed");
        return status;
    }

    /* Set MT & NT & KT */
    NB = CHAMELEON_NB;

    morse_sequence_create( morse, &sequence );

    /* Submit the matrix conversion */
    morse_zlap2tile( morse, &descAl, &descAt, ChamDescInput, uplo,
                     A, NB, NB, LDA, An, Am, An, sequence, &request );
    morse_zlap2tile( morse, &descBl, &descBt, ChamDescInout, uplo,
                     B, NB, NB, LDB, N, M, N, sequence, &request );

    /* Call the tile interface */
    CHAMELEON_ztradd_Tile_Async( uplo, trans, alpha, &descAt, beta, &descBt, sequence, &request );

    /* Submit the matrix conversion back */
    morse_ztile2lap( morse, &descAl, &descAt,
                     ChamDescInput, uplo, sequence, &request );
    morse_ztile2lap( morse, &descBl, &descBt,
                     ChamDescInout, uplo, sequence, &request );

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
 *  CHAMELEON_ztradd_Tile - Performs a matrix addition similarly to the pztradd()
 *  function from the PBLAS library.
 *  Tile equivalent of CHAMELEON_ztradd().
 *  Operates on matrices stored by tiles.
 *  All matrices are passed through descriptors.
 *  All dimensions are taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in] uplo
 *          Specifies the shape of A and B matrices:
 *          = ChamUpperLower: A and B are general matrices.
 *          = ChamUpper: op(A) and B are upper trapezoidal matrices.
 *          = ChamLower: op(A) and B are lower trapezoidal matrices.
 *
 * @param[in] trans
 *          Specifies whether the matrix A is transposed, not transposed or
 *          conjugate transposed:
 *          = ChamNoTrans:   A is not transposed;
 *          = ChamTrans:     A is transposed;
 *          = ChamConjTrans: A is conjugate transposed.
 *
 * @param[in] alpha
 *          alpha specifies the scalar alpha
 *
 * @param[in] A
 *          A is a LDA-by-ka matrix, where ka is N when trans = ChamNoTrans,
 *          and is M otherwise.
 *
 * @param[in] beta
 *          beta specifies the scalar beta
 *
 * @param[in,out] B
 *          B is a LDB-by-N matrix.
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_ztradd
 * @sa CHAMELEON_ztradd_Tile_Async
 * @sa CHAMELEON_ctradd_Tile
 * @sa CHAMELEON_dtradd_Tile
 * @sa CHAMELEON_stradd_Tile
 *
 */
int CHAMELEON_ztradd_Tile( cham_uplo_t uplo, cham_trans_t trans,
                       CHAMELEON_Complex64_t alpha, CHAM_desc_t *A,
                       CHAMELEON_Complex64_t beta,  CHAM_desc_t *B )
{
    CHAM_context_t *morse;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_ztradd_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create( morse, &sequence );

    CHAMELEON_ztradd_Tile_Async( uplo, trans, alpha, A, beta, B, sequence, &request );

    CHAMELEON_Desc_Flush( A, sequence );
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
 *  CHAMELEON_ztradd_Tile_Async - Performs a matrix addition similarly to the
 *  pztradd() function from the PBLAS library.
 *  Non-blocking equivalent of CHAMELEON_ztradd_Tile().
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
 * @sa CHAMELEON_ztradd
 * @sa CHAMELEON_ztradd_Tile
 * @sa CHAMELEON_ctradd_Tile_Async
 * @sa CHAMELEON_dtradd_Tile_Async
 * @sa CHAMELEON_stradd_Tile_Async
 *
 */
int CHAMELEON_ztradd_Tile_Async( cham_uplo_t uplo, cham_trans_t trans,
                             CHAMELEON_Complex64_t alpha, CHAM_desc_t *A,
                             CHAMELEON_Complex64_t beta,  CHAM_desc_t *B,
                             RUNTIME_sequence_t *sequence, RUNTIME_request_t *request )
{
    CHAM_context_t *morse;
    int M, N;
    int Am, An, Ai, Aj, Amb, Anb;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_ztradd_Tile_Async", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("CHAMELEON_ztradd_Tile_Async", "NULL sequence");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("CHAMELEON_ztradd_Tile_Async", "NULL request");
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
        morse_error("CHAMELEON_ztradd_Tile_Async", "invalid first descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(B) != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_ztradd_Tile_Async", "invalid second descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if ((trans < ChamNoTrans) || (trans > ChamConjTrans)) {
        morse_error("CHAMELEON_ztradd_Tile_Async", "illegal value of trans");
        return morse_request_fail(sequence, request, -1);
    }

    if ( trans == ChamNoTrans ) {
        Am  = A->m;
        An  = A->n;
        Amb = A->mb;
        Anb = A->nb;
        Ai  = A->i;
        Aj  = A->j;
    } else {
        Am  = A->n;
        An  = A->m;
        Amb = A->nb;
        Anb = A->mb;
        Ai  = A->j;
        Aj  = A->i;
    }

    if ( (Amb != B->mb) || (Anb != B->nb) ) {
        morse_error("CHAMELEON_ztradd_Tile_Async", "tile sizes have to match");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if ( (Am != B->m) || (An != B->n) ) {
        morse_error("CHAMELEON_ztradd_Tile_Async", "sizes of matrices have to match");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if ( (Ai != B->i) || (Aj != B->j) ) {
        morse_error("CHAMELEON_ztradd_Tile_Async", "start indexes have to match");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }

    M = B->m;
    N = B->n;

    /* Quick return */
    if (M == 0 || N == 0 ||
        ((alpha == (CHAMELEON_Complex64_t)0.0) && beta == (CHAMELEON_Complex64_t)1.0))
        return CHAMELEON_SUCCESS;

    morse_pztradd( uplo, trans, alpha, A, beta, B, sequence, request );

    return CHAMELEON_SUCCESS;
}
