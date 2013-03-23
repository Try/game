LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main
LOCAL_CFLAGS := -std=gnu++11

Tempest_PATH := C:/Users/Try/Home/Programming/Tempest/Tempest
Tinyxml_PATH := C:/Users/Try/Home/Programming/SharedLibs/tinyxml

LOCAL_C_INCLUDES := $(Tempest_PATH)/include\
                    $(Tempest_PATH)/math \
                    $(Tinyxml_PATH)/
LOCAL_C_INCLUDES += freetype/include
LOCAL_C_INCLUDES += C:/Users/Try/Home/Programming/SharedLibs/boost_1_52_0/boost_1_52_0

LOCAL_CFLAGS := -std=gnu++11 -DTEMPEST_OPENGL -DNO_PHYSIC -DNO_SOUND
LOCAL_CFLAGS += -D_STLP_NO_EXCEPTIONS

LOCAL_SRC_FILES := \
  $(subst $(LOCAL_PATH)/,,\
  $(wildcard $(LOCAL_PATH)/algo/*.cpp) \
  $(wildcard $(LOCAL_PATH)/behavior/*.cpp) \
  $(wildcard $(LOCAL_PATH)/game/*.cpp) \
  $(wildcard $(LOCAL_PATH)/game/missions/*.cpp) \
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

LOCAL_STATIC_LIBRARIES := freetype2 tinyxml
LOCAL_SHARED_LIBRARIES := Tempest

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -ljnigraphics

include $(BUILD_SHARED_LIBRARY)

