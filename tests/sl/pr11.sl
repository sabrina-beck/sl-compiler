// Conditionals

void Example()
  vars x, y: integer;
       p: boolean;
{
  x = 10; y = 20;
  p = (x<y);
  if (p) 
      { write(x, y); }
  else   
      { write(y, x); }
  write(x+y);
}
