// Functions as parameters

void Example()
  vars 
    x, y: integer;
    
  functions
    integer f(a, b: integer, integer g(x: integer)) 
    {
      return g(a)*g(b);
    }
    
    integer h(x: integer)
    {
      return 2*x+1;
    }
    
{ // main
  x = 5; y = 10;
  write(f(x, y, h));
}
