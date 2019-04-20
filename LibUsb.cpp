#include "LibUsb.hpp"

#include <stdexcept>
#include <string>

using namespace std::literals;



LibUsb::LibUsbError::LibUsbError(int errorCode, const std::string& message) :
  std::runtime_error(message),
  errorCode(errorCode) {}


LibUsb::LibUsbError::LibUsbError(int errorCode) :
  LibUsbError(errorCode, "libusb error "s + std::to_string(errorCode) + " : "s + libusb_error_name(errorCode))
{}



LibUsb::InitializeError::InitializeError(int errorCode) :
  LibUsbError(errorCode)
{}



void LibUsb::CheckLibUsbResult(int errorCode) {
  if (errorCode != 0) {
    throw LibUsbError(errorCode);
  }
}


LibUsb& LibUsb::GetDefaultInstance() {
  static LibUsb defaultLibUsb(true);
  return defaultLibUsb;
}


LibUsb::LibUsb(LibUsb&& other) noexcept :
  mValid(true),
  mContext(other.mContext)
{
  other.mContext = nullptr;
  other.mValid = false;
}


LibUsb& LibUsb::operator=(LibUsb&& other) noexcept {
  if (mValid) {
    libusb_exit(mContext);
  }
  mContext = other.mContext;
  other.mContext = nullptr;
  other.mValid = false;
  return *this;
}


LibUsb::LibUsb(bool useDefaultContext) :
  mValid(true),
  mContext(nullptr)
{
  if (const auto errorCode = libusb_init(useDefaultContext ? nullptr : &mContext); errorCode != 0) {
    throw InitializeError(errorCode);
  }
}


LibUsb::LibUsb() :
  LibUsb(false)
{}


LibUsb::~LibUsb() {
  if (mValid) {
    libusb_exit(mContext);
    mValid = false;
    mContext = nullptr;
  }
}


LibUsb::operator libusb_context* () const {
  return mContext;
}


libusb_context* LibUsb::GetContext() const {
  return mContext;
}


void LibUsb::SetLogLevel(libusb_log_level logLevel) {
  LibUsb::CheckLibUsbResult(libusb_set_option(mContext, LIBUSB_OPTION_LOG_LEVEL, logLevel));
}


void LibUsb::UseUsbDk() {
  LibUsb::CheckLibUsbResult(libusb_set_option(mContext, LIBUSB_OPTION_USE_USBDK));
}
