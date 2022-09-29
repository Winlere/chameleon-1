/**
 *
 * @file starpu/codelet_zherk.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zherk StarPU codelet
 *
 * @version 1.2.0
 * @author Hatem Ltaief
 * @author Jakub Kurzak
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Lucas Barros de Assis
 * @author Florent Pruvost
 * @author Gwenole Lucas
 * @date 2022-02-22
 * @precisions normal z -> c
 *
 */
#include "chameleon_starpu.h"
#include "runtime_codelet_z.h"

struct cl_zherk_args_s {
    cham_uplo_t uplo;
    cham_trans_t trans;
    int n;
    int k;
    double alpha;
    CHAM_tile_t *tileA;
    double beta;
    CHAM_tile_t *tileC;
};

#if !defined(CHAMELEON_SIMULATION)
static void
cl_zherk_cpu_func(void *descr[], void *cl_arg)
{
    struct cl_zherk_args_s *clargs = (struct cl_zherk_args_s *)cl_arg;
    CHAM_tile_t *tileA;
    CHAM_tile_t *tileC;

    tileA = cti_interface_get(descr[0]);
    tileC = cti_interface_get(descr[1]);

    TCORE_zherk( clargs->uplo, clargs->trans, clargs->n, clargs->k,
                 clargs->alpha, tileA, clargs->beta, tileC );
}

#if defined(CHAMELEON_USE_CUDA)
static void
cl_zherk_cuda_func(void *descr[], void *cl_arg)
{
    cublasHandle_t handle = starpu_cublas_get_local_handle();
    struct cl_zherk_args_s *clargs = (struct cl_zherk_args_s *)cl_arg;
    CHAM_tile_t *tileA;
    CHAM_tile_t *tileC;

    tileA = cti_interface_get(descr[0]);
    tileC = cti_interface_get(descr[1]);

    CUDA_zherk(
        clargs->uplo, clargs->trans, clargs->n, clargs->k,
        &(clargs->alpha),
        tileA->mat, tileA->ld,
        &(clargs->beta),
        tileC->mat, tileC->ld,
        handle );
}
#endif /* defined(CHAMELEON_USE_CUDA) */
#endif /* !defined(CHAMELEON_SIMULATION) */

/*
 * Codelet definition
 */
CODELETS( zherk, cl_zherk_cpu_func, cl_zherk_cuda_func, STARPU_CUDA_ASYNC )

void INSERT_TASK_zherk( const RUNTIME_option_t *options,
                        cham_uplo_t uplo, cham_trans_t trans,
                        int n, int k, int nb,
                        double alpha, const CHAM_desc_t *A, int Am, int An,
                        double beta,  const CHAM_desc_t *C, int Cm, int Cn )
{
    if ( alpha == 0. ) {
        INSERT_TASK_zlascal( options, uplo, n, n, nb,
                             beta, C, Cm, Cn );
        return;
    }

    struct cl_zherk_args_s *clargs = NULL;
    void (*callback)(void*);
    int                      accessC;
    int                      exec = 0;
    char                    *cl_name = "zherk";

    /* Handle cache */
    CHAMELEON_BEGIN_ACCESS_DECLARATION;
    CHAMELEON_ACCESS_R(A, Am, An);
    CHAMELEON_ACCESS_RW(C, Cm, Cn);
    exec = __chameleon_need_exec;
    CHAMELEON_END_ACCESS_DECLARATION;

    if ( exec ) {
        clargs = malloc( sizeof( struct cl_zherk_args_s ) );
        clargs->uplo  = uplo;
        clargs->trans = trans;
        clargs->n     = n;
        clargs->k     = k;
        clargs->alpha = alpha;
        clargs->tileA = A->get_blktile( A, Am, An );
        clargs->beta  = beta;
        clargs->tileC = C->get_blktile( C, Cm, Cn );
    }

    /* Callback fro profiling information */
    callback = options->profiling ? cl_zherk_callback : NULL;

    /* Reduce the C access if needed */
    accessC = ( beta == 0. ) ? STARPU_W : STARPU_RW;

#if defined(CHAMELEON_KERNELS_TRACE)
    {
        char *cl_fullname;
        chameleon_asprintf( &cl_fullname, "%s( %s, %s )", cl_name, clargs->tileA->name, clargs->tileC->name );
        cl_name = cl_fullname;
    }
#endif

    /* Insert the task */
    rt_starpu_insert_task(
        &cl_zherk,
        /* Task codelet arguments */
        STARPU_CL_ARGS, clargs, sizeof(struct cl_zherk_args_s),
        STARPU_R,      RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An),
        accessC,       RTBLKADDR(C, CHAMELEON_Complex64_t, Cm, Cn),

        /* Common task arguments */
        STARPU_PRIORITY,          options->priority,
        STARPU_CALLBACK,          callback,
        STARPU_EXECUTE_ON_WORKER, options->workerid,
#if defined(CHAMELEON_CODELETS_HAVE_NAME)
        STARPU_NAME,              cl_name,
#endif

        0 );

    (void)nb;
}
