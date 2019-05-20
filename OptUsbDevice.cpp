#define NOMINMAX

#include "OptUsbDevice.hpp"

#include "Sleep.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "AGBProgram.hpp"

using namespace std::literals;



OptUsbDevice::OptUsbDevice(LibUsb& libusb, unsigned int wait, unsigned int timeout) :
  LibUsbDevice(libusb, VendorId, ProductId),
  mWait(wait),
  mTimeout(timeout)
{}


void OptUsbDevice::SendCommand(std::uint_fast32_t command, std::uint_fast32_t param1, std::uint_fast32_t param2, std::uint_fast32_t param3, std::uint32_t* retBuffer, std::uint_fast32_t retLength) {
  std::byte data[18] = {
    static_cast<std::byte>(UsbCommandCommand),
    //
    static_cast<std::byte>(4),
    //
    static_cast<std::byte>((command >> 0) & 0xFF),
    static_cast<std::byte>((command >> 8) & 0xFF),
    static_cast<std::byte>((command >> 16) & 0xFF),
    static_cast<std::byte>((command >> 24) & 0xFF),
    //
    static_cast<std::byte>((param1 >> 0) & 0xFF),
    static_cast<std::byte>((param1 >> 8) & 0xFF),
    static_cast<std::byte>((param1 >> 16) & 0xFF),
    static_cast<std::byte>((param1 >> 24) & 0xFF),
    //
    static_cast<std::byte>((param2 >> 0) & 0xFF),
    static_cast<std::byte>((param2 >> 8) & 0xFF),
    static_cast<std::byte>((param2 >> 16) & 0xFF),
    static_cast<std::byte>((param2 >> 24) & 0xFF),
    //
    static_cast<std::byte>((param3 >> 0) & 0xFF),
    static_cast<std::byte>((param3 >> 8) & 0xFF),
    static_cast<std::byte>((param3 >> 16) & 0xFF),
    static_cast<std::byte>((param3 >> 24) & 0xFF),
  };
  Send(EndpointCommand, data, sizeof(data), mTimeout);
  if (retLength) {
    std::byte data2[3] = {
      static_cast<std::byte>(UsbCommandRead),
      static_cast<std::byte>((retLength >> 0) & 0xFF),
      static_cast<std::byte>((retLength >> 8) & 0xFF),
    };
    Send(EndpointCommand, data2, sizeof(data2), mTimeout);
    Receive(EndpointData, reinterpret_cast<std::byte*>(retBuffer), retLength * sizeof(std::uint32_t), mTimeout);
  }
}


void OptUsbDevice::SendCommand(std::uint_fast32_t command, std::uint_fast32_t param1, std::uint_fast32_t param2, std::uint_fast32_t param3) {
  SendCommand(command, param1, param2, param3, nullptr, 0);
}


std::uint_fast32_t OptUsbDevice::Receive32(std::uint_fast8_t number) {
  {
    std::byte data[2] = {
      static_cast<std::byte>(UsbCommandStatus),
      static_cast<std::byte>(number),
    };

    Send(EndpointCommand, data, sizeof(data), mTimeout);
  }

  optusbx::Sleep(mWait);

  {
    std::byte data[4]{};
    Receive(EndpointData, data, sizeof(data), mTimeout);

    return
      (static_cast<std::uint_fast32_t>(data[0]) << 0) |
      (static_cast<std::uint_fast32_t>(data[1]) << 8) |
      (static_cast<std::uint_fast32_t>(data[2]) << 16) |
      (static_cast<std::uint_fast32_t>(data[3]) << 24);
  }
}


std::uint_fast32_t OptUsbDevice::Send32(std::uint_fast8_t number, std::uint_fast32_t code) {
  {
    std::byte data[6] = {
      static_cast<std::byte>(UsbCommandStatus),
      static_cast<std::byte>(0x80 | number),
      static_cast<std::byte>((code >> 0) & 0xFF),
      static_cast<std::byte>((code >> 8) & 0xFF),
      static_cast<std::byte>((code >> 16) & 0xFF),
      static_cast<std::byte>((code >> 24) & 0xFF),
    };

    Send(EndpointCommand, data, sizeof(data), mTimeout);
  }

  optusbx::Sleep(mWait);

  {
    std::byte data[4]{};
    Receive(EndpointData, data, sizeof(data), mTimeout);

    return
      (static_cast<std::uint_fast32_t>(data[0]) << 0) |
      (static_cast<std::uint_fast32_t>(data[1]) << 8) |
      (static_cast<std::uint_fast32_t>(data[2]) << 16) |
      (static_cast<std::uint_fast32_t>(data[3]) << 24);
  }
}


void OptUsbDevice::SendProgram(const std::byte* programData, std::size_t programSize) {
  const auto alignedProgramSize = (programSize + 3) & ~static_cast<std::size_t>(3);

  {
    std::byte data[3] = {
      static_cast<std::byte>(UsbCommandWrite),
      // 1 DWORD : program size
      static_cast<std::byte>(1),
      static_cast<std::byte>(0),
    };

    Send(EndpointCommand, data, sizeof(data), mTimeout);
  }

  optusbx::Sleep(mWait);

  {
    std::byte data[4] = {
      static_cast<std::byte>((programSize >> 0) & 0xFF),
      static_cast<std::byte>((programSize >> 8) & 0xFF),
      static_cast<std::byte>((programSize >> 16) & 0xFF),
      static_cast<std::byte>((programSize >> 24) & 0xFF),
    };

    Send(EndpointData, data, sizeof(data), mTimeout);
  }

  optusbx::Sleep(mWait);

  {
    const auto alignedProgramSizeInDWord = alignedProgramSize / 4;

    std::byte data[3] = {
      static_cast<std::byte>(UsbCommandWrite),
      // program size in DWORD
      static_cast<std::byte>((alignedProgramSizeInDWord >> 0) & 0xFF),
      static_cast<std::byte>((alignedProgramSizeInDWord >> 8) & 0xFF),
    };

    Send(EndpointCommand, data, sizeof(data), mTimeout);
  }

  optusbx::Sleep(mWait);

  {
    auto alignedProgramData = std::make_unique<std::byte[]>(alignedProgramSize);
    std::memcpy(alignedProgramData.get(), programData, programSize);
    std::memset(alignedProgramData.get() + programSize, 0, alignedProgramSize - programSize);
    Send(EndpointData, alignedProgramData.get(), alignedProgramSize, mTimeout);
  }
}


void OptUsbDevice::TransferProgram(const std::byte* programData, std::size_t programSize) {
  ClaimInterface(0);

  //

  if (const auto code = Receive32(1); code == 0x12345678) {
    throw std::runtime_error("negotiation error [1]");
  }

  Send32(2, NegotiationMagickNumber);
  if (const auto code = Receive32(2); code != NegotiationMagickNumber) {
    throw std::runtime_error("negotiation error [2]");
  }

  Send32(2, 0x00000000);
  if (const auto code = Receive32(0); !(code & StatusBitReady) || !(code & StatusBitPower)) {
    throw std::runtime_error("negotiation error [3] (GBA not ready)");
  }

  //

  if (const auto code = Receive32(1); code == 0x12345678) {
    throw std::runtime_error("negotiation error [4]");
  }

  SendProgram(programData, programSize);
}


std::vector<std::byte> OptUsbDevice::ReadROM(ProgressCallback progressCallback) {
  TransferProgram(reinterpret_cast<const std::byte*>(gAGBProgram), sizeof(gAGBProgram));

  // wait for program starts
  optusbx::Sleep(150);

  std::uint32_t romInfo[3];
  SendCommand(CMD_ROM_PROBE, 0, 0, 0, romInfo, 3);
  const int_fast32_t romSize = romInfo[1];

  if (progressCallback) {
    progressCallback(0, romSize);
  }

  SendCommand(CMD_READ2, 0x08000000, romSize, 0, NULL, 0);

  std::vector<std::byte> ret(romSize + 3);

  std::byte* ptr = ret.data();
  std::uint_fast32_t restSize = romSize;
  while (restSize) {
    const std::uint_fast32_t readSize = std::min<std::uint_fast32_t>(restSize, 0x1000);
    const std::uint_fast32_t alignedReadSize = (readSize + 3) & ~static_cast<std::uint_fast32_t>(3);
    const std::uint_fast32_t alignedReadSizeInDword = alignedReadSize >> 2;
    std::byte data[64] = {
      static_cast<std::byte>(UsbCommandRead),
      static_cast<std::byte>((alignedReadSizeInDword >> 0) & 0xFF),
      static_cast<std::byte>((alignedReadSizeInDword >> 8) & 0xFF),
      static_cast<std::byte>(7),
      static_cast<std::byte>(0),
    };
    Send(EndpointCommand, data, sizeof(data), mTimeout);
    Receive(EndpointData, ptr, alignedReadSize, mTimeout);
    restSize -= readSize;
    ptr += readSize;

    if (progressCallback) {
      progressCallback(romSize - restSize, romSize);
    }
  }

  ret.resize(romSize);

  return ret;
}