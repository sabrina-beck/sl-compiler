// Non-void parameterless function

void Example()
  vars 
    x, y: integer;
  functions
    integer f() {
      x = x+100;
      return x;
    }
{
  x = -200+3*(-100);
  y = f();
  write(x, y);
}

