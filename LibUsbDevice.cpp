#include "LibUsbDevice.hpp"
#include "LibUsb.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

using namespace std::literals;



LibUsbDevice::OpenError::OpenError(int errorCode) :
  LibUsbError(errorCode)
{}



LibUsbDevice::LibUsbDevice(LibUsbDevice&& other) noexcept :
  mDeviceHandle(other.mDeviceHandle)
{
  other.mDeviceHandle = nullptr;
}


LibUsbDevice& LibUsbDevice::operator=(LibUsbDevice&& other) noexcept {
  if (mDeviceHandle) {
    libusb_close(mDeviceHandle);
  }
  mDeviceHandle = other.mDeviceHandle;
  other.mDeviceHandle = nullptr;
  return *this;
}


LibUsbDevice::LibUsbDevice(const LibUsb& libusb, std::uint_fast16_t VendorId, std::uint_fast16_t ProductId) :
  mDeviceHandle(nullptr)
{
  mDeviceHandle = libusb_open_device_with_vid_pid(libusb, VendorId, ProductId);
  if (!mDeviceHandle) {
    throw OpenError(0);
  }
}


LibUsbDevice::~LibUsbDevice() {
  if (mDeviceHandle) {
    libusb_close(mDeviceHandle);
    mDeviceHandle = nullptr;
  }
}


LibUsbDevice::operator libusb_device_handle*() const {
  return mDeviceHandle;
}


libusb_device_handle* LibUsbDevice::GetDeviceHandle() const {
  return mDeviceHandle;
}


void LibUsbDevice::ClaimInterface(int interfaceNumber) {
  if (!mDeviceHandle) {
    throw std::runtime_error("invalidated LibUsbDevice instance used"s);
  }
  LibUsb::CheckLibUsbResult(libusb_claim_interface(mDeviceHandle, interfaceNumber));
}


void LibUsbDevice::Send(std::uint_fast8_t endpoint, std::byte* data, std::size_t dataSize, unsigned int timeout) {
  if (!mDeviceHandle) {
    throw std::runtime_error("invalidated LibUsbDevice instance used"s);
  }
  int transferredSize;
  LibUsb::CheckLibUsbResult(libusb_bulk_transfer(mDeviceHandle, LIBUSB_ENDPOINT_OUT | endpoint, reinterpret_cast<unsigned char*>(data), dataSize, &transferredSize, timeout));
  if (transferredSize != dataSize) {
    throw std::runtime_error("libusb error: sent partially"s);
  }
}


void LibUsbDevice::Receive(std::uint_fast8_t endpoint, std::byte* data, std::size_t dataSize, unsigned int timeout) {
  if (!mDeviceHandle) {
    throw std::runtime_error("invalidated LibUsbDevice instance used"s);
  }
  int transferredSize;
  LibUsb::CheckLibUsbResult(libusb_bulk_transfer(mDeviceHandle, LIBUSB_ENDPOINT_IN | endpoint, reinterpret_cast<unsigned char*>(data), dataSize, &transferredSize, timeout));
  if (transferredSize != dataSize) {
    throw std::runtime_error("libusb error: received partiallys");
  }
}
