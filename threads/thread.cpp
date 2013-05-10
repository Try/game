#include "thread.h"

#include <iostream>

Thread::Thread():valid(1) {
  pthread_create( &thread, 0, &Thread::runImpl, this );

  //std::clog << "Thread::Thread()" << std::endl;
  }

Thread::~Thread() {
  //std::clog << "Thread::~Thread()" << std::endl;
  join();
  }

void Thread::join() {
  if( !valid )
    return;

  pthread_join(thread,0);
  valid = false;
  }

void Thread::cancel() {
  if( !valid )
    return;
#ifndef __ANDROID__
  pthread_cancel(thread);
#else
  pthread_kill(thread, 1);
  //pthread_kill(thread, SIGUSR1);
#endif
  valid = false;
  }

void Thread::run() {
  }

void* Thread::runImpl( void *t ) {
  Thread &th = *((Thread*)t);
  th.run();

  //delete &th;
  pthread_exit(0);
  return 0;
  }
