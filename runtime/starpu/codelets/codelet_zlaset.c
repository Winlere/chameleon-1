/**
 *
 * @file starpu/codelet_zlaset.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlaset StarPU codelet
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 1.0.0
 * @author Hatem Ltaief
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2010-11-15
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_starpu.h"
#include "runtime_codelet_z.h"

#if !defined(CHAMELEON_SIMULATION)
static void cl_zlaset_cpu_func(void *descr[], void *cl_arg)
{
    cham_uplo_t uplo;
    int M;
    int N;
    CHAMELEON_Complex64_t alpha;
    CHAMELEON_Complex64_t beta;
    CHAMELEON_Complex64_t *A;
    int LDA;

    A = (CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[0]);
    starpu_codelet_unpack_args(cl_arg, &uplo, &M, &N, &alpha, &beta, &LDA);
    CORE_zlaset(uplo, M, N, alpha, beta, A, LDA);
}
#endif /* !defined(CHAMELEON_SIMULATION) */

/*
 * Codelet definition
 */
CODELETS_CPU(zlaset, 1, cl_zlaset_cpu_func)

/**
 *
 * @ingroup INSERT_TASK_Complex64_t
 *
 *  CORE_zlaset - Sets the elements of the matrix A on the diagonal
 *  to beta and on the off-diagonals to alpha
 *
 *******************************************************************************
 *
 * @param[in] uplo
 *          Specifies which elements of the matrix are to be set
 *          = ChamUpper: Upper part of A is set;
 *          = ChamLower: Lower part of A is set;
 *          = ChamUpperLower: ALL elements of A are set.
 *
 * @param[in] M
 *          The number of rows of the matrix A.  M >= 0.
 *
 * @param[in] N
 *         The number of columns of the matrix A.  N >= 0.
 *
 * @param[in] alpha
 *         The constant to which the off-diagonal elements are to be set.
 *
 * @param[in] beta
 *         The constant to which the diagonal elements are to be set.
 *
 * @param[in,out] A
 *         On entry, the M-by-N tile A.
 *         On exit, A has been set accordingly.
 *
 * @param[in] LDA
 *         The leading dimension of the array A.  LDA >= max(1,M).
 *
 */
void INSERT_TASK_zlaset(const RUNTIME_option_t *options,
                       cham_uplo_t uplo, int M, int N,
                       CHAMELEON_Complex64_t alpha, CHAMELEON_Complex64_t beta,
                       const CHAM_desc_t *A, int Am, int An, int LDA)
{

    struct starpu_codelet *codelet = &cl_zlaset;
    void (*callback)(void*) = options->profiling ? cl_zlaset_callback : NULL;

    CHAMELEON_BEGIN_ACCESS_DECLARATION;
    CHAMELEON_ACCESS_W(A, Am, An);
    CHAMELEON_END_ACCESS_DECLARATION;

    starpu_insert_task(
        starpu_mpi_codelet(codelet),
        STARPU_VALUE,  &uplo,                sizeof(int),
        STARPU_VALUE,     &M,                        sizeof(int),
        STARPU_VALUE,     &N,                        sizeof(int),
        STARPU_VALUE, &alpha,         sizeof(CHAMELEON_Complex64_t),
        STARPU_VALUE,  &beta,         sizeof(CHAMELEON_Complex64_t),
        STARPU_W,      RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An),
        STARPU_VALUE,   &LDA,                        sizeof(int),
        STARPU_PRIORITY,    options->priority,
        STARPU_CALLBACK,    callback,
#if defined(CHAMELEON_CODELETS_HAVE_NAME)
        STARPU_NAME, "zlaset",
#endif
        0);
}
