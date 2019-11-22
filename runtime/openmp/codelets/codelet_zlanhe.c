/**
 *
 * @file openmp/codelet_zlanhe.c
 *
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlanhe OpenMP codelet
 *
 * @version 0.9.2
 * @author Philippe Virouleau
 * @author Mathieu Faverge
 * @date 2019-11-19
 * @precisions normal z -> c
 *
 */
#include "chameleon_openmp.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_ztile.h"

void INSERT_TASK_zlanhe( const RUNTIME_option_t *options,
                         cham_normtype_t norm, cham_uplo_t uplo, int N, int NB,
                         const CHAM_desc_t *A, int Am, int An,
                         const CHAM_desc_t *B, int Bm, int Bn )
{
    CHAM_tile_t *tileA = A->get_blktile( A, Am, An );
    CHAM_tile_t *tileB = B->get_blktile( B, Bm, Bn );
    int ws_size = options->ws_wsize;

#pragma omp task firstprivate( ws_size, norm, uplo, N, tileA, tileB ) depend( in:tileA[0] ) depend( inout:tileB[0] )
    {
        double work[ws_size];
        TCORE_zlanhe( norm, uplo, N, tileA, work, tileB->mat );
    }
}
