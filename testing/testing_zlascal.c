/**
 *
 * @file testing_zlascal.c
 *
 * @copyright 2019-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlascal testing
 *
 * @version 1.2.0
 * @author Lucas Barros de Assis
 * @author Mathieu Faverge
 * @author Alycia Lisito
 * @date 2022-02-22
 * @precisions normal z -> c d s
 *
 */
#include <chameleon.h>
#include "testings.h"
#include "testing_zcheck.h"
#include <chameleon/flops.h>

static cham_fixdbl_t
flops_zlascal( cham_uplo_t uplo, int M, int N )
{
    cham_fixdbl_t flops = 0.;
    int           minMN = chameleon_min( M, N );
    switch ( uplo ) {
        case ChamUpper:
            flops = ( minMN * ( minMN + 1 ) / 2 ) + M * chameleon_max( 0, N - M );
            break;
        case ChamLower:
            flops = ( minMN * ( minMN + 1 ) / 2 ) + N * chameleon_max( 0, M - N );
            break;
        case ChamUpperLower:
        default:
            flops = M * N;
    }

#if defined(PRECISION_z) || defined(PRECISION_c)
    /* 1 multiplications per element */
    flops *= 6.;
#endif

    return flops;
}

int
testing_zlascal_desc( run_arg_list_t *args, int check )
{
    testdata_t test_data = { .args = args };
    int        hres      = 0;

    /* Read arguments */
    int                   async  = parameters_getvalue_int( "async" );
    intptr_t              mtxfmt = parameters_getvalue_int( "mtxfmt" );
    int                   nb     = run_arg_get_int( args, "nb", 320 );
    int                   P      = parameters_getvalue_int( "P" );
    cham_uplo_t           uplo   = run_arg_get_uplo( args, "uplo", ChamUpper );
    int                   N      = run_arg_get_int( args, "N", 1000 );
    int                   M      = run_arg_get_int( args, "M", N );
    int                   LDA    = run_arg_get_int( args, "LDA", M );
    CHAMELEON_Complex64_t alpha  = run_arg_get_complex64( args, "alpha", 1. );
    int                   seedA  = run_arg_get_int( args, "seedA", random() );
    int                   Q      = parameters_compute_q( P );

    /* Descriptors */
    CHAM_desc_t *descA, *descAinit;

    CHAMELEON_Set( CHAMELEON_TILE_SIZE, nb );

    /* Creates the matrix */
    CHAMELEON_Desc_Create(
        &descA, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDA, N, 0, 0, M, N, P, Q );

    /* Fills the matrix with random values */
    CHAMELEON_zplrnt_Tile( descA, seedA );

    /* Scales the matrix */
    testing_start( &test_data );
    if ( async ) {
        hres = CHAMELEON_zlascal_Tile_Async( uplo, alpha, descA,
                                             test_data.sequence, &test_data.request );
        CHAMELEON_Desc_Flush( descA, test_data.sequence );
    }
    else {
        hres = CHAMELEON_zlascal_Tile( uplo, alpha, descA );
    }
    test_data.hres = hres;
    testing_stop( &test_data, flops_zlascal( uplo, M, N ) );

    /* Checks the solution */
    if ( check ) {
        CHAMELEON_Desc_Create(
            &descAinit, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDA, N, 0, 0, M, N, P, Q );
        CHAMELEON_zplrnt_Tile( descAinit, seedA );

        hres += check_zscale( args, uplo, alpha, descAinit, descA );

        CHAMELEON_Desc_Destroy( &descAinit );
    }

    CHAMELEON_Desc_Destroy( &descA );

    return hres;
}

testing_t   test_zlascal;
const char *zlascal_params[] = { "mtxfmt", "nb", "uplo", "m", "n", "lda", "alpha", "seedA", NULL };
const char *zlascal_output[] = { NULL };
const char *zlascal_outchk[] = { "RETURN", NULL };

/**
 * @brief Testing registration function
 */
void testing_zlascal_init( void ) __attribute__( ( constructor ) );
void
testing_zlascal_init( void )
{
    test_zlascal.name   = "zlascal";
    test_zlascal.helper = "General matrix scaling";
    test_zlascal.params = zlascal_params;
    test_zlascal.output = zlascal_output;
    test_zlascal.outchk = zlascal_outchk;
    test_zlascal.fptr_desc = testing_zlascal_desc;
    test_zlascal.fptr_std  = NULL;
    test_zlascal.next   = NULL;

    testing_register( &test_zlascal );
}
