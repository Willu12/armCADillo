#pragma once
#include "vec.hpp"
#include <memory>

class Mouse {
public:
  static std::shared_ptr<Mouse> getInstance() {
    if (!_instance) {
      _instance = std::shared_ptr<Mouse>(new Mouse());
    }
    return _instance;
  }

  algebra::Vec2f _position;
  bool _leftClicked = false;
  bool _rightClicked = false;

private:
  Mouse() = default;
  //  ~Mouse() = default;
  Mouse(const Mouse &) = delete;
  Mouse &operator=(const Mouse &) = delete;
  static std::shared_ptr<Mouse> _instance;
};

std::shared_ptr<Mouse> Mouse::_instance = nullptr;
