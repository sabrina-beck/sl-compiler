// Function parameters with array parameters

void Example()
  types
     Matrix = integer[10][10];
  vars
     U,W: Matrix;
     i, j: integer;
     
  functions
     void f(A: Matrix, var B: Matrix, m: integer, 
            void g(X: Matrix, var Y: Matrix, m: integer))
     {
       g(A,B,m);
     } // end f
     
     void h(S: Matrix, var T: Matrix, m: integer)
       vars i, j: integer;
     {
       i = 0;
       while (i<m) {
       	 j = 0;
         while (j<m) {
           T[i][j] = S[i][j];
           j = j+1;
         }
         i = i+1;
       }
     } // end h
     
{ // main
  i = 0;
  while (i<10) {
    j=0;
    while (j<10) {
      U[i][j] = i*j;
      j = j+1;
    }
    i = i+1;
  }
  f(U,W,10,h);
  i = 0;
  while (i<10) {
    j=0;
    while (j<10) {
      write(W[i][j]);
      j = j+1;
    }
    i = i+1;
  }  
} // end Example
