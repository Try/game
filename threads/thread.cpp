#include "thread.h"

#include <iostream>

Thread::Thread():thread(0), valid(1) {
  int th = pthread_create( &thread, 0, &Thread::runImpl, this );
  valid = th==0;

  //std::clog << "Thread::Thread()" << std::endl;
  }

Thread::~Thread() {
  //std::clog << "Thread::~Thread()" << std::endl;
  join();
  }

void Thread::join() {
  if( !valid )
    return;

  valid = false;
  pthread_join(thread,0);
  }

void Thread::cancel() {
  if( !valid )
    return;
  valid = false;
#ifndef __ANDROID__
  pthread_cancel(thread);
#else
  pthread_kill(thread, 1);
  //pthread_kill(thread, SIGUSR1);
#endif
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
