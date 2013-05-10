#include "time.h"

#include <ctime>
#include <cstdint>

#ifdef __WIN32
#include <windows.h>
#endif

#ifdef __ANDROID__
#include <unistd.h>
#endif
#include <time.h>

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

  return 1000*ts.tv_sec + ts.tv_nsec/1000;
#endif
}

void Time::sleep(uint64_t msec) {

#ifdef __WIN32
  Sleep(msec);
#else
  if( msec>=1000)
    sleep(msec/1000);

  if( msec%1000 )
    usleep( 1000*(msec%1000) );
#endif

  //timespec s;
  //s.tv_sec  = msec/1000;
  //s.tv_nsec = (msec%1000)*1000000;

  //nanosleep(&s,0);
  }
