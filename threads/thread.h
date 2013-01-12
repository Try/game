#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

class Thread {
  public:
    Thread();
    Thread( const Thread& other ) = delete;
    virtual ~Thread();
    Thread& operator = ( const Thread& t ) = delete;

    void join();
    void cancel();

  protected:
    virtual void run();

  private:
    pthread_t thread;

    static void *runImpl( void* t );
  };

#endif // THREAD_H
