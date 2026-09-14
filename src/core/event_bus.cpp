#include "core/event_bus.h"

// ==========================================================
// SUBSCRIBE
// ==========================================================

bool EventBus::Subscribe(EventType type, IEventSubscriber* subscriber) {
  if (subscriber == nullptr) {
    return false;
  }
  uint8_t idx = static_cast<uint8_t>(type);
  if (subscriber_count_[idx] >= kMaxSubscribersPerType) {
    return false;
  }
  subscribers_[idx][subscriber_count_[idx]++] = subscriber;
  return true;
}

// ==========================================================
// POST EVENT
// ==========================================================

bool EventBus::Post(const Event& event) {
  if (IsQueueFull()) {
    return false;
  }
  queue_[queue_tail_] = event;
  queue_tail_ = (queue_tail_ + 1) % kEventQueueSize;
  if (queue_tail_ == queue_head_) {
    queue_full_ = true;
  }
  return true;
}

// ==========================================================
// DISPATCH EVENTS
// ==========================================================

void EventBus::Dispatch() {
  while (!IsQueueEmpty()) {
    Event event = queue_[queue_head_];
    queue_head_ = (queue_head_ + 1) % kEventQueueSize;
    queue_full_ = false;

    uint8_t idx = static_cast<uint8_t>(event.type);
    for (uint8_t i = 0; i < subscriber_count_[idx]; ++i) {
      subscribers_[idx][i]->OnEvent(event);
    }
  }
}

// ==========================================================
// QUEUE STATUS
// ==========================================================

bool EventBus::IsQueueEmpty() const {
  return (!queue_full_ && (queue_head_ == queue_tail_));
}

bool EventBus::IsQueueFull() const {
  return queue_full_;
}