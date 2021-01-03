// Incompatible parameter function types

void Example()

  functions
    integer f(a, b: integer, integer g(x: integer))
    {
      return g(a)*g(b);
    }

    integer h(x, y: integer)
    {
      return x+y;
    }

 { // main
  write(f(x, y, h));
}

