#ifndef MODEL_MX_H
#define MODEL_MX_H

#include <Tempest/Model>
#include <Tempest/Half>
#include <cstring>
#include <functional>

struct MVertex {
  Tempest::Half x,y,z,u;
  //Tempest::Half u,v;
  Tempest::Half normal[3], v;
  //Tempest::Half color[4];

  Tempest::Half bnormal[3], h;

  bool operator == ( const MVertex& v ) const {
    return memcmp( this, &v, sizeof(*this) )==0;
    }

  struct hash{
    size_t operator()( const MVertex& mv ) const;
    };

  static const Tempest::VertexDeclaration::Declarator& decl();

  private:
    static Tempest::VertexDeclaration::Declarator mkDecl();
  };

class Model : public Tempest::Model<MVertex> {
  public:
    Model();

    void loadMX( Tempest::VertexBufferHolder &vboHolder,
                 Tempest::IndexBufferHolder &iboHolder,
                 const std::string & f );

    std::vector< Model > groups;

    float cenX() const;
    float cenY() const;
    float cenZ() const;

    float radius() const;

    enum PhysicType{
      Sphere,
      Box
      } physicType;


    float boxSzX() const;
    float boxSzY() const;
    float boxSzZ() const;
  private:
    static void computeBiNormal( MVertex & a,
                                 MVertex & b,
                                 MVertex & c );

    void setModelData(const Tempest::Model<MVertex>& md);

    float cen[3], r, pbounds[3][2];
  };

#endif // MODEL_MX_H
