// Nested functions

void Example()
  vars 
    x, y: integer;
    
  functions
  
    integer f(a, b: integer) 
      functions
        integer g(a: integer) {
          return 2*a;
        }
    // body of f    
    {
      return g(a*b);
    }
    
    integer g(a: integer) {
      return 2*a;
    }
    
{ // main
  x = f(200, 300);
  y = g(400);
  write(x, y);
  write(f(g(10), g(20)));
}
