#pragma once

struct Cutter {
  enum class Type : bool { Flat, Ball };

  Type type_;
  float diameter_;
  float height_;
};