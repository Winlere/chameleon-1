/**
 *
 * @file openmp/runtime_context.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon StarPU context routines
 *
 * @version 1.0.0
 * @author Cedric Augonnet
 * @author Mathieu Faverge
 * @author Cedric Castagnede
 * @author Philippe Virouleau
 * @date 2020-03-03
 *
 */
#include "chameleon_openmp.h"

/**
 *  Create new context
 */
void RUNTIME_context_create( CHAM_context_t *chamctxt )
{
    chamctxt->scheduler = RUNTIME_SCHED_OPENMP;
    return;
}

/**
 *  Clean the context
 */
void RUNTIME_context_destroy( CHAM_context_t *chamctxt )
{
    (void)chamctxt;
    return;
}

/**
 *
 */
void RUNTIME_enable( void *runtime_ctxt, int lever )
{
    switch (lever)
    {
    case CHAMELEON_DAG:
        fprintf(stderr, "DAG is not available with OpenMP\n");
        break;
    case CHAMELEON_PROFILING_MODE:
        fprintf(stderr, "Profiling is not available with OpenMP\n");
        break;
    case CHAMELEON_BOUND:
        fprintf(stderr, "Bound computation is not available with OpenMP\n");
        break;
    default:
        return;
    }
    (void)runtime_ctxt;
    return;
}

/**
 *
 */
void RUNTIME_disable( void *runtime_ctxt, int lever )
{
    switch (lever)
    {
    case CHAMELEON_DAG:
        fprintf(stderr, "DAG is not available with OpenMP\n");
        break;
    case CHAMELEON_PROFILING_MODE:
        fprintf(stderr, "Profiling is not available with OpenMP\n");
        break;
    case CHAMELEON_BOUND:
        fprintf(stderr, "Bound computation is not available with OpenMP\n");
        break;
    default:
        return;
    }
    (void)runtime_ctxt;
    return;
}
