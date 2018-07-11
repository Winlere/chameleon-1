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

      SUBROUTINE ALAREQ( PATH, NMATS, DOTYPE, NTYPES, NIN, NOUT )
*
*  -- LAPACK test routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      CHARACTER*3        PATH
      INTEGER            NIN, NMATS, NOUT, NTYPES
*     ..
*     .. Array Arguments ..
      LOGICAL            DOTYPE( * )
*     ..
*
*  Purpose
*  =======
*
*  ALAREQ handles input for the LAPACK test program.  It is called
*  to evaluate the input line which requested NMATS matrix types for
*  PATH.  The flow of control is as follows:
*
*  If NMATS = NTYPES then
*     DOTYPE(1:NTYPES) = .TRUE.
*  else
*     Read the next input line for NMATS matrix types
*     Set DOTYPE(I) = .TRUE. for each valid type I
*  endif
*
*  Arguments
*  =========
*
*  PATH    (input) CHARACTER*3
*          An LAPACK path name for testing.
*
*  NMATS   (input) INTEGER
*          The number of matrix types to be used in testing this path.
*
*  DOTYPE  (output) LOGICAL array, dimension (NTYPES)
*          The vector of flags indicating if each type will be tested.
*
*  NTYPES  (input) INTEGER
*          The maximum number of matrix types for this path.
*
*  NIN     (input) INTEGER
*          The unit number for input.  NIN >= 1.
*
*  NOUT    (input) INTEGER
*          The unit number for output.  NOUT >= 1.
*
*  =====================================================================
*
*     .. Local Scalars ..
      LOGICAL            FIRSTT
      CHARACTER          C1
      CHARACTER*10       INTSTR
      CHARACTER*80       LINE
      INTEGER            I, I1, IC, J, K, LENP, NT
*     ..
*     .. Local Arrays ..
      INTEGER            NREQ( 100 )
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          LEN
*     ..
*     .. Data statements ..
      DATA               INTSTR / '0123456789' /
*     ..
*     .. Executable Statements ..
*
      IF( NMATS.GE.NTYPES ) THEN
*
*        Test everything if NMATS >= NTYPES.
*
         DO 10 I = 1, NTYPES
            DOTYPE( I ) = .TRUE.
   10    CONTINUE
      ELSE
         DO 20 I = 1, NTYPES
            DOTYPE( I ) = .FALSE.
   20    CONTINUE
         FIRSTT = .TRUE.
*
*        Read a line of matrix types if 0 < NMATS < NTYPES.
*
         IF( NMATS.GT.0 ) THEN
            READ( NIN, FMT = '(A80)', END = 90 )LINE
            LENP = LEN( LINE )
            I = 0
            DO 60 J = 1, NMATS
               NREQ( J ) = 0
               I1 = 0
   30          CONTINUE
               I = I + 1
               IF( I.GT.LENP ) THEN
                  IF( J.EQ.NMATS .AND. I1.GT.0 ) THEN
                     GO TO 60
                  ELSE
                     WRITE( NOUT, FMT = 9995 )LINE
                     WRITE( NOUT, FMT = 9994 )NMATS
                     GO TO 80
                  END IF
               END IF
               IF( LINE( I: I ).NE.' ' .AND. LINE( I: I ).NE.',' ) THEN
                  I1 = I
                  C1 = LINE( I1: I1 )
*
*              Check that a valid integer was read
*
                  DO 40 K = 1, 10
                     IF( C1.EQ.INTSTR( K: K ) ) THEN
                        IC = K - 1
                        GO TO 50
                     END IF
   40             CONTINUE
                  WRITE( NOUT, FMT = 9996 )I, LINE
                  WRITE( NOUT, FMT = 9994 )NMATS
                  GO TO 80
   50             CONTINUE
                  NREQ( J ) = 10*NREQ( J ) + IC
                  GO TO 30
               ELSE IF( I1.GT.0 ) THEN
                  GO TO 60
               ELSE
                  GO TO 30
               END IF
   60       CONTINUE
         END IF
         DO 70 I = 1, NMATS
            NT = NREQ( I )
            IF( NT.GT.0 .AND. NT.LE.NTYPES ) THEN
               IF( DOTYPE( NT ) ) THEN
                  IF( FIRSTT )
     $               WRITE( NOUT, FMT = * )
                  FIRSTT = .FALSE.
                  WRITE( NOUT, FMT = 9997 )NT, PATH
               END IF
               DOTYPE( NT ) = .TRUE.
            ELSE
               WRITE( NOUT, FMT = 9999 )PATH, NT, NTYPES
 9999          FORMAT( ' *** Invalid type request for ', A3, ', type  ',
     $               I4, ': must satisfy  1 <= type <= ', I2 )
            END IF
   70    CONTINUE
   80    CONTINUE
      END IF
      RETURN
*
   90 CONTINUE
      WRITE( NOUT, FMT = 9998 )PATH
 9998 FORMAT( /' *** End of file reached when trying to read matrix ',
     $      'types for ', A3, /' *** Check that you are requesting the',
     $      ' right number of types for each path', / )
 9997 FORMAT( ' *** Warning:  duplicate request of matrix type ', I2,
     $      ' for ', A3 )
 9996 FORMAT( //' *** Invalid integer value in column ', I2,
     $      ' of input', ' line:', /A79 )
 9995 FORMAT( //' *** Not enough matrix types on input line', /A79 )
 9994 FORMAT( ' ==> Specify ', I4, ' matrix types on this line or ',
     $      'adjust NTYPES on previous line' )
      WRITE( NOUT, FMT = * )
      STOP
*
*     End of ALAREQ
*
      END
