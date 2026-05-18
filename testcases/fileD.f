      PROGRAM FILE_D
      ! This file tests complex equivalence
      REAL X(10)
      REAL Y(10)
      REAL Z(10)

      COMMON /EQ_TEST_BLOCK/ X, Y

      EQUIVALENCE (X(1), Z(1))
      EQUIVALENCE (Y(1), Z(2))

      X(1) = 1.0
      Y(1) = 2.0
      Z(1) = 3.0

      END PROGRAM FILE_D
