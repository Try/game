#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "button.h"

class CheckBox: public Button {
  public:
    CheckBox( Resource & res );

    Tempest::signal<bool> checked;

    Texture imgCheck;

    void setChecked(bool c = true);
    bool isClicked() const;
  protected:
    virtual Tempest::Rect viewRect() const;
    void paintEvent(Tempest::PaintEvent &p);

    void drawBack( Tempest::Painter &p );
    void drawFrame( Tempest::Painter &p );

    void emitClick();
  private:
    bool state;
  };

#endif // CHECKBOX_H
