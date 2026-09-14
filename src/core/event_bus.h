#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <Arduino.h>

#include "common/event_types.h"

// ==========================================================
// EVENT SUBSCRIBER INTERFACE
// ==========================================================

class IEventSubscriber {
public:
  virtual ~IEventSubscriber() = default;
  virtual void OnEvent(const Event& event) = 0;
};

// ==========================================================
// EVENT BUS
// ==========================================================
//
// A simple publish‑subscribe system. Subscribers register
// for specific event types. Events are queued and dispatched
// when Dispatch() is called from the main loop.
//
// ==========================================================

class EventBus {
public:
  bool Subscribe(EventType type, IEventSubscriber* subscriber);
  bool Post(const Event& event);
  void Dispatch();

private:
  static constexpr uint8_t kMaxSubscribersPerType = 4;
  static constexpr uint8_t kEventQueueSize = 16;

  // Subscriber registry
  IEventSubscriber* subscribers_[static_cast<uint8_t>(EventType::Count)][kMaxSubscribersPerType] = {{nullptr}};
  uint8_t subscriber_count_[static_cast<uint8_t>(EventType::Count)] = {0};

  // Event queue (ring buffer)
  Event queue_[kEventQueueSize];
  uint8_t queue_head_ = 0;
  uint8_t queue_tail_ = 0;
  bool queue_full_ = false;

  bool IsQueueEmpty() const;
  bool IsQueueFull() const;
};

#endif