// Simple indexing and array assignment

void Example()
  types
    Vector = integer[10];
    
  vars
    v, w: Vector;
    i: integer;
    
{ // main
  i = 0;
  while (i<10) {
    v[i] = i;
    i = i+1;
  }
  i = 0;
  while (i<10) {
    w[i] = v[i]+1;
    i = i+1;
  }
  w = v;
}
