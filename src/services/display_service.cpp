#include "services/display_service.h"

#include "drivers/oled/oled.h"
#include "core/system.h"

DisplayService display_service;

bool DisplayService::Begin() {
  if (oled.Begin()) {
    initialized_ = true;
    return true;
  }
  return false;
}

void DisplayService::OnEvent(const Event& event) {
  if (!initialized_ || event.type != EventType::DisplayRequest) {
    return;
  }

  // Only the foreground app may draw: exclusive display
  // ownership is enforced here (replaces the old
  // ResourceManager).
  const IApp* owner = app_manager.GetForeground();
  if (owner == nullptr || owner->GetAppId() != event.sender) {
    return;
  }

  const DisplayRequest& req = event.display_request;
  switch (req.type) {
    case DisplayRequestType::ShowWord:
      ShowWord(req);
      break;
    case DisplayRequestType::ShowLine:
      ShowLine(req);
      break;
    case DisplayRequestType::ShowLines:
      ShowLines(req);
      break;
    case DisplayRequestType::ShowAppMenu:
      ShowAppMenu(req);
      break;
    case DisplayRequestType::ShowBigTime:
      ShowBigTime(req);
      break;
    case DisplayRequestType::ClearDisplay:
      ClearDisplay();
      break;
    default:
      break;
  }
}

// ==========================================================
// SHOW WORD
// ==========================================================

void DisplayService::ShowWord(const DisplayRequest& req) {
  if (req.text == nullptr) return;
  oled.Clear();
  SetFont(req.text_size);
  int16_t text_width = oled.GetTextWidth(req.text);
  int16_t text_height = oled.GetTextHeight();
  int16_t x = (oled.Width() - text_width) / 2;
  int16_t y = (oled.Height() + text_height) / 2;
  oled.DrawText(x, y, req.text);
  oled.Update();
}

// ==========================================================
// SHOW LINE
// ==========================================================

void DisplayService::ShowLine(const DisplayRequest& req) {
  if (req.text == nullptr) return;
  oled.Clear();
  SetFont(req.text_size);
  int16_t x = CalculateX(req.text, req.alignment);
  int16_t y = CalculateCenteredY();
  oled.DrawText(x, y, req.text);
  oled.Update();
}

// ==========================================================
// SHOW MULTIPLE LINES
// ==========================================================

void DisplayService::ShowLines(const DisplayRequest& req) {
  if (req.lines == nullptr || req.line_count == 0) return;
  oled.Clear();
  SetFont(req.text_size);
  int16_t line_height = oled.GetTextHeight();
  int16_t total_height = req.line_count * line_height;
  int16_t start_y = (oled.Height() - total_height) / 2 + line_height;
  for (uint8_t i = 0; i < req.line_count; ++i) {
    int16_t x = CalculateX(req.lines[i], req.alignment);
    int16_t y = start_y + i * line_height;
    oled.DrawText(x, y, req.lines[i]);
  }
  oled.Update();
}

// ==========================================================
// SHOW APP MENU
// ==========================================================

void DisplayService::ShowAppMenu(const DisplayRequest& req) {
  if (req.lines == nullptr || req.line_count == 0) return;

  oled.Clear();

  uint8_t count = req.line_count;
  uint8_t sel = (req.selected < count) ? req.selected : (uint8_t)(count - 1);
  const char* prev_name = (count > 1) ? req.lines[(sel + count - 1) % count] : nullptr;
  const char* next_name = (count > 1) ? req.lines[(sel + 1) % count] : nullptr;
  const char* current_name = req.lines[sel];

  constexpr int16_t topY = 8;

  // Previous app (top-left).
  oled.SetFont(Oled::Font::Small);
  if (prev_name != nullptr) {
    oled.DrawText(2, topY, "<");
    oled.DrawText(10, topY, prev_name);
  }

  // Next app (top-right).
  if (next_name != nullptr) {
    int16_t total = oled.GetTextWidth(next_name) + 8;
    oled.DrawText(oled.Width() - total, topY, next_name);
    oled.DrawText(oled.Width() - 6, topY, ">");
  }

  // Current app (center, large).
  oled.SetFont(Oled::Font::Medium);
  int16_t x = (oled.Width() - oled.GetTextWidth(current_name)) / 2;
  if (x < 0) x = 0;
  int16_t y = (oled.Height() + oled.GetTextHeight()) / 2;
  oled.DrawText(x, y, current_name);

  // Footer.
  oled.SetFont(Oled::Font::Small);
  oled.DrawText(0, oled.Height() - 8, "ENTER open  BACK exit");

  oled.Update();
}

// ==========================================================
// SHOW BIG TIME
// ==========================================================
// A large central time with small labels above and below.
//   top_label:    small font, top of screen
//   big_time:     large font, vertically centered
//   bottom_status:small font, bottom of screen
//   blink:        when true, the time is hidden (for flashing)
// ==========================================================

void DisplayService::ShowBigTime(const DisplayRequest& req) {
  oled.Clear();

  // Top label (small font).
  if (req.top_label != nullptr) {
    oled.SetFont(Oled::Font::Small);
    int16_t x = CalculateX(req.top_label, req.alignment);
    oled.DrawText(x, 12, req.top_label);
  }

  // Big time (large font, centered) - hidden while blinking.
  if (req.big_time != nullptr && !req.blink) {
    oled.SetFont(Oled::Font::Large);
    int16_t x = CalculateX(req.big_time, req.alignment);
    int16_t y = (oled.Height() + oled.GetTextHeight()) / 2;
    oled.DrawText(x, y, req.big_time);
  }

  // Bottom status (small font).
  if (req.bottom_status != nullptr) {
    oled.SetFont(Oled::Font::Small);
    int16_t x = CalculateX(req.bottom_status, req.alignment);
    oled.DrawText(x, oled.Height() - 4, req.bottom_status);
  }

  oled.Update();
}

// ==========================================================
// CLEAR DISPLAY
// ==========================================================

void DisplayService::ClearDisplay() {
  oled.Clear();
  oled.Update();
}

// ==========================================================
// SET FONT
// ==========================================================

void DisplayService::SetFont(TextSize size) {
  switch (size) {
    case TextSize::Small:
      oled.SetFont(Oled::Font::Small);
      break;
    case TextSize::Medium:
      oled.SetFont(Oled::Font::Medium);
      break;
    case TextSize::Large:
      oled.SetFont(Oled::Font::Large);
      break;
  }
}

// ==========================================================
// CALCULATE X POSITION
// ==========================================================

int16_t DisplayService::CalculateX(const char* text, TextAlign align) {
  int16_t text_width = oled.GetTextWidth(text);
  int16_t screen_width = oled.Width();
  switch (align) {
    case TextAlign::Left:
      return 0;
    case TextAlign::Center:
      return (screen_width - text_width) / 2;
    case TextAlign::Right:
      return screen_width - text_width;
  }
  return 0;
}

// ==========================================================
// CALCULATE CENTERED Y
// ==========================================================

int16_t DisplayService::CalculateCenteredY() {
  int16_t screen_height = oled.Height();
  int16_t text_height = oled.GetTextHeight();
  return (screen_height + text_height) / 2;
}