#include "services/storage_service.h"

#include <LittleFS.h>

// Global storage service instance.
StorageService storage_service;

// ==========================================================
// BEGIN
// ==========================================================

bool StorageService::Begin() {
  if (initialized_) {
    return true;
  }

  if (!LittleFS.begin()) {
    initialized_ = false;
    return false;
  }

  initialized_ = true;
  return true;
}

// ==========================================================
// EXISTS
// ==========================================================

bool StorageService::Exists(const char* path) const {
  if (!initialized_ || path == nullptr) {
    return false;
  }
  return LittleFS.exists(path);
}

// ==========================================================
// OPEN
// ==========================================================

File StorageService::Open(const char* path, const char* mode) const {
  if (!initialized_ || path == nullptr || mode == nullptr) {
    return File();
  }
  return LittleFS.open(path, mode);
}

// ==========================================================
// REMOVE
// ==========================================================

bool StorageService::Remove(const char* path) {
  if (!initialized_ || path == nullptr) {
    return false;
  }
  return LittleFS.remove(path);
}

// ==========================================================
// RENAME
// ==========================================================

bool StorageService::Rename(const char* old_path, const char* new_path) {
  if (!initialized_ || old_path == nullptr || new_path == nullptr) {
    return false;
  }
  return LittleFS.rename(old_path, new_path);
}

// ==========================================================
// FILE SIZE
// ==========================================================

size_t StorageService::FileSize(const char* path) const {
  if (!initialized_ || path == nullptr) {
    return 0;
  }
  File file = LittleFS.open(path, FILE_READ);
  if (!file) {
    return 0;
  }
  size_t size = file.size();
  file.close();
  return size;
}

// ==========================================================
// IS READY
// ==========================================================

bool StorageService::IsReady() const {
  return initialized_;
}