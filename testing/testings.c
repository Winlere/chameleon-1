/**
 *
 * @file testings.c
 *
 * @copyright 2019-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 ***
 *
 * @brief Chameleon auxiliary routines for testing structures
 *
 * @version 1.2.0
 * @author Lucas Barros de Assis
 * @author Mathieu Faverge
 * @author Alycia Lisito
 * @date 2022-03-01
 *
 */
#include "testings.h"

/**
 * @brief List of all the testings available.
 */
static testing_t *testings = NULL;

/**
 * @brief Function to register a new testing
 */
void
testing_register( testing_t *test )
{
    assert( test->next == NULL );
    test->next = testings;
    testings = test;
}

/**
 * @brief Get the testing structure associated to a test
 */
testing_t *
testing_gettest( const char *prog_name,
                 const char *func_name )
{
    testing_t *test = testings;
    int rank = 0;
#if !defined(CHAMELEON_TESTINGS_VENDOR)
    rank = CHAMELEON_Comm_rank();
#endif

    if ( func_name == NULL ) {
        print_usage( prog_name );
        exit(1);
    }

    while( test != NULL ) {
        /* Check the name with and without the precision */
        if ( (strcasecmp( func_name, test->name     ) == 0) ||
             (strcasecmp( func_name, test->name + 1 ) == 0) )
        {
            break;
        }
        test = test->next;
    }

    if ( test == NULL ) {
        if ( rank == 0 ) {
            printf( "Operation %s not found\n", func_name );
            printf( "The available operations are:\n" );
            test = testings;
            while( test != NULL ) {
                printf( "  %-10s %s\n", test->name, test->helper );
                test = test->next;
            }
        }
        exit(1);
    }

    return test;
}

/**
 * @brief Starts the measure for the testing.
 */
void
testing_start( testdata_t *tdata )
{
#if defined(CHAMELEON_TESTINGS_VENDOR)
    /*
     * If we test the vendor functions, we want to use all the threads of the
     * machine and in order to do so we need to free the thread of the Chameleon
     * scheduler.
     */
    CHAMELEON_Pause();
#else
    int splitsub = parameters_getvalue_int( "splitsub" );
    int async    = parameters_getvalue_int( "async" ) || splitsub;
#endif

    tdata->sequence         = NULL;
    tdata->request.status   = 0;
    tdata->request.schedopt = NULL;

#if !defined(CHAMELEON_TESTINGS_VENDOR)
#if defined(CHAMELEON_USE_MPI)
    CHAMELEON_Distributed_start();
#endif

    if ( async ) {
        CHAMELEON_Sequence_Create( &(tdata->sequence) );
    }

    if ( splitsub ) {
        CHAMELEON_Pause();
    }
#endif

    /* Register starting time */
    tdata->tsub  = RUNTIME_get_time();
    tdata->texec = tdata->tsub;
}

/**
 * @brief Stops the measure for the testing.
 */
void
testing_stop( testdata_t *tdata, cham_fixdbl_t flops )
{
    cham_fixdbl_t t0, t1, t2, gflops;

#if !defined(CHAMELEON_TESTINGS_VENDOR)
    int splitsub = parameters_getvalue_int( "splitsub" );
    int async    = parameters_getvalue_int( "async" ) || splitsub;

    /* Submission is done, we need to start the computations */
    if ( async ) {
        tdata->tsub = RUNTIME_get_time();
        if ( splitsub ) {
            CHAMELEON_Resume();
        }
        CHAMELEON_Sequence_Wait( tdata->sequence );
        CHAMELEON_Sequence_Destroy( tdata->sequence );
    }
#if defined(CHAMELEON_USE_MPI)
    CHAMELEON_Distributed_stop();
#endif
#endif
    t2 = RUNTIME_get_time();

    t0 = tdata->texec;
    t1 = tdata->tsub;
    /*
     * texec / Submission / tsub / Execution / t
     *
     * => texec = t2 - t1
     * => tsub  = t1 - t0
     */
    tdata->tsub  = t1 - t0;
    tdata->texec = t2 - t0;

#if !defined(CHAMELEON_TESTINGS_VENDOR)
    if ( splitsub ) {
        tdata->texec = t2 - t1;
    }
#endif

    gflops = flops * 1.e-9 / tdata->texec;
    run_arg_add_fixdbl( tdata->args, "time", tdata->texec );
    run_arg_add_fixdbl( tdata->args, "tsub", tdata->tsub );
    run_arg_add_fixdbl( tdata->args, "gflops", ( tdata->hres == CHAMELEON_SUCCESS ) ? gflops : -1. );

#if defined(CHAMELEON_TESTINGS_VENDOR)
    /*
     * If we test the vendor functions, we now want to resume all the threads of
     * the machine in order to do the check and potentially the next matrix
     * initialization.
     */
    CHAMELEON_Resume();
#endif
}
