# This is a makefile for complie gsky server
# Org: pwnsky
# Creator: i0gan

GCC     := gcc
CC      := g++ 
CCFLAGS := -O3 -std=c++11 
CCFLAGS += -DDEBUG
CCFLAGS += -DINFO

INCLUDE := -I.
#LDFLAGS := -lpthread

RM      := rm -rf 
CP      := cp -r
MKDIR   := mkdir -p
BUILD_PATH   := ./build
INSTALL_PATH := /usr/lib

# gsky path
NET_PATH    :=  ./gsky/net
NET_PP_PATH :=  ./gsky/net/pp
THREAD_PATH :=  ./gsky/thread
WORK_PATH   :=  ./gsky/work
LOG_PATH    :=  ./gsky/log
THIRD_PATH  :=  ./gsky/third
UTIL_PATH   :=  ./gsky/util
CRYPTO_PATH :=  ./gsky/crypto
MAIN_PATH   :=  ./gsky
LOG_FILE := ./gsky.log

#---------------------OBJ-------------------------
OBJS :=
# main
MAIN_SRC := $(wildcard $(MAIN_PATH)/*.cc)  
MAIN_OBJ := $(patsubst %.cc, %.o, $(MAIN_SRC)) 
OBJS += $(MAIN_OBJ)

# util
UTIL_SRC := $(wildcard $(UTIL_PATH)/*.cc)  
UTIL_OBJ := $(patsubst %.cc, %.o, $(UTIL_SRC)) 
OBJS += $(UTIL_OBJ)

# net
NET_SRC := $(wildcard $(NET_PATH)/*.cc)  
NET_OBJ := $(patsubst %.cc, %.o, $(NET_SRC)) 
OBJS += $(NET_OBJ)

# net
NET_PP_SRC := $(wildcard $(NET_PP_PATH)/*.cc)  
NET_PP_OBJ := $(patsubst %.cc, %.o, $(NET_PP_SRC)) 
OBJS += $(NET_PP_OBJ)

# thread
THREAD_SRC := $(wildcard $(THREAD_PATH)/*.cc)  
THREAD_OBJ := $(patsubst %.cc, %.o, $(THREAD_SRC)) 
OBJS += $(THREAD_OBJ)

# work
WORK_SRC := $(wildcard $(WORK_PATH)/*.cc)  
WORK_OBJ := $(patsubst %.cc, %.o, $(WORK_SRC)) 
OBJS += $(WORK_OBJ)

# log
LOG_SRC := $(wildcard $(LOG_PATH)/*.cc)  
LOG_OBJ := $(patsubst %.cc, %.o, $(LOG_SRC)) 
OBJS += $(LOG_OBJ)

# crypto
CRYPTO_SRC := $(wildcard $(CRYPTO_PATH)/*.cc)  
CRYPTO_OBJ := $(patsubst %.cc, %.o, $(CRYPTO_SRC)) 
OBJS += $(CRYPTO_OBJ)

# complie
libgsky.so:$(OBJS)
	$(CC) -fPIC -shared $^ -o $(BUILD_PATH)/lib/$@
	$(CP) gsky/* $(BUILD_PATH)/include/gsky/
	find $(BUILD_PATH)/include/ -type f -not -name "*.h*" | xargs rm

libgsky.a:$(OBJS)
	$(CC) $^ -o $(BUILD_PATH)/lib/$@ $(CFLAGS)  --static

$(OBJS):%.o:%.cc
	$(CC) -fPIC -shared -c $^ -o $@ $(DEBUGFLAGS) $(INCLUDE) $(CCFLAGS)

print:
	@echo $(COBJS)

run: $(TARGET) server_1
	./server_1 -c conf/gsky.conf

server_1: example/server_1.cc
	$(CC) $< -o $@ -lgsky -lpthread

.PHONY:clean
clean:
	$(RM) $(BUILD_PATH)/lib/*
	$(RM) $(BUILD_PATH)/include/gsky/*
	$(RM) $(OBJS)
	$(RM) $(LOG_FILE)

install:
	@sudo $(CP) $(BUILD_PATH)/include/gsky /usr/include
	@sudo $(CP) $(BUILD_PATH)/lib/libgsky.so /usr/lib
	@echo 'gsky lib has installed'

uninstall:
	@sudo $(RM) /usr/lib/libgsky.so
	@sudo $(RM) /usr/include/gsky
	@echo 'gsky lib has uninstalled'
