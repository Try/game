#ifndef FACTORY_H
#define FACTORY_H

#include <string>
#include <vector>
#include <memory>
#include <typeinfo>

//#include "behavior/behavior.h"

class GameObject;
class AbstractBehavior;

template< class Base, class ... ConstructArgs >
class Factory{
  public:
    struct AbstractProduct{
      AbstractProduct( const std::string & n ):name(n){
        }

      virtual Base* create( ConstructArgs&&... a ) = 0;
      virtual const std::type_info& typeInfo() const = 0;
      std::string name;
      size_t id;
      };

    template< class T >
    struct Product : public AbstractProduct {
      Product( const std::string & n ):AbstractProduct(n){}
      virtual Base* create( ConstructArgs&&... a ) { return new T( a... ); }
      virtual const std::type_info& typeInfo() const {
        return typeid(T);
        }
      };

    static Base* create( const std::string & name, ConstructArgs&& ... a ){
      size_t dummy;
      return create(name, dummy, a...);
      }

    static Base* create( const std::string & name, size_t& id, ConstructArgs&& ... a ){
      for( size_t i=0; i<products.size(); ++i )
        if( products[i]->name == name ){
          id = products[i]->id;
          return products[i]->create( a... );
          }

      id = -1;
      return 0;
      }

    template< class T >
    static void addProduct( const std::string & name ){
      std::shared_ptr<AbstractProduct> p;
      p.reset( new Product<T>(name) );
      p->id = ProductID::template id<T>();

      products.push_back(p);
      }

    struct ProductID{
      template< class P >
      static size_t id(){
        static size_t id = nextid();
        return id;
        }

      private: static size_t nextid(){
        static size_t nid = -1;
        ++nid;
        return nid;
        }
      };

    static size_t productsCount() {
      return products.size();
      }

    static std::string productName( const Base* p ){
      const std::type_info& t = typeid(*p);

      for( size_t i=0; i<products.size(); ++i ){
        if( products[i]->typeInfo()==t )
          return products[i]->name;
        }

      return "";
      }
  private:
    static std::vector< std::shared_ptr<AbstractProduct> > products;
  };

template< class Base, class ... ConstructArgs >
std::vector< std::shared_ptr< typename Factory<Base, ConstructArgs...>::AbstractProduct > >
  Factory<Base, ConstructArgs...>::products;

void initFactorys();

#endif // FACTORY_H
