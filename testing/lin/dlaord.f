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

      SUBROUTINE DLAORD( JOB, N, X, INCX )
*
*  -- LAPACK auxiliary routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      CHARACTER          JOB
      INTEGER            INCX, N
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION   X( * )
*     ..
*
*  Purpose
*  =======
*
*  DLAORD sorts the elements of a vector x in increasing or decreasing
*  order.
*
*  Arguments
*  =========
*
*  JOB     (input) CHARACTER
*          = 'I':  Sort in increasing order
*          = 'D':  Sort in decreasing order
*
*  N       (input) INTEGER
*          The length of the vector X.
*
*  X       (input/output) DOUBLE PRECISION array, dimension
*                         (1+(N-1)*INCX)
*          On entry, the vector of length n to be sorted.
*          On exit, the vector x is sorted in the prescribed order.
*
*  INCX    (input) INTEGER
*          The spacing between successive elements of X.  INCX >= 0.
*
*  =====================================================================
*
*     .. Local Scalars ..
      INTEGER            I, INC, IX, IXNEXT
      DOUBLE PRECISION   TEMP
*     ..
*     .. External Functions ..
      LOGICAL            LSAME
      EXTERNAL           LSAME
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          ABS
*     ..
*     .. Executable Statements ..
*
      INC = ABS( INCX )
      IF( LSAME( JOB, 'I' ) ) THEN
*
*        Sort in increasing order
*
         DO 20 I = 2, N
            IX = 1 + ( I-1 )*INC
   10       CONTINUE
            IF( IX.EQ.1 )
     $         GO TO 20
            IXNEXT = IX - INC
            IF( X( IX ).GT.X( IXNEXT ) ) THEN
               GO TO 20
            ELSE
               TEMP = X( IX )
               X( IX ) = X( IXNEXT )
               X( IXNEXT ) = TEMP
            END IF
            IX = IXNEXT
            GO TO 10
   20    CONTINUE
*
      ELSE IF( LSAME( JOB, 'D' ) ) THEN
*
*        Sort in decreasing order
*
         DO 40 I = 2, N
            IX = 1 + ( I-1 )*INC
   30       CONTINUE
            IF( IX.EQ.1 )
     $         GO TO 40
            IXNEXT = IX - INC
            IF( X( IX ).LT.X( IXNEXT ) ) THEN
               GO TO 40
            ELSE
               TEMP = X( IX )
               X( IX ) = X( IXNEXT )
               X( IXNEXT ) = TEMP
            END IF
            IX = IXNEXT
            GO TO 30
   40    CONTINUE
      END IF
      RETURN
*
*     End of DLAORD
*
      END
