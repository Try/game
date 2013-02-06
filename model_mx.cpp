#include "model_mx.h"

#include <cstring>
#include <cmath>

MyGL::VertexDeclaration::Declarator MVertex::decl() {
  MyGL::VertexDeclaration::Declarator d;
  d   .add( MyGL::Decl::float3, MyGL::Usage::Position )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord )
      .add( MyGL::Decl::float3, MyGL::Usage::Normal   )
      .add( MyGL::Decl::float4, MyGL::Usage::Color    )
      .add( MyGL::Decl::float4, MyGL::Usage::BiNormal );

  return d;
  }

Model::Model() {
  std::fill( cen, cen+3, 0 );
  }

void Model::computeBiNormal( MVertex &va, MVertex &vb, MVertex &vc ) {
  float a[3] = { va.x-vc.x, va.y-vc.y, va.z-vc.z };
  float b[3] = { vb.x-vc.x, vb.y-vc.y, vb.z-vc.z };

  float t1[2] = { va.u-vc.u, va.v-vc.v };
  float t2[2] = { vb.u-vc.u, vb.v-vc.v };

  if( fabs(t2[1]) > 0.00001 ){
    float k = t1[1]/t2[1];
    float m = ( t1[0]-t2[0]*k );

    float u[4] = { a[0]-b[0]*k, a[1]-b[1]*k, a[2]-b[2]*k, 0 };
    for( int i=0; i<3; ++i )
      u[i] /= m;
    /*
    float l = 0;
    for( int i=0; i<3; ++i )
      l += u[i]*u[i];//u[i]*=m;
    l = sqrt(l);
    for( int i=0; i<3; ++i )
      u[i]/= l;
      */

    for( int i=0; i<4; ++i ){
      va.bnormal[i] = -u[i];
      vb.bnormal[i] = -u[i];
      vc.bnormal[i] = -u[i];
      }
    //float v[3] = {};
    } else {
    float u[4] = { b[0]/t2[0], b[1]/t2[0], b[2]/t2[0], 0 };

    for( int i=0; i<4; ++i ){
      va.bnormal[i] = -u[i];
      vb.bnormal[i] = -u[i];
      vc.bnormal[i] = -u[i];
      }
    }
  }

void Model::setModelData(const MyGL::Model<MVertex> &md) {
  MyGL::Model<MVertex>::operator = (md);

  }

void Model::loadMX( MyGL::VertexBufferHolder & vboHolder,
                    MyGL::IndexBufferHolder  & iboHolder,
                    const std::string &fname) {
  std::fstream fin( fname.data(), std::fstream::in | std::fstream::binary );

  char magic[6] = {};
  fin.read( magic, 5 );

  if( std::string(magic)!="Model" ){
    fin.close();
    return;
    }

  uint16_t ver  = 0,
           size = 0;

  fin.read( (char*)&ver,  sizeof(ver)  );
  fin.read( (char*)&size, sizeof(size) );

  MyGL::Model<MyGL::DefaultVertex>::Raw raw;
  raw.hasIndex = false;
  raw.vertex.resize( size );

  fin.read( (char*)&raw.vertex[0], size*sizeof(MyGL::DefaultVertex) );

  Raw rawN;
  rawN.vertex.resize( raw.vertex.size() );
  for( size_t i=0; i<rawN.vertex.size(); ++i ){
    MVertex             &v = rawN.vertex[i];
    MyGL::DefaultVertex &d = raw.vertex[i];

    memcpy( &v, &d, sizeof(d) );
    std::fill( v.color, v.color+4, 1) ;
    }

  if( rawN.vertex.size()%3==0 ){
    for( size_t i=0; i<rawN.vertex.size(); i+=3 ){
      computeBiNormal( rawN.vertex[i  ],
                       rawN.vertex[i+1],
                       rawN.vertex[i+2] );
      }
    }

  for( size_t r=0; r<rawN.vertex.size(); ++r ){
    cen[0] += rawN.vertex[r].x;
    cen[1] += rawN.vertex[r].y;
    cen[2] += rawN.vertex[r].z;
    }

  for( int r=0; r<3; ++r )
    cen[r] /= rawN.vertex.size();

  load( vboHolder, iboHolder, rawN, MVertex::decl() );

  if( ver>=1 ){
    uint16_t groupsSz = 0;
    fin.read( (char*)&groupsSz,  sizeof(groupsSz)  );
    groups.resize( groupsSz );

    for( size_t i=0; i<groupsSz; ++i ){
      uint16_t begin = 0;
      uint16_t sz    = 0;

      fin.read( (char*)&begin, sizeof(begin) );
      fin.read( (char*)&sz,    sizeof(sz)    );

      groups[i].setModelData( this->slice( begin, sz ) );

      for( size_t r=begin; r<begin+sz; ++r ){
        groups[i].cen[0] += rawN.vertex[r].x;
        groups[i].cen[1] += rawN.vertex[r].y;
        groups[i].cen[2] += rawN.vertex[r].z;
        }

      for( int r=0; r<3; ++r )
        groups[i].cen[r] /= sz;

      float maxR = 0, minR = 1000000;
      for( size_t r=begin; r<begin+sz; ++r ){
        float dx = groups[i].cen[0] - rawN.vertex[r].x;
        float dy = groups[i].cen[1] - rawN.vertex[r].y;
        float dz = groups[i].cen[2] - rawN.vertex[r].z;

        float l = sqrt(dx*dx+dy*dy+dz*dz);
        maxR = std::max(maxR, l);
        minR = std::min(minR, l);
        }

      groups[i].r = (maxR*0.7+minR*0.3);
      }
    }

  fin.close();
  }

float Model::cenX() const {
  return cen[0];
  }

float Model::cenY() const {
  return cen[1];
  }

float Model::cenZ() const {
  return cen[2];
  }

float Model::radius() const {
  return r;
  }
