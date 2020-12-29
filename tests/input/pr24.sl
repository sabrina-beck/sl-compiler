// Simple function -- lexical error detected on line 9

void Example()
  vars 
    x: integer;
  functions
    void f() {
      x = x+100;
      return x%10 ;
    }
{
  x = -10;
  f();
  write(x);
}
