/**
 *
 * @file codelet_ztstrf.c
 *
 * @copyright 2009-2015 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon ztstrf PaRSEC codelet
 *
 * @version 1.0.0
 * @author Reazul Hoque
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_parsec.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_z.h"

static inline int
CORE_ztstrf_parsec( parsec_execution_stream_t *context,
                    parsec_task_t             *this_task )
{
    int m;
    int n;
    int ib;
    int nb;
    CHAMELEON_Complex64_t *U;
    int ldu;
    CHAMELEON_Complex64_t *A;
    int lda;
    CHAMELEON_Complex64_t *L;
    int ldl;
    int *IPIV;
    CHAMELEON_Complex64_t *WORK;
    int ldwork;
    CHAMELEON_bool *check_info;
    int iinfo;

    int info;

    parsec_dtd_unpack_args(
        this_task, &m, &n, &ib, &nb, &U, &ldu, &A, &lda, &L, &ldl, &IPIV, &WORK, &ldwork, &check_info, &iinfo );

    CORE_ztstrf( m, n, ib, nb, U, ldu, A, lda, L, ldl, IPIV, WORK, ldwork, &info );

    (void)context;
    return PARSEC_HOOK_RETURN_DONE;
}

void INSERT_TASK_ztstrf(const RUNTIME_option_t *options,
                       int m, int n, int ib, int nb,
                       const CHAM_desc_t *U, int Um, int Un, int ldu,
                       const CHAM_desc_t *A, int Am, int An, int lda,
                       const CHAM_desc_t *L, int Lm, int Ln, int ldl,
                       int *IPIV,
                       CHAMELEON_bool check_info, int iinfo)
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_ztstrf_parsec, options->priority, "tstrf",
        sizeof(int),           &m,                                VALUE,
        sizeof(int),           &n,                                VALUE,
        sizeof(int),           &ib,                               VALUE,
        sizeof(int),           &nb,                               VALUE,
        PASSED_BY_REF,         RTBLKADDR( U, CHAMELEON_Complex64_t, Um, Un ), morse_parsec_get_arena_index( U ) | INOUT,
        sizeof(int),           &ldu,                              VALUE,
        PASSED_BY_REF,         RTBLKADDR( A, CHAMELEON_Complex64_t, Am, An ), morse_parsec_get_arena_index( A ) | INOUT | AFFINITY,
        sizeof(int),           &lda,                              VALUE,
        PASSED_BY_REF,         RTBLKADDR( L, CHAMELEON_Complex64_t, Lm, Ln ), morse_parsec_get_arena_index( L ) | OUTPUT,
        sizeof(int),           &ldl,                              VALUE,
        sizeof(int*),          &IPIV,                             VALUE,
        sizeof(CHAMELEON_Complex64_t)*ib*nb,    NULL,                 SCRATCH,
        sizeof(int),           &nb,                               VALUE,
        sizeof(int),           &check_info,                       VALUE,
        sizeof(int),           &iinfo,                            VALUE,
        PARSEC_DTD_ARG_END );

    (void)nb;
}
