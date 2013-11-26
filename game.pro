QT    -= core gui
CONFIG -= app_bundle
CONFIG -= qt

TARGET = game
QMAKE_CXXFLAGS += -std=gnu++0x -Wall
#QMAKE_LFLAGS   += -pg

win32:RC_FILE = game.rc

INCLUDEPATH += include

include(game_libs_dir.pri)

CONFIG += directx ogl
CONFIG += sound
CONFIG += physic

ogl:{
  directx:{
    LIBS += -l"Tempest"
    } else {
    LIBS += -l"Tempest_gl"
    }
  } else {
  LIBS += -l"Tempest_dx"
  }

physic:{
  LIBS += -lbullet
  } else {
  DEFINES += NO_PHYSIC
  }

sound:{
  LIBS += -lbass
  } else {
  DEFINES += NO_SOUND
  }

LIBS += -l"gdi32" -l"ws2_32" -l"comdlg32" -l"pthread" -lz
LIBS += -l"d3d9" -l"d3dx9"
LIBS += -l"cg" -l"cgD3D9"
LIBS += -l"DevIL"
LIBS += -l"freetype"

HEADERS += \
    graphics/graphicssystem.h \
    game/gameobject.h \
    resource.h \
    game/protoobject.h \
    physics/physics.h \
    game/envobject.h \
    landscape/terrain.h \
    util/array2d.h \
    game/world.h \
    xml/abstractxmlreader.h \
    prototypesloader.h \
    util/lexicalcast.h \
    gui/maingui.h \
    util/math.h \
    gui/scroolbar.h \
    gui/button.h \
    gui/panel.h \
    util/randomgen.h \
    gui/scroolwidget.h \
    game.h \
    behavior/behavior.h \
    behavior/abstractbehavior.h \
    behavior/behaviormsgqueue.h \
    behavior/movebehavior.h \
    util/factory.h \
    algo/wayfindalgo.h \
    algo/algo.h \
    behavior/buildingbehavior.h \
    game/player.h \
    util/serialize.h \
    behavior/resourcebehavior.h \
    behavior/workerbehavior.h \
    util/weakworldptr.h \
    behavior/warehousebehavior.h \
    pixmapspool.h \
    gui/commandspanel.h \
    behavior/recruterbehavior.h \
    behavior/builderbehavior.h \
    gui/inputhook.h \
    behavior/incompletebuilding.h \
    behavior/warriorbehavior.h \
    gui/overlaywidget.h \
    gui/centralwidget.h \
    gui/listbox.h \
    gui/formbuilder.h \
    gui/propertyeditor.h \
    gui/lineedit.h \
    gui/ingamecontrols.h \
    gui/abstractlistbox.h \
    gui/modalwindow.h \
    gui/ingamemenu.h \
    gui/gen/ui_ingamemenu.h \
    gui/gen/ui_toppanel.h \
    gui/unitlist.h \
    game/bullet.h \
    game/gameobjectview.h \
    gui/nativesavedialog.h \
    util/gameserializer.h \
    gui/gen/ui_loaddialog.h \
    gui/savedialog.h \
    gui/unitinfo.h \
    gui/gen/ui_unitinfo.h \
    gui/textureview.h \
    gui/unitview.h \
    gui/minimapview.h \
    graphics/particlesystem.h \
    graphics/particlesystemengine.h \
    gui/tabwidget.h \
    gui/editterrainpanel.h \
    game/spatialindex.h \
    graphics/hudanim.h \
    landscape/terrainchunk.h \
    graphics/smallobjectsview.h \
    network/localserver.h \
    threads/thread.h \
    threads/async.h \
    network/client.h \
    network/netuser.h \
    util/fileserialize.h \
    util/bytearrayserialize.h \
    threads/mutex.h \
    gui/progressbar.h \
    sound/sound.h \
    behavior/bonusbehavior.h \
    algo/wayfindrequest.h \
    lang/lang.h \
    gui/hintsys.h \
    game/scenario.h \
    game/missions/scenariomission1.h \
    game/ability.h \
    game/spell.h \
    gui/richtext.h \
    model_mx.h \
    gui/missiontargets.h \
    game/particlesystemdeclaration.h \
    graphics/material.h \
    graphics/scene.h \
    graphics/octotree.h \
    threads/time.h \
    util/ifstream.h \
    util/tnloptimize.h \
    gui/graphicssettingswidget.h \
    game/missions/desertstrikescenario.h \
    util/behaviorsfactory.h \
    util/scenariofactory.h \
    game/missions/deatmachscenario.h \
    game/missions/desertstrikescenariowidgets.h \
    gui/checkbox.h \
    gui/gamemessages.h \
    network/netuserclient.h \
    algo/a_star.h \
    graphics/decalobject.h \
    graphics/packetobject.h \
    game/upgrade.h \
    graphics/graphics.h \
    graphics/translate/vertexinputassembly.h \
    graphics/translate/shadersource.h \
    graphics/translate/objectcode.h \
    graphics/translate/buildinfunction.h \
    graphics/frustum.h \
    graphics/translate/mxassembly.h \
    graphics/translate/shadermaterial.h \
    graphics/translate/guiassembly.h \
    graphics/materialserver.h \
    graphics/gbufffer.h \
    graphics/translate/compileoptions.h \
    graphics/translate/operationdef.h \
    gui/loadscreen.h \
    gui/mainmenu.h \
    gui/optionswidget.h \
    gui/mapselectmenu.h \
    game/missions/desertstriketutorialscenario.h \
    game/objectefect.h \
    gamesettings.h \
    gui/closedialog.h

SOURCES += \
    main.cpp \
    graphics/graphicssystem.cpp \
    game/gameobject.cpp \
    resource.cpp \
    game/protoobject.cpp \
    physics/physics.cpp \
    game/envobject.cpp \
    landscape/terrain.cpp \
    util/array2d.cpp \
    game/world.cpp \
    prototypesloader.cpp \
    util/lexicalcast.cpp \
    gui/maingui.cpp \
    util/math.cpp \
    gui/scroolbar.cpp \
    gui/button.cpp \
    gui/panel.cpp \
    util/randomgen.cpp \
    gui/scroolwidget.cpp \
    game.cpp \
    behavior/behavior.cpp \
    behavior/abstractbehavior.cpp \
    behavior/behaviormsgqueue.cpp \
    behavior/movebehavior.cpp \
    util/factory.cpp \
    algo/wayfindalgo.cpp \
    behavior/buildingbehavior.cpp \
    game/player.cpp \
    util/serialize.cpp \
    behavior/resourcebehavior.cpp \
    behavior/workerbehavior.cpp \
    util/weakworldptr.cpp \
    behavior/warehousebehavior.cpp \
    pixmapspool.cpp \
    gui/commandspanel.cpp \
    behavior/recruterbehavior.cpp \
    behavior/builderbehavior.cpp \
    gui/inputhook.cpp \
    behavior/incompletebuilding.cpp \
    behavior/warriorbehavior.cpp \
    gui/overlaywidget.cpp \
    gui/centralwidget.cpp \
    gui/listbox.cpp \
    gui/formbuilder.cpp \
    gui/propertyeditor.cpp \
    gui/lineedit.cpp \
    gui/ingamecontrols.cpp \
    gui/abstractlistbox.cpp \
    gui/modalwindow.cpp \
    gui/ingamemenu.cpp \
    gui/gen/ui_ingamemenu.cpp \
    gui/gen/ui_toppanel.cpp \
    gui/unitlist.cpp \
    game/bullet.cpp \
    game/gameobjectview.cpp \
    gui/nativesavedialog.cpp \
    util/gameserializer.cpp \
    gui/gen/ui_loaddialog.cpp \
    gui/savedialog.cpp \
    gui/unitinfo.cpp \
    gui/gen/ui_unitinfo.cpp \
    gui/textureview.cpp \
    gui/unitview.cpp \
    gui/minimapview.cpp \
    graphics/particlesystem.cpp \
    graphics/particlesystemengine.cpp \
    gui/tabwidget.cpp \
    gui/editterrainpanel.cpp \
    game/spatialindex.cpp \
    graphics/hudanim.cpp \
    landscape/terrainchunk.cpp \
    graphics/smallobjectsview.cpp \
    network/localserver.cpp \
    threads/thread.cpp \
    threads/async.cpp \
    network/client.cpp \
    network/netuser.cpp \
    util/fileserialize.cpp \
    util/bytearrayserialize.cpp \
    threads/mutex.cpp \
    gui/progressbar.cpp \
    sound/sound.cpp \
    behavior/bonusbehavior.cpp \
    algo/wayfindrequest.cpp \
    lang/lang.cpp \
    gui/hintsys.cpp \
    game/scenario.cpp \
    game/missions/scenariomission1.cpp \
    game/ability.cpp \
    game/spell.cpp \
    gui/richtext.cpp \
    model_mx.cpp \
    gui/missiontargets.cpp \
    game/particlesystemdeclaration.cpp \
    graphics/material.cpp \
    graphics/scene.cpp \
    threads/time.cpp \
    util/ifstream.cpp \
    util/tnloptimize.cpp \
    gui/graphicssettingswidget.cpp \
    game/missions/desertstrikescenario.cpp \
    game/missions/deatmachscenario.cpp \
    game/missions/desertstrikescenariowidgets.cpp \
    gui/checkbox.cpp \
    gui/gamemessages.cpp \
    network/netuserclient.cpp \
    graphics/decalobject.cpp \
    graphics/packetobject.cpp \
    game/upgrade.cpp \
    graphics/graphics.cpp \
    graphics/translate/vertexinputassembly.cpp \
    graphics/translate/shadersource.cpp \
    graphics/translate/objectcode.cpp \
    graphics/translate/buildinfunction.cpp \
    graphics/frustum.cpp \
    graphics/translate/mxassembly.cpp \
    graphics/translate/shadermaterial.cpp \
    graphics/translate/guiassembly.cpp \
    graphics/materialserver.cpp \
    graphics/gbufffer.cpp \
    graphics/translate/compileoptions.cpp \
    graphics/translate/operationdef.cpp \
    android_dummy.cpp \
    gui/loadscreen.cpp \
    gui/mainmenu.cpp \
    gui/optionswidget.cpp \
    gui/mapselectmenu.cpp \
    game/missions/desertstriketutorialscenario.cpp \
    game/objectefect.cpp \
    gamesettings.cpp \
    gui/closedialog.cpp

OTHER_FILES += \
    ../game-build-desktop/data/shadow_map.vert \
    ../game-build-desktop/data/shadow_map.frag \
    ../game-build-desktop/data/blitShader.vert \
    ../game-build-desktop/data/blitShader.frag \
    ../game-build-desktop/data/main_material.frag \
    ../game-build-desktop/data/main_material.vert \
    ../game-build-desktop/data/data.xml \
    ../game-build-desktop/data/game.xml \
    ../game-build-desktop/data/shaders.xml \
    ../game-build-desktop/data/pixmaps.xml \
    ../game-build-desktop/data/icons.xml \
    gui/gen/ui_loaddialog.xml \
    ../game-build-desktop/data/game_objects/land.xml \
    ../game-build-desktop/data/spell.xml \
    ../game-build-desktop/data/hud.xml \
    ../game-build-desktop/data/particle.xml \
    Android.mk \
    ../game-build-desktop/data/game_objects/rocks.xml \
    ../game-build-desktop/data/game_objects/bush.xml \
    ../game-build-desktop/data/game_objects/tree.xml \
    ../game-build-desktop/data/upgrades.xml \
    ../game-build-desktop/data/data.json \
    ../game-build-desktop/data/pixmaps.json \
    ../game-build-desktop/data/icons.json \
    ../game-build-desktop/data/shaders.json \
    ../game-build-desktop/data/game.json \
    ../game-build-desktop/data/game_objects/bush.json \
    ../game-build-desktop/data/game_objects/land.json \
    ../game-build-desktop/data/game_objects/rocks.json \
    ../game-build-desktop/data/hud.json \
    ../game-build-desktop/campagin/td.xml \
    ../game-build-desktop/campagin/td.json \
    ../game-build-desktop/data/particle.json \
    ../game-build-desktop/data/spell.json \
    ../game-build-desktop/data/upgrades.json \
    ../game-build-desktop/data/game_objects/tree.json \
    game_libs_dir.pri

