#include "ISubscribable.hpp"
#include "ISubscriber.hpp"

void ISubscriber::subscribe(ISubscribable &publisher) {
  publisher.subscribe(*this);
}