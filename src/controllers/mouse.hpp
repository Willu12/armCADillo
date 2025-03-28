#pragma once
#include "vec.hpp"
#include <memory>

class Mouse {
public:
  static Mouse &getInstance() {
    static Mouse mouse;
    return mouse;
  }

  algebra::Vec2f _position;
  bool _leftClicked = false;
  bool _rightClicked = false;

private:
  Mouse() = default;
  ~Mouse() = default;
  Mouse(const Mouse &) = delete;
  Mouse &operator=(const Mouse &) = delete;
};
