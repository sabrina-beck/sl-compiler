// Non-local goto

void Example()
  labels lab1, lab2;
  vars x, y: integer;
  
  functions
    void f(a, b: integer) {
      if (a<=b) { goto lab1; }
      else { goto lab2; }
    }
    
{ // main
  x = 10; y = 20;
  lab1:
    write(x);
  lab2:
    write(y);
  write(x,y);
}
