#include "system_ui/system_ui.h"

#include "core/system.h"
#include "common/event_types.h"

SystemUi system_ui;

bool SystemUi::Begin() {
  selection_ = 0;
  return true;
}

bool SystemUi::OnActivate() {
  return ShowMenu();
}

void SystemUi::OnDeactivate() {
}

bool SystemUi::HandleInput(const InputEvent& event) {
  uint8_t count = app_manager.GetAppCount();
  if (count == 0) return false;

  switch (event.type) {
    case InputType::RotateLeft:
      selection_ = (selection_ == 0) ? (uint8_t)(count - 1)
                                     : (uint8_t)(selection_ - 1);
      return ShowMenu();

    case InputType::RotateRight:
      selection_ = (uint8_t)((selection_ + 1) % count);
      return ShowMenu();

    case InputType::Enter: {
      IApp* target = app_manager.GetAppAt(selection_);
      if (target != nullptr) app_manager.LaunchApp(target->GetAppId());
      return true;
    }

    case InputType::Back:
      // Root of the navigation stack: nothing to pop.
      return false;
  }
  return true;
}

bool SystemUi::ShowMenu() {
  uint8_t count = app_manager.GetAppCount();
  if (count == 0 || count > kMaxMenuItems) return false;

  for (uint8_t i = 0; i < count; ++i) {
    IApp* app = app_manager.GetAppAt(i);
    menu_items_[i] = (app != nullptr) ? app->GetName() : "";
  }

  Event event;
  event.type = EventType::DisplayRequest;
  event.sender = GetAppId();
  event.display_request.type = DisplayRequestType::ShowAppMenu;
  event.display_request.lines = menu_items_;
  event.display_request.line_count = count;
  event.display_request.selected = selection_;
  return event_bus.Post(event);
}