/**
 *
 * @file testing_zgetrf.c
 *
 * @copyright 2019-2023 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgetrf testing
 *
 * @version 1.3.0
 * @author Lucas Barros de Assis
 * @author Mathieu Faverge
 * @author Alycia Lisito
 * @author Matthieu Kuhn
 * @author Lionel Eyraud-Dubois
 * @date 2023-07-05
 * @precisions normal z -> c d s
 *
 */
#include <chameleon.h>
#include "testings.h"
#include "testing_zcheck.h"
#include <chameleon/flops.h>
#include <coreblas/lapacke.h>

#if !defined(CHAMELEON_TESTINGS_VENDOR)
int
testing_zgetrf_desc( run_arg_list_t *args, int check )
{
    testdata_t test_data = { .args = args };
    int        hres      = 0;

    /* Read arguments */
    int         async = parameters_getvalue_int( "async" );
    int         nb    = run_arg_get_int( args, "nb", 320 );
    int         N     = run_arg_get_int( args, "N", 1000 );
    int         M     = run_arg_get_int( args, "M", N );
    int         LDA   = run_arg_get_int( args, "LDA", M );
    int         seedA = run_arg_get_int( args, "seedA", testing_ialea() );
    cham_diag_t diag  = run_arg_get_diag( args, "diag", ChamUnit );
    int         minMN = chameleon_min( M, N );

    /* Descriptors */
    CHAM_desc_t *descA, *descIPIV;
    void        *ws = NULL;

    CHAMELEON_Set( CHAMELEON_TILE_SIZE, nb );

    /* Creates the matrices */
    parameters_desc_create( "A", &descA, ChamComplexDouble, nb, nb, LDA, N, M, N );
    CHAMELEON_Desc_Create(
        &descIPIV, CHAMELEON_MAT_ALLOC_TILE, ChamInteger, nb, 1, nb, minMN, 1, 0, 0, minMN, 1, CHAMELEON_Comm_size(), 1 );

    /* Fills the matrix with random values */
    if ( diag == ChamUnit ) {
        CHAMELEON_zplgtr_Tile( 0,     ChamUpper, descA, seedA   );
        CHAMELEON_zplgtr_Tile( minMN, ChamLower, descA, seedA+1 );
    }
    else {
        CHAMELEON_zplrnt_Tile( descA, seedA );
    }

    if ( async ) {
        ws = CHAMELEON_zgetrf_WS_Alloc( descA );
    }

    /* Calculates the solution */
    testing_start( &test_data );
    if ( async ) {
        hres = CHAMELEON_zgetrf_Tile_Async( descA, descIPIV, ws, test_data.sequence, &test_data.request );
        CHAMELEON_Desc_Flush( descA, test_data.sequence );
        CHAMELEON_Desc_Flush( descIPIV, test_data.sequence );
    }
    else {
        hres = CHAMELEON_zgetrf_Tile( descA, descIPIV );
    }
    test_data.hres = hres;
    testing_stop( &test_data, flops_zgetrf( M, N ) );

    /* Checks the factorization and residual */
#if !defined(CHAMELEON_SIMULATION)
    if ( check ) {
        CHAM_desc_t *descA0c, *descIPIVc;
        CHAM_desc_t *descA0 = CHAMELEON_Desc_Copy( descA, CHAMELEON_MAT_ALLOC_TILE );
        int         *ipiv;

        /* Create A0c as local to rank 0 on all nodes to gather the matrix */
        CHAMELEON_Desc_Create_User(
            &descA0c, (void*)CHAMELEON_MAT_ALLOC_GLOBAL, ChamComplexDouble,
            nb, nb, nb*nb, M, N, 0, 0, M, N, 1, 1,
            chameleon_getaddr_cm, chameleon_getblkldd_cm, NULL, NULL );
        CHAMELEON_Desc_Create_User(
            &descIPIVc, (void*)CHAMELEON_MAT_ALLOC_GLOBAL, ChamInteger,
            nb, 1, nb, M, 1, 0, 0, M, 1, 1, 1,
            chameleon_getaddr_cm, chameleon_getblkldd_cm, NULL, NULL );

        if ( diag == ChamUnit ) {
            CHAMELEON_zplgtr_Tile( 0,     ChamUpper, descA0c, seedA   );
            CHAMELEON_zplgtr_Tile( minMN, ChamLower, descA0c, seedA+1 );
        }
        else {
            CHAMELEON_zplrnt_Tile( descA0c, seedA );
        }

        /* Cheat code: float (s) is the same size as int */
        CHAMELEON_slacpy_Tile( ChamUpperLower, descIPIV, descIPIVc );
        ipiv = descIPIVc->mat;

        /* Compute the permutation of A0: P * A0 */
        if ( CHAMELEON_Comm_rank() == 0 ) {
            LAPACKE_zlaswp( LAPACK_COL_MAJOR, N, descA0c->mat, M, 1, M, ipiv, 1 );
        }

        CHAMELEON_zlacpy_Tile( ChamUpperLower, descA0c, descA0 );
        CHAMELEON_Desc_Destroy( &descA0c );
        CHAMELEON_Desc_Destroy( &descIPIVc );

        hres += check_zxxtrf( args, ChamGeneral, ChamUpperLower,
                              descA0, descA );

        CHAMELEON_Desc_Destroy( &descA0 );
    }
#endif /* !defined(CHAMELEON_SIMULATION) */

    if ( ws != NULL ) {
        CHAMELEON_zgetrf_WS_Free( ws );
    }

    parameters_desc_destroy( &descA );
    CHAMELEON_Desc_Destroy( &descIPIV );

    return hres;
}
#endif

testing_t   test_zgetrf;
const char *zgetrf_params[] = { "mtxfmt", "nb", "m", "n", "lda", "seedA", "diag", NULL };
const char *zgetrf_output[] = { NULL };
const char *zgetrf_outchk[] = { "||A||", "||A-fact(A)||", "RETURN", NULL };

/**
 * @brief Testing registration function
 */
void testing_zgetrf_init( void ) __attribute__( ( constructor ) );
void
testing_zgetrf_init( void )
{
    test_zgetrf.name   = "zgetrf";
    test_zgetrf.helper = "General LU factorization (with partial pivoting)";
    test_zgetrf.params = zgetrf_params;
    test_zgetrf.output = zgetrf_output;
    test_zgetrf.outchk = zgetrf_outchk;
#if defined(CHAMELEON_TESTINGS_VENDOR)
    test_zgetrf.fptr_desc = NULL;
#else
    test_zgetrf.fptr_desc = testing_zgetrf_desc;
#endif
    test_zgetrf.fptr_std  = NULL; /* testing_zgetrf_std; */
    test_zgetrf.next   = NULL;

    testing_register( &test_zgetrf );
}
