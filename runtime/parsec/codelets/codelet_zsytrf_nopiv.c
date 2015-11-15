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
 * @file codelet_zsytrf_nopiv.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @author Reazul Hoque
 * @precisions normal z -> c
 *
 **/
#include "runtime/parsec/include/morse_parsec.h"

static int
CORE_zsytrf_nopiv_parsec(dague_execution_unit_t *context, dague_execution_context_t *this_task)
{
    MORSE_enum *uplo;
    int *n;
    MORSE_Complex64_t *A;
    int *lda;
    int *iinfo;
    int info;

    dague_dtd_unpack_args(this_task,
                          UNPACK_VALUE, &uplo,
                          UNPACK_VALUE, &n,
                          UNPACK_DATA,  &A,
                          UNPACK_VALUE, &lda,
                          UNPACK_VALUE, &iinfo
                        );


    CORE_zsytf2_nopiv(*uplo, *n, A, *lda);

    return 0;
}

void MORSE_TASK_zsytrf_nopiv(MORSE_option_t *options,
                             MORSE_enum uplo, int n, int nb,
                             MORSE_desc_t *A, int Am, int An, int lda,
                             int iinfo)
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_zsytrf_nopiv_parsec,               "sytrf_nopiv",
        sizeof(MORSE_enum),              &uplo,                VALUE,
        sizeof(int),                     &n,                   VALUE,
        PASSED_BY_REF,         RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     INOUT | REGION_FULL,
        sizeof(int),                     &lda,                 VALUE,
        sizeof(int),                     &iinfo,               VALUE,
        0);
}
