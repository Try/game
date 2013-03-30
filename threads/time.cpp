#include "time.h"

#include <ctime>
#include <cstdint>

#ifdef __WIN32
#include <windows.h>
#endif

size_t Time::tickCount() {
  uint64_t c = clock();
  c = 1000*c/CLOCKS_PER_SEC;
  return c;

#ifdef __WIN32
 return GetTickCount();
#else
  timespec ts;

  if(clock_gettime(CLOCK_MONOTONIC,&ts) != 0) {
    return -1;
    }

  return 1000*ts.tv_sec + ts.tv_nsec;
#endif
  }
