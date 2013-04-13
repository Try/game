#ifndef STLCONF_H
#define STLCONF_H

#include <utility>
#include <memory>
#include <exception>

#ifdef __ANDROID__

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>

namespace std{
namespace tr1{}
using namespace tr1;

using boost::shared_ptr;
/*
template< class A0, class ... Args >
struct typle{
  typle( const A0& a0, const Args&... args ):a(a0), nested(args...){}

  A0 a;
  typle<Args...> nested;
  };

template< class A0 >
struct typle<A0>{
  typle( const A0& a0 ):a(a0){}

  A0 a;
  };

template< class ... Args >
typle<Args...> tie( const Args&... a){
  return typle<Args..>(a...);
  }
*/

template< class T, class ... Args >
shared_ptr<T> make_shared( Args ... t ){
  return shared_ptr<T>( new T(t...) );
  }

template< class T >
struct unique_ptr: public boost::scoped_ptr<T>{
  unique_ptr( T* t = 0):boost::scoped_ptr<T>(t){}
  }; 

}


#endif

#endif // STLCONF_H
