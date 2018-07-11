/**
 *
 * @file codelet_zlanhe.c
 *
 * @copyright 2009-2015 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlanhe PaRSEC codelet
 *
 * @version 1.0.0
 * @author Reazul Hoque
 * @precisions normal z -> c
 *
 */
#include "chameleon_parsec.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_z.h"

static inline int
CORE_zlanhe_parsec( parsec_execution_stream_t *context,
                    parsec_task_t             *this_task )
{
    cham_normtype_t norm;
    cham_uplo_t uplo;
    int N;
    CHAMELEON_Complex64_t *A;
    int LDA;
    double *work;
    double *normA;

    parsec_dtd_unpack_args(
        this_task,   &norm,   &uplo,   &N, &A,   &LDA, &work, &normA );

    CORE_zlanhe( norm, uplo, N, A, LDA, work, normA );

    (void)context;
    return PARSEC_HOOK_RETURN_DONE;
}

void INSERT_TASK_zlanhe(const RUNTIME_option_t *options,
                       cham_normtype_t norm, cham_uplo_t uplo, int N, int NB,
                       const CHAM_desc_t *A, int Am, int An, int LDA,
                       const CHAM_desc_t *B, int Bm, int Bn)
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    int szeW = chameleon_max( 1, N );

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_zlanhe_parsec, options->priority, "LANHE",
        sizeof(int),            &norm,          VALUE,
        sizeof(int),            &uplo,          VALUE,
        sizeof(int),                   &N,             VALUE,
        PASSED_BY_REF,         RTBLKADDR( A, CHAMELEON_Complex64_t, Am, An ), morse_parsec_get_arena_index( A ) | INPUT,
        sizeof(int),                   &LDA,           VALUE,
        sizeof(double)*szeW,           NULL,           SCRATCH,
        PASSED_BY_REF,         RTBLKADDR( B, double, Bm, Bn ),            OUTPUT | AFFINITY,
        PARSEC_DTD_ARG_END );

    (void)NB;
}
