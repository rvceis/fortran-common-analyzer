      PROGRAM FILE_C
      ! This file uses complex Fortran syntax that regex would struggle with
      REAL :: A(10)
      REAL, DIMENSION(20) :: B
      INTEGER I

      COMMON /COMPLEX_BLOCK/ A, 
     + B

      ! Random loop and conditions to confuse basic regex
      DO 10 I = 1, 10
         A(I) = I * 2.0
         IF (A(I) .GT. 10.0) THEN
            B(I) = A(I)
         ENDIF
   10 CONTINUE

      END PROGRAM FILE_C
