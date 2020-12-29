// Void function with parameters

void Example()
  vars 
    x, y: integer;
  functions
    void f(a, b: integer) {
      y = a+b;
      return;
    }
{
  x = 0;
  f(200, 300);
  write(x, y);
}

    