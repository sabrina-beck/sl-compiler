// Simple function

void Example()
  vars 
    x: integer;
  functions
    void f() {
      x = x+100;
      return;
    }
{
  x = -10;
  f();
  write(x);
}
