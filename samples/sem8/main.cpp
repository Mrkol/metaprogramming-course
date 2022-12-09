#include "tag_invoke.hpp"
#include "any_object.hpp"
#include "shapes.hpp"





int main() {
  shapes::square sq(42);

  shapes::any_shape s{sq};

  shapes::area(s);
  auto s2 = shapes::scale_by(s, 2);

  return 0;
}
