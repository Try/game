LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE   := main
Tempest_PATH   := C:/Users/Try/Home/Programming/Tempest/Tempest
Tinyxml_PATH   := C:/Users/Try/Home/Programming/SharedLibs/tinyxml
Rapidjson_PATH := C:/Users/Try/Home/Programming/SharedLibs/rapidjson-0.11/rapidjson
Bullet_PATH    := C:/Users/Try/Home/Programming/SharedLibs/bullet-2.80-rev2531/bullet-2.80-rev2531/src

LOCAL_C_INCLUDES := $(Tempest_PATH)/include\
                    $(Tempest_PATH)/math \
                    $(Rapidjson_PATH)/include \
                    $(Tinyxml_PATH)/ \
                    $(Bullet_PATH)/
LOCAL_C_INCLUDES += C:/Users/Try/Home/Programming/android/game_ndk/game/jni/freetype/include

LOCAL_CFLAGS    := -std=c++0x -march=armv7-a
LOCAL_CPPFLAGS  := -D__STDC_INT64__ -Dsigset_t="unsigned int"
LOCAL_CPPFLAGS  += -DTEMPEST_OPENGL -DNO_SOUND
#LOCAL_CPPFLAGS  += -DNO_PHYSIC

LOCAL_SRC_FILES := \
  $(subst $(LOCAL_PATH)/,,\
  $(wildcard $(LOCAL_PATH)/algo/*.cpp) \
  $(wildcard $(LOCAL_PATH)/behavior/*.cpp) \
  $(wildcard $(LOCAL_PATH)/game/*.cpp) \
  $(wildcard $(LOCAL_PATH)/game/missions/*.cpp) \
  $(wildcard $(LOCAL_PATH)/graphics/translate/*.cpp) \
  $(wildcard $(LOCAL_PATH)/graphics/*.cpp) \
  $(wildcard $(LOCAL_PATH)/gui/*.cpp) \
  $(wildcard $(LOCAL_PATH)/gui/gen/*.cpp) \
  $(wildcard $(LOCAL_PATH)/landscape/*.cpp) \
  $(wildcard $(LOCAL_PATH)/lang/*.cpp) \
  $(wildcard $(LOCAL_PATH)/network/*.cpp) \
  $(wildcard $(LOCAL_PATH)/physics/*.cpp) \
  $(wildcard $(LOCAL_PATH)/sound/*.cpp) \
  $(wildcard $(LOCAL_PATH)/threads/*.cpp) \
  $(wildcard $(LOCAL_PATH)/util/*.cpp) \
  $(wildcard $(LOCAL_PATH)/xml/*.cpp) \
  $(wildcard $(LOCAL_PATH)/*.cpp) )

LOCAL_STATIC_LIBRARIES := freetype2 rapidjson bullet
LOCAL_SHARED_LIBRARIES := Tempest

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -ljnigraphics

include $(BUILD_SHARED_LIBRARY)

