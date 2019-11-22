/**
 *
 * @file parsec/codelet_zhessq.c
 *
 * @copyright 2009-2015 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zhessq PaRSEC codelet
 *
 * @version 0.9.2
 * @author Reazul Hoque
 * @date 2015-11-04
 * @precisions normal z -> c
 *
 */
#include "chameleon_parsec.h"
#include "chameleon/tasks_z.h"

void INSERT_TASK_zhessq( const RUNTIME_option_t *options,
                         cham_store_t storev, cham_uplo_t uplo, int n,
                         const CHAM_desc_t *A, int Am, int An,
                         const CHAM_desc_t *SCALESUMSQ, int SCALESUMSQm, int SCALESUMSQn )
{
    INSERT_TASK_zsyssq( options, storev, uplo, n,
                        A, Am, An,
                        SCALESUMSQ, SCALESUMSQm, SCALESUMSQn );
}
