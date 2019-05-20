#pragma once

#include <cstddef>
#include <cstdint>

#include "LibUsb.hpp"
#include "LibUsbDevice.hpp"


class OptUsbDevice : public LibUsbDevice {
public:
  static constexpr std::size_t MaxProgramSize = 256 * 1024;    // 256 KiB

  static constexpr std::uint_fast16_t VendorId = 0x0BFE;
  static constexpr std::uint_fast16_t ProductId = 0x3000;

  static constexpr std::uint_fast32_t NegotiationMagickNumber = 0xFEDCBA98;

  static constexpr std::uint_fast8_t EndpointCommand = 0x01;    // cmd_h
  static constexpr std::uint_fast8_t EndpointData    = 0x02;    // i_h (Data | LIBUSB_ENDPOINT_IN) / o_h (Data | LIBUSB_ENDPOINT_OUT)

  static constexpr std::uint_fast8_t UsbCommandCommand = 0x00;    // USB_CMD
  static constexpr std::uint_fast8_t UsbCommandRead    = 0x01;    // USB_READ
  static constexpr std::uint_fast8_t UsbCommandWrite   = 0x02;    // USB_WRITE
  static constexpr std::uint_fast8_t UsbCommandStatus  = 0x03;    // USB_STATUS
  static constexpr std::uint_fast8_t UsbCommandAck     = 0x04;    // USB_ACK

  static constexpr std::uint_fast8_t StatusBitPower = 1 << 0;
  static constexpr std::uint_fast8_t StatusBitReady = 1 << 1;

private:
  unsigned int mWait;
  unsigned int mTimeout;

public:
  OptUsbDevice(LibUsb& libusb, unsigned int wait, unsigned int timeout);

  std::uint_fast32_t Receive32(std::uint_fast8_t number);
  std::uint_fast32_t Send32(std::uint_fast8_t number, std::uint_fast32_t code);
  void SendProgram(const std::byte* programData, std::size_t programSize);
  void TransferProgram(const std::byte* programData, std::size_t programSize);
};
