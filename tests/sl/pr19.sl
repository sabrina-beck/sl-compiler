// Two functions -- semantic error:
//    incompatible function call with declaration 

void Example()
  vars 
    x, y: integer;
    
  functions
    integer f(a,b: integer, p: boolean) {
      if (p && (a>b)) {return a-b;}
      else {
             if (a<2*b) {return a+b;}
             else {return 2*a+b;}
           }
    }
    
    integer g(a: integer) {
      return 2*a;
    }
    
{ // main
  x = f(200, 300, true);
  y = g(400);
  write(x, y);
  write(f(g(10), g(20)));
}
