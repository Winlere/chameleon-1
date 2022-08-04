/**
 *
 * @file starpu/codelet_zgersum.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgersum StarPU codelet
 *
 * @version 1.2.0
 * @author Romain Peressoni
 * @author Mathieu Faverge
 * @date 2022-02-22
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_starpu.h"
#include "runtime_codelet_z.h"

#if !defined(CHAMELEON_SIMULATION)
static void cl_zgersum_redux_cpu_func(void *descr[], void *cl_arg)
{
    CHAM_tile_t *tileA;
    CHAM_tile_t *tileB;

    tileA = cti_interface_get(descr[0]);
    tileB = cti_interface_get(descr[1]);

    assert( tileA->m == tileB->m );
    assert( tileA->n == tileB->n );

    TCORE_zgeadd( ChamNoTrans, tileA->m, tileA->n, 1., tileB, 1., tileA );

    return;
}

#ifdef CHAMELEON_USE_CUBLAS
static void cl_zgersum_redux_cuda_func(void *descr[], void *cl_arg)
{
    cublasHandle_t handle = starpu_cublas_get_local_handle();
    CHAMELEON_Complex64_t zone = 1.;
    CHAM_tile_t *tileA;
    CHAM_tile_t *tileB;

    tileA = cti_interface_get(descr[0]);
    tileB = cti_interface_get(descr[1]);

    assert( tileA->m == tileB->m );
    assert( tileA->n == tileB->n );

    CUDA_zgeadd( ChamNoTrans, tileA->m, tileA->n,
                 &zone, tileB->mat, tileB->ld,
                 &zone, tileA->mat, tileA->ld,
                 handle );

    return;
}
#endif /* defined(CHAMELEON_USE_CUBLAS) */
#endif /* !defined(CHAMELEON_SIMULATION) */

/*
 * Codelet definition
 */
#if defined(CHAMELEON_USE_CUBLAS)
CODELETS(zgersum_redux, cl_zgersum_redux_cpu_func, cl_zgersum_redux_cuda_func, STARPU_CUDA_ASYNC)
#else
CODELETS_CPU(zgersum_redux, cl_zgersum_redux_cpu_func)
#endif

#if !defined(CHAMELEON_SIMULATION)
static void
cl_zgersum_init_cpu_func( void *descr[], void *cl_arg )
{
    CHAM_tile_t *tileA;

    tileA = cti_interface_get(descr[0]);

    TCORE_zlaset( ChamUpperLower, tileA->m, tileA->n, 0., 0., tileA );

    (void)cl_arg;
}
#endif /* !defined(CHAMELEON_SIMULATION) */

/*
 * Codelet definition
 */
CODELETS_CPU( zgersum_init, cl_zgersum_init_cpu_func );

void
RUNTIME_zgersum_set_methods( const CHAM_desc_t *A, int Am, int An )
{
#if defined(HAVE_STARPU_MPI_REDUX)
    starpu_data_set_reduction_methods( RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An),
                                       &cl_zgersum_redux,
                                       &cl_zgersum_init );
#endif
}

void
RUNTIME_zgersum_submit_tree( const RUNTIME_option_t *options,
                             const CHAM_desc_t *A, int Am, int An )
{
#if defined(HAVE_STARPU_MPI_REDUX) && defined(CHAMELEON_USE_MPI)
    starpu_mpi_redux_data_prio_tree( MPI_COMM_WORLD,
                                     RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An),
                                     options->priority + 1,
                                     2 /* Arbre binaire */ );
#else
    (void)options;
    (void)A;
    (void)Am;
    (void)An;
#endif
}

void RUNTIME_zgersum_init( void ) __attribute__( ( constructor ) );
void RUNTIME_zgersum_init( void )
{
    cl_zgersum_init.nbuffers = 1;
    cl_zgersum_init.modes[0] = STARPU_W;

    cl_zgersum_redux.nbuffers = 2;
    cl_zgersum_redux.modes[0] = STARPU_RW | STARPU_COMMUTE;
    cl_zgersum_redux.modes[1] = STARPU_R;
}
