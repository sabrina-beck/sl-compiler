// Non-void function with params

void Example()
  vars 
    x, y: integer;
  functions
    integer f(a, b: integer) {
      y = a+b;
      return 2*y;
    }
{
  x = 250;
  write(f(200, 300));
  write(x, y);
}

    