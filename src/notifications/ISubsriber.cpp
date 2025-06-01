#include "ISubscribable.hpp"
#include "ISubscriber.hpp"
#include <functional>

ISubscriber::~ISubscriber() {
  for (auto &publisher : _publishers) {
    publisher.get().removeSubscriber(*this);
  }
}

void ISubscriber::subscribe(ISubscribable &publisher) {
  publisher.subscribe(*this);
  _publishers.emplace_back(publisher);
}

void ISubscriber::unsubscribe(const ISubscribable &publisher) {
  std::erase_if(_publishers, [&publisher](const auto &ref) {
    return &ref.get() == &publisher;
  });
}