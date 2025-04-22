#pragma once

#include <functional>
#include <vector>
class ISubscribable;

class ISubscriber {
public:
  ~ISubscriber();
  void subscribe(const ISubscribable &publisher);
  virtual void onSubscribableDestroyed(const ISubscribable &publisher) = 0;
  virtual void update() = 0;

protected:
  mutable std::vector<std::reference_wrapper<const ISubscribable>> _publishers;
};