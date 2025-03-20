#pragma once
#include "vec.hpp"

class Mouse {
public:
  algebra::Vec2f _position;
  bool _leftClicked = false;
  bool _rightClicked = false;
};