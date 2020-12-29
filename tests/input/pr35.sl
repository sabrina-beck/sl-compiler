// Multi-dimensional array indexing

void Example()
  types
     Matrix = integer[3][4];
     
  vars
     i, j: integer;
     A: integer[3][4];
     B: Matrix;
     
{
  i = 0;
  while (i<3) {
    j = 0;
    while (j<4) {
      A[i][j] = i*j;
      write(i,j,A[i][j]);
      j = j+1;
    }
    B[i] = A[i];
    i = i+1;
  }
  B = A;
  
}
