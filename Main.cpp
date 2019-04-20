#pragma comment(lib, "libusb-1.0.lib")

#include "LibUsb.hpp"
#include "OptUsbDevice.hpp"

#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "libusb/libusb.h"

#include "sleep.hpp"

using namespace std::literals;

// references:
// - http://optimize.ath.cx/bootcable/btusb.html
// - http://optimize.ath.cx/bootcable/tech.html



#ifdef _WIN32

// for Windows: use Unicode

int wmain(int argc, wchar_t* argv[]) {
  try {
    if (argc != 2) {
      std::wcerr << L"optusbx v1.0.0"sv << std::endl << L"usage: "sv << argv[0] << L" <data.bin>"sv << std::endl;
      return 1;
    }

    std::ifstream ifs;
    
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
      ifs.open(argv[1], std::ifstream::in | std::ifstream::binary);
    } catch (const std::ios_base::failure& error) {
      throw std::runtime_error("failed to open program file ("s + error.what() + ")"s);
    }

    ifs.seekg(0, std::ifstream::end);
    const std::streamsize fileSize = ifs.tellg();
    if (fileSize > OptUsbDevice::MaxProgramSize) {
      throw std::runtime_error("program file too large (max: 256 KiB)"s);
    }

    auto fileData = std::make_unique<std::byte[]>(static_cast<std::size_t>(fileSize));

    ifs.seekg(0, std::ifstream::beg);
    ifs.read(reinterpret_cast<char*>(fileData.get()), static_cast<std::size_t>(fileSize));

    ifs.close();

    {
      auto& libUsb = LibUsb::GetDefaultInstance();

      //libUsb.SetLogLevel(LIBUSB_LOG_LEVEL_DEBUG);

      std::optional<OptUsbDevice> optUsbDevice;

      try {
        optUsbDevice.emplace(libUsb, 50, 1000);
      } catch (const OptUsbDevice::OpenError& error) {
        throw std::runtime_error("failed to open device; make sure that the device is connected and a proper driver is installed"s);
      }

      optUsbDevice.value().TransferProgram(fileData.get(), fileSize);
    }

    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
  return 1;
}

#else

// TODO: write main function for other platforms

#endif
