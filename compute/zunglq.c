/**
 *
 * @file zunglq.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zunglq wrappers
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 1.0.0
 * @author Hatem Ltaief
 * @author Jakub Kurzak
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2010-11-15
 * @precisions normal z -> s d c
 *
 */
#include "control/common.h"

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t
 *
 *  CHAMELEON_zunglq - Generates an M-by-N matrix Q with orthonormal rows, which is defined as the
 *  first M rows of a product of the elementary reflectors returned by CHAMELEON_zgelqf.
 *
 *******************************************************************************
 *
 * @param[in] M
 *          The number of rows of the matrix Q. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrix Q. N >= M.
 *
 * @param[in] K
 *          The number of rows of elementary tile reflectors whose product defines the matrix Q.
 *          M >= K >= 0.
 *
 * @param[in] A
 *          Details of the LQ factorization of the original matrix A as returned by CHAMELEON_zgelqf.
 *
 * @param[in] LDA
 *          The leading dimension of the array A. LDA >= max(1,M).
 *
 * @param[in] descT
 *          Auxiliary factorization data, computed by CHAMELEON_zgelqf.
 *
 * @param[out] Q
 *          On exit, the M-by-N matrix Q.
 *
 * @param[in] LDQ
 *          The leading dimension of the array Q. LDQ >= max(1,M).
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *          \retval CHAMELEON_SUCCESS <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zunglq_Tile
 * @sa CHAMELEON_zunglq_Tile_Async
 * @sa CHAMELEON_cunglq
 * @sa CHAMELEON_dorglq
 * @sa CHAMELEON_sorglq
 * @sa CHAMELEON_zgelqf
 *
 */
int CHAMELEON_zunglq( int M, int N, int K,
                  CHAMELEON_Complex64_t *A, int LDA,
                  CHAM_desc_t *descT,
                  CHAMELEON_Complex64_t *Q, int LDQ )
{
    int NB;
    int status;
    CHAM_context_t *morse;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    CHAM_desc_t descAl, descAt;
    CHAM_desc_t descQl, descQt;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zunglq", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    /* Check input arguments */
    if (M < 0) {
        morse_error("CHAMELEON_zunglq", "illegal value of M");
        return -1;
    }
    if (N < M) {
        morse_error("CHAMELEON_zunglq", "illegal value of N");
        return -2;
    }
    if (K < 0 || K > M) {
        morse_error("CHAMELEON_zunglq", "illegal value of K");
        return -3;
    }
    if (LDA < chameleon_max(1, M)) {
        morse_error("CHAMELEON_zunglq", "illegal value of LDA");
        return -5;
    }
    if (LDQ < chameleon_max(1, M)) {
        morse_error("CHAMELEON_zunglq", "illegal value of LDQ");
        return -8;
    }
    /* Quick return - currently NOT equivalent to LAPACK's:
     * CALL DLASET( 'Full', MAX( M, N ), NRHS, ZERO, ZERO, B, LDQ ) */
    if (chameleon_min(M, chameleon_min(N, K)) == 0)
        return CHAMELEON_SUCCESS;

    /* Tune NB & IB depending on M, N & NRHS; Set NBNB */
    status = morse_tune(CHAMELEON_FUNC_ZGELS, M, N, 0);
    if (status != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zunglq", "morse_tune() failed");
        return status;
    }

    /* Set NT */
    NB = CHAMELEON_NB;

    morse_sequence_create( morse, &sequence );

    /* Submit the matrix conversion */
    morse_zlap2tile( morse, &descAl, &descAt, ChamDescInput, ChamUpper,
                     A, NB, NB, LDA, N, K, N, sequence, &request );
    morse_zlap2tile( morse, &descQl, &descQt, ChamDescInout, ChamUpperLower,
                     Q, NB, NB, LDQ, N, M, N, sequence, &request );

    /* Call the tile interface */
    CHAMELEON_zunglq_Tile_Async( &descAt, descT, &descQt, sequence, &request );

    /* Submit the matrix conversion back */
    morse_ztile2lap( morse, &descAl, &descAt,
                     ChamDescInput, ChamUpper, sequence, &request );
    morse_ztile2lap( morse, &descQl, &descQt,
                     ChamDescInout, ChamUpperLower, sequence, &request );
    CHAMELEON_Desc_Flush( descT, sequence );

    morse_sequence_wait( morse, sequence );

    /* Cleanup the temporary data */
    morse_ztile2lap_cleanup( morse, &descAl, &descAt );
    morse_ztile2lap_cleanup( morse, &descQl, &descQt );

    status = sequence->status;
    morse_sequence_destroy( morse, sequence );
    return status;
}

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t_Tile
 *
 * CHAMELEON_zunglq_Tile - Generates an M-by-N matrix Q with orthonormal rows, which is defined as the
 * first M rows of a product of the elementary reflectors returned by CHAMELEON_zgelqf.
 * All matrices are passed through descriptors. All dimensions are taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in] A
 *          Details of the LQ factorization of the original matrix A as returned by CHAMELEON_zgelqf.
 *
 * @param[in] T
 *          Auxiliary factorization data, computed by CHAMELEON_zgelqf.
 *
 * @param[out] Q
 *          On exit, the M-by-N matrix Q.
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zunglq
 * @sa CHAMELEON_zunglq_Tile_Async
 * @sa CHAMELEON_cunglq_Tile
 * @sa CHAMELEON_dorglq_Tile
 * @sa CHAMELEON_sorglq_Tile
 * @sa CHAMELEON_zgelqf_Tile
 *
 */
int CHAMELEON_zunglq_Tile( CHAM_desc_t *A, CHAM_desc_t *T, CHAM_desc_t *Q )
{
    CHAM_context_t *morse;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zunglq_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create( morse, &sequence );

    CHAMELEON_zunglq_Tile_Async( A, T, Q, sequence, &request );

    CHAMELEON_Desc_Flush( A, sequence );
    CHAMELEON_Desc_Flush( T, sequence );
    CHAMELEON_Desc_Flush( Q, sequence );

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
 *  Non-blocking equivalent of CHAMELEON_zunglq_Tile().
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
 * @sa CHAMELEON_zunglq
 * @sa CHAMELEON_zunglq_Tile
 * @sa CHAMELEON_cunglq_Tile_Async
 * @sa CHAMELEON_dorglq_Tile_Async
 * @sa CHAMELEON_sorglq_Tile_Async
 * @sa CHAMELEON_zgelqf_Tile_Async
 *
 */
int CHAMELEON_zunglq_Tile_Async( CHAM_desc_t *A, CHAM_desc_t *T, CHAM_desc_t *Q,
                             RUNTIME_sequence_t *sequence, RUNTIME_request_t *request )
{
    CHAM_context_t *morse;
    CHAM_desc_t D, *Dptr = NULL;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zunglq_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("CHAMELEON_zunglq_Tile", "NULL sequence");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("CHAMELEON_zunglq_Tile", "NULL request");
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
        morse_error("CHAMELEON_zunglq_Tile", "invalid first descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(T) != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zunglq_Tile", "invalid second descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(Q) != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zunglq_Tile", "invalid third descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (A->nb != A->mb || Q->nb != Q->mb) {
        morse_error("CHAMELEON_zunglq_Tile", "only square tiles supported");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    /* Quick return - currently NOT equivalent to LAPACK's:
     * CALL DLASET( 'Full', MAX( M, N ), NRHS, ZERO, ZERO, Q, LDQ ) */
    /*
     if (chameleon_min(M, N) == 0)
     return CHAMELEON_SUCCESS;
     */

#if defined(CHAMELEON_COPY_DIAG)
    {
        int m = chameleon_min(A->mt, A->nt) * A->mb;
        morse_zdesc_alloc(D, A->mb, A->nb, m, A->n, 0, 0, m, A->n, );
        Dptr = &D;
    }
#endif

    morse_pzlaset( ChamUpperLower, 0., 1., Q, sequence, request );
    if (morse->householder == ChamFlatHouseholder) {
        morse_pzunglq( A, Q, T, Dptr, sequence, request );
    }
    else {
        morse_pzunglqrh( A, Q, T, Dptr, CHAMELEON_RHBLK, sequence, request );
    }

    if (Dptr != NULL) {
        CHAMELEON_Desc_Flush( A, sequence );
        CHAMELEON_Desc_Flush( Q, sequence );
        CHAMELEON_Desc_Flush( T, sequence );
        CHAMELEON_Desc_Flush( Dptr, sequence );
        morse_sequence_wait( morse, sequence );
        morse_desc_mat_free( Dptr );
    }
    (void)D;
    return CHAMELEON_SUCCESS;
}
