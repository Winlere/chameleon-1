/**
 *
 * @copyright (c) 2009-2015 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2015 Inria. All rights reserved.
 * @copyright (c) 2012-2015 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 * @file codelet_zsyr2k.c
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
#include "runtime/parsec/include/morse_parsec.h"

static int
CORE_zsyr2k_parsec(dague_execution_unit_t *context, dague_execution_context_t *this_task)
{
    MORSE_enum *uplo;
    MORSE_enum *trans;
    int *n;
    int *k;
    MORSE_Complex64_t *alpha;
    MORSE_Complex64_t *A;
    int *lda;
    MORSE_Complex64_t *B;
    int *ldb;
    MORSE_Complex64_t *beta;
    MORSE_Complex64_t *C;
    int *ldc;

    dague_dtd_unpack_args(this_task,
                          UNPACK_VALUE, &uplo,
                          UNPACK_VALUE, &trans,
                          UNPACK_VALUE, &n,
                          UNPACK_VALUE, &k,
                          UNPACK_VALUE, &alpha,
                          UNPACK_DATA,  &A,
                          UNPACK_VALUE, &lda,
                          UNPACK_DATA,  &B,
                          UNPACK_VALUE, &ldb,
                          UNPACK_VALUE, &beta,
                          UNPACK_DATA,  &C,
                          UNPACK_VALUE, &ldc
                        );


    CORE_zsyr2k(*uplo, *trans, *n, *k,
                *alpha, A, *lda,
                        B, *ldb,
                *beta,  C, *ldc);

    return 0;
}

void MORSE_TASK_zsyr2k(MORSE_option_t *options,
                       MORSE_enum uplo, MORSE_enum trans,
                       int n, int k, int nb,
                       MORSE_Complex64_t alpha, MORSE_desc_t *A, int Am, int An, int lda,
                       MORSE_desc_t *B, int Bm, int Bn, int ldb,
                       MORSE_Complex64_t beta, MORSE_desc_t *C, int Cm, int Cn, int ldc)
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_zsyr2k_parsec,    "syr2k",
                            sizeof(MORSE_enum),     &uplo,                  VALUE,
                            sizeof(MORSE_enum),     &trans,                 VALUE,
                            sizeof(int),            &n,                     VALUE,
                            sizeof(int),            &k,                     VALUE,
                            sizeof(MORSE_Complex64_t), &alpha,              VALUE,
                            PASSED_BY_REF,          RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     INPUT | REGION_FULL,
                            sizeof(int),            &lda,                   VALUE,
                            PASSED_BY_REF,          RTBLKADDR( B, MORSE_Complex64_t, Bm, Bn ),     INPUT | REGION_FULL,
                            sizeof(int),            &ldb,                   VALUE,
                            sizeof(MORSE_Complex64_t), &beta,               VALUE,
                            PASSED_BY_REF,          RTBLKADDR( C, MORSE_Complex64_t, Cm, Cn ),     INOUT | REGION_FULL,
                            sizeof(int),            &ldc,                   VALUE,
                             0);
}
