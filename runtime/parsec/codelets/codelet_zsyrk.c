/**
 *
 * @copyright (c) 2009-2015 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2015 Inria. All rights reserved.
 * @copyright (c) 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 * @file codelet_zsyrk.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @author Reazul Hoque
 * @precisions normal z -> c d s
 *
 **/
#include "chameleon_parsec.h"
#include "chameleon/morse_tasks_z.h"
#include "coreblas/coreblas_z.h"

static int
CORE_zsyrk_parsec(parsec_execution_stream_t *context, parsec_task_t *this_task)
{
    MORSE_enum *uplo;
    MORSE_enum *trans;
    int *n;
    int *k;
    MORSE_Complex64_t *alpha;
    MORSE_Complex64_t *A;
    int *lda;
    MORSE_Complex64_t *beta;
    MORSE_Complex64_t *C;
    int *ldc;

    parsec_dtd_unpack_args(
        this_task,
        UNPACK_VALUE, &uplo,
        UNPACK_VALUE, &trans,
        UNPACK_VALUE, &n,
        UNPACK_VALUE, &k,
        UNPACK_VALUE, &alpha,
        UNPACK_DATA,  &A,
        UNPACK_VALUE, &lda,
        UNPACK_VALUE, &beta,
        UNPACK_DATA,  &C,
        UNPACK_VALUE, &ldc );

    CORE_zsyrk(*uplo, *trans, *n, *k,
               *alpha, A, *lda,
               *beta,  C, *ldc);

    return 0;
}

void MORSE_TASK_zsyrk(const MORSE_option_t *options,
                      MORSE_enum uplo, MORSE_enum trans,
                      int n, int k, int nb,
                      MORSE_Complex64_t alpha, const MORSE_desc_t *A, int Am, int An, int lda,
                      MORSE_Complex64_t beta, const MORSE_desc_t *C, int Cm, int Cn, int ldc)
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_zsyrk_parsec, options->priority, "syrk",
        sizeof(MORSE_enum),    &uplo,                              VALUE,
        sizeof(MORSE_enum),    &trans,                             VALUE,
        sizeof(int),           &n,                                 VALUE,
        sizeof(int),           &k,                                 VALUE,
        sizeof(MORSE_Complex64_t),           &alpha,               VALUE,
        PASSED_BY_REF,         RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     INPUT | morse_parsec_get_arena_index(A),
        sizeof(int),           &lda,                               VALUE,
        sizeof(MORSE_Complex64_t),           &beta,                VALUE,
        PASSED_BY_REF,         RTBLKADDR( C, MORSE_Complex64_t, Cm, Cn ),     INOUT | morse_parsec_get_arena_index(C) | AFFINITY,
        sizeof(int),           &ldc,                               VALUE,
        0);
}
