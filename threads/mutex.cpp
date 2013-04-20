#include "mutex.h"

Mutex::Mutex() {
  pthread_mutex_init( &impl, NULL);
  }

Mutex::~Mutex() {
  pthread_mutex_destroy( &impl );
  }


void Mutex::lock() {
  pthread_mutex_lock(&impl);
  }

void Mutex::unlock() {
  pthread_mutex_unlock(&impl);
  }

bool Mutex::tryLock() {
  return 0==pthread_mutex_trylock(&impl);
  }
