#pragma once

#include <string>

#include "dawn/dawn_export.h"

/**
 * @brief Reports the name of the library
 *
 * Please see the note above for considerations when creating shared libraries.
 */
class DAWN_EXPORT exported_class {
public:
  /**
   * @brief Initializes the name field to the name of the project
   */
  exported_class();

  /**
   * @brief Returns a non-owning pointer to the string stored in this class
   */
  [[nodiscard]] auto name() const -> const char*;

private:
  DAWN_SUPPRESS_C4251
  std::string m_name;
};
