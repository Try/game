#ifndef ARRAY2D_H
#define ARRAY2D_H

#include <vector>
#include <assert.h>

template< class T >
class array2d {
  public:
    array2d():w(0), h(0) {}
    array2d( int iw, int ih, T e = T() ):w(iw), h(ih), data(w*h,e){

      }

    struct LineRef{
      T& operator []( int y ){
        assert( 0<=y && y< maxY );
        return data[y];
        }

      const T& operator []( int y ) const {
        assert( 0<=y && y< maxY );
        return cdata[y];
        }

      private:
        T* data;
        const T* cdata;
        int maxY;

        template< class X>
        friend class array2d;

        LineRef(){}
        LineRef( const LineRef& ){}
      };

    LineRef operator [] ( int x ){
      assert( 0<=x && x< w );

      LineRef r;
      r.data= &data[x*h];
      r.maxY = h;

      return r;
      }

    const LineRef operator [] ( int x ) const{
      assert( 0<=x && x< w );

      LineRef r;
      r.cdata= &data[x*h];
      r.maxY = h;

      return r;
      }

    const T& at( int x, int y ) const{
      return data[y+x*h];
      }

    void resize( int iw, int ih ){
      w = iw;
      h = ih;
      data.resize( w*h );
      }

    bool validate( int x, int y ){
      return ( 0<=x && x<w && 0<=y && y<h );
      }

    int width()  const{ return w; }
    int height() const{ return h; }

    typedef typename std::vector<T>::iterator iterator;

    iterator begin(){ return data.begin(); }
    iterator end()  { return data.end();   }
  private:
    int w, h;
    std::vector<T> data;
  };

#endif // ARRAY2D_H
