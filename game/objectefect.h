#ifndef OBJECTEFECT_H
#define OBJECTEFECT_H

class ObjectEfect {
  public:
    ObjectEfect();
    virtual ~ObjectEfect();

    virtual void tick();
    virtual bool isFinished() const;
  };

class HpDropEfect: public ObjectEfect {

  };

#endif // OBJECTEFECT_H
