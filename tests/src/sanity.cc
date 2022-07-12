#include <string>

#include "dawn/dawn.h"

auto main() -> int
{
  auto const exported = exported_class {};

  return std::string("dawn") == exported.name() ? 0 : 1;
}
