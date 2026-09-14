#include <Arduino.h>

#include "core/system.h"
#include "core/app_manager.h"
#include "core/event_bus.h"

#include "services/storage_service.h"
#include "services/display_service.h"
#include "services/input_service.h"
#include "services/audio_service.h"

#include "system_ui/system_ui.h"
#include "apps/lyrics/lyrics_app.h"
#include "apps/clock/stopwatch_app.h"

// app_manager is defined in core/app_manager.cpp (see core/system.h).
EventBus event_bus;

void setup() {
  Serial.begin(115200);

  if (!storage_service.Begin()) {
    Serial.println("Storage init failed");
    while (true) { delay(1000); }
  }

  if (!display_service.Begin()) {
    Serial.println("Display init failed");
    while (true) { delay(1000); }
  }

  input_service.Begin();
  audio_service.Begin();

  // Register every launchable app. The SystemUi (launcher) is
  // booted separately and never appears inside its own menu.
  app_manager.RegisterApp(&lyrics_app);
  app_manager.RegisterApp(&stopwatch_app);

  lyrics_app.Begin();
  stopwatch_app.Begin();

  event_bus.Subscribe(EventType::DisplayRequest, &display_service);
  event_bus.Subscribe(EventType::InputEvent, &app_manager);

  system_ui.Begin();

  if (!app_manager.Boot(&system_ui)) {
    Serial.println("SystemUi boot failed");
    while (true) { delay(1000); }
  }

  Serial.println("READY");
}

void loop() {
  input_service.Poll();           // hardware -> InputEvents
  input_service.PollSerialDebug(); // optional serial debug input
  event_bus.Dispatch();           // render display requests, route inputs
  app_manager.Update();           // pump the foreground app
}