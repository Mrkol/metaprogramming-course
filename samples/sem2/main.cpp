#include <iostream>
#include <span>
#include <numeric>
#include <vector>




template<class T>
  requires requires(T t) { --t; }
void f(T);

template<class T>
  requires requires(T t) { --t; } && requires(T t)
void f(T);


int main(int argc, char** argv)
{
  return 0;
}
