#include "missiontargets.h"

#include <Tempest/Painter>
#include "maingui.h"

#include "richtext.h"
#include "lang/lang.h"

#include "game.h"

MissionTargets::MissionTargets( Game & game, Resource &res )
               :game(game), res(res) {
  frame = res.pixmap("gui/hintFrame");

  ckFrame = res.pixmap("gui/ckBoxFrame");
  ck      = res.pixmap("gui/ckBox");

  game.updateMissionTargets.bind( this, &MissionTargets::setupTagets);

  setupTagets();
  }

void MissionTargets::paintEvent(Tempest::PaintEvent &e) {
  Tempest::Painter p(e);

  p.setBlendMode( Tempest::alphaBlend );
  MainGui::drawFrame(p, frame, box->pos(), box->size() );

  for( size_t i=0; i<box->layout().widgets().size(); ++i ){
    Tempest::Widget *wx = box->layout().widgets()[i];
    int sz = wx->h();
    p.setTexture( ckFrame );
    p.drawRect( 7, wx->pos().y, sz, sz,
                 0, 0, ckFrame.width(), ckFrame.height() );

    if( game.scenario().tagets()[i].done ){
      p.setTexture( ck );
      p.drawRect( 7, wx->pos().y, sz, sz,
                  0, 0, ck.width(), ck.height() );
      }
    }

  paintNested(e);
  }

void MissionTargets::setupTagets() {
  layout().removeAll();

  int w = 180, h = 20;

  setLayout( Tempest::Vertical );

  box = new Tempest::Widget();
  box->setLayout( Tempest::Vertical );

  for( size_t i=0; i<game.scenario().tagets().size(); ++i ){
    RichText *t = new RichText(res);
    std::wstring str = Lang::tr( game.scenario().tagets()[i].hint );
    t->setText( str );

    Tempest::Size sz = RichText::bounds(res, str);
    t->setMaximumSize( sz );

    w = std::max(w, sz.w);
    h+= sz.h;

    box->layout().add( t );
    }

  box->setMaximumSize( w+40, h );
  box->layout().setMargin( 20, 10, 10, 10 );

  box->setSizePolicy( Tempest::FixedMax );
  setMaximumSize( box->sizePolicy().maxSize.w, sizePolicy().maxSize.h );

  setVisible( box->layout().widgets().size() );

  layout().add( box );
  layout().add( new Widget() );
  }
