/**
 *
 * @file starpu/codelet_zgeadd.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgeadd StarPU codelet
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 1.0.0
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2010-11-15
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_starpu.h"
#include "runtime_codelet_z.h"

/**
 ******************************************************************************
 *
 * @ingroup INSERT_TASK_Complex64_t
 *
 *  INSERT_TASK_zgeadd adds two general matrices together as in PBLAS pzgeadd.
 *
 *       B <- alpha * op(A)  + beta * B,
 *
 * where op(X) = X, X', or conj(X')
 *
 *******************************************************************************
 *
 * @param[in] trans
 *          Specifies whether the matrix A is non-transposed, transposed, or
 *          conjugate transposed
 *          = ChamNoTrans:   op(A) = A
 *          = ChamTrans:     op(A) = A'
 *          = ChamConjTrans: op(A) = conj(A')
 *
 * @param[in] M
 *          Number of rows of the matrices op(A) and B.
 *
 * @param[in] N
 *          Number of columns of the matrices op(A) and B.
 *
 * @param[in] alpha
 *          Scalar factor of A.
 *
 * @param[in] A
 *          Matrix of size LDA-by-N, if trans = ChamNoTrans, LDA-by-M
 *          otherwise.
 *
 * @param[in] LDA
 *          Leading dimension of the array A. LDA >= max(1,k), with k=M, if
 *          trans = ChamNoTrans, and k=N otherwise.
 *
 * @param[in] beta
 *          Scalar factor of B.
 *
 * @param[in,out] B
 *          Matrix of size LDB-by-N.
 *          On exit, B = alpha * op(A) + beta * B
 *
 * @param[in] LDB
 *          Leading dimension of the array B. LDB >= max(1,M)
 *
 *******************************************************************************
 *
 * @return
 *          \retval CHAMELEON_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 */
void INSERT_TASK_zgeadd(const RUNTIME_option_t *options,
                       cham_trans_t trans, int m, int n, int nb,
                       CHAMELEON_Complex64_t alpha, const CHAM_desc_t *A, int Am, int An, int lda,
                       CHAMELEON_Complex64_t beta,  const CHAM_desc_t *B, int Bm, int Bn, int ldb)
{
    struct starpu_codelet *codelet = &cl_zgeadd;
    void (*callback)(void*) = options->profiling ? cl_zgeadd_callback : NULL;

    CHAMELEON_BEGIN_ACCESS_DECLARATION;
    CHAMELEON_ACCESS_R(A, Am, An);
    CHAMELEON_ACCESS_RW(B, Bm, Bn);
    CHAMELEON_END_ACCESS_DECLARATION;

    starpu_insert_task(
        starpu_mpi_codelet(codelet),
        STARPU_VALUE,    &trans,              sizeof(int),
        STARPU_VALUE,    &m,                  sizeof(int),
        STARPU_VALUE,    &n,                  sizeof(int),
        STARPU_VALUE,    &alpha,              sizeof(CHAMELEON_Complex64_t),
        STARPU_R,         RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An),
        STARPU_VALUE,    &lda,                sizeof(int),
        STARPU_VALUE,    &beta,               sizeof(CHAMELEON_Complex64_t),
        STARPU_RW,        RTBLKADDR(B, CHAMELEON_Complex64_t, Bm, Bn),
        STARPU_VALUE,    &ldb,                sizeof(int),
        STARPU_PRIORITY,  options->priority,
        STARPU_CALLBACK,  callback,
#if defined(CHAMELEON_CODELETS_HAVE_NAME)
        STARPU_NAME, "zgeadd",
#endif
        0);

    (void)nb;
}


#if !defined(CHAMELEON_SIMULATION)
static void cl_zgeadd_cpu_func(void *descr[], void *cl_arg)
{
    cham_trans_t trans;
    int M;
    int N;
    CHAMELEON_Complex64_t alpha;
    CHAMELEON_Complex64_t *A;
    int LDA;
    CHAMELEON_Complex64_t beta;
    CHAMELEON_Complex64_t *B;
    int LDB;

    A = (CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[0]);
    B = (CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[1]);
    starpu_codelet_unpack_args(cl_arg, &trans, &M, &N, &alpha, &LDA, &beta, &LDB);
    CORE_zgeadd(trans, M, N, alpha, A, LDA, beta, B, LDB);
    return;
}

#ifdef CHAMELEON_USE_CUBLAS_V2
static void cl_zgeadd_cuda_func(void *descr[], void *cl_arg)
{
    cham_trans_t trans;
    int M;
    int N;
    cuDoubleComplex alpha;
    const cuDoubleComplex *A;
    int lda;
    cuDoubleComplex beta;
    cuDoubleComplex *B;
    int ldb;

    A = (const cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[0]);
    B = (cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[1]);
    starpu_codelet_unpack_args(cl_arg, &trans, &M, &N, &alpha, &lda, &beta, &ldb);

    RUNTIME_getStream( stream );

    CUDA_zgeadd(
        trans,
        M, N,
        &alpha, A, lda,
        &beta,  B, ldb,
        stream);

#ifndef STARPU_CUDA_ASYNC
    cudaStreamSynchronize( stream );
#endif

    return;
}
#endif /* defined(CHAMELEON_USE_CUBLAS_V2) */
#endif /* !defined(CHAMELEON_SIMULATION) */

/*
 * Codelet definition
 */
#if defined(CHAMELEON_USE_CUBLAS_V2)
CODELETS(zgeadd, 2, cl_zgeadd_cpu_func, cl_zgeadd_cuda_func, STARPU_CUDA_ASYNC)
#else
CODELETS_CPU(zgeadd, 2, cl_zgeadd_cpu_func)
#endif
