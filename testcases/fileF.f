      PROGRAM FILE_F
      ! Tests blank (unnamed) COMMON block
      REAL X(10)
      INTEGER Y
      
      COMMON X, Y
      
      X(1) = 5.0
      Y = 10
      END PROGRAM FILE_F
