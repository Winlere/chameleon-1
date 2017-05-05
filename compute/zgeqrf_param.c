/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2014 Inria. All rights reserved.
 * @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file zgeqrf_param.c
 *
 *  MORSE computational routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for MORSE 1.0.0
 * @author Jakub Kurzak
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Raphael Boucherie
 * @date 2010-11-15
 * @precisions normal z -> s d c
 *
 **/
#include "control/common.h"

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t
 *
 * MORSE_zgeqrf_param - Computes the tile QR factorization of a complex M-by-N
 * matrix A: A = Q * R.
 *
 *******************************************************************************
 *
 * @param[in] qrtree
 *          The tree used for the factorization
 *
 * @param[in] M
 *          The number of rows of the matrix A. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrix A.  N >= 0.
 *
 * @param[in, out] A
 *          On entry, the M-by-N matrix A.
 *          On exit, the elements on and above the diagonal of the array contain the min(M,N)-by-N
 *          upper trapezoidal matrix R (R is upper triangular if M >= N); the elements below the
 *          diagonal represent the unitary matrix Q as a product of elementary reflectors stored
 *          by tiles.
 *
 * @param[in] LDA
 *          The leading dimension of the array A. LDA >= max(1,M).
 *
 * @param[out] descTS
 *          On exit, auxiliary factorization data, required by MORSE_zgeqrs to solve the system
 *          of equations.
 *
 * @param[out] descTT
 *          On exit, auxiliary factorization data, required by MORSE_zgeqrs to solve the system
 *          of equations.
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa MORSE_zgeqrf_param_Tile
 * @sa MORSE_zgeqrf_param_Tile_Async
 * @sa MORSE_cgeqrf
 * @sa MORSE_dgeqrf
 * @sa MORSE_sgeqrf
 * @sa MORSE_zgeqrs
 *
 ******************************************************************************/
int MORSE_zgeqrf_param(const libhqr_tree_t *qrtree, int M, int N,
                       MORSE_Complex64_t *A, int LDA,
                       MORSE_desc_t *descTS, MORSE_desc_t *descTT)
{
    int NB;
    int status;
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    MORSE_desc_t descA;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zgeqrf_param", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }

    /* Check input arguments */
    if (M < 0) {
        morse_error("MORSE_zgeqrf_param", "illegal value of M");
        return -1;
    }
    if (N < 0) {
        morse_error("MORSE_zgeqrf_param", "illegal value of N");
        return -2;
    }
    if (LDA < chameleon_max(1, M)) {
        morse_error("MORSE_zgeqrf_param", "illegal value of LDA");
        return -4;
    }

    /* Quick return */
    if (chameleon_min(M, N) == 0)
        return MORSE_SUCCESS;

    /* Tune NB & IB depending on M, N & NRHS; Set NBNBSIZE */
    status = morse_tune(MORSE_FUNC_ZGELS, M, N, 0);
    if (status != MORSE_SUCCESS) {
        morse_error("MORSE_zgeqrf_param", "morse_tune() failed");
        return status;
    }

    /* Set NT */
    NB = MORSE_NB;

    morse_sequence_create(morse, &sequence);

/*    if ( MORSE_TRANSLATION == MORSE_OUTOFPLACE ) {*/
        morse_zooplap2tile( descA, A, NB, NB, LDA, N, 0, 0, M, N, sequence, &request,
                             morse_desc_mat_free(&(descA)) );
/*    } else {*/
/*        morse_ziplap2tile( descA, A, NB, NB, LDA, N, 0, 0, M, N,*/
/*                            sequence, &request);*/
/*    }*/

    /* Call the tile interface */
        MORSE_zgeqrf_param_Tile_Async(qrtree, &descA, descTS, descTT, sequence, &request);

/*    if ( MORSE_TRANSLATION == MORSE_OUTOFPLACE ) {*/
        morse_zooptile2lap(descA, A, NB, NB, LDA, N,  sequence, &request);
        morse_sequence_wait(morse, sequence);
        morse_desc_mat_free(&descA);
/*    } else {*/
/*        morse_ziptile2lap( descA, A, NB, NB, LDA, N,  sequence, &request);*/
/*        morse_sequence_wait(morse, sequence);*/
/*    }*/

    status = sequence->status;
    morse_sequence_destroy(morse, sequence);
    return status;
}

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t_Tile
 *
 *  MORSE_zgeqrf_param_Tile - Computes the tile QR factorization of a matrix.
 *  Tile equivalent of MORSE_zgeqrf_param().
 *  Operates on matrices stored by tiles.
 *  All matrices are passed through descriptors.
 *  All dimensions are taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in,out] A
 *          On entry, the M-by-N matrix A.
 *          On exit, the elements on and above the diagonal of the array contain the min(M,N)-by-N
 *          upper trapezoidal matrix R (R is upper triangular if M >= N); the elements below the
 *          diagonal represent the unitary matrix Q as a product of elementary reflectors stored
 *          by tiles.
 *
 * @param[out] TS
 *          On exit, auxiliary factorization data, required by MORSE_zgeqrs to solve the system
 *          of equations.
 *
 * @param[out] TT
 *          On exit, auxiliary factorization data, required by MORSE_zgeqrs to solve the system
 *          of equations.
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa MORSE_zgeqrf_param
 * @sa MORSE_zgeqrf_param_Tile_Async
 * @sa MORSE_cgeqrf_param_Tile
 * @sa MORSE_dgeqrf_param_Tile
 * @sa MORSE_sgeqrf_param_Tile
 * @sa MORSE_zgeqrs_param_Tile
 *
 ******************************************************************************/
int MORSE_zgeqrf_param_Tile(const libhqr_tree_t *qrtree, MORSE_desc_t *A, MORSE_desc_t *TS, MORSE_desc_t *TT)
{
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zgeqrf_param_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create(morse, &sequence);
    MORSE_zgeqrf_param_Tile_Async(qrtree, A, TS, TT, sequence, &request);
    morse_sequence_wait(morse, sequence);
    RUNTIME_desc_getoncpu(A);

    status = sequence->status;
    morse_sequence_destroy(morse, sequence);
    return status;
}

/**
 *****************************************************************************
 *
 * @ingroup MORSE_Complex64_t_Tile_Async
 *
 *  MORSE_zgeqrf_param_Tile_Async - Computes the tile QR factorization of a matrix.
 *  Non-blocking equivalent of MORSE_zgeqrf_param_Tile().
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
 * @sa MORSE_zgeqrf_param
 * @sa MORSE_zgeqrf_param_Tile
 * @sa MORSE_cgeqrf_param_Tile_Async
 * @sa MORSE_dgeqrf_param_Tile_Async
 * @sa MORSE_sgeqrf_param_Tile_Async
 * @sa MORSE_zgeqrs_param_Tile_Async
 *
 ******************************************************************************/
int MORSE_zgeqrf_param_Tile_Async(const libhqr_tree_t *qrtree, MORSE_desc_t *A, MORSE_desc_t *TS, MORSE_desc_t *TT,
                             MORSE_sequence_t *sequence, MORSE_request_t *request)
{
    MORSE_context_t *morse;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_error("MORSE_zgeqrf_param_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("MORSE_zgeqrf_param_Tile", "NULL sequence");
        return MORSE_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("MORSE_zgeqrf_param_Tile", "NULL request");
        return MORSE_ERR_UNALLOCATED;
    }
    /* Check sequence status */
    if (sequence->status == MORSE_SUCCESS)
        request->status = MORSE_SUCCESS;
    else
        return morse_request_fail(sequence, request, MORSE_ERR_SEQUENCE_FLUSHED);

    /* Check descriptors for correctness */
    if (morse_desc_check(A) != MORSE_SUCCESS) {
        morse_error("MORSE_zgeqrf_param_Tile", "invalid first descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(TS) != MORSE_SUCCESS) {
        morse_error("MORSE_zgeqrf_param_Tile", "invalid second descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(TT) != MORSE_SUCCESS) {
        morse_error("MORSE_zgeqrf_param_Tile", "invalid second descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (A->nb != A->mb) {
        morse_error("MORSE_zgeqrf_param_Tile", "only square tiles supported");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    /* Quick return */
/*
    if (chameleon_min(M, N) == 0)
        return MORSE_SUCCESS;
*/
    morse_pzgeqrf_param(qrtree, A, TS, TT, sequence, request);

    return MORSE_SUCCESS;
}
