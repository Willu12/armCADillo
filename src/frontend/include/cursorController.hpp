#pragma once

#include "cursor.hpp"

class CursorController {
public:
  CursorController() : _cursor(std::make_unique<Cursor>()) {}

private:
  std::unique_ptr<Cursor> _cursor;
};