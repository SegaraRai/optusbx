#include "OptUsbDevice.hpp"

#include "Sleep.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

using namespace std::literals;



OptUsbDevice::OptUsbDevice(LibUsb& libusb, unsigned int wait, unsigned int timeout) :
  LibUsbDevice(libusb, VendorId, ProductId),
  mWait(wait),
  mTimeout(timeout)
{}


std::uint_fast32_t OptUsbDevice::Receive32(std::uint_fast8_t number) {
  {
    std::byte data[2] = {
      static_cast<std::byte>(CommandStatus),
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
      static_cast<std::byte>(CommandStatus),
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


void OptUsbDevice::SendProgram(std::byte* programData, std::size_t programSize) {
  const auto alignedProgramSize = (programSize + 3) & ~static_cast<std::size_t>(3);

  {
    std::byte data[3] = {
      static_cast<std::byte>(CommandWrite),
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
      static_cast<std::byte>(CommandWrite),
      // program size in DWORD
      static_cast<std::byte>((alignedProgramSizeInDWord >> 0) & 0xFF),
      static_cast<std::byte>((alignedProgramSizeInDWord >> 8) & 0xFF),
    };

    Send(EndpointCommand, data, sizeof(data), mTimeout);
  }

  optusbx::Sleep(mWait);

  {
    //*
    Send(EndpointData, programData, programSize, mTimeout);

    if (alignedProgramSize != programSize) {
      std::byte zeros[4]{};
      Send(EndpointData, zeros, alignedProgramSize - programSize, mTimeout);
    }
    /*/
    auto alignedProgramData = std::make_unique<std::byte[]>(alignedProgramSize);
    std::memcpy(alignedProgramData.get(), programData, programSize);
    std::memset(alignedProgramData.get() + programSize, 0, alignedProgramSize - programSize);
    Send(EndpointData, alignedProgramData.get(), alignedProgramSize, mTimeout);
    //*/
  }
}


void OptUsbDevice::TransferProgram(std::byte* programData, std::size_t programSize) {
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
