#pragma once

struct Cutter {
  enum class Type : bool { Flat, Ball };

  Type type_;
  float diameter_;
  float height_;

  float radius() const { return diameter_ / 2.f; }
};