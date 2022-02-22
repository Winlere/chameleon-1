/**
 *
 * @file testing_zcheck.h
 *
 * @copyright 2019-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon CHAMELEON_Complex64_t auxiliary testings header
 *
 * @version 1.2.0
 * @author Lucas Barros de Assis
 * @author Florent Pruvost
 * @author Mathieu Faverge
 * @author Nathalie Furmento
 * @author Alycia Lisito
 * @date 2022-02-22
 * @precisions normal z -> c d s
 *
 */

#ifndef _testing_zcheck_h_
#define _testing_zcheck_h_

#include "testings.h"
#include <math.h>

#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

#define CHECK_TRMM 3
#define CHECK_TRSM 4

#if defined(CHAMELEON_SIMULATION)

static inline int check_zmatrices_std ( run_arg_list_t *args, cham_uplo_t uplo, int M, int N, CHAMELEON_Complex64_t *A, int LDA, CHAMELEON_Complex64_t *B, int LDB ) { return 0; }
static inline int check_zmatrices     ( run_arg_list_t *args, cham_uplo_t uplo, CHAM_desc_t *descA, CHAM_desc_t *descB ) { return 0; }
static inline int check_znorm_std     ( run_arg_list_t *args, cham_mtxtype_t matrix_type, cham_normtype_t norm_type, cham_uplo_t uplo,
                                        cham_diag_t diag, double norm_cham, int M, int N, CHAMELEON_Complex64_t *A, int LDA ) { return 0; }
static inline int check_znorm         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_normtype_t norm_type, cham_uplo_t uplo,
                                        cham_diag_t diag, double norm_cham, CHAM_desc_t *descA ) { return 0; }
static inline int check_zsum          ( run_arg_list_t *args, cham_uplo_t uplo, cham_trans_t trans, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                                        CHAMELEON_Complex64_t beta, CHAM_desc_t *descBref, CHAM_desc_t *descBcham ) { return 0; }
static inline int check_zscale        ( run_arg_list_t *args, cham_uplo_t uplo, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA1, CHAM_desc_t *descA2 ) { return 0; }
static inline int check_zgemm_std     ( run_arg_list_t *args, cham_trans_t transA, cham_trans_t transB, CHAMELEON_Complex64_t alpha, int M, int N, int K, CHAMELEON_Complex64_t *A, int LDA,
                                        CHAMELEON_Complex64_t *B, int LDB, CHAMELEON_Complex64_t beta, CHAMELEON_Complex64_t *Cref, CHAMELEON_Complex64_t *C, int LDC )  { return 0; }
static inline int check_zgemm         ( run_arg_list_t *args, cham_trans_t transA, cham_trans_t transB, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                                        CHAM_desc_t *descB, CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC ) { return 0; }
static inline int check_zsymm_std     ( run_arg_list_t *args, cham_mtxtype_t matrix_type, cham_side_t side, cham_uplo_t uplo, int M, int N, CHAMELEON_Complex64_t alpha, CHAMELEON_Complex64_t *A, int LDA,
                                        CHAMELEON_Complex64_t *B, int LDB, CHAMELEON_Complex64_t beta, CHAMELEON_Complex64_t *Cref, CHAMELEON_Complex64_t *C, int LDC ) { return 0; }
static inline int check_zsymm         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_side_t side, cham_uplo_t uplo, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA, CHAM_desc_t *descB,
                                        CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC ) { return 0; }
static inline int check_zsyrk_std     ( run_arg_list_t *args, cham_mtxtype_t matrix_type, cham_uplo_t uplo, cham_trans_t trans, int N, int K, CHAMELEON_Complex64_t alpha, CHAMELEON_Complex64_t *A, int LDA,
                                        CHAMELEON_Complex64_t *B, int LDB, CHAMELEON_Complex64_t beta, CHAMELEON_Complex64_t *Cref, CHAMELEON_Complex64_t *C, int LDC ) { return 0; }
static inline int check_zsyrk         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, cham_trans_t trans, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                                        CHAM_desc_t *descB, CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC ) { return 0; }
static inline int check_ztrmm_std     ( run_arg_list_t *args, int check_func, cham_side_t side, cham_uplo_t uplo, cham_trans_t trans, cham_diag_t diag, int M, int N, CHAMELEON_Complex64_t alpha,
                                        CHAMELEON_Complex64_t *A, int LDA, CHAMELEON_Complex64_t *Bref, CHAMELEON_Complex64_t *B, int LDB ) { return 0; }
static inline int check_ztrmm         ( run_arg_list_t *args, int check_func, cham_side_t side, cham_uplo_t uplo, cham_trans_t trans, cham_diag_t diag,
                                        CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA, CHAM_desc_t *descBref, CHAM_desc_t *descB ) { return 0; }
static inline int check_zlauum        ( run_arg_list_t *args, cham_uplo_t uplo, CHAM_desc_t *descA1, CHAM_desc_t *descA2 ) { return 0; }
static inline int check_zxxtrf        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, CHAM_desc_t *descA1, CHAM_desc_t *descA2 ) { return 0; }
static inline int check_zxxtrf_std    ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, int M, int N, CHAMELEON_Complex64_t *A, CHAMELEON_Complex64_t *LU, int LDA ) { return 0; }
static inline int check_zsolve        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_trans_t trans, cham_uplo_t uplo,
                                        CHAM_desc_t *descA, CHAM_desc_t *descX, CHAM_desc_t *descB ) { return 0; }
static inline int check_zsolve_std    ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_trans_t trans, cham_uplo_t uplo, int N, int NRHS,
                                        CHAMELEON_Complex64_t *A, int LDA, CHAMELEON_Complex64_t *X, CHAMELEON_Complex64_t *B, int LDB ) { return 0; }
static inline int check_ztrtri        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, cham_diag_t diag,
                                        CHAM_desc_t *descA, CHAM_desc_t *descAi ) { return 0; }
static inline int check_ztrtri_std    ( run_arg_list_t *args, cham_mtxtype_t matrix_type, cham_uplo_t uplo, cham_diag_t diag,
                                        int N, CHAMELEON_Complex64_t *A0, CHAMELEON_Complex64_t *Ai, int LDA ) { return 0; }

/* Using QR factorization */
static inline int check_zortho        ( run_arg_list_t *args, CHAM_desc_t *descQ ) { return 0; }
static inline int check_zgeqrf        ( run_arg_list_t *args, CHAM_desc_t *descA, CHAM_desc_t *descAF, CHAM_desc_t *descQ ) { return 0; }
static inline int check_zgelqf        ( run_arg_list_t *args, CHAM_desc_t *descA, CHAM_desc_t *descAF, CHAM_desc_t *descQ ) { return 0; }
static inline int check_zgels         ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, CHAM_desc_t *descX, CHAM_desc_t *descB ) { return 0; }
static inline int check_zgeqrs        ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, double Bnorm, CHAM_desc_t *descR ) { return 0; }
static inline int check_zgelqs        ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, double Bnorm, CHAM_desc_t *descR ) { return 0; }
static inline int check_zqc           ( run_arg_list_t *args, cham_side_t side, cham_trans_t trans, CHAM_desc_t *descC, CHAM_desc_t *descQ, CHAM_desc_t *descCC ) { return 0; }

/* Matrix Generators */
static inline int check_zrankk        ( run_arg_list_t *args, int K, CHAM_desc_t *descA ) { return 0; }

/* Polar decomposition */
static inline int check_zgepdf_qr     ( run_arg_list_t *args, CHAM_desc_t *descA1, CHAM_desc_t *descA2,
                                        CHAM_desc_t *descQ1, CHAM_desc_t *descQ2, CHAM_desc_t *descAF1 ) { return 0; }
static inline int check_zxxpd         ( run_arg_list_t *args,
                                        CHAM_desc_t *descA, CHAM_desc_t *descU, CHAM_desc_t *descH ) { return 0; }

#else /* !defined(CHAMELEON_SIMULATION) */

int check_zmatrices_std ( run_arg_list_t *args, cham_uplo_t uplo, int M, int N, CHAMELEON_Complex64_t *A, int LDA, CHAMELEON_Complex64_t *B, int LDB );
int check_zmatrices     ( run_arg_list_t *args, cham_uplo_t uplo, CHAM_desc_t *descA, CHAM_desc_t *descB );
int check_znorm_std     ( run_arg_list_t *args, cham_mtxtype_t matrix_type, cham_normtype_t norm_type, cham_uplo_t uplo,
                          cham_diag_t diag, double norm_cham, int M, int N, CHAMELEON_Complex64_t *A, int LDA );
int check_znorm         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_normtype_t norm_type, cham_uplo_t uplo,
                          cham_diag_t diag, double norm_cham, CHAM_desc_t *descA );
int check_zsum          ( run_arg_list_t *args, cham_uplo_t uplo, cham_trans_t trans, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                          CHAMELEON_Complex64_t beta, CHAM_desc_t *descBref, CHAM_desc_t *descBcham );
int check_zscale        ( run_arg_list_t *args, cham_uplo_t uplo, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA1, CHAM_desc_t *descA2 );
int check_zgemm_std     ( run_arg_list_t *args, cham_trans_t transA, cham_trans_t transB, CHAMELEON_Complex64_t alpha, int M, int N, int K, CHAMELEON_Complex64_t *A, int LDA,
                          CHAMELEON_Complex64_t *B, int LDB, CHAMELEON_Complex64_t beta, CHAMELEON_Complex64_t *Cref, CHAMELEON_Complex64_t *C, int LDC );
int check_zgemm         ( run_arg_list_t *args, cham_trans_t transA, cham_trans_t transB, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                          CHAM_desc_t *descB, CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC );
int check_zsymm_std     ( run_arg_list_t *args, cham_mtxtype_t matrix_type, cham_side_t side, cham_uplo_t uplo, int M, int N, CHAMELEON_Complex64_t alpha, CHAMELEON_Complex64_t *A, int LDA,
                          CHAMELEON_Complex64_t *B, int LDB, CHAMELEON_Complex64_t beta, CHAMELEON_Complex64_t *Cref, CHAMELEON_Complex64_t *C, int LDC );
int check_zsymm         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_side_t side, cham_uplo_t uplo, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA, CHAM_desc_t *descB,
                          CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC );
int check_zsyrk_std     ( run_arg_list_t *args, cham_mtxtype_t matrix_type, cham_uplo_t uplo, cham_trans_t trans, int N, int K, CHAMELEON_Complex64_t alpha, CHAMELEON_Complex64_t *A, int LDA,
                          CHAMELEON_Complex64_t *B, int LDB, CHAMELEON_Complex64_t beta, CHAMELEON_Complex64_t *Cref, CHAMELEON_Complex64_t *C, int LDC );
int check_zsyrk         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, cham_trans_t trans, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                          CHAM_desc_t *descB, CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC );
int check_ztrmm_std     ( run_arg_list_t *args, int check_func, cham_side_t side, cham_uplo_t uplo, cham_trans_t trans, cham_diag_t diag, int M, int N, CHAMELEON_Complex64_t alpha,
                          CHAMELEON_Complex64_t *A, int LDA, CHAMELEON_Complex64_t *Bref, CHAMELEON_Complex64_t *B, int LDB );
int check_ztrmm         ( run_arg_list_t *args, int check_func, cham_side_t side, cham_uplo_t uplo, cham_trans_t trans, cham_diag_t diag,
                          CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA, CHAM_desc_t *descBref, CHAM_desc_t *descB );
int check_zlauum        ( run_arg_list_t *args, cham_uplo_t uplo, CHAM_desc_t *descA1, CHAM_desc_t *descA2 );
int check_zxxtrf        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, CHAM_desc_t *descA1, CHAM_desc_t *descA2 );
int check_zxxtrf_std    ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, int M, int N, CHAMELEON_Complex64_t *A, CHAMELEON_Complex64_t *LU, int LDA );
int check_zsolve        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_trans_t trans, cham_uplo_t uplo,
                          CHAM_desc_t *descA, CHAM_desc_t *descX, CHAM_desc_t *descB );
int check_zsolve_std    ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_trans_t trans, cham_uplo_t uplo, int N, int NRHS,
                          CHAMELEON_Complex64_t *A, int LDA, CHAMELEON_Complex64_t *X, CHAMELEON_Complex64_t *B, int LDB );
int check_ztrtri        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, cham_diag_t diag,
                          CHAM_desc_t *descA, CHAM_desc_t *descAi );
int check_ztrtri_std    ( run_arg_list_t *args, cham_mtxtype_t matrix_type, cham_uplo_t uplo, cham_diag_t diag,
                          int N, CHAMELEON_Complex64_t *A0, CHAMELEON_Complex64_t *Ai, int LDA );

/* Using QR factorization */
int check_zortho        ( run_arg_list_t *args, CHAM_desc_t *descQ );
int check_zgeqrf        ( run_arg_list_t *args, CHAM_desc_t *descA, CHAM_desc_t *descAF, CHAM_desc_t *descQ );
int check_zgelqf        ( run_arg_list_t *args, CHAM_desc_t *descA, CHAM_desc_t *descAF, CHAM_desc_t *descQ );
int check_zgels         ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, CHAM_desc_t *descX, CHAM_desc_t *descB );
int check_zgeqrs        ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, double Bnorm, CHAM_desc_t *descR );
int check_zgelqs        ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, double Bnorm, CHAM_desc_t *descR );
int check_zqc           ( run_arg_list_t *args, cham_side_t side, cham_trans_t trans, CHAM_desc_t *descC, CHAM_desc_t *descQ, CHAM_desc_t *descCC );

/* Matrix Generators */
int check_zrankk        ( run_arg_list_t *args, int K, CHAM_desc_t *descA );

/* Polar decomposition */
int check_zgepdf_qr     ( run_arg_list_t *args, CHAM_desc_t *descA1, CHAM_desc_t *descA2,
                          CHAM_desc_t *descQ1, CHAM_desc_t *descQ2, CHAM_desc_t *descAF1 );
int check_zxxpd         ( run_arg_list_t *args,
                          CHAM_desc_t *descA, CHAM_desc_t *descU, CHAM_desc_t *descH );

#endif /* defined(CHAMELEON_SIMULATION) */

#endif /* _testing_zcheck_h_ */
