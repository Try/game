#ifndef BUILDINFUNCTION_H
#define BUILDINFUNCTION_H

#include <vector>
#include <memory>
#include <string>

class BuildInFunction {
  public:
    BuildInFunction();

    enum ArgsSize{
      asMinimaize,
      asMaximize,
      asDontCare,
      asSetTo3
      };

    typedef void(*F1)( const float*, int ,
                       float *, int&  );
    typedef void(*F2)( const float*, int ,
                       const float*, int ,
                       float *, int&  );

    void add( const std::string& name,
              F2 foo,
              ArgsSize sz = asMaximize ){
      Func2<F2>* fx = new Func2<F2>();
      fx->foo   = foo;
      fx->name  = name;
      fx->argsS = sz;

      f2.push_back( std::shared_ptr<AbstractFunc2>(fx) );
      }

    void add( const std::string& name,
              F1 foo ){
      Func1<F1>* fx = new Func1<F1>();
      fx->foo  = foo;
      fx->name = name;

      f1.push_back( std::shared_ptr<AbstractFunc1>(fx) );
      }

    int outSz( const std::string& f, int s );
    int outSz( const std::string& f, int s, int s1 );

    ArgsSize argsSz( const std::string& s );

    void exec( const std::string &f,
               const float* a, int sa,
               float *out, int& osz ) const;
    void exec( const std::string &f,
               const float* a, int sa,
               const float* b, int sb,
               float *out, int& osz ) const;

    int argsCount( const std::string& f ) const;
  private:
    struct AbstractFunc1 {
      virtual void exec( const float* a, int sa,
                         float *out, int& osz ) const = 0;
      virtual int outSize(int a) const = 0;

      std::string name;
      };

    struct AbstractFunc2 {
      virtual void exec( const float* a, int sa,
                         const float *b, int sb,
                         float *out, int& osz ) const = 0;
      virtual int outSize(int a, int b) const = 0;

      std::string name;
      ArgsSize    argsS;
      };

    template< class F >
    struct Func1:AbstractFunc1 {
      void exec( const float* a, int sa,
                 float *out, int& osz ) const {
        (*foo)(a, sa, out, osz);
        }

      int outSize(int a) const {
        int sz;
        (*foo)(0, a, 0, sz);

        return sz;
        }

      F1 foo;
      };

    template< class F >
    struct Func2:AbstractFunc2 {
      void exec( const float* a, int sa,
                 const float *b, int sb,
                 float *out, int& osz ) const {
        (*foo)(a, sa, b, sb, out, osz);
        }

      int outSize(int a, int b) const {
        int sz;
        (*foo)(0, a, 0, b, 0, sz);

        return sz;
        }

      F2 foo;
      };

    std::vector< std::shared_ptr<AbstractFunc1> > f1;
    std::vector< std::shared_ptr<AbstractFunc2> > f2;

    static void dot( const float* a, int sa,
                     const float *b, int sb,
                     float *out, int& osz );    
    static void min( const float* a, int sa,
                     const float *b, int sb,
                     float *out, int& osz );
    static void max( const float* a, int sa,
                     const float *b, int sb,
                     float *out, int& osz );
    static void pow( const float* a, int sa,
                     const float *b, int sb,
                     float *out, int& osz );
    static void reflect( const float* a, int sa,
                         const float *b, int sb,
                         float *out, int& osz );
    static void cross( const float* a, int sa,
                       const float *b, int sb,
                       float *out, int& osz );

    static void length( const float* a, int sa,
                        float *out, int& osz );
    static void normalize( const float* a, int sa,
                           float *out, int& osz );
    static void sin( const float* a, int sa,
                     float *out, int& osz );
    static void cos( const float* a, int sa,
                     float *out, int& osz );
  };

#endif // BUILDINFUNCTION_H
