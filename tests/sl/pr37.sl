// Incompatible array types

void Example()
  types
     Vector = integer[10];
     Matrix = Vector[10];

  vars
    M: Matrix;
    V: Vector;
    
{
  M = V;
}
