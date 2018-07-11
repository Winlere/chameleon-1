/**
 *
 * @file runtime_async.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2017 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon Quark asynchronous routines
 *
 * @version 1.0.0
 * @author Jakub Kurzak
 * @author Vijay Joshi
 * @author Cedric Castagnede
 * @date 2011-10-29
 *
 */
#include <stdlib.h>
#include "chameleon_quark.h"

/**
 *  Create a sequence
 */
int RUNTIME_sequence_create( CHAM_context_t  *morse,
                             RUNTIME_sequence_t *sequence )
{
    sequence->schedopt = (void*)QUARK_Sequence_Create((Quark*)(morse->schedopt));

    if (sequence->schedopt == NULL) {
        morse_error("CHAMELEON_Sequence_Create", "QUARK_Sequence_Create() failed");
        return CHAMELEON_ERR_OUT_OF_RESOURCES;
    }
    sequence->status = CHAMELEON_SUCCESS;
    return CHAMELEON_SUCCESS;
}

/**
 *  Destroy a sequence
 */
int RUNTIME_sequence_destroy( CHAM_context_t  *morse,
                              RUNTIME_sequence_t *sequence )
{
    QUARK_Sequence_Destroy( (Quark*)(morse->schedopt),
                            (Quark_Sequence *)(sequence->schedopt) );
    return CHAMELEON_SUCCESS;
}

/**
 *  Wait for the completion of a sequence
 */
int RUNTIME_sequence_wait( CHAM_context_t  *morse,
                           RUNTIME_sequence_t *sequence )
{
    QUARK_Sequence_Wait( (Quark*)(morse->schedopt),
                         (Quark_Sequence *)(sequence->schedopt) );
    return CHAMELEON_SUCCESS;
}

/**
 *  Terminate a sequence
 */
void RUNTIME_sequence_flush( CHAM_context_t  *morse,
                             RUNTIME_sequence_t *sequence,
                             RUNTIME_request_t  *request,
                             int status )
{
    sequence->request = request;
    sequence->status = status;
    request->status = status;
    QUARK_Sequence_Cancel( (Quark*)(morse),
                           (Quark_Sequence *)(sequence->schedopt) );
}
