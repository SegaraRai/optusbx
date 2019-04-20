#pragma once

#include "libusb/libusb.h"

#include <stdexcept>
#include <string>


class LibUsb {
public:
  class LibUsbError : public std::runtime_error {
  public:
    int errorCode;

    LibUsbError(int errorCode, const std::string& message);
    LibUsbError(int errorCode);
  };

  class InitializeError : public LibUsbError {
  public:
    InitializeError(int errorCode);
  };

  static void CheckLibUsbResult(int code);

  static LibUsb& GetDefaultInstance();

private:
  bool mValid;
  libusb_context* mContext;

  LibUsb(bool useDefaultContext);

public:
  LibUsb(const LibUsb&) = delete;
  LibUsb& operator=(const LibUsb&) = delete;

  LibUsb(LibUsb&& other) noexcept;
  LibUsb& operator=(LibUsb&& other) noexcept;

  LibUsb();
  virtual ~LibUsb();

  operator libusb_context*() const;

  libusb_context* GetContext() const;

  void SetLogLevel(libusb_log_level logLevel);
  void UseUsbDk();
};
