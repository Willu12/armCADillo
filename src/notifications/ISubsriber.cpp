#include "ISubscribable.hpp"
#include "ISubscriber.hpp"

ISubscriber::~ISubscriber() {
  for (auto &publisher : _publishers) {
    publisher.get().removeSubscriber(*this);
  }
}

void ISubscriber::subscribe(const ISubscribable &publisher) {
  publisher.subscribe(*this);
  _publishers.emplace_back(publisher);
}