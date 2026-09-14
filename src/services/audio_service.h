#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

#include "core/event_bus.h"

// ==========================================================
// AUDIO SERVICE (STUB)
// ==========================================================
//
// Placeholder for future audio playback.
//
// ==========================================================

class AudioService : public IEventSubscriber {
public:
  bool Begin() { return true; }
  void OnEvent(const Event& event) override {}
};

extern AudioService audio_service;

#endif