#pragma once

class Mouse;

class IController {
public:
  virtual ~IController() = default;
  virtual bool processMouse() = 0;
  virtual bool processScroll() = 0;

  virtual void process(const Mouse &mouse) = 0;
};