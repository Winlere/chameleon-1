/**
 *
 * @file pzhemm.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zhemm parallel algorithm
 *
 * @version 1.2.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 0.9.2
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Florent Pruvost
 * @author Alycia Lisito
 * @date 2022-02-22
 * @precisions normal z -> c
 *
 */
#include "control/common.h"

#define A(  _m_, _n_ ) A,  (_m_), (_n_)
#define B(  _m_, _n_ ) B,  (_m_), (_n_)
#define C(  _m_, _n_ ) C,  (_m_), (_n_)
#define WA( _m_, _n_ ) WA, (_m_), (_n_)
#define WB( _m_, _n_ ) WB, (_m_), (_n_)

/**
 *  Parallel tile matrix-matrix multiplication.
 *  Generic algorithm for any data distribution with a stationnary A.
 *
 * Assuming A has been setup with a proper getrank function to account for symmetry
 */
static inline void
chameleon_pzhemm_Astat( CHAM_context_t *chamctxt, cham_side_t side, cham_uplo_t uplo,
                        CHAMELEON_Complex64_t alpha, CHAM_desc_t *A, CHAM_desc_t *B,
                        CHAMELEON_Complex64_t beta,  CHAM_desc_t *C,
                        RUNTIME_option_t *options )
{
    const CHAMELEON_Complex64_t zone = (CHAMELEON_Complex64_t)1.0;
    RUNTIME_sequence_t *sequence = options->sequence;
    int                 k, m, n, l, Am, An;
    int                 tempmm, tempnn, tempkn, tempkm;
    int                 myrank = RUNTIME_comm_rank( chamctxt );
    int                 reduceC[ C->mt * C->nt ];

    /* Set C tiles to redux mode */
    for (n = 0; n < C->nt; n++) {
        for (m = 0; m < C->mt; m++) {
            reduceC[ n * C->mt + m ] = 0;

            /* The node owns the C tile. */
            if ( C->get_rankof( C(m, n) ) == myrank ) {
                reduceC[ n * C->mt + m ] = 1;
                RUNTIME_zgersum_set_methods( C(m, n) );
                continue;
            }

            /*
             * The node owns the A tile that will define the locality of the
             * computations.
             */
            /* Select row or column based on side */
            l = ( side == ChamLeft ) ? m : n;

            if ( uplo == ChamLower ) {
                for (k = 0; k < A->mt; k++) {
                    Am = k;
                    An = k;

                    if (k < l) {
                        Am = l;
                    }
                    else if (k > l) {
                        An = l;
                    }

                    if ( A->get_rankof( A( Am, An ) ) == myrank ) {
                        reduceC[ n * C->mt + m ] = 1;
                        RUNTIME_zgersum_set_methods( C(m, n) );
                        break;
                    }
                }
            }
            else {
                for (k = 0; k < A->mt; k++) {
                    Am = k;
                    An = k;

                    if (k < l) {
                        An = l;
                    }
                    else if (k > l) {
                        Am = l;
                    }

                    if ( A->get_rankof( A( Am, An ) ) == myrank ) {
                        reduceC[ n * C->mt + m ] = 1;
                        RUNTIME_zgersum_set_methods( C(m, n) );
                        break;
                    }
                }
            }
        }
    }

    for(n = 0; n < C->nt; n++) {
        tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;
        for(m = 0; m < C->mt; m++) {
            tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;

            /* Scale C */
            options->forcesub = 0;
            INSERT_TASK_zlascal( options, ChamUpperLower, tempmm, tempnn, C->mb,
                                 beta, C, m, n );
            options->forcesub = reduceC[ n * C->mt + m ];

            /*
             *  ChamLeft / ChamLower
             */
            /* Select row or column based on side */
            l = ( side == ChamLeft ) ? m : n;

            if (side == ChamLeft) {
                if (uplo == ChamLower) {
                    for (k = 0; k < C->mt; k++) {
                        tempkm = k == C->mt-1 ? C->m-k*C->mb : C->mb;

                        if (k < m) {
                            INSERT_TASK_zgemm_Astat(
                                options,
                                ChamNoTrans, ChamNoTrans,
                                tempmm, tempnn, tempkm, A->mb,
                                alpha, A(m, k),  /* lda * K */
                                       B(k, n),  /* ldb * Y */
                                zone,  C(m, n)); /* ldc * Y */
                        }
                        else if (k == m) {
                                INSERT_TASK_zhemm_Astat(
                                    options,
                                    side, uplo,
                                    tempmm, tempnn, A->mb,
                                    alpha, A(k, k),  /* ldak * X */
                                           B(k, n),  /* ldb  * Y */
                                    zone,  C(m, n)); /* ldc  * Y */
                        }
                        else {
                            INSERT_TASK_zgemm_Astat(
                                options,
                                ChamConjTrans, ChamNoTrans,
                                tempmm, tempnn, tempkm, A->mb,
                                alpha, A(k, m),  /* ldak * X */
                                       B(k, n),  /* ldb  * Y */
                                zone,  C(m, n)); /* ldc  * Y */
                        }
                    }
                }
                /*
                 *  ChamLeft / ChamUpper
                 */
                else {
                    for (k = 0; k < C->mt; k++) {
                        tempkm = k == C->mt-1 ? C->m-k*C->mb : C->mb;

                        if (k < m) {
                            INSERT_TASK_zgemm_Astat(
                                options,
                                ChamConjTrans, ChamNoTrans,
                                tempmm, tempnn, tempkm, A->mb,
                                alpha, A(k, m),  /* ldak * X */
                                       B(k, n),  /* ldb  * Y */
                                zone,  C(m, n)); /* ldc  * Y */
                        }
                        else if (k == m) {
                            INSERT_TASK_zhemm_Astat(
                                options,
                                side, uplo,
                                tempmm, tempnn, A->mb,
                                alpha, A(k, k),  /* ldak * K */
                                       B(k, n),  /* ldb  * Y */
                                zone,  C(m, n)); /* ldc  * Y */
                        }
                        else {
                            INSERT_TASK_zgemm_Astat(
                                options,
                                ChamNoTrans, ChamNoTrans,
                                tempmm, tempnn, tempkm, A->mb,
                                alpha, A(m, k),  /* lda * K */
                                       B(k, n),  /* ldb * Y */
                                zone,  C(m, n)); /* ldc * Y */
                        }
                    }
                }
            }
            /*
             *  ChamRight / ChamLower
             */
            else {
                if (uplo == ChamLower) {
                    for (k = 0; k < C->nt; k++) {
                        tempkn = k == C->nt-1 ? C->n-k*C->nb : C->nb;

                        if (k < n) {
                            INSERT_TASK_zgemm_Astat(
                                options,
                                ChamNoTrans, ChamConjTrans,
                                tempmm, tempnn, tempkn, A->mb,
                                alpha, B(m, k),  /* ldb * K */
                                       A(n, k),  /* lda * K */
                                zone,  C(m, n)); /* ldc * Y */
                        }
                        else if (k == n) {
                            INSERT_TASK_zhemm_Astat(
                                options,
                                side, uplo,
                                tempmm, tempnn, A->mb,
                                alpha, A(k, k),  /* ldak * Y */
                                       B(m, k),  /* ldb  * Y */
                                zone,  C(m, n)); /* ldc  * Y */
                        }
                        else {
                            INSERT_TASK_zgemm_Astat(
                                options,
                                ChamNoTrans, ChamNoTrans,
                                tempmm, tempnn, tempkn, A->mb,
                                alpha, B(m, k),  /* ldb  * K */
                                       A(k, n),  /* ldak * Y */
                                zone,  C(m, n)); /* ldc  * Y */
                        }
                    }
                }
                /*
                 *  ChamRight / ChamUpper
                 */
                else {
                    for (k = 0; k < C->nt; k++) {
                        tempkn = k == C->nt-1 ? C->n-k*C->nb : C->nb;

                        if (k < n) {
                            INSERT_TASK_zgemm_Astat(
                                options,
                                ChamNoTrans, ChamNoTrans,
                                tempmm, tempnn, tempkn, A->mb,
                                alpha, B(m, k),  /* ldb  * K */
                                       A(k, n),  /* ldak * Y */
                                zone,  C(m, n)); /* ldc  * Y */
                        }
                        else if (k == n) {
                            INSERT_TASK_zhemm_Astat(
                                options,
                                side, uplo,
                                tempmm, tempnn, A->mb,
                                alpha, A(k, k),  /* ldak * Y */
                                       B(m, k),  /* ldb  * Y */
                                zone,  C(m, n)); /* ldc  * Y */
                        }
                        else {
                            INSERT_TASK_zgemm_Astat(
                                options,
                                ChamNoTrans, ChamConjTrans,
                                tempmm, tempnn, tempkn, A->mb,
                                alpha, B(m, k),  /* ldb * K */
                                       A(n, k),  /* lda * K */
                                zone,  C(m, n)); /* ldc * Y */
                        }
                    }
                }
            }

            RUNTIME_zgersum_submit_tree( options, C(m, n) );
            RUNTIME_data_flush( sequence, C(m, n) );
        }
    }
    options->forcesub = 0;
    (void)chamctxt;
}


/**
 *  Parallel tile hermitian matrix-matrix multiplication.
 *  SUMMA algorithm for 2D block-cyclic data distribution.
 */
static inline void
chameleon_pzhemm_summa_left( CHAM_context_t *chamctxt, cham_uplo_t uplo,
                             CHAMELEON_Complex64_t alpha, CHAM_desc_t *A, CHAM_desc_t *B,
                             CHAMELEON_Complex64_t beta,  CHAM_desc_t *C,
                             CHAM_desc_t *WA, CHAM_desc_t *WB,
                             RUNTIME_option_t *options )
{
    RUNTIME_sequence_t *sequence = options->sequence;
    cham_trans_t transA;
    int m, n, k, p, q, KT, K, lp, lq;
    int tempmm, tempnn, tempkk;
    int lookahead, myp, myq;

    CHAMELEON_Complex64_t zbeta;
    CHAMELEON_Complex64_t zone = (CHAMELEON_Complex64_t)1.0;

    lookahead = chamctxt->lookahead;
    KT  = A->nt;
    K   = A->n;
    myp = C->myrank / C->q;
    myq = C->myrank % C->q;

    for (k = 0; k < KT; k++ ) {
        lp = (k % lookahead) * C->p;
        lq = (k % lookahead) * C->q;
        tempkk = k == KT - 1 ? K - k * A->nb : A->nb;
        zbeta = k == 0 ? beta : zone;

        /* Transfert ownership of the k column of A or B */
        for (m = 0; m < C->mt; m ++ ) {
            int Am, Ak;
            int tempam, tempak;

            tempmm = m == C->mt-1 ? C->m - m * C->mb : C->mb;

            if ( (( uplo == ChamUpper ) && ( m > k )) ||
                 (( uplo == ChamLower ) && ( m < k )) )
            {
                    /* Let's take A( k, m ) */
                Am = k;
                Ak = m;
                tempam = tempkk;
                tempak = tempmm;
            }
            else {
                /* Let's take A( m, k ) */
                Am = m;
                Ak = k;
                tempam = tempmm;
                tempak = tempkk;
            }

            INSERT_TASK_zlacpy(
                options,
                ChamUpperLower, tempam, tempak,
                A( Am, Ak ),
                WA( m, (k % C->q) + lq ) );

            RUNTIME_data_flush( sequence, A( Am, Ak ) );

            for ( q=1; q < C->q; q++ ) {
                INSERT_TASK_zlacpy(
                    options,
                    ChamUpperLower, tempam, tempak,
                    WA( m, ((k+q-1) % C->q) + lq ),
                    WA( m, ((k+q)   % C->q) + lq ) );
            }
        }

        /* Transfert ownership of the k row of B, or A */
        for (n = 0; n < C->nt; n++) {

            tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;

            INSERT_TASK_zlacpy(
                options,
                ChamUpperLower, tempkk, tempnn,
                B(   k,              n ),
                WB( (k % C->p) + lp, n ) );

            RUNTIME_data_flush( sequence, B( k, n ) );

            for ( p=1; p < C->p; p++ ) {
                INSERT_TASK_zlacpy(
                    options,
                    ChamUpperLower, tempkk, tempnn,
                    WB( ((k+p-1) % C->p) + lp, n ),
                    WB( ((k+p)   % C->p) + lp, n ) );
            }
        }

        /* Perform the update of this iteration */
        for (m = myp; m < C->mt; m+=C->p) {
            tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;

            if ( k == m ) {
                for (n = myq; n < C->nt; n+=C->q) {
                    tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;

                    INSERT_TASK_zhemm(
                        options, ChamLeft, uplo,
                        tempmm, tempnn, A->mb,
                        alpha, WA( m,        myq + lq ),
                               WB( myp + lp, n        ),
                        zbeta, C(  m,        n        ) );
                }
            }
            else {
                if ( (( uplo == ChamUpper ) && ( m > k )) ||
                     (( uplo == ChamLower ) && ( m < k )) )
                {
                    transA = ChamConjTrans;
                }
                else {
                    transA = ChamNoTrans;
                }

                for (n = myq; n < C->nt; n+=C->q) {
                    tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;

                    INSERT_TASK_zgemm(
                        options, transA, ChamNoTrans,
                        tempmm, tempnn, tempkk, A->mb,
                        alpha, WA( m,        myq + lq ),
                               WB( myp + lp, n        ),
                        zbeta, C(  m,        n        ) );
                }
            }
        }
    }
}

/**
 *  Parallel tile hermitian matrix-matrix multiplication.
 *  SUMMA algorithm for 2D block-cyclic data distribution.
 */
static inline void
chameleon_pzhemm_summa_right( CHAM_context_t *chamctxt, cham_uplo_t uplo,
                              CHAMELEON_Complex64_t alpha, CHAM_desc_t *A, CHAM_desc_t *B,
                              CHAMELEON_Complex64_t beta,  CHAM_desc_t *C,
                              CHAM_desc_t *WA, CHAM_desc_t *WB,
                              RUNTIME_option_t *options )
{
    RUNTIME_sequence_t *sequence = options->sequence;
    cham_trans_t transA;
    int m, n, k, p, q, KT, K, lp, lq;
    int tempmm, tempnn, tempkk;
    int lookahead, myp, myq;

    CHAMELEON_Complex64_t zbeta;
    CHAMELEON_Complex64_t zone = (CHAMELEON_Complex64_t)1.0;

    lookahead = chamctxt->lookahead;
    KT  = A->mt;
    K   = A->m;
    myp = C->myrank / C->q;
    myq = C->myrank % C->q;

    for (k = 0; k < KT; k++ ) {
        lp = (k % lookahead) * C->p;
        lq = (k % lookahead) * C->q;
        tempkk = k == KT - 1 ? K - k * A->nb : A->nb;
        zbeta = k == 0 ? beta : zone;

        /* Transfert ownership of the k column of A or B */
        for (m = 0; m < C->mt; m++ ) {

            tempmm = m == C->mt-1 ? C->m - m * C->mb : C->mb;

            INSERT_TASK_zlacpy(
                options,
                ChamUpperLower, tempmm, tempkk,
                B(  m,  k ),
                WA( m, (k % C->q) + lq ) );

            RUNTIME_data_flush( sequence, B( m, k ) );

            for ( q=1; q < C->q; q++ ) {
                INSERT_TASK_zlacpy(
                    options,
                    ChamUpperLower, tempmm, tempkk,
                    WA( m, ((k+q-1) % C->q) + lq ),
                    WA( m, ((k+q)   % C->q) + lq ) );
            }
        }

        /* Transfert ownership of the k row of B, or A */
        for (n = 0; n < C->nt; n++) {
            int Ak, An;
            int tempak, tempan;

            tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;

            if ( (( uplo == ChamUpper ) && ( n < k )) ||
                 (( uplo == ChamLower ) && ( n > k )) )
            {
                Ak = n;
                An = k;
                tempak = tempnn;
                tempan = tempkk;
            }
            else
            {
                Ak = k;
                An = n;
                tempak = tempkk;
                tempan = tempnn;
            }

            INSERT_TASK_zlacpy(
                options,
                ChamUpperLower, tempak, tempan,
                A(  Ak,              An ),
                WB( (k % C->p) + lp, n  ) );

            RUNTIME_data_flush( sequence, A( Ak, An ) );

            for ( p=1; p < C->p; p++ ) {
                INSERT_TASK_zlacpy(
                    options,
                    ChamUpperLower, tempak, tempan,
                    WB( ((k+p-1) % C->p) + lp, n ),
                    WB( ((k+p)   % C->p) + lp, n ) );
            }
        }

        /* Perform the update of this iteration */
        for (n = myq; n < C->nt; n+=C->q) {
            tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;

            if ( k == n ) {
                for (m = myp; m < C->mt; m+=C->p) {
                    tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;

                    /* A has been stored in WA or WB for the summa ring */
                    INSERT_TASK_zhemm(
                        options, ChamRight, uplo,
                        tempmm, tempnn, A->mb,
                        alpha, WB( myp + lp, n        ),
                               WA( m,        myq + lq ),
                        zbeta, C(  m,        n        ) );
                }
            }
            else {
                if ( (( uplo == ChamUpper ) && ( n < k )) ||
                     (( uplo == ChamLower ) && ( n > k )) )
                {
                    transA = ChamConjTrans;
                }
                else {
                    transA = ChamNoTrans;
                }

                for (m = myp; m < C->mt; m+=C->p) {
                    tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;

                    INSERT_TASK_zgemm(
                        options, ChamNoTrans, transA,
                        tempmm, tempnn, tempkk, A->mb,
                        alpha, WA( m,        myq + lq ),
                               WB( myp + lp, n        ),
                        zbeta, C(  m,        n        ) );
                }
            }
        }
    }
}

/**
 *  Parallel tile hermitian matrix-matrix multiplication.
 *  SUMMA algorithm for 2D block-cyclic data distribution.
 */
static inline void
chameleon_pzhemm_summa( CHAM_context_t *chamctxt, cham_side_t side, cham_uplo_t uplo,
                        CHAMELEON_Complex64_t alpha, CHAM_desc_t *A, CHAM_desc_t *B,
                        CHAMELEON_Complex64_t beta,  CHAM_desc_t *C,
                        CHAM_desc_t *WA, CHAM_desc_t *WB,
                        RUNTIME_option_t *options )
{
    RUNTIME_sequence_t *sequence = options->sequence;

    if (side == ChamLeft) {
        chameleon_pzhemm_summa_left( chamctxt, uplo, alpha, A, B, beta, C,
                                     WA, WB, options );
    }
    else {
        chameleon_pzhemm_summa_right( chamctxt, uplo, alpha, A, B, beta, C,
                                      WA, WB, options );
    }

    CHAMELEON_Desc_Flush( WA, sequence );
    CHAMELEON_Desc_Flush( WB, sequence );
}

/**
 *  Parallel tile hermitian matrix-matrix multiplication.
 *  Generic algorithm for any data distribution.
 */
static inline void
chameleon_pzhemm_generic( CHAM_context_t *chamctxt, cham_side_t side, cham_uplo_t uplo,
                          CHAMELEON_Complex64_t alpha, CHAM_desc_t *A, CHAM_desc_t *B,
                          CHAMELEON_Complex64_t beta,  CHAM_desc_t *C,
                          RUNTIME_option_t *options )
{
    int k, m, n;
    int tempmm, tempnn, tempkn, tempkm;

    CHAMELEON_Complex64_t zbeta;
    CHAMELEON_Complex64_t zone = (CHAMELEON_Complex64_t)1.0;

    for(m = 0; m < C->mt; m++) {
        tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;
        for(n = 0; n < C->nt; n++) {
            tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;
            /*
             *  ChamLeft / ChamLower
             */
            if (side == ChamLeft) {
                if (uplo == ChamLower) {
                    for (k = 0; k < C->mt; k++) {
                        tempkm = k == C->mt-1 ? C->m-k*C->mb : C->mb;
                        zbeta = k == 0 ? beta : zone;
                        if (k < m) {
                            INSERT_TASK_zgemm(
                                options,
                                ChamNoTrans, ChamNoTrans,
                                tempmm, tempnn, tempkm, A->mb,
                                alpha, A(m, k),  /* lda * K */
                                       B(k, n),  /* ldb * Y */
                                zbeta, C(m, n)); /* ldc * Y */
                        }
                        else {
                            if (k == m) {
                                INSERT_TASK_zhemm(
                                    options,
                                    side, uplo,
                                    tempmm, tempnn, A->mb,
                                    alpha, A(k, k),  /* ldak * X */
                                           B(k, n),  /* ldb  * Y */
                                    zbeta, C(m, n)); /* ldc  * Y */
                            }
                            else {
                                INSERT_TASK_zgemm(
                                    options,
                                    ChamConjTrans, ChamNoTrans,
                                    tempmm, tempnn, tempkm, A->mb,
                                    alpha, A(k, m),  /* ldak * X */
                                           B(k, n),  /* ldb  * Y */
                                    zbeta, C(m, n)); /* ldc  * Y */
                            }
                        }
                    }
                }
                /*
                 *  ChamLeft / ChamUpper
                 */
                else {
                    for (k = 0; k < C->mt; k++) {
                        tempkm = k == C->mt-1 ? C->m-k*C->mb : C->mb;
                        zbeta = k == 0 ? beta : zone;
                        if (k < m) {
                            INSERT_TASK_zgemm(
                                options,
                                ChamConjTrans, ChamNoTrans,
                                tempmm, tempnn, tempkm, A->mb,
                                alpha, A(k, m),  /* ldak * X */
                                       B(k, n),  /* ldb  * Y */
                                zbeta, C(m, n)); /* ldc  * Y */
                        }
                        else {
                            if (k == m) {
                                INSERT_TASK_zhemm(
                                    options,
                                    side, uplo,
                                    tempmm, tempnn, A->mb,
                                    alpha, A(k, k),  /* ldak * K */
                                           B(k, n),  /* ldb  * Y */
                                    zbeta, C(m, n)); /* ldc  * Y */
                            }
                            else {
                                INSERT_TASK_zgemm(
                                    options,
                                    ChamNoTrans, ChamNoTrans,
                                    tempmm, tempnn, tempkm, A->mb,
                                    alpha, A(m, k),  /* lda * K */
                                           B(k, n),  /* ldb * Y */
                                    zbeta, C(m, n)); /* ldc * Y */
                            }
                        }
                    }
                }
            }
            /*
             *  ChamRight / ChamLower
             */
            else {
                if (uplo == ChamLower) {
                    for (k = 0; k < C->nt; k++) {
                        tempkn = k == C->nt-1 ? C->n-k*C->nb : C->nb;
                        zbeta = k == 0 ? beta : zone;
                        if (k < n) {
                            INSERT_TASK_zgemm(
                                options,
                                ChamNoTrans, ChamConjTrans,
                                tempmm, tempnn, tempkn, A->mb,
                                alpha, B(m, k),  /* ldb * K */
                                       A(n, k),  /* lda * K */
                                zbeta, C(m, n)); /* ldc * Y */
                        }
                        else {
                            if (k == n) {
                                INSERT_TASK_zhemm(
                                    options,
                                    side, uplo,
                                    tempmm, tempnn, A->mb,
                                    alpha, A(k, k),  /* ldak * Y */
                                           B(m, k),  /* ldb  * Y */
                                    zbeta, C(m, n)); /* ldc  * Y */
                            }
                            else {
                                INSERT_TASK_zgemm(
                                    options,
                                    ChamNoTrans, ChamNoTrans,
                                    tempmm, tempnn, tempkn, A->mb,
                                    alpha, B(m, k),  /* ldb  * K */
                                           A(k, n),  /* ldak * Y */
                                    zbeta, C(m, n)); /* ldc  * Y */
                            }
                        }
                    }
                }
                /*
                 *  ChamRight / ChamUpper
                 */
                else {
                    for (k = 0; k < C->nt; k++) {
                        tempkn = k == C->nt-1 ? C->n-k*C->nb : C->nb;
                        zbeta = k == 0 ? beta : zone;
                        if (k < n) {
                            INSERT_TASK_zgemm(
                                options,
                                ChamNoTrans, ChamNoTrans,
                                tempmm, tempnn, tempkn, A->mb,
                                alpha, B(m, k),  /* ldb  * K */
                                       A(k, n),  /* ldak * Y */
                                zbeta, C(m, n)); /* ldc  * Y */
                        }
                        else {
                            if (k == n) {
                                INSERT_TASK_zhemm(
                                    options,
                                    side, uplo,
                                    tempmm, tempnn, A->mb,
                                    alpha, A(k, k),  /* ldak * Y */
                                           B(m, k),  /* ldb  * Y */
                                    zbeta, C(m, n)); /* ldc  * Y */
                            }
                            else {
                                INSERT_TASK_zgemm(
                                    options,
                                    ChamNoTrans, ChamConjTrans,
                                    tempmm, tempnn, tempkn, A->mb,
                                    alpha, B(m, k),  /* ldb * K */
                                           A(n, k),  /* lda * K */
                                    zbeta, C(m, n)); /* ldc * Y */
                            }
                        }
                    }
                }
            }
        }
    }
    (void)chamctxt;
}

/**
 *  Parallel tile hermitian matrix-matrix multiplication. wrapper.
 */
void
chameleon_pzhemm( struct chameleon_pzgemm_s *ws,
                  cham_side_t side, cham_uplo_t uplo,
                  CHAMELEON_Complex64_t alpha, CHAM_desc_t *A, CHAM_desc_t *B,
                  CHAMELEON_Complex64_t beta,  CHAM_desc_t *C,
                  RUNTIME_sequence_t *sequence, RUNTIME_request_t *request )
{
    CHAM_context_t *chamctxt;
    RUNTIME_option_t options;
    cham_gemm_t alg = (ws != NULL) ? ws->alg : ChamGemmAlgGeneric;

    chamctxt = chameleon_context_self();
    if (sequence->status != CHAMELEON_SUCCESS) {
        return;
    }
    RUNTIME_options_init( &options, chamctxt, sequence, request );

    switch( alg ) {
    case ChamGemmAlgAuto:
    case ChamGemmAlgSummaB: /* Switch back to generic since it does not exist yet. */
    case ChamGemmAlgGeneric:
        chameleon_pzhemm_generic( chamctxt, side, uplo, alpha, A, B, beta, C, &options );
        break;

    case ChamGemmAlgSummaC:
        chameleon_pzhemm_summa( chamctxt, side, uplo, alpha, A, B, beta, C,
                                &(ws->WA), &(ws->WB), &options );
        break;

    case ChamGemmAlgSummaA:
        if ( side == ChamLeft ) {
            chameleon_pzhemm_Astat( chamctxt, side, uplo, alpha, A, B, beta, C, &options );
        }
        else {
            chameleon_pzhemm_generic( chamctxt, side, uplo, alpha, A, B, beta, C, &options );
        }
        break;
    }

    RUNTIME_options_finalize( &options, chamctxt );
}
