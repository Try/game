#ifndef ASYNC_H
#define ASYNC_H

#include "stlconf.h"
#include "thread.h"
#include "mutex.h"
#include <vector>

#include <memory>

class Future;

class AsyncThread : public Thread {
  explicit AsyncThread( void (*f)(void*),
                        void* args ):
    Thread(), foo(f), args(args){
    //qDebug() << "AsyncThread::AsyncThread()";
    }

  public:
    ~AsyncThread(){
      //qDebug() << "AsyncThread::~AsyncThread()";
      }

  private:
    void run(){
      foo( args );
      }

    void (* foo)(void*);
    void  * args;


    template< class F >
    friend Future async( F *f, void* args );

    template< class C, class F >
    friend Future async( C *c, F f, void* args );
  };

struct Task{
  void (* foo)(void*);
  void  * args;

  void run(){
    foo( args );
    }
  };

class Future{
  public:

    void join(){
      if( th )
        th->join();
      }

    void cancel(){
      if( th )
        th->cancel();

      th.reset();
      }

  private:
    std::shared_ptr<AsyncThread> th;

    template< class F >
    friend Future async( F *f, void* args );

    template< class C, class F >
    friend Future async( C *c, F f, void* args );
  };

template< class F >
Future async( F *f, void* args ){
  Future ft;
  ft.th.reset( new AsyncThread(f, args) );

  return ft;
  }

template< class C, class F >
Future async( C *c, F f, void* args ){
  struct L{
    static void foo( void * in ){
      L &l = *((L*)in);
      C & c = *l.c;

      (c.*l.f)(l.args);

      delete &l;
      }

    void* args;
    C   * c;
    F     f;
    } *l = new L();
  l->c = c;
  l->f = f;
  l->args = args;


  Future ft;
  ft.th.reset( new AsyncThread(L::foo, (void*)l) );
  //t->start();

  return ft;
  }

#endif // ASYNC_H
