#pragma once

#include "ISubscriber.hpp"
#include <cstdio>
#include <functional>
#include <vector>

class ISubscribable {
public:
  virtual ~ISubscribable() {
    for (const auto &subscriber : _subscribers) {
      subscriber.get().onSubscribableDestroyed(*this);
    }
  }
  void subscribe(ISubscriber &subscriber) const {
    _subscribers.emplace_back(subscriber);
  }

  void notifySubscribers() {
    for (const auto subscriber : _subscribers)
      subscriber.get().update();
  }

  void removeSubscriber(const ISubscriber &subscriber) const {
    std::erase_if(_subscribers, [&subscriber](const auto &ref) {
      return &ref.get() == &subscriber;
    });
  }

private:
  mutable std::vector<std::reference_wrapper<ISubscriber>> _subscribers;
};