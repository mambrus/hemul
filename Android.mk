LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := hemul
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS += -fPIC
LOCAL_CFLAGS += -DNDEBUG -DHAVE_ANDROID_OS
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libmtime/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/patchlibc/include

ifndef LIB_DYNAMIC
LOCAL_LDFLAGS += -Wl,--undefined=__mtime_init
endif

LOCAL_SRC_FILES := \
   doc.c \
   init.c \
   assert_np.c \
   runengine.c \
   userio.c \
   main.c

LOCAL_SHARED_LIBRARIES := libmtime libmqueue

LOCAL_LDLIBS += -lm

include $(LOCAL_PATH)/common.mk
include $(BUILD_EXECUTABLE)
$(call import-module,libmtime)
$(call import-module,libmqueue)
