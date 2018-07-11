/**
 *
 * @file zunmqr.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zunmqr wrappers
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
 *  CHAMELEON_zunmqr - Overwrites the general complex M-by-N matrix C with
 *
 *                  SIDE = 'L'     SIDE = 'R'
 *  TRANS = 'N':      Q * C          C * Q
 *  TRANS = 'C':      Q**H * C       C * Q**H
 *
 *  where Q is a complex unitary matrix defined as the product of k
 *  elementary reflectors
 *
 *        Q = H(1) H(2) . . . H(k)
 *
 *  as returned by CHAMELEON_zgeqrf. Q is of order M if SIDE = ChamLeft
 *  and of order N if SIDE = ChamRight.
 *
 *******************************************************************************
 *
 * @param[in] side
 *          Intended usage:
 *          = ChamLeft:  apply Q or Q**H from the left;
 *          = ChamRight: apply Q or Q**H from the right.
 *
 * @param[in] trans
 *          Intended usage:
 *          = ChamNoTrans:   no transpose, apply Q;
 *          = ChamConjTrans: conjugate transpose, apply Q**H.
 *
 * @param[in] M
 *          The number of rows of the matrix C. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrix C. N >= 0.
 *
 * @param[in] K
 *          The number of elementary reflectors whose product defines
 *          the matrix Q.
 *          If side == ChamLeft,  M >= K >= 0.
 *          If side == ChamRight, N >= K >= 0.
 *
 * @param[in] A
 *          Details of the QR factorization of the original matrix A as returned by CHAMELEON_zgeqrf.
 *
 * @param[in] LDA
 *          The leading dimension of the array A.
 *          If side == ChamLeft,  LDA >= max(1,M).
 *          If side == ChamRight, LDA >= max(1,N).
 *
 * @param[in] descT
 *          Auxiliary factorization data, computed by CHAMELEON_zgeqrf.
 *
 * @param[in,out] C
 *          On entry, the M-by-N matrix C.
 *          On exit, C is overwritten by Q*C or Q**H*C.
 *
 * @param[in] LDC
 *          The leading dimension of the array C. LDC >= max(1,M).
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zunmqr_Tile
 * @sa CHAMELEON_zunmqr_Tile_Async
 * @sa CHAMELEON_cunmqr
 * @sa CHAMELEON_dormqr
 * @sa CHAMELEON_sormqr
 * @sa CHAMELEON_zgeqrf
 *
 */
int CHAMELEON_zunmqr( cham_side_t side, cham_trans_t trans, int M, int N, int K,
                  CHAMELEON_Complex64_t *A, int LDA,
                  CHAM_desc_t *descT,
                  CHAMELEON_Complex64_t *C, int LDC )
{
    int NB, Am;
    int status;
    CHAM_context_t *morse;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    CHAM_desc_t descAl, descAt;
    CHAM_desc_t descCl, descCt;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zunmqr", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }

    if ( side == ChamLeft ) {
        Am = M;
    } else {
        Am = N;
    }

    /* Check input arguments */
    if ((side != ChamLeft) && (side != ChamRight)) {
        morse_error("CHAMELEON_zunmqr", "illegal value of side");
        return -1;
    }
    if ((trans != ChamConjTrans) && (trans != ChamNoTrans)){
        morse_error("CHAMELEON_zunmqr", "illegal value of trans");
        return -2;
    }
    if (M < 0) {
        morse_error("CHAMELEON_zunmqr", "illegal value of M");
        return -3;
    }
    if (N < 0) {
        morse_error("CHAMELEON_zunmqr", "illegal value of N");
        return -4;
    }
    if ((K < 0) || (K > Am)) {
        morse_error("CHAMELEON_zunmqr", "illegal value of K");
        return -5;
    }
    if (LDA < chameleon_max(1, Am)) {
        morse_error("CHAMELEON_zunmqr", "illegal value of LDA");
        return -7;
    }
    if (LDC < chameleon_max(1, M)) {
        morse_error("CHAMELEON_zunmqr", "illegal value of LDC");
        return -10;
    }
    /* Quick return - currently NOT equivalent to LAPACK's:
     * CALL DLASET( 'Full', MAX( M, N ), NRHS, ZERO, ZERO, C, LDC ) */
    if (chameleon_min(M, chameleon_min(N, K)) == 0)
        return CHAMELEON_SUCCESS;

    /* Tune NB & IB depending on M, K & N; Set NBNB */
    status = morse_tune(CHAMELEON_FUNC_ZGELS, M, K, N);
    if (status != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zunmqr", "morse_tune() failed");
        return status;
    }

    /* Set MT, NT & NTRHS */
    NB   = CHAMELEON_NB;
    morse_sequence_create( morse, &sequence );

    /* Submit the matrix conversion */
    morse_zlap2tile( morse, &descAl, &descAt, ChamDescInput, ChamLower,
                     A, NB, NB, LDA, K, Am, K, sequence, &request );
    morse_zlap2tile( morse, &descCl, &descCt, ChamDescInout, ChamUpperLower,
                     C, NB, NB, LDC, N, M,  N, sequence, &request );

    /* Call the tile interface */
    CHAMELEON_zunmqr_Tile_Async(  side, trans, &descAt, descT, &descCt, sequence, &request );

    /* Submit the matrix conversion back */
    morse_ztile2lap( morse, &descAl, &descAt,
                     ChamDescInput, ChamLower, sequence, &request );
    morse_ztile2lap( morse, &descCl, &descCt,
                     ChamDescInout, ChamUpperLower, sequence, &request );
    CHAMELEON_Desc_Flush( descT, sequence );

    morse_sequence_wait( morse, sequence );

    /* Cleanup the temporary data */
    morse_ztile2lap_cleanup( morse, &descAl, &descAt );
    morse_ztile2lap_cleanup( morse, &descCl, &descCt );

    status = sequence->status;
    morse_sequence_destroy( morse, sequence );
    return status;
}

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t_Tile
 *
 *  CHAMELEON_zunmqr_Tile - overwrites the general M-by-N matrix C with Q*C, where Q is an orthogonal
 *  matrix (unitary in the complex case) defined as the product of elementary reflectors returned
 *  by CHAMELEON_zgeqrf_Tile Q is of order M.
 *  All matrices are passed through descriptors. All dimensions are taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in] side
 *          Intended usage:
 *          = ChamLeft:  apply Q or Q**H from the left;
 *          = ChamRight: apply Q or Q**H from the right.
 *          Currently only ChamLeft is supported.
 *
 * @param[in] trans
 *          Intended usage:
 *          = ChamNoTrans:   no transpose, apply Q;
 *          = ChamConjTrans: conjugate transpose, apply Q**H.
 *          Currently only ChamConjTrans is supported.
 *
 * @param[in] A
 *          Details of the QR factorization of the original matrix A as returned by CHAMELEON_zgeqrf.
 *
 * @param[in] T
 *          Auxiliary factorization data, computed by CHAMELEON_zgeqrf.
 *          Can be obtained with CHAMELEON_Alloc_Workspace_zgeqrf
 *
 * @param[in,out] C
 *          On entry, the M-by-N matrix C.
 *          On exit, C is overwritten by Q*C or Q**H*C.
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zunmqr
 * @sa CHAMELEON_zunmqr_Tile_Async
 * @sa CHAMELEON_cunmqr_Tile
 * @sa CHAMELEON_dormqr_Tile
 * @sa CHAMELEON_sormqr_Tile
 * @sa CHAMELEON_zgeqrf_Tile
 *
 */
int CHAMELEON_zunmqr_Tile( cham_side_t side, cham_trans_t trans,
                       CHAM_desc_t *A, CHAM_desc_t *T, CHAM_desc_t *C )
{
    CHAM_context_t *morse;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zunmqr_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create( morse, &sequence );

    CHAMELEON_zunmqr_Tile_Async(side, trans, A, T, C, sequence, &request );

    CHAMELEON_Desc_Flush( A, sequence );
    CHAMELEON_Desc_Flush( T, sequence );
    CHAMELEON_Desc_Flush( C, sequence );

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
 *  Non-blocking equivalent of CHAMELEON_zunmqr_Tile().
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
 * @sa CHAMELEON_zunmqr
 * @sa CHAMELEON_zunmqr_Tile
 * @sa CHAMELEON_cunmqr_Tile_Async
 * @sa CHAMELEON_dormqr_Tile_Async
 * @sa CHAMELEON_sormqr_Tile_Async
 * @sa CHAMELEON_zgeqrf_Tile_Async
 *
 */
int CHAMELEON_zunmqr_Tile_Async( cham_side_t side, cham_trans_t trans,
                             CHAM_desc_t *A, CHAM_desc_t *T, CHAM_desc_t *C,
                             RUNTIME_sequence_t *sequence, RUNTIME_request_t *request )
{
    CHAM_context_t *morse;
    CHAM_desc_t D, *Dptr = NULL;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_zunmqr_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("CHAMELEON_zunmqr_Tile", "NULL sequence");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("CHAMELEON_zunmqr_Tile", "NULL request");
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
        morse_error("CHAMELEON_zunmqr_Tile", "invalid first descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(T) != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zunmqr_Tile", "invalid second descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(C) != CHAMELEON_SUCCESS) {
        morse_error("CHAMELEON_zunmqr_Tile", "invalid third descriptor");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (A->nb != A->mb || C->nb != C->mb) {
        morse_error("CHAMELEON_zunmqr_Tile", "only square tiles supported");
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if ((side != ChamLeft) && (side != ChamRight)) {
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if ((trans != ChamConjTrans) && (trans != ChamNoTrans)){
        return morse_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    /* Quick return - currently NOT equivalent to LAPACK's:
     * CALL DLASET( 'Full', MAX( M, N ), NRHS, ZERO, ZERO, C, LDC ) */
    /*
     if (chameleon_min(M, chameleon_min(N, K)) == 0)
     return CHAMELEON_SUCCESS;
     */

#if defined(CHAMELEON_COPY_DIAG)
    {
        int n = chameleon_min(A->mt, A->nt) * A->nb;
        morse_zdesc_alloc(D, A->mb, A->nb, A->m, n, 0, 0, A->m, n, );
        Dptr = &D;
    }
#endif

    if (morse->householder == ChamFlatHouseholder) {
        if ( (trans == ChamConjTrans) &&
             (side == ChamLeft) ) {
            morse_pzunmqr( side, trans, A, C, T, Dptr, sequence, request );
        }
        else {
            morse_pzunmqr( side, trans, A, C, T, Dptr, sequence, request );
        }
    }
    else {
        morse_pzunmqrrh( side, trans, A, C, T, Dptr, CHAMELEON_RHBLK, sequence, request );
    }

    if (Dptr != NULL) {
        CHAMELEON_Desc_Flush( A, sequence );
        CHAMELEON_Desc_Flush( C, sequence );
        CHAMELEON_Desc_Flush( T, sequence );
        CHAMELEON_Desc_Flush( Dptr, sequence );
        morse_sequence_wait( morse, sequence );
        morse_desc_mat_free( Dptr );
    }
    (void)D;
    return CHAMELEON_SUCCESS;
}
