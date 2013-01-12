#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

class Lock;

class Mutex {
  public:
    Mutex();
    ~Mutex();

    Mutex( const Mutex& ) = delete;
    Mutex& operator = ( const Mutex& ) = delete;

    volatile void lock();
    volatile void unlock();
    volatile bool tryLock();
  private:
    pthread_mutex_t impl;
  };

class Lock{
  public:
    Lock( Mutex& m ):m(m){
      m.lock();
      isOk = true;
      }

    enum Mode{
      TryLock
      };

    Lock( Mutex& m, Mode mode ):m(m){
      if( mode==TryLock )
        isOk = m.tryLock();

      //m.lock();
      //isOk = true;
      }

    ~Lock(){
      if( isOk )
        m.unlock();
      }

    operator bool () const {
      return isOk;
      }

    Lock( const Lock& ) = delete;
    Lock& operator = ( const Lock& ) = delete;

  private:
    Mutex & m;
    bool isOk;
  };

#endif // MUTEX_H
