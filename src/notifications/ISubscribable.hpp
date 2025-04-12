#pragma once

#include "ISubscriber.hpp"
#include <functional>
#include <vector>

class ISubscribable {
public:
  virtual ~ISubscribable() {
    for (const auto &subscriber : _subscribers) {
      subscriber.get().onSubscribableDestroyed(*this);
    }
  }
  void subscribe(ISubscriber &subscriber) {
    _subscribers.push_back(subscriber);
  }
  void notifySubscribers() {
    for (const auto subscriber : _subscribers)
      subscriber.get().update();
  }

private:
  std::vector<std::reference_wrapper<ISubscriber>> _subscribers;
};