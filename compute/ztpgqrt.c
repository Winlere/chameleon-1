/**
 *
 * @file ztpgqrt.c
 *
 * @copyright 2009-2016 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 * @copyright 2016-2018 KAUST. All rights reserved.
 *
 ***
 *
 * @brief Chameleon ztpgqrt wrappers
 *
 * @version 1.0.0
 * @author Mathieu Faverge
 * @date 2016-12-15
 * @precisions normal z -> s d c
 *
 */
#include "control/common.h"

/**
 ******************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t
 *
 *  CHAMELEON_ztpgqrt - Generates a partial Q matrix formed with a blocked QR
 *  factorization of a "triangular-pentagonal" matrix C, which is composed of an
 *  unused triangular block and a pentagonal block V, using the compact
 *  representation for Q. See CHAMELEON_ztpqrt() to generate V.
 *
 *******************************************************************************
 *
 * @param[in] M
 *          The number of rows of the matrices Q2, and V2. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrices Q1, and Q1. N >= 0.
 *
 * @param[in] K
 *          The number of elementary reflectors whose product defines
 *          the matrix Q in the matrix V.
 *          The number of rows of the matrices V1, and Q1.
 *          The number of columns of the matrices V1, and V2.
 *
 * @param[in] L
 *          The number of rows of the upper trapezoidal part of V2.
 *          MIN(M,N) >= L >= 0.  See Further Details.
 *
 * @param[in] V1
 *          The i-th row must contain the vector which defines the
 *          elementary reflector H(i), for i = 1,2,...,k, as returned by
 *          CHAMELEON_ztpqrt().
 *          V1 is a matrix of size K-by-K.
 *
 * @param[in] LDV1
 *          The leading dimension of the array V1. LDV1 >= max(1,K).
 *
 * @param[int] descT1
 *          The auxiliary factorization data generated by the call to
 *          CHAMELEON_zgeqrf() on V1.
 *
 * @param[in] V2
 *          The i-th row must contain the vector which defines the
 *          elementary reflector H(i), for i = 1,2,...,k, as returned by
 *          CHAMELEON_ztpqrt() in the first k rows of its array argument V2.
 *          V2 is a matrix of size M-by-K. The first M-L rows
 *          are rectangular, and the last L rows are upper trapezoidal.
 *
 * @param[in] LDV2
 *          The leading dimension of the array V2. LDV2 >= max(1,M).
 *
 * @param[int] descT2
 *          The auxiliary factorization data, generated by the call to
 *          CHAMELEON_ztpqrt() on V1 and V2, and associated to V2.
 *
 * @param[in,out] Q1
 *          Q1 is COMPLEX*16 array, dimension (LDQ1,N)
 *          On entry, the K-by-N matrix Q1.
 *          On exit, Q1 is overwritten by the corresponding block of
 *          Q*Q1.  See Further Details.
 *
 * @param[in] LDQ1
 *          The leading dimension of the array Q1. LDQ1 >= max(1,K).
 *
 * @param[in,out] Q2
 *          On entry, the pentagonal M-by-N matrix Q2.
 *          On exit, Q2 contains Q.
 *
 * @param[in] LDQ2
 *          The leading dimension of the array Q2.  LDQ2 >= max(1,M).
 *
 * @par Further Details:
 * =====================
 *
 *  The input matrix Q is a (K+M)-by-N matrix
 *
 *               Q = [ Q1 ]
 *                   [ Q2 ]
 *
 *  where Q1 is an identity matrix, and Q2 is a M-by-N matrix of 0.
 *  V is a matrix of householder reflectors with a pentagonal shape consisting
 *  of a K-by-K rectangular matrix V1 on top of a matrix V2 composed of
 *  (M-L)-by-K rectangular part on top of a L-by-N upper trapezoidal matrix:
 *
 *               V = [ V1  ]  <-     K-by-K rectangular
 *                   [ V2a ]  <- (M-L)-by-K rectangular
 *                   [ V2b ]  <-     L-by-K upper trapezoidal.
 *
 *  The upper trapezoidal part of the matrix V2 consists of the first L rows of
 *  a K-by-K upper triangular matrix, where 0 <= L <= MIN(M,K).  If L=0, V2 is
 *  rectangular M-by-K; if M=L=K, V2 is upper triangular. Those are the two
 *  cases only handled for now.
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_ztpgqrt_Tile
 * @sa CHAMELEON_ztpgqrt_Tile_Async
 * @sa CHAMELEON_ctpgqrt
 * @sa CHAMELEON_dtpgqrt
 * @sa CHAMELEON_stpgqrt
 * @sa CHAMELEON_zgeqrs
 *
 */
int CHAMELEON_ztpgqrt( int M, int N, int K, int L,
                   CHAMELEON_Complex64_t *V1, int LDV1, CHAM_desc_t *descT1,
                   CHAMELEON_Complex64_t *V2, int LDV2, CHAM_desc_t *descT2,
                   CHAMELEON_Complex64_t *Q1, int LDQ1,
                   CHAMELEON_Complex64_t *Q2, int LDQ2 )
{
    int NB;
    int status;
    CHAM_context_t *chamctxt;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    CHAM_desc_t descQ1l, descQ1t;
    CHAM_desc_t descQ2l, descQ2t;
    CHAM_desc_t descV1l, descV1t;
    CHAM_desc_t descV2l, descV2t;
    int minMK = chameleon_min( M, K );

    chamctxt = chameleon_context_self();
    if (chamctxt == NULL) {
        chameleon_fatal_error("CHAMELEON_ztpgqrt", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }

    /* Check input arguments */
    if (M < 0) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of M");
        return -1;
    }
    if (N < 0) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of N");
        return -2;
    }
    if (K < 0) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of K");
        return -3;
    }
    if ((L < 0) || ((L > minMK) && (minMK > 0))) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of N");
        return -4;
    }
    if (K != N) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of K and N. K must be equal to N");
        return -3;
    }
    if (LDV1 < chameleon_max(1, K)) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of LDV1");
        return -6;
    }
    if (LDV2 < chameleon_max(1, M)) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of LDV2");
        return -9;
    }
    if (LDQ1 < chameleon_max(1, K)) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of LDQ1");
        return -11;
    }
    if (LDQ2 < chameleon_max(1, M)) {
        chameleon_error("CHAMELEON_ztpgqrt", "illegal value of LDQ2");
        return -13;
    }

    /* Quick return */
    if (minMK == 0)
        return CHAMELEON_SUCCESS;

    /* Tune NB & IB depending on M, N & NRHS; Set NBNBSIZE */
    status = chameleon_tune(CHAMELEON_FUNC_ZGELS, M, K, 0);
    if (status != CHAMELEON_SUCCESS) {
        chameleon_error("CHAMELEON_ztpgqrt", "chameleon_tune() failed");
        return status;
    }

    /* Set NT */
    NB = CHAMELEON_NB;

    chameleon_sequence_create( chamctxt, &sequence );

    /* Submit the matrix conversion */
    chameleon_zlap2tile( chamctxt, &descV1l, &descV1t, ChamDescInput, ChamUpperLower,
                     V1, NB, NB, LDV1, K, M, K, sequence, &request );
    chameleon_zlap2tile( chamctxt, &descV2l, &descV2t, ChamDescInput, ChamUpperLower,
                     V2, NB, NB, LDV2, K, M, K, sequence, &request );
    chameleon_zlap2tile( chamctxt, &descQ1l, &descQ1t, ChamDescInout, ChamUpperLower,
                     Q1, NB, NB, LDQ1, N, K, N, sequence, &request );
    chameleon_zlap2tile( chamctxt, &descQ2l, &descQ2t, ChamDescInout, ChamUpperLower,
                     Q2, NB, NB, LDQ2, N, M, N, sequence, &request );

    /* Call the tile interface */
    CHAMELEON_ztpgqrt_Tile_Async( L, &descV1t, descT1, &descV2t, descT2, &descQ1t, &descQ2t, sequence, &request );

    /* Submit the matrix conversion back */
    chameleon_ztile2lap( chamctxt, &descV1l, &descV1t,
                     ChamDescInput, ChamUpperLower, sequence, &request );
    chameleon_ztile2lap( chamctxt, &descV2l, &descV2t,
                     ChamDescInput, ChamUpperLower, sequence, &request );
    chameleon_ztile2lap( chamctxt, &descQ1l, &descQ1t,
                     ChamDescInout, ChamUpperLower, sequence, &request );
    chameleon_ztile2lap( chamctxt, &descQ2l, &descQ2t,
                     ChamDescInout, ChamUpperLower, sequence, &request );
    CHAMELEON_Desc_Flush( descT1, sequence );
    CHAMELEON_Desc_Flush( descT2, sequence );

    chameleon_sequence_wait( chamctxt, sequence );

    /* Cleanup the temporary data */
    chameleon_ztile2lap_cleanup( chamctxt, &descV1l, &descV1t );
    chameleon_ztile2lap_cleanup( chamctxt, &descV2l, &descV2t );
    chameleon_ztile2lap_cleanup( chamctxt, &descQ1l, &descQ1t );
    chameleon_ztile2lap_cleanup( chamctxt, &descQ2l, &descQ2t );

    status = sequence->status;
    chameleon_sequence_destroy( chamctxt, sequence );
    return status;
}

/**
 *******************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t_Tile
 *
 *  CHAMELEON_ztpgqrt_Tile - Generates a partial Q matrix formed with a blocked QR
 *  factorization of a "triangular-pentagonal" matrix C, which is composed of an
 *  unused triangular block and a pentagonal block V, using the compact
 *  representation for Q. See CHAMELEON_ztpqrt() to generate V.
 *
 *******************************************************************************
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_ztpgqrt
 * @sa CHAMELEON_ztpgqrt_Tile_Async
 * @sa CHAMELEON_ctpgqrt_Tile
 * @sa CHAMELEON_dtpgqrt_Tile
 * @sa CHAMELEON_stpgqrt_Tile
 * @sa CHAMELEON_zgeqrs_Tile
 *
 */
int CHAMELEON_ztpgqrt_Tile( int L,
                        CHAM_desc_t *V1, CHAM_desc_t *T1,
                        CHAM_desc_t *V2, CHAM_desc_t *T2,
                        CHAM_desc_t *Q1, CHAM_desc_t *Q2 )
{
    CHAM_context_t *chamctxt;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t request = RUNTIME_REQUEST_INITIALIZER;
    int status;

    chamctxt = chameleon_context_self();
    if (chamctxt == NULL) {
        chameleon_fatal_error("CHAMELEON_ztpgqrt_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    chameleon_sequence_create( chamctxt, &sequence );

    CHAMELEON_ztpgqrt_Tile_Async( L, V1, T1, V2, T2, Q1, Q2, sequence, &request );

    CHAMELEON_Desc_Flush( V1, sequence );
    CHAMELEON_Desc_Flush( T1, sequence );
    CHAMELEON_Desc_Flush( V2, sequence );
    CHAMELEON_Desc_Flush( T2, sequence );
    CHAMELEON_Desc_Flush( Q1, sequence );
    CHAMELEON_Desc_Flush( Q2, sequence );

    chameleon_sequence_wait( chamctxt, sequence );
    status = sequence->status;
    chameleon_sequence_destroy( chamctxt, sequence );
    return status;
}

/**
 *******************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t_Tile_Async
 *
 *  CHAMELEON_ztpgqrt_Tile_Async - Generates a partial Q matrix formed with a blocked QR
 *  factorization of a "triangular-pentagonal" matrix C, which is composed of an
 *  unused triangular block and a pentagonal block V, using the compact
 *  representation for Q. See CHAMELEON_ztpqrt() to generate V.
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
 * @sa CHAMELEON_ztpgqrt
 * @sa CHAMELEON_ztpgqrt_Tile
 * @sa CHAMELEON_ctpgqrt_Tile_Async
 * @sa CHAMELEON_dtpgqrt_Tile_Async
 * @sa CHAMELEON_stpgqrt_Tile_Async
 * @sa CHAMELEON_zgeqrs_Tile_Async
 *
 */
int CHAMELEON_ztpgqrt_Tile_Async( int L,
                                  CHAM_desc_t *V1, CHAM_desc_t *T1,
                                  CHAM_desc_t *V2, CHAM_desc_t *T2,
                                  CHAM_desc_t *Q1, CHAM_desc_t *Q2,
                                  RUNTIME_sequence_t *sequence, RUNTIME_request_t *request )
{
    CHAM_context_t *chamctxt;
    CHAM_desc_t D, *Dptr = NULL;
    int KT;

    chamctxt = chameleon_context_self();
    if (chamctxt == NULL) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        chameleon_fatal_error("CHAMELEON_ztpgqrt_Tile", "NULL sequence");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        chameleon_fatal_error("CHAMELEON_ztpgqrt_Tile", "NULL request");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    /* Check sequence status */
    if (sequence->status == CHAMELEON_SUCCESS) {
        request->status = CHAMELEON_SUCCESS;
    }
    else {
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_SEQUENCE_FLUSHED);
    }

    /* Check descriptors for correctness */
    if (chameleon_desc_check(V1) != CHAMELEON_SUCCESS) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "invalid V1 descriptor");
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (chameleon_desc_check(T1) != CHAMELEON_SUCCESS) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "invalid T1 descriptor");
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (chameleon_desc_check(V2) != CHAMELEON_SUCCESS) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "invalid V2 descriptor");
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (chameleon_desc_check(T2) != CHAMELEON_SUCCESS) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "invalid T2 descriptor");
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (chameleon_desc_check(Q1) != CHAMELEON_SUCCESS) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "invalid Q1 descriptor");
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if (chameleon_desc_check(Q2) != CHAMELEON_SUCCESS) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "invalid Q2 descriptor");
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (Q1->nb != Q1->mb) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "only square tiles supported");
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }
    if ( (L != 0) && (((Q2->m - L) % Q2->mb) != 0) ) {
        chameleon_error("CHAMELEON_ztpgqrt_Tile", "Triangular part must be aligned with tiles");
        return chameleon_request_fail(sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE);
    }

    if (V1->m > V1->n) {
        KT = V1->nt;
    } else {
        KT = V1->mt;
    }

#if defined(CHAMELEON_COPY_DIAG)
    {
        chameleon_zdesc_alloc(D, V1->mb, V1->nb, V1->m, KT*V1->nb, 0, 0, V1->m, KT*V1->nb, );
        Dptr = &D;
    }
#endif

    /* if (chamctxt->householder == ChamFlatHouseholder) { */
    chameleon_pzlaset( ChamUpperLower, 0., 1., Q1, sequence, request );
    chameleon_pzlaset( ChamUpperLower, 0., 0., Q2, sequence, request );
    chameleon_pztpgqrt( KT, L, V2, T2, Q1, Q2, sequence, request );
    chameleon_pzungqr( 1, V1, Q1, T1, Dptr, sequence, request );

    if (Dptr != NULL) {
        CHAMELEON_Desc_Flush( V1, sequence );
        CHAMELEON_Desc_Flush( T1, sequence );
        CHAMELEON_Desc_Flush( V2, sequence );
        CHAMELEON_Desc_Flush( T2, sequence );
        CHAMELEON_Desc_Flush( Q1, sequence );
        CHAMELEON_Desc_Flush( Q2, sequence );
        CHAMELEON_Desc_Flush( Dptr, sequence );
        chameleon_sequence_wait( chamctxt, sequence );
        chameleon_desc_destroy( Dptr );
    }
    (void)D;
    return CHAMELEON_SUCCESS;
}
