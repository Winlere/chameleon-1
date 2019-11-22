/**
 *
 * @file quark/codelet_zlascal.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlascal Quark codelet
 *
 * @version 0.9.2
 * @author Julien Langou
 * @author Henricus Bouwmeester
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2016-11-30
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_quark.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_ztile.h"

static inline void CORE_zlascal_quark(Quark *quark)
{
    cham_uplo_t uplo;
    int M;
    int N;
    CHAMELEON_Complex64_t alpha;
    CHAM_tile_t *tileA;

    quark_unpack_args_5(quark, uplo, M, N, alpha, tileA);
    TCORE_zlascal(uplo, M, N, alpha, tileA);
}

void INSERT_TASK_zlascal(const RUNTIME_option_t *options,
                        cham_uplo_t uplo,
                        int m, int n, int nb,
                        CHAMELEON_Complex64_t alpha,
                        const CHAM_desc_t *A, int Am, int An)
{
    quark_option_t *opt = (quark_option_t*)(options->schedopt);
    DAG_CORE_LASCAL;
    QUARK_Insert_Task(opt->quark, CORE_zlascal_quark, (Quark_Task_Flags*)opt,
        sizeof(int),              &uplo,  VALUE,
        sizeof(int),                     &m,     VALUE,
        sizeof(int),                     &n,     VALUE,
        sizeof(CHAMELEON_Complex64_t),       &alpha, VALUE,
        sizeof(void*), RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An), INOUT,
        0);
}


