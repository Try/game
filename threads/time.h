#ifndef TIME_H
#define TIME_H

#include <cstddef>
#include <cstdint>

class Time {
  public:
    static size_t tickCount();
    static void sleep(uint64_t msec);
  };

#endif // TIME_H
