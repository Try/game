#include "lineedit.h"

#include "resource.h"

using namespace Tempest;

LineEdit::LineEdit(Resource &res):res(res) {
  frame = res.pixmap("gui/colors");
  sedit = 0;
  eedit = 0;

  editable = 1;
  isEdited = false;

  setFocusPolicy( ClickFocus );

  setText(L"Text");

  onFocusChange.bind( *this, &LineEdit::storeText );

  scrool = 0;

  SizePolicy p = sizePolicy();
  p.maxSize.h = font.size() + font.size()/2;
  p.minSize.h = p.maxSize.h;
  p.typeV = FixedMax;

  setSizePolicy(p);
  }

void LineEdit::setText(const std::wstring &t) {
  if( txt!=t ){
    font.fetch(t);
    txt = t;

    sedit = std::max<size_t>(0, std::min(sedit, txt.size() ));
    eedit = std::max<size_t>(0, std::min(eedit, txt.size() ));

    onTextChanged(txt);
    update();
    }
  }

const std::wstring &LineEdit::text() const {
  return txt;
  }

size_t LineEdit::selectionBegin() {
  return sedit;
  }

size_t LineEdit::selectionEnd() {
  return eedit;
  }

void LineEdit::setSelectionBounds(size_t begin, size_t end) {
  if( begin > end )
    std::swap(begin, end);

  begin = std::min( txt.size(), begin );
  end   = std::min( txt.size(), end   );

  sedit = begin;
  eedit = end;
  }

void LineEdit::setEditable(bool e) {
  editable = e;
  }

bool LineEdit::isEditable() const {
  return editable;
  }

void LineEdit::mouseDownEvent(Tempest::MouseEvent &e) {
  sp = e.pos();
  ep = e.pos();

  updateSel();
  update();
  }

void LineEdit::mouseUpEvent(Tempest::MouseEvent &) {
  if( sedit > eedit )
    std::swap( sedit, eedit );
  update();
  }

void LineEdit::mouseDragEvent(MouseEvent &e) {
  if( Rect(0,0,w(),h()).contains( e.pos() ) ){
    ep = e.pos();
    updateSel();
    update();
    }
  }

void LineEdit::paintEvent( Tempest::PaintEvent &pe ) {
  Painter p(pe);

  p.setFont( font );
  p.drawText( scrool, 0, w()-scrool, h(), txt );

  p.setTexture(frame);
  p.setBlendMode( addBlend );

  int x = 0, y = 0;

  size_t s = std::min( sedit, eedit );
  size_t e = std::max( sedit, eedit );

  for( size_t i=0; i<s && i<txt.size(); ++i ){
    Font::Leter l = font.leter(txt[i]);
    x+= l.advance.x;
    y+= l.advance.y;
    }

  int sx = x;

  for( size_t i=s; i<e && i<txt.size(); ++i ){
    Font::Leter l = font.leter(txt[i]);
    x+= l.advance.x;
    y+= l.advance.y;
    }

  int oldSc = scrool;
  if( editable && sx==x ){
    --sx;
    if( x+oldSc > w() ){
      scrool += ( w() - (x+oldSc) );
      //scrool += w()/3;
      }

    if( x+oldSc < 0 ){
      scrool -= (x+oldSc);
      //scrool -= w()/3;
      }

    x += 1;
    }

  p.drawRect( sx+oldSc, 0, x-sx, h(),
              2,0, 1,1 );

  if( hasFocus() ){
    //p.drawRect( 0, 0, w(), h(),
    //            0,0, 1,1 );
    }
  }

void LineEdit::keyDownEvent( KeyEvent &e ) {
  if( e.key==KeyEvent::K_NoKey && editable ){
    wchar_t ch[2] = { wchar_t(e.u16), 0 };
    if( sedit < eedit )
      txt.erase( sedit, eedit-1 );

    txt.insert( sedit, ch );
    ++sedit;
    eedit = sedit;

    isEdited = true;
    onTextChanged( txt );
    onTextEdited(txt);
    update();
    return;
    }

  if( e.key==KeyEvent::K_Return ){
    storeText(0);
    return;
    }

  if( e.key==KeyEvent::K_Left ){
    if( sedit>0 )
      --sedit;

    eedit = sedit;
    update();
    return;
    }

  if( e.key==KeyEvent::K_Right ){
    if( sedit<txt.size() )
      ++sedit;

    eedit = sedit;
    update();
    return;
    }

  if( e.key==KeyEvent::K_Back && editable ){
    if( sedit < eedit )
      txt.erase( sedit, eedit-sedit ); else
    if( sedit > 0 ){
      txt.erase( sedit-1, 1 );
      --sedit;
      }

    eedit = sedit;

    isEdited = true;
    onTextChanged( txt );
    onTextEdited(txt);
    update();
    return;
    }

  if( e.key==KeyEvent::K_Delete && editable ){
    if( sedit < eedit )
      txt.erase( sedit, eedit-sedit ); else
      txt.erase( sedit, 1 );

    eedit = sedit;

    isEdited = true;
    onTextChanged( txt );
    onTextEdited(txt);
    update();
    return;
    }

  e.ignore();
  }

void LineEdit::updateSel() {
  Point a = sp, b = ep;

  if( a.x > b.x )
    std::swap(a,b);

  int x = scrool, y = 0;
  for( size_t i=0; i<txt.size(); ++i ){
    Font::Leter l = font.leter(txt[i]);

    if( Tempest::Rect( l.dpos.x+x, 0,
                        w(), h() ).contains(a) ){
      sedit = i;
      }

    x+= l.advance.x;
    y+= l.advance.y;
    }
  if( Rect( x, y,
            w(), h() ).contains(a) ){
    sedit = txt.size();
    }

  if( a.x >= w() ){
    sedit = txt.size();
    eedit = txt.size();
    return;
    }

  x = scrool;
  y = 0;

  if( b.x >= w() ){
    eedit = txt.size();
    return;
    }

  eedit = sedit;
  for( size_t i=0; i<txt.size(); ++i ){
    Font::Leter l = font.leter(txt[i]);

    if( Rect( l.dpos.x+x, 0,
              w(), h() ).contains(b) ){
      eedit = i;
      }

    x+= l.advance.x;
    y+= l.advance.y;
    }

  if( Rect( x, y,
            w(), h() ).contains(b) ){
    eedit = txt.size();
    }
  }

void LineEdit::storeText(bool) {
  if( isEdited ){
    isEdited = 0;
    onEditingFinished( txt );
    }
  }
