#include "thread.h"

#include <iostream>

Thread::Thread() {
  pthread_create( &thread, 0, &Thread::runImpl, this );

  //std::clog << "Thread::Thread()" << std::endl;
  }

Thread::~Thread() {
  //std::clog << "Thread::~Thread()" << std::endl;
  join();
  }

void Thread::join() {
  pthread_join(thread,0);
  }

void Thread::cancel() {
  pthread_cancel(thread);
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
