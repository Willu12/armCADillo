#pragma once

class IController {
public:
  virtual ~IController() = default;
  virtual bool processMouse() = 0;
  virtual bool processScroll() = 0;
};