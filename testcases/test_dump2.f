      PROGRAM DUMP2
      COMMON /MYCOM/ A, B
      REAL A
      INTEGER B
      REAL C
      SAVE /MYCOM/
      EQUIVALENCE (A, C)
      END
