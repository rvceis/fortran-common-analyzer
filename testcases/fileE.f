      PROGRAM FILE_E
      ! Tests multiple variables in a single EQUIVALENCE
      REAL A, B, C, D
      COMMON /MULTI_EQ/ A

      EQUIVALENCE (A, B, C, D)
      
      A = 10.0
      END PROGRAM FILE_E
