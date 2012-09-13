#ifndef FACTORY_H
#define FACTORY_H

#include <string>
#include <vector>
#include <memory>

#include "behavior/behavior.h"

class GameObject;
class AbstractBehavior;

template< class Base, class ... ConstructArgs >
class Factory{
  public:
    struct AbstractProduct{
      AbstractProduct( const std::string & n ):name(n){}

      virtual Base* create( ConstructArgs... a ) = 0;
      std::string name;
      };

    template< class T >
    struct Product : public AbstractProduct {
      Product( const std::string & n ):AbstractProduct(n){}
      virtual Base* create( ConstructArgs... a ) { return new T( a... ); }
      };

    static Base* create( const std::string & name, ConstructArgs ... a ){
      for( size_t i=0; i<products.size(); ++i )
        if( products[i]->name == name )
          return products[i]->create( a... );

      return 0;
      }

    template< class T >
    static void addProduct( const std::string & name ){
      std::shared_ptr<AbstractProduct> p;
      p.reset( new Product<T>(name) );

      products.push_back(p);
      }
  private:
    static std::vector< std::shared_ptr<AbstractProduct> > products;
  };

template< class Base, class ... ConstructArgs >
std::vector< std::shared_ptr< typename Factory<Base, ConstructArgs...>::AbstractProduct > >
  Factory<Base, ConstructArgs...>::products;

void initFactorys();

typedef Factory<AbstractBehavior, GameObject&, Behavior::Closure&> BehaviorsFactory;

#endif // FACTORY_H
