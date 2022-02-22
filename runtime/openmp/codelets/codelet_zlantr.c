/**
 *
 * @file openmp/codelet_zlantr.c
 *
 * @copyright 2012-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlantr OpenMP codelet
 *
 * @version 1.2.0
 * @author Philippe Virouleau
 * @author Mathieu Faverge
 * @date 2022-02-22
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_openmp.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_ztile.h"

void INSERT_TASK_zlantr( const RUNTIME_option_t *options,
                         cham_normtype_t norm, cham_uplo_t uplo, cham_diag_t diag,
                         int m, int n, int nb,
                         const CHAM_desc_t *A, int Am, int An,
                         const CHAM_desc_t *B, int Bm, int Bn )
{
    CHAM_tile_t *tileA = A->get_blktile( A, Am, An );
    CHAM_tile_t *tileB = B->get_blktile( B, Bm, Bn );
    int ws_wsize = options->ws_wsize;
#pragma omp task firstprivate( ws_wsize, norm, uplo, diag, m, n, tileA, tileB ) depend( in:tileA[0] ) depend( inout:tileB[0] )
    {
        double work[ws_wsize];
        TCORE_zlantr( norm, uplo, diag, m, n, tileA, work, tileB->mat );
    }

    (void)options;
    (void)nb;
}
