#pragma once

#include <functional>
#include <vector>
class ISubscribable;

class ISubscriber {
public:
  ~ISubscriber();
  void subscribe(ISubscribable &publisher);
  virtual void onSubscribableDestroyed(const ISubscribable &publisher) = 0;
  virtual void update() = 0;

protected:
  std::vector<std::reference_wrapper<ISubscribable>> _publishers;
};