// Simple expressions

void Example()
  vars x, y: integer;
{ 
  read(x, y);
  x = x+10;
  y = x+y;
  x = y-10;
  write(x, y, x*y);
}
