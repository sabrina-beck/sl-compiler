// Repetition and conditional

void Example()
  vars x, y: integer;
{
  x = 10; y = 15;
  while ((x<y) && (2*x>y)) {
    if (x>0) { write(x); }
    x = x+2;
    y = y+1;
  }
  write(x, y);
}
