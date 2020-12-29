// Arrays as parameters

void Example()
  types
     Vector = integer[10];
     Matrix = Vector[10];
     
  vars
     M: Matrix;
     S, T: Vector;
     i, j: integer;
     
  functions
     void mat(A, B: Vector, var C: Matrix, m: integer)
       vars i, j: integer;
     {
       i = 0;
       while (i<m) {
       	 j = 0;
         while (j<m) {
           C[i][j] = A[i]*B[j];
           j = j+1;
         }
         i = i+1;
       }
     } // mat
     
{ // main
  i = 0;
  while (i<10) {
    S[i] = i+1;
    T[i] = 2*i+1;
    i = i+1;
  }
  mat(S, T, M, 10);
  while (i<10) {
    i=0;
    while (j<10) {
      write(M[i][j]);
      j = j+1;
    }
    i = i+1;
  }
}
