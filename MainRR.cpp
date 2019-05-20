#pragma comment(lib, "libusb-1.0.lib")

#include "LibUsb.hpp"
#include "OptUsbDevice.hpp"

#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#ifdef _WIN32
# include <Windows.h>
#endif

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
      std::wcerr << L"optusby-rr v1.0.0"sv << std::endl << L"usage: "sv << argv[0] << L" <data.gba>"sv << std::endl;
      return 1;
    }

    std::ofstream ofs;

    ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    try {
      ofs.open(argv[1], std::ofstream::out | std::ifstream::binary);
    } catch (const std::ios_base::failure & error) {
      throw std::runtime_error("failed to open program file ("s + error.what() + ")"s);
    }

    {
      auto& libUsb = LibUsb::GetDefaultInstance();

      //libUsb.SetLogLevel(LIBUSB_LOG_LEVEL_DEBUG);

      std::optional<OptUsbDevice> optUsbDevice;

      try {
        optUsbDevice.emplace(libUsb, 50, 1000);
      } catch (const OptUsbDevice::OpenError& error) {
        throw std::runtime_error("failed to open device; make sure that the device is connected and a proper driver is installed"s);
      }

      int prevCount = -1;
      DWORD nextUpdateTime = GetTickCount();
      const auto rom = optUsbDevice.value().ReadROM([&prevCount, &nextUpdateTime](std::uint_fast32_t readSize, std::uint_fast32_t totalSize) {
        constexpr int NumProgressIndicators = 32;
        constexpr int UpdateInterval = 40;

        int digits = 0;
        std::uint_fast32_t temp = totalSize;
        do {
          temp /= 10;
          digits++;
        } while (temp);

        const int numCount = readSize * NumProgressIndicators / totalSize;

        if (prevCount == numCount && GetTickCount() < nextUpdateTime) {
          return;
        }
        prevCount = numCount;
        nextUpdateTime = GetTickCount() + UpdateInterval;

        std::wcout << L"\rREADING ["sv;

        int i = 0;
        for (; i < numCount; i++) {
          std::wcout << L"#"sv;
        }
        for (; i < NumProgressIndicators; i++) {
          std::wcout << L" "sv;
        }
        std::wcout << L"] "sv << std::setfill(L'0') << std::setw(digits) << readSize << L" / "sv << totalSize;
      });

      std::wcout << std::endl;

      ofs.write(reinterpret_cast<const char*>(rom.data()), rom.size());
    }

    ofs.close();

    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
  return 1;
}

#else

// TODO: write main function for other platforms

#endif
