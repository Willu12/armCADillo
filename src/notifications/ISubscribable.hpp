#pragma once

#include "ISubscriber.hpp"
#include <cstdio>
#include <functional>
#include <vector>

class ISubscribable {
public:
  virtual ~ISubscribable() = default;
  void subscribe(ISubscriber &subscriber) const {
    _subscribers.emplace_back(subscriber);
  }

  virtual void notifySubscribers() {
    for (auto &subscriber : _subscribers)
      subscriber.get().update();
  }

  void removeSubscriber(ISubscriber &subscriber) const {
    std::erase_if(_subscribers, [&subscriber](const auto &ref) {
      return &ref.get() == &subscriber;
    });
  }

  const std::vector<std::reference_wrapper<ISubscriber>> &
  getSubscribers() const {
    return _subscribers;
  }

protected:
  mutable std::vector<std::reference_wrapper<ISubscriber>> _subscribers;
};