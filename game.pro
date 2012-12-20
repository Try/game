QT    -= core gui

TARGET = cgDemo
QMAKE_CXXFLAGS += -std=gnu++0x -Wall

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
               "$$(FREETYPE_INCLUDE_PATH)/freetype2"

LIBS += -l"gdi32" -l"ws2_32" -l"comdlg32"

LIBS += -L"$$(DXSDK_DIR)Lib/x86" -l"d3d9" -l"d3dx9"
LIBS += -L"$$(CG_LIB_PATH)" -l"cg" -l"cgD3D9"
LIBS += -L"$$(DEVIL_LIB_PATH)" -l"DevIL"
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
    graphics/mainpass.h \
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
    util/gameserializer.h

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
    graphics/mainpass.cpp \
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
    util/gameserializer.cpp

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
    ../game-build-desktop/data/icons.xml

