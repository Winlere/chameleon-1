/**
 *
 * @file core_zplghe.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2023 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon core_zplghe CPU kernel
 *
 * @version 1.2.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 0.9.2
 * @author Piotr Luszczek
 * @author Pierre Lemarinier
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Florent Pruvost
 * @author Guillaume Sylvand
 * @date 2022-02-22
 * @precisions normal z -> c
 *
 */
#include "coreblas.h"
#include "coreblas/random.h"

#if defined(PRECISION_z) || defined(PRECISION_c)
#define NBELEM   2
#else
#define NBELEM   1
#endif

//  CORE_zplghe - Generate a tile for random hermitian (positive definite if bump is large enough) matrix.
void CORE_zplghe( double bump, int m, int n, CHAMELEON_Complex64_t *A, int lda,
                  int bigM, int m0, int n0, unsigned long long int seed )
{
    CHAMELEON_Complex64_t *tmp = A;
    int64_t i, j;
    unsigned long long int ran, jump;

    jump = (unsigned long long int)m0 + (unsigned long long int)n0 * (unsigned long long int)bigM;

    /*
     * Tile diagonal
     */
    if ( m0 == n0 ) {
        int minmn = chameleon_min( m, n );

        /* Lower part */
        for (j = 0; j < minmn; j++) {
            ran = CORE_rnd64_jump( NBELEM * jump, seed );

            *tmp = CORE_zlaran( &ran );
            *tmp = creal( *tmp + bump ); /* Take only the real part */
            tmp++;

            for (i = j+1; i < m; i++) {
                *tmp = CORE_zlaran( &ran );
                tmp++;
            }
            tmp  += (lda - i + j + 1);
            jump += bigM + 1;
        }

        /* Upper part */
        jump = (unsigned long long int)m0 + (unsigned long long int)n0 * (unsigned long long int)bigM;

        for (i = 0; i < minmn; i++) {
            ran = CORE_rnd64_jump( NBELEM * (jump+i+1), seed );

            for (j = i+1; j < n; j++) {
                A[j*lda+i] = conj( CORE_zlaran( &ran ) );
            }
            jump += bigM;
        }
    }
    /*
     * Lower part
     */
    else if ( m0 > n0 ) {
        for (j = 0; j < n; j++) {
            ran = CORE_rnd64_jump( NBELEM * jump, seed );

            for (i = 0; i < m; i++) {
                *tmp = CORE_zlaran( &ran );
                tmp++;
            }
            tmp  += (lda - i);
            jump += bigM;
        }
    }
    /*
     * Upper part
     */
    else if ( m0 < n0 ) {
        /* Overwrite jump */
        jump = (unsigned long long int)n0 + (unsigned long long int)m0 * (unsigned long long int)bigM;

        for (i = 0; i < m; i++) {
            ran = CORE_rnd64_jump( NBELEM * jump, seed );

            for (j = 0; j < n; j++) {
                A[j*lda+i] = conj( CORE_zlaran( &ran ) );
            }
            jump += bigM;
        }
    }
}
