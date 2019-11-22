/**
 *
 * @file openmp/codelet_ztplqt.c
 *
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon ztplqt OpenMP codelet
 *
 * @version 0.9.2
 * @author Philippe Virouleau
 * @author Mathieu Faverge
 * @date 2019-11-19
 * @precisions normal z -> s d c
 *
 */
#include "chameleon_openmp.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_ztile.h"

void INSERT_TASK_ztplqt( const RUNTIME_option_t *options,
                         int M, int N, int L, int ib, int nb,
                         const CHAM_desc_t *A, int Am, int An,
                         const CHAM_desc_t *B, int Bm, int Bn,
                         const CHAM_desc_t *T, int Tm, int Tn )
{
    CHAM_tile_t *tileA = A->get_blktile( A, Am, An );
    CHAM_tile_t *tileB = B->get_blktile( B, Bm, Bn );
    CHAM_tile_t *tileT = T->get_blktile( T, Tm, Tn );
    int ws_size = options->ws_wsize;

#pragma omp task firstprivate( ws_size, M, N, L, ib, tileA, tileB, tileT ) depend( inout:tileA[0], tileB[0] ) depend( out:tileT[0] )
    {
        CHAMELEON_Complex64_t work[ws_size];

        TCORE_zlaset( ChamUpperLower, ib, M, 0., 0., tileT );
        TCORE_ztplqt( M, N, L, ib,
                      tileA, tileB, tileT, work );
    }
}
