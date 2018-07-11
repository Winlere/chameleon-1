/**
 *
 * @file async.h
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon asynchronous management header
 *
 * @version 1.0.0
 * @author Jakub Kurzak
 * @author Cedric Castagnede
 * @date 2010-11-15
 *
 */
#ifndef _CHAMELEON_ASYNC_H_
#define _CHAMELEON_ASYNC_H_

#include "chameleon/struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Internal routines
 */
int morse_request_fail     (RUNTIME_sequence_t *sequence, RUNTIME_request_t *request, int error);
int morse_sequence_create  (CHAM_context_t *CHAMELEON, RUNTIME_sequence_t **sequence);
int morse_sequence_destroy (CHAM_context_t *CHAMELEON, RUNTIME_sequence_t *sequence);
int morse_sequence_wait    (CHAM_context_t *CHAMELEON, RUNTIME_sequence_t *sequence);

#ifdef __cplusplus
}
#endif

#endif
