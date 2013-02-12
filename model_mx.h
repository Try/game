#ifndef MODEL_MX_H
#define MODEL_MX_H

#include <MyGL/Model>

struct MVertex {
  float x,y,z;
  float u,v;
  float normal[3];
  float color[4];

  float bnormal[4];

  static MyGL::VertexDeclaration::Declarator decl();
  };

class Model : public MyGL::Model<MVertex> {
  public:
    Model();

    void loadMX( MyGL::VertexBufferHolder &vboHolder,
                 MyGL::IndexBufferHolder &iboHolder,
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

    void setModelData(const MyGL::Model<MVertex>& md);

    float cen[3], r, pbounds[3][2];
  };

#endif // MODEL_MX_H
