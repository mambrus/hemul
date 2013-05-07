LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := sampler
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS += -fPIC
LOCAL_CFLAGS += -DNDEBUG
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libmtime/include

LOCAL_SRC_FILES:= \
   main.c \
   doc.c \
   assert_np.c \
   runengine.c

LOCAL_SHARED_LIBRARIES := libmtime


include $(LOCAL_PATH)/common.mk
include $(BUILD_EXECUTABLE)
$(call import-module,)
$(call import-module,libmtime)
