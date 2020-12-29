// Repetition -- syntax error detected on line 15

void Example()
  vars x, y: integer;
      p: boolean;
{
  x = 10; y = 15;
  p = true;
  while (x<y) {
    write(x, y);
    x = x+1;
  p = !p;
  write(x,y);
}
