#pragma once

#include <functional>
#include <vector>
class ISubscribable;

class ISubscriber {
public:
  virtual ~ISubscriber();
  void subscribe(ISubscribable &publisher);
  void unsubscribe(const ISubscribable &publisher);
  virtual void onSubscribableDestroyed(ISubscribable &publisher) = 0;
  virtual void update() = 0;
  virtual void markToUpdate() = 0;

protected:
  mutable std::vector<std::reference_wrapper<ISubscribable>> _publishers;
};