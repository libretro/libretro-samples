LOCAL_PATH := $(call my-dir)

CORE_DIR := $(LOCAL_PATH)/..

include $(LOCAL_PATH)/../Makefile.common

COREFLAGS := $(COREDEFINES) -DPSS_STYLE=1 $(INCFLAGS)

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
	COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE    := retro
LOCAL_SRC_FILES := $(SOURCES_C) $(SOURCES_CXX)
LOCAL_CFLAGS    := $(COREFLAGS)
LOCAL_CXXFLAGS  := $(COREFLAGS)
LOCAL_LDFLAGS   := -Wl,-version-script=$(CORE_DIR)/link.T
include $(BUILD_SHARED_LIBRARY)
