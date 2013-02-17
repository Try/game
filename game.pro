QT    -= core gui

TARGET = game
QMAKE_CXXFLAGS += -std=gnu++0x -Wall
#QMAKE_LFLAGS   += -pg

INCLUDEPATH += include

INCLUDEPATH += "C:/Users/Try/Home/Programming/MyGL2/MyGL/include"
LIBS        += -L"C:/Users/Try/Home/Programming/MyGL2/lib" -l"MyGL"

INCLUDEPATH += "C:/Users/Try/Home/Programming/SharedLibs/tinyxml"
LIBS        += -L"C:/Users/Try/Home/Programming/SharedLibs/tinyxml-build/debug" -l"tinyxml"


INCLUDEPATH += C:/Users/Try/Home/Programming/GuiCore/MyWidget/include
LIBS        += -L"C:/Users/Try/Home/Programming/GuiCore/MyWidget-build-desktop/debug" -l"MyWidget"

INCLUDEPATH += "$$(DXSDK_DIR)/include"\
               "$$(CG_INC_PATH)" \
               "$$(BULLET_INCLUDE_PATH)" \
               "$$(FREETYPE_INCLUDE_PATH)" \
               "$$(FREETYPE_INCLUDE_PATH)/freetype2" \
               "$$(BASS_INCLUDE_PATH)"

LIBS += -l"gdi32" -l"ws2_32" -l"comdlg32" -l"pthread"

LIBS += -L"$$(DXSDK_DIR)Lib/x86" -l"d3d9" -l"d3dx9"
LIBS += -L"$$(CG_LIB_PATH)" -l"cg" -l"cgD3D9"
LIBS += -L"$$(DEVIL_LIB_PATH)" -l"DevIL"
LIBS += -L"$$(BASS_LIB_PATH)" -lbass

LIBS += -L"$$(BULLET_LIB_PATH)" \
        -l"BulletDynamics"   \
        -l"BulletCollision"  \
        -l"BulletFileLoader" \
        -l"BulletMultiThreaded" \
        -l"BulletSoftBody"      \
        -l"BulletWorldImporter" \
        -l"LinearMath"

LIBS += -L"$$(FREETYPE_LIB_PATH)" -l"freetype"

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
    graphics/guipass.h \
    gui/maingui.h \
    graphics/paintergui.h \
    graphics/displacematerial.h \
    graphics/glowmaterial.h \
    util/math.h \
    graphics/transparentmaterial.h \
    gui/scroolbar.h \
    gui/button.h \
    gui/panel.h \
    util/randomgen.h \
    gui/scroolwidget.h \
    graphics/addmaterial.h \
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
    graphics/watermaterial.h \
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
    graphics/mainmaterial.h \
    behavior/warriorbehavior.h \
    gui/overlaywidget.h \
    gui/centralwidget.h \
    gui/listbox.h \
    graphics/omnimaterial.h \
    gui/font.h \
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
    graphics/blushmaterial.h \
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
    graphics/terrainminormaterial.h \
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
    graphics/warfogmaterial.h \
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
    game/missions/deatmachscenarion.h \
    game/particlesystemdeclaration.h \
    graphics/grassmaterial.h

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
    xml/abstractxmlreader.cpp \
    prototypesloader.cpp \
    util/lexicalcast.cpp \
    graphics/guipass.cpp \
    gui/maingui.cpp \
    graphics/paintergui.cpp \
    graphics/displacematerial.cpp \
    graphics/glowmaterial.cpp \
    util/math.cpp \
    graphics/transparentmaterial.cpp \
    gui/scroolbar.cpp \
    gui/button.cpp \
    gui/panel.cpp \
    util/randomgen.cpp \
    gui/scroolwidget.cpp \
    graphics/addmaterial.cpp \
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
    graphics/watermaterial.cpp \
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
    graphics/mainmaterial.cpp \
    behavior/warriorbehavior.cpp \
    gui/overlaywidget.cpp \
    gui/centralwidget.cpp \
    gui/listbox.cpp \
    graphics/omnimaterial.cpp \
    gui/font.cpp \
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
    graphics/blushmaterial.cpp \
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
    graphics/terrainminormaterial.cpp \
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
    graphics/warfogmaterial.cpp \
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
    game/missions/deatmachscenarion.cpp \
    game/particlesystemdeclaration.cpp \
    graphics/grassmaterial.cpp

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
    ../game-build-desktop/data/land.xml \
    ../game-build-desktop/data/spell.xml \
    ../game-build-desktop/data/hud.xml \
    ../game-build-desktop/data/particle.xml

