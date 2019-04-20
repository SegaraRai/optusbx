#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "libusb/libusb.h"

#include "LibUsb.hpp"


class LibUsbDevice {
public:
  class OpenError : public LibUsb::LibUsbError {
  public:
    OpenError(int errorCode);
  };

private:
  libusb_device_handle* mDeviceHandle;

public:
  LibUsbDevice(const LibUsbDevice&) = delete;
  LibUsbDevice& operator=(const LibUsbDevice&) = delete;

  LibUsbDevice(LibUsbDevice&& other) noexcept;
  LibUsbDevice& operator=(LibUsbDevice&& other) noexcept;

  LibUsbDevice(const LibUsb& libusb, std::uint_fast16_t VendorId, std::uint_fast16_t ProductId);
  virtual ~LibUsbDevice();

  operator libusb_device_handle*() const;

  libusb_device_handle* GetDeviceHandle() const;

  void ClaimInterface(int interfaceNumber);
  void Send(std::uint_fast8_t endpoint, std::byte* data, std::size_t dataSize, unsigned int timeout);
  void Receive(std::uint_fast8_t endpoint, std::byte* data, std::size_t dataSize, unsigned int timeout);
};
