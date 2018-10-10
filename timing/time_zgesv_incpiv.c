/**
 *
 * @file time_zgesv_incpiv.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @version 1.0.0
 * @precisions normal z -> c d s
 *
 */
#define _TYPE  CHAMELEON_Complex64_t
#define _PREC  double
#define _LAMCH LAPACKE_dlamch_work

#define _NAME  "CHAMELEON_zgesv_incpiv"
/* See Lawn 41 page 120 */
#define _FMULS (FMULS_GETRF( N, N ) + FMULS_GETRS( N, NRHS ))
#define _FADDS (FADDS_GETRF( N, N ) + FADDS_GETRS( N, NRHS ))

#include "./timing.c"
#include "timing_zauxiliary.h"

static int
RunTest(int *iparam, double *dparam, chameleon_time_t *t_) 
{
    CHAM_desc_t *L;
    int *piv;
    PASTE_CODE_IPARAM_LOCALS( iparam );
    
    if ( M != N ) {
        fprintf(stderr, "This timing works only with M == N\n");
        return -1;
    }
    
    /* Allocate Data */
    PASTE_CODE_ALLOCATE_MATRIX( A, 1, CHAMELEON_Complex64_t, LDA, N    );
    PASTE_CODE_ALLOCATE_MATRIX( X, 1, CHAMELEON_Complex64_t, LDB, NRHS );
    
    /* Initialize Data */
    CHAMELEON_zplrnt( N, N,    A, LDA,   51 );
    CHAMELEON_zplrnt( N, NRHS, X, LDB, 5673 );

    CHAMELEON_Alloc_Workspace_zgesv_incpiv(N, &L, &piv, P, Q);

    /* Save A and b  */
    PASTE_CODE_ALLOCATE_COPY( Acpy, check, CHAMELEON_Complex64_t, A, LDA, N    );
    PASTE_CODE_ALLOCATE_COPY( B,    check, CHAMELEON_Complex64_t, X, LDB, NRHS );

    START_TIMING();
    CHAMELEON_zgesv_incpiv( N, NRHS, A, N, L, piv, X, LDB );
    STOP_TIMING();
    
    /* Check the solution */
    if (check)
    {
        dparam[IPARAM_RES] = z_check_solution(N, N, NRHS, Acpy, LDA, B, X, LDB,
                                              &(dparam[IPARAM_ANORM]), 
                                              &(dparam[IPARAM_BNORM]), 
                                              &(dparam[IPARAM_XNORM]));
        free(Acpy); free(B);
    }

    CHAMELEON_Dealloc_Workspace( &L );
    free( piv );
    free( X );
    free( A );


    return 0;
}
