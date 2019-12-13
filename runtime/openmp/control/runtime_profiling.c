/**
 *
 * @file openmp/runtime_profiling.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon StarPU profiling routines
 *
 * @version 0.9.2
 * @author Cedric Augonnet
 * @author Mathieu Faverge
 * @author Cedric Castagnede
 * @date 2018-06-15
 *
 */
#include "chameleon_openmp.h"
#include "chameleon/timer.h"

double RUNTIME_get_time(){
    return CHAMELEON_timer();
}

void RUNTIME_start_profiling()
{
}

void RUNTIME_stop_profiling()
{
}

void RUNTIME_start_stats()
{
    chameleon_warning("RUNTIME_start_stats()", "pruning stats are not available with OpenMP\n");
}

void RUNTIME_stop_stats()
{
    chameleon_warning("RUNTIME_stop_stats()", "pruning stats are not available with OpenMP\n");
}

void RUNTIME_schedprofile_display(void)
{
    chameleon_warning("RUNTIME_schedprofile_display(openmp)", "Scheduler profiling is not available with OpenMP\n");
}

void RUNTIME_kernelprofile_display(void)
{
    chameleon_warning("RUNTIME_kernelprofile_display(openmp)", "Kernel profiling is not available with OpenMP\n");
}

/**
 *  Set iteration numbers for traces
 */
void RUNTIME_iteration_push( CHAM_context_t *chamctxt, unsigned long iteration )
{
    (void)chamctxt; (void)iteration;
    return;
}
void RUNTIME_iteration_pop( CHAM_context_t *chamctxt )
{
    (void)chamctxt;
    return;
}
