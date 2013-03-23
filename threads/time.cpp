#include "time.h"

#include <ctime>

#ifdef __WIN32
#include <windows.h>
#endif

size_t Time::tickCount() {
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
