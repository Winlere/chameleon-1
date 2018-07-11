/**
 *
 * @file workspace.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon workspace routines
 *
 * @version 1.0.0
 * @author Mathieu Faverge
 * @author Cedric Castagnede
 * @date 2012-09-15
 *
 ***
 *
 * @defgroup Workspace
 * @brief Group routines exposed to users about specific workspaces management
 *
 */

#include <stdlib.h>
#include "control/common.h"
#include "control/auxiliary.h"
#include "control/workspace.h"

/**
 *
 */
int morse_alloc_ibnb_tile(int M, int N, cham_tasktype_t func, int type, CHAM_desc_t **desc, int p, int q)
{
    int status;
    int IB, NB, MT, NT;
    int64_t lm, ln;
    CHAM_context_t *morse;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("morse_alloc_ibnb_tile", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }

    /* Tune NB & IB depending on M & N; Set IBNBSIZE */
    status = morse_tune(func, M, N, 0);
    if (status != CHAMELEON_SUCCESS) {
        morse_error("morse_alloc_ibnb_tile", "morse_tune() failed");
        return CHAMELEON_ERR_UNEXPECTED;
    }

    /* Set MT & NT & allocate */
    NB = CHAMELEON_NB;
    IB = CHAMELEON_IB;
    MT = (M%NB==0) ? (M/NB) : (M/NB+1);
    NT = (N%NB==0) ? (N/NB) : (N/NB+1);

    /* Size is doubled for RH QR to store the reduction T */
    if ((morse->householder == ChamTreeHouseholder) &&
        ((func == CHAMELEON_FUNC_SGELS)  ||
         (func == CHAMELEON_FUNC_DGELS)  ||
         (func == CHAMELEON_FUNC_CGELS)  ||
         (func == CHAMELEON_FUNC_ZGELS)  ||
         (func == CHAMELEON_FUNC_SGESVD) ||
         (func == CHAMELEON_FUNC_DGESVD) ||
         (func == CHAMELEON_FUNC_CGESVD) ||
         (func == CHAMELEON_FUNC_ZGESVD)))
        NT *= 2;

    lm = IB * MT;
    ln = NB * NT;

    /* Allocate and initialize descriptor */
    *desc = (CHAM_desc_t*)malloc(sizeof(CHAM_desc_t));
    if (*desc == NULL) {
        morse_error("morse_alloc_ibnb_tile", "malloc() failed");
        return CHAMELEON_ERR_OUT_OF_RESOURCES;
    }
    **desc = morse_desc_init(type, IB, NB, IB*NB, lm, ln, 0, 0, lm, ln, p, q);

    /* Allocate matrix */
    if (morse_desc_mat_alloc(*desc)) {
        morse_error("morse_alloc_ibnb_tile", "malloc() failed");
        free(*desc);
        return CHAMELEON_ERR_OUT_OF_RESOURCES;
    }

    RUNTIME_desc_create( *desc );

    /* Check that everything is ok */
    status = morse_desc_check(*desc);
    if (status != CHAMELEON_SUCCESS) {
        morse_error("morse_alloc_ibnb_tile", "invalid descriptor");
        free(*desc);
        return status;
    }

    return CHAMELEON_SUCCESS;
}

/**
 *
 */
int morse_alloc_ipiv(int M, int N, cham_tasktype_t func, int type, CHAM_desc_t **desc, void **IPIV, int p, int q)
{
    int status;
    int NB, IB, MT, NT;
    int64_t lm, ln;
    size_t size;
    CHAM_context_t *morse;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("morse_alloc_ipiv", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }

    /* Tune NB & IB depending on M & N; Set IBNBSIZE */
    status = morse_tune(func, M, N, 0);
    if (status != CHAMELEON_SUCCESS) {
        morse_error("morse_alloc_ipiv", "morse_tune() failed");
        return CHAMELEON_ERR_UNEXPECTED;
    }

    /* Set MT & NT & allocate */
    NB = CHAMELEON_NB;
    IB = CHAMELEON_IB;

    NT = (N%NB==0) ? (N/NB) : ((N/NB)+1);
    MT = (M%NB==0) ? (M/NB) : ((M/NB)+1);

    lm = IB * MT;
    ln = NB * NT;

    size = (size_t)(chameleon_min(MT, NT) * NB * NT * sizeof(int));
    if (size == 0) {
        *IPIV = NULL;
        return CHAMELEON_SUCCESS;
    }
    /* TODO: Fix the distribution for IPIV */
    *IPIV = (int*)malloc( size );

    *desc = (CHAM_desc_t*)malloc(sizeof(CHAM_desc_t));
    **desc = morse_desc_init(type, IB, NB, IB*NB, lm, ln, 0, 0, lm, ln, p, q );

    if ( morse_desc_mat_alloc(*desc) ) {
        morse_error("morse_alloc_ipiv", "malloc() failed");
        free(*desc);
        return CHAMELEON_ERR_OUT_OF_RESOURCES;
    }

    RUNTIME_desc_create( *desc );

    return CHAMELEON_SUCCESS;
}

/**
 *
 * @ingroup Workspace
 *
 *  CHAMELEON_Dealloc_Worksapce - Deallocate workspace descriptor allocated by
 *                            any workspace allocation routine.
 *
 *******************************************************************************
 *
 * @param[in] desc
 *          Workspace descriptor
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *
 */
int CHAMELEON_Dealloc_Workspace(CHAM_desc_t **desc)
{
    CHAM_context_t *morse;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("CHAMELEON_Dealloc_Workspace", "CHAMELEON not initialized");
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if (*desc == NULL) {
        morse_error("CHAMELEON_Dealloc_Workspace", "attempting to deallocate a NULL descriptor");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    if ((*desc)->mat == NULL && (*desc)->use_mat == 1) {
        morse_error("CHAMELEON_Dealloc_Worspace", "attempting to deallocate a NULL pointer");
        return CHAMELEON_ERR_UNALLOCATED;
    }
    morse_desc_mat_free( *desc );
    RUNTIME_desc_destroy( *desc );

    free(*desc);
    *desc = NULL;
    return CHAMELEON_SUCCESS;
}
