#pragma once

#include <cassert>

#define ASSERT(expr, message) \
  {                           \
    void(message);            \
    assert(expr);             \
  }

#define MOVABLE_ONLY(CLASS_NAME)                     \
  CLASS_NAME(const CLASS_NAME&) = delete;            \
  CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
  CLASS_NAME(CLASS_NAME&&) noexcept = default;       \
  CLASS_NAME& operator=(CLASS_NAME&&) noexcept = default;

#define ENABLE_VALIDATION_LAYERS 1