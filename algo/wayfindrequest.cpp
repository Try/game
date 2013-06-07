#include "wayfindrequest.h"

#include "game/gameobject.h"
#include "landscape/terrain.h"

#include "algo.h"
#include "behavior/movebehavior.h"

WayFindRequest::WayFindRequest( const Terrain &t ):terr(t), algo(t) {
  visited.reserve(400);
  group  .reserve(400);

  rq.reserve(256);
  }

void WayFindRequest::findWay( int x, int y, GameObject *obj ) {
  if( abs(obj->x()-x)/Terrain::quadSize + abs(obj->y()-y)/Terrain::quadSize < 5 ){
    //int qs = Terrain::quadSize/2;

    algo.findWay( *obj,
                  (obj->x())/Terrain::quadSize,
                  (obj->y())/Terrain::quadSize,
                  x/Terrain::quadSize,
                  y/Terrain::quadSize );
    MoveBehavior *b = obj->behavior.find<MoveBehavior>();
    if( b )
      b->setWay( algo.way );

    return;
    }

  Tg tg;
  tg.x = x/Terrain::quadSize;
  tg.y = y/Terrain::quadSize;

  std::vector<TgGroup>::iterator i = std::lower_bound( rq.begin(), rq.end(), tg );

  if( i==rq.end() ){
    TgGroup g;
    g.x = tg.x;
    g.y = tg.y;
    g.obj.reserve(16);
    g.obj.push_back(obj);

    rq.push_back( g );
    } else {
    if( i->x==tg.x && i->y==tg.y ){
      i->obj.push_back(obj);
      } else {
      TgGroup g;
      g.x = tg.x;
      g.y = tg.y;
      g.obj.reserve(16);
      g.obj.push_back(obj);

      rq.insert( i, g );
      }
    }
  //rq[tg].push_back(obj);
  }

void WayFindRequest::tick() {
  for( auto i=rq.begin(); i!=rq.end(); ++i ){
    //findWay( i->first.x, i->first.y, i->second, terr );
    findWay( i->x, i->y, i->obj, terr );
    }

  rq.clear();
  }

void WayFindRequest::findWay( int x, int y,
                              std::vector<GameObject *> &objs,
                              const Terrain &t ) {
  visited.clear();
  visited.resize(objs.size(), 0);

  for( size_t i=0; i<objs.size(); ++i ){
    if( !visited[i] ){
      visited[i] = true;
      makeGroup( objs[i], objs, visited, group );
      findWayGr( x, y, group, t );
      }
    }
  }

void WayFindRequest::makeGroup( GameObject * obj,
                                std::vector<GameObject *> &objs,
                                std::vector<bool> & visited,
                                std::vector<GameObject *> &gr ) {
  gr.clear();
  gr.push_back(obj);
  //return;

  std::vector< GameObject* > stk[2], *stk1, *stk2;
  stk[0].push_back(obj);

  stk1 = &stk[0];
  stk2 = &stk[1];

  while( stk1->size() ){
    for( size_t i=0; i<stk1->size(); ++i ){
      GameObject & ob = *stk1->at(i);
      for( size_t r=0; r<ob.colisions.size(); ++r )
        for( size_t q=0; q<objs.size(); ++q ){
          if( objs[q]==ob.colisions[r] && !visited[q] ){
            gr.push_back( objs[q] );
            visited[q] = true;
            stk2->push_back( objs[q] );
            }
          }
      }

    std::swap(stk1, stk2);
    stk2->clear();
    }
  }

void WayFindRequest::findWayGr( int x, int y,
                                std::vector<GameObject *> &objs,
                                const Terrain & ) {
  //int qx = x*Terrain::quadSize,
  //    qy = y*Terrain::quadSize;

  /*
  int d = objs[0]->distanceSQ(qx,qy);
  GameObject *obj = objs[0];

  for( size_t i=0; i<objs.size(); ++i ){
    int d2 = objs[i]->distanceSQ(qx,qy);

    if( d2<d ){
      d = d2;
      obj = objs[i];
      }
    }*/
  int cx = 0, cy = 0;
  for( size_t i=0; i<objs.size(); ++i ){
    cx += objs[i]->x()/objs.size();
    cy += objs[i]->y()/objs.size();
    }

  int d = objs[0]->distanceSQ(cx,cy);
  GameObject *obj = objs[0];

  int vMapRef = 0;
  for( size_t i=0; i<objs.size(); ++i ){
    int d2 = objs[i]->distanceSQ(cx,cy);

    if( d2<d ){
      d = d2;
      obj = objs[i];
      }

    if( i*i<=objs.size() )
      vMapRef = i;
    }

  algo.findWay( *obj,
                obj->x()/Terrain::quadSize,
                obj->y()/Terrain::quadSize,
                x,
                y,
                vMapRef );

  for( size_t i=0; i<objs.size(); ++i ){
    MoveBehavior *b = objs[i]->behavior.find<MoveBehavior>();
    if( b )
      b->setWay( algo.way );
    }
  }
