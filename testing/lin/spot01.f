!!!
!
! -- Inria
! -- (C) Copyright 2012
!
! This software is a computer program whose purpose is to process
! Matrices Over Runtime Systems @ Exascale (CHAMELEON). More information
! can be found on the following website: http://www.inria.fr/en/teams/morse.
! 
! This software is governed by the CeCILL-B license under French law and
! abiding by the rules of distribution of free software.  You can  use, 
! modify and/ or redistribute the software under the terms of the CeCILL-B
! license as circulated by CEA, CNRS and INRIA at the following URL
! "http://www.cecill.info". 
! 
! As a counterpart to the access to the source code and  rights to copy,
! modify and redistribute granted by the license, users are provided only
! with a limited warranty  and the software's author,  the holder of the
! economic rights,  and the successive licensors  have only  limited
! liability. 
! 
! In this respect, the user's attention is drawn to the risks associated
! with loading,  using,  modifying and/or developing or reproducing the
! software by the user in light of its specific status of free software,
! that may mean  that it is complicated to manipulate,  and  that  also
! therefore means  that it is reserved for developers  and  experienced
! professionals having in-depth computer knowledge. Users are therefore
! encouraged to load and test the software's suitability as regards their
! requirements in conditions enabling the security of their systems and/or 
! data to be ensured and,  more generally, to use and operate it in the 
! same conditions as regards security. 
! 
! The fact that you are presently reading this means that you have had
! knowledge of the CeCILL-B license and that you accept its terms.
!
!!!

      SUBROUTINE SPOT01( UPLO, N, A, LDA, AFAC, LDAFAC, RWORK, RESID )
*
*  -- LAPACK test routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      CHARACTER          UPLO
      INTEGER            LDA, LDAFAC, N
      REAL               RESID
*     ..
*     .. Array Arguments ..
      REAL               A( LDA, * ), AFAC( LDAFAC, * ), RWORK( * )
*     ..
*
*  Purpose
*  =======
*
*  SPOT01 reconstructs a symmetric positive definite matrix  A  from
*  its L*L' or U'*U factorization and computes the residual
*     norm( L*L' - A ) / ( N * norm(A) * EPS ) or
*     norm( U'*U - A ) / ( N * norm(A) * EPS ),
*  where EPS is the machine epsilon.
*
*  Arguments
*  ==========
*
*  UPLO    (input) CHARACTER*1
*          Specifies whether the upper or lower triangular part of the
*          symmetric matrix A is stored:
*          = 'U':  Upper triangular
*          = 'L':  Lower triangular
*
*  N       (input) INTEGER
*          The number of rows and columns of the matrix A.  N >= 0.
*
*  A       (input) REAL array, dimension (LDA,N)
*          The original symmetric matrix A.
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A.  LDA >= max(1,N)
*
*  AFAC    (input/output) REAL array, dimension (LDAFAC,N)
*          On entry, the factor L or U from the L*L' or U'*U
*          factorization of A.
*          Overwritten with the reconstructed matrix, and then with the
*          difference L*L' - A (or U'*U - A).
*
*  LDAFAC  (input) INTEGER
*          The leading dimension of the array AFAC.  LDAFAC >= max(1,N).
*
*  RWORK   (workspace) REAL array, dimension (N)
*
*  RESID   (output) REAL
*          If UPLO = 'L', norm(L*L' - A) / ( N * norm(A) * EPS )
*          If UPLO = 'U', norm(U'*U - A) / ( N * norm(A) * EPS )
*
*  =====================================================================
*
*     .. Parameters ..
      REAL               ZERO, ONE
      PARAMETER          ( ZERO = 0.0E+0, ONE = 1.0E+0 )
*     ..
*     .. Local Scalars ..
      INTEGER            I, J, K
      REAL               ANORM, EPS, T
*     ..
*     .. External Functions ..
      LOGICAL            LSAME
      REAL               SDOT, SLAMCH, SLANSY
      EXTERNAL           LSAME, SDOT, SLAMCH, SLANSY
*     ..
*     .. External Subroutines ..
      EXTERNAL           SSCAL, SSYR, STRMV
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          REAL
*     ..
*     .. Executable Statements ..
*
*     Quick exit if N = 0.
*
      IF( N.LE.0 ) THEN
         RESID = ZERO
         RETURN
      END IF
*
*     Exit with RESID = 1/EPS if ANORM = 0.
*
      EPS = SLAMCH( 'Epsilon' )
      ANORM = SLANSY( '1', UPLO, N, A, LDA, RWORK )
      IF( ANORM.LE.ZERO ) THEN
         RESID = ONE / EPS
         RETURN
      END IF
*
*     Compute the product U'*U, overwriting U.
*
      IF( LSAME( UPLO, 'U' ) ) THEN
         DO 10 K = N, 1, -1
*
*           Compute the (K,K) element of the result.
*
            T = SDOT( K, AFAC( 1, K ), 1, AFAC( 1, K ), 1 )
            AFAC( K, K ) = T
*
*           Compute the rest of column K.
*
            CALL STRMV( 'Upper', 'Transpose', 'Non-unit', K-1, AFAC,
     $                  LDAFAC, AFAC( 1, K ), 1 )
*
   10    CONTINUE
*
*     Compute the product L*L', overwriting L.
*
      ELSE
         DO 20 K = N, 1, -1
*
*           Add a multiple of column K of the factor L to each of
*           columns K+1 through N.
*
            IF( K+1.LE.N )
     $         CALL SSYR( 'Lower', N-K, ONE, AFAC( K+1, K ), 1,
     $                    AFAC( K+1, K+1 ), LDAFAC )
*
*           Scale column K by the diagonal element.
*
            T = AFAC( K, K )
            CALL SSCAL( N-K+1, T, AFAC( K, K ), 1 )
*
   20    CONTINUE
      END IF
*
*     Compute the difference  L*L' - A (or U'*U - A).
*
      IF( LSAME( UPLO, 'U' ) ) THEN
         DO 40 J = 1, N
            DO 30 I = 1, J
               AFAC( I, J ) = AFAC( I, J ) - A( I, J )
   30       CONTINUE
   40    CONTINUE
      ELSE
         DO 60 J = 1, N
            DO 50 I = J, N
               AFAC( I, J ) = AFAC( I, J ) - A( I, J )
   50       CONTINUE
   60    CONTINUE
      END IF
*
*     Compute norm( L*U - A ) / ( N * norm(A) * EPS )
*
      RESID = SLANSY( '1', UPLO, N, AFAC, LDAFAC, RWORK )
*
      RESID = ( ( RESID / REAL( N ) ) / ANORM ) / EPS
*
      RETURN
*
*     End of SPOT01
*
      END
