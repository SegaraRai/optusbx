#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "LibUsb.hpp"
#include "LibUsbDevice.hpp"


class OptUsbDevice : public LibUsbDevice {
public:
  using ProgressCallback = std::function<void(std::uint_fast32_t read, std::uint_fast32_t total)>;

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

  static constexpr std::uint_fast32_t CMD_ROM_PROBE = 0xFFFF0001;     //
  static constexpr std::uint_fast32_t CMD_WRAM_LOADEXEC = 0xFFFF0002; // len
  static constexpr std::uint_fast32_t CMD_ROM_BERASE = 0xFFFF0003;    // add
  static constexpr std::uint_fast32_t CMD_ROM_WRITE = 0xFFFF0004;     // add, len
  static constexpr std::uint_fast32_t CMD_ROM_BWRITE = 0xFFFF0005;    // addw, lenw
  static constexpr std::uint_fast32_t CMD_READ = 0xFFFF0006;          // add, len
  static constexpr std::uint_fast32_t CMD_READ2 = 0xFFFF0012;         // add, len (ULTRA)
  static constexpr std::uint_fast32_t CMD_WRITE = 0xFFFF0007;         // add, len
  static constexpr std::uint_fast32_t CMD_FIND = 0xFFFF0008;          // add, len, strlen
  static constexpr std::uint_fast32_t CMD_BOOT_ROM = 0xFFFF0009;      //
  static constexpr std::uint_fast32_t CMD_SRAM2EWRAM = 0xFFFF000A;    // bank   ERAM:0x02000000
  static constexpr std::uint_fast32_t CMD_EWRAM2SRAM = 0xFFFF000B;    // bank   ERAM:0x02000000
  static constexpr std::uint_fast32_t CMD_BU_PROBE = 0xFFFF000C;      // ret 0:none 1:SRAM256K 2:SRAM512K 3:FLASH512 4:EEP4K 5:EEP64K 6:FLASH1M 7:FLASH8M
  static constexpr std::uint_fast32_t CMD_SRAM_READ = 0xFFFF000D;     // ofs, len
  static constexpr std::uint_fast32_t CMD_EEP_READ = 0xFFFF000E;      // ofs, len
  static constexpr std::uint_fast32_t CMD_EEP_WRITE = 0xFFFF000F;     // ofs, len
  static constexpr std::uint_fast32_t CMD_SRAM_WRITE = 0xFFFF0010;    // ofs, len
  static constexpr std::uint_fast32_t CMD_FLASH_WRITE = 0xFFFF0011;   // ofs, len
  static constexpr std::uint_fast32_t CMD_BLANK = 0xFFFF0020;         //
  static constexpr std::uint_fast32_t CMD_IS_PRO = 0xFFFF0021;        //


private:
  unsigned int mWait;
  unsigned int mTimeout;

public:
  OptUsbDevice(LibUsb& libusb, unsigned int wait, unsigned int timeout);

  void SendCommand(std::uint_fast32_t command, std::uint_fast32_t param1, std::uint_fast32_t param2, std::uint_fast32_t param3, std::uint32_t* retBuffer, std::uint_fast32_t retLength);
  void SendCommand(std::uint_fast32_t command, std::uint_fast32_t param1, std::uint_fast32_t param2, std::uint_fast32_t param3);
  std::uint_fast32_t Receive32(std::uint_fast8_t number);
  std::uint_fast32_t Send32(std::uint_fast8_t number, std::uint_fast32_t code);
  void SendProgram(const std::byte* programData, std::size_t programSize);
  void TransferProgram(const std::byte* programData, std::size_t programSize);
  std::vector<std::byte> ReadROM(ProgressCallback progressCallback);
};
