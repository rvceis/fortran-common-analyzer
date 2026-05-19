      PROGRAM FILE_G
      ! Tests implicitly typed COMMON blocks
      ! A and B have no explicit type, so they should default to REAL
      COMMON /IMPLICIT_BLOCK/ A, B
      
      A = 3.14
      B = 6.28
      END PROGRAM FILE_G
