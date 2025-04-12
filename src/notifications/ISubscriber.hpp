#pragma once

class ISubscribable;

class ISubscriber {
public:
  virtual ~ISubscriber() = default;
  void subscribe(ISubscribable &publisher);
  virtual void onSubscribableDestroyed(const ISubscribable &publisher) = 0;
  virtual void update() = 0;
};