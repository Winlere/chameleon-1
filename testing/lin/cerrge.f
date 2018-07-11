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

      SUBROUTINE CERRGE( PATH, NUNIT )
*
      INCLUDE 'morse_fortran.h'
*
*  -- LAPACK test routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      CHARACTER*3        PATH
      INTEGER            NUNIT
*     ..
*
*  Purpose
*  =======
*
*  CERRGE tests the error exits for the COMPLEX routines
*  for general matrices.
*
*  Arguments
*  =========
*
*  PATH    (input) CHARACTER*3
*          The LAPACK path name for the routines to be tested.
*
*  NUNIT   (input) INTEGER
*          The unit number for output.
*
*  =====================================================================
*
*     .. Parameters ..
      INTEGER            NMAX
      PARAMETER          ( NMAX = 4 )
*     ..
*     .. Local Scalars ..
      CHARACTER*2        C2
      INTEGER            I, INFO, J
      REAL               ANRM, CCOND, RCOND
*     ..
*     .. Local Arrays ..
      INTEGER            IP( NMAX )
      INTEGER            HL( 2 ), HPIV( 2 )
      REAL               R( NMAX ), R1( NMAX ), R2( NMAX )
      COMPLEX            A( NMAX, NMAX ), AF( NMAX, NMAX ), B( NMAX ),
     $                   W( 2*NMAX ), X( NMAX )
*     ..
*     .. External Functions ..
      LOGICAL            LSAMEN
      EXTERNAL           LSAMEN
*     ..
*     .. External Subroutines ..
      EXTERNAL           ALAESM, CGBCON, CGBEQU, CGBRFS, CGBTF2, CGBTRF,
     $                   CGBTRS, CGECON, CGEEQU, CGERFS, CGETF2, CGETRF,
     $                   CGETRI, CGETRS, CHKXER
*     ..
*     .. Scalars in Common ..
      LOGICAL            LERR, OK
      CHARACTER*32       SRNAMT
      INTEGER            INFOT, NOUT
*     ..
*     .. Common blocks ..
      COMMON             / INFOC / INFOT, NOUT, OK, LERR
      COMMON             / SRNAMC / SRNAMT
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          CMPLX, REAL
*     ..
*     .. Executable Statements ..
*
      NOUT = NUNIT
      WRITE( NOUT, FMT = * )
      C2 = PATH( 2: 3 )
*
*     Disable CHAMELEON warnings/errors
* 
      CALL CHAMELEON_DISABLE( CHAMELEON_WARNINGS, INFO )
      CALL CHAMELEON_DISABLE( CHAMELEON_ERRORS,   INFO )
*
*     Set the variables to innocuous values.
*
      DO 20 J = 1, NMAX
         DO 10 I = 1, NMAX
            A( I, J ) = CMPLX( 1. / REAL( I+J ), -1. / REAL( I+J ) )
            AF( I, J ) = CMPLX( 1. / REAL( I+J ), -1. / REAL( I+J ) )
   10    CONTINUE
         B( J ) = 0.
         R1( J ) = 0.
         R2( J ) = 0.
         W( J ) = 0.
         X( J ) = 0.
         IP( J ) = J
   20 CONTINUE
      OK = .TRUE.
*
*     Test error exits of the routines that use the LU decomposition
*     of a general matrix.
*
      IF( LSAMEN( 2, C2, 'GE' ) ) THEN
*
*        ALLOCATE L and IPIV
*
         CALL CHAMELEON_ALLOC_WORKSPACE_CGETRF_INCPIV( 
     $        2, 1, HL, HPIV, INFO )
*
*        CGETRF
*
         SRNAMT = 'CGETRF'
         INFOT = 1
         CALL CHAMELEON_CGETRF_INCPIV( -1, 0, A, 1, HL, HPIV, INFO )
         CALL CHKXER( 'CGETRF', INFOT, NOUT, INFO, OK )
         INFOT = 2
         CALL CHAMELEON_CGETRF_INCPIV( 0, -1, A, 1, HL, HPIV, INFO )
         CALL CHKXER( 'CGETRF', INFOT, NOUT, INFO, OK )
         INFOT = 4
         CALL CHAMELEON_CGETRF_INCPIV( 2, 1, A, 1, HL, HPIV, INFO )
         CALL CHKXER( 'CGETRF', INFOT, NOUT, INFO, OK )
*
*        CGETRS
*
         SRNAMT = 'CGETRS'
         INFOT = 103
         CALL CHAMELEON_CGETRS_INCPIV( '/', -1, 0, A, 1, HL, HPIV, 
     $        B, 1, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
         INFOT = 2
         CALL CHAMELEON_CGETRS_INCPIV( CHAMELEONNOTRANS, -1, 0, A, 1, HL, 
     $        HPIV, B, 1, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
         INFOT = 3
         CALL CHAMELEON_CGETRS_INCPIV( CHAMELEONNOTRANS, 0, -1, A, 1, HL, 
     $        HPIV, B, 1, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
         INFOT = 5
         CALL CHAMELEON_CGETRS_INCPIV( CHAMELEONNOTRANS, 2, 1, A, 1, HL, 
     $        HPIV, B, 2, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
         INFOT = 9
         CALL CHAMELEON_CGETRS_INCPIV( CHAMELEONNOTRANS, 2, 1, A, 2, HL, 
     $        HPIV, B, 1, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
*
*        DEALLOCATE L and IPIV
*
         CALL CHAMELEON_DEALLOC_HANDLE( HL, INFO )
         CALL CHAMELEON_DEALLOC_HANDLE( HPIV, INFO )
*
*        LAPACK Interface
*        CGETRF
*
         SRNAMT = 'CGETRF'
         INFOT = 1
         CALL CHAMELEON_CGETRF( -1, 0, A, 1, IP, INFO )
         CALL CHKXER( 'CGETRF', INFOT, NOUT, INFO, OK )
         INFOT = 2
         CALL CHAMELEON_CGETRF( 0, -1, A, 1, IP, INFO )
         CALL CHKXER( 'CGETRF', INFOT, NOUT, INFO, OK )
         INFOT = 4
         CALL CHAMELEON_CGETRF( 2, 1, A, 1, IP, INFO )
         CALL CHKXER( 'CGETRF', INFOT, NOUT, INFO, OK )
*
*        CGETRS
*
         SRNAMT = 'CGETRS'
         INFOT = 1
         CALL CHAMELEON_CGETRS( '/', 0, 0, A, 1, IP,
     $        B, 1, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
         INFOT = 2
         CALL CHAMELEON_CGETRS( CHAMELEONNOTRANS, -1, 0, A, 1, IP,
     $        B, 1, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
         INFOT = 3
         CALL CHAMELEON_CGETRS( CHAMELEONNOTRANS, 0, -1, A, 1, IP,
     $        B, 1, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
         INFOT = 5
         CALL CHAMELEON_CGETRS( CHAMELEONNOTRANS, 2, 1, A, 1, IP,
     $        B, 2, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
         INFOT = 7
         CALL CHAMELEON_CGETRS( CHAMELEONNOTRANS, 2, 1, A, 2, IP,
     $        B, 1, INFO )
         CALL CHKXER( 'CGETRS', INFOT, NOUT, INFO, OK )
*
      ENDIF
*
*     Print a summary line.
*
      CALL ALAESM( PATH, OK, NOUT )
*
*     Enable CHAMELEON warnings/errors
* 
      CALL CHAMELEON_ENABLE( CHAMELEON_WARNINGS, INFO )
      CALL CHAMELEON_ENABLE( CHAMELEON_ERRORS,   INFO )
*
      RETURN
*
*     End of CERRGE
*
      END
