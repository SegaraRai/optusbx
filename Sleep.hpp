#pragma once

namespace optusbx {
  inline void Sleep(unsigned int milliseconds);
}


#ifdef _WIN32

# include <Windows.h>

namespace optusbx {
  inline void Sleep(unsigned int milliseconds) {
    ::Sleep(milliseconds);
  }
}

#else

# include <unistd.h>

namespace optusbx {
  inline void Sleep(unsigned int milliseconds) {
    const auto seconds = milliseconds / 1000;
    sleep(seconds);
    usleep((milliseconds - seconds * 1000) * 1000);
  }
}

#endif
