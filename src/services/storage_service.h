#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include <Arduino.h>
#include <FS.h>

// ==========================================================
// STORAGE SERVICE
// ==========================================================
//
// Provides filesystem access to applications and other
// system modules without exposing the underlying filesystem
// implementation. Current backend: ESP32 LittleFS.
//
// ==========================================================

class StorageService {
public:
  // Initialize and mount the filesystem.
  bool Begin();

  // Check whether a file or directory exists.
  bool Exists(const char* path) const;

  // Open a file. Returns a File object (invalid if fails).
  File Open(const char* path, const char* mode) const;

  // Delete a file.
  bool Remove(const char* path);

  // Rename a file or directory.
  bool Rename(const char* old_path, const char* new_path);

  // Get the size of a file in bytes.
  size_t FileSize(const char* path) const;

  // Check whether storage has been successfully initialized.
  bool IsReady() const;

private:
  bool initialized_ = false;
};

// Global storage service instance.
extern StorageService storage_service;

#endif