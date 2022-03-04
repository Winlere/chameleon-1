/**
 *
 * @file chameleon_ztesting.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon CHAMELEON_Complex64_t auxiliary testings routines
 *
 * @version 1.2.0
 * @author Mathieu Faverge
 * @author Cédric Castagnède
 * @author Lucas Barros de Assis
 * @author Florent Pruvost
 * @author AGULLO Emmanuel
 * @author Alycia Lisito
 * @author Philippe Swartvagher
 * @date 2022-02-22
 * @precisions normal z -> c d s
 *
 */
#include "testings.h"

int main (int argc, char **argv) {

    int ncores, ngpus, human, generic, check, i, niter;
    int trace, nowarmup, profile, forcegpu, api;
    int rc, info = 0;
    int run_id = 0;
    char *func_name;
    char *input_file;
    run_list_t *runlist;
    testing_t * test;
    run_list_elt_t *run, *next;

    /* Reads the arguments from command line */
    parameters_parser( argc, argv );
    input_file = parameters_getvalue_str( "file" );
    if ( input_file != NULL ) {
        parameters_read_file( input_file );
        free(input_file);
    }
    ncores    = parameters_getvalue_int( "threads"  );
    ngpus     = parameters_getvalue_int( "gpus"     );
    check     = parameters_getvalue_int( "check"    );
    human     = parameters_getvalue_int( "human"    );
    generic   = parameters_getvalue_int( "generic"  );
    func_name = parameters_getvalue_str( "op"       );
    niter     = parameters_getvalue_int( "niter"    );
    trace     = parameters_getvalue_int( "trace"    );
    nowarmup  = parameters_getvalue_int( "nowarmup" );
    profile   = parameters_getvalue_int( "profile"  );
    forcegpu  = parameters_getvalue_int( "forcegpu" );
    api       = parameters_getvalue_int( "api"      );

    rc = CHAMELEON_Init( ncores, ngpus );
    if ( rc != CHAMELEON_SUCCESS ) {
        fprintf( stderr, "CHAMELEON_Init failed and returned %d.\n", rc );
        info = 1;
        goto end;
    }

    /* Set ncores to the right value */
    if ( ncores == -1 ) {
        parameter_t *param;
        param = parameters_get( 't' );
        param->value.ival = CHAMELEON_GetThreadNbr();
    }

    /* Binds the right function to be called and builds the parameters combinations */
    test = testing_gettest( argv[0], func_name );
    free(func_name);
    test_fct_t fptr = (api == 0) ? test->fptr_desc : test->fptr_std;
    if ( fptr == NULL ) {
        fprintf( stderr, "The %s API is not available for function %s\n",
                 (api == 0) ? "descriptor" : "standard", func_name );
        info = 1;
        goto end;
    }

    /* Generate the cartesian product of the parameters */
    runlist = run_list_generate( test->params );

    /* Executes the tests */
    run_print_header( test, check, human );
    run = runlist->head;

    /* Force all possible kernels on GPU */
    if ( forcegpu ) {
        if ( ngpus == 0 ) {
            fprintf( stderr,
                     "--forcegpu can't be enable without GPU (-g 0).\n"
                     "  Please specify a larger number of GPU or disable this option\n" );
            info = 1;
            goto end;
        }
        RUNTIME_zlocality_allrestrict( RUNTIME_CUDA );
    }

    /* Warmup */
    if ( !nowarmup ) {
        run_arg_list_t copy = run_arg_list_copy( &(run->args) );
        fptr( &copy, check );
        run_arg_list_destroy( &copy );
    }

    /* Start kernel statistics */
    if ( profile ) {
        CHAMELEON_Enable( CHAMELEON_KERNELPROFILE_MODE );
    }

    /* Start tracing */
    if ( trace ) {
        CHAMELEON_Enable( CHAMELEON_PROFILING_MODE );
    }

    if ( generic ) {
        CHAMELEON_Enable( CHAMELEON_GENERIC );
    }

    /* Perform all runs */
    while ( run != NULL ) {
        for(i=0; i<niter; i++) {
            run_arg_list_t copy = run_arg_list_copy( &(run->args) );
            rc = fptr( &copy, check );

            /* If rc < 0, we skipped the test */
            if ( rc >= 0 ) {
                run_arg_add_int( &copy, "RETURN", rc );
                run_print_line( test, &copy, check, human, run_id );
                run_id++;
                info += rc;
            }
            run_arg_list_destroy( &copy );
        }

        /* Move to next run */
        next = run->next;
        run_list_destroy( run );
        run = next;
    }

    /* Stop tracing */
    if ( trace ) {
        CHAMELEON_Disable( CHAMELEON_PROFILING_MODE );
    }

    /* Stop kernel statistics and display results */
    if ( profile ) {
        CHAMELEON_Disable( CHAMELEON_KERNELPROFILE_MODE );
        RUNTIME_kernelprofile_display();
    }
    free( runlist );

  end:
    ;/* OpenMP end */
    CHAMELEON_Finalize();
    parameters_destroy();

    return info;
}
