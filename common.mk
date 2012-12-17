# Makefile include for uROSnode

# Remember to define <UROSLLDSRC> to load low-level, library dependent files.

# Please define <UROS> so that it points to the uROSnode root folder!
ifeq ($(UROS),)
  $(error Please define <UROS> so that it points to the uROSnode root folder!)
endif

# List of all the related source files.
UROSSRC = $(UROS)/src/urosBase.c \
          $(UROS)/src/urosConn.c \
          $(UROS)/src/urosNode.c \
          $(UROS)/src/urosRpcCall.c \
          $(UROS)/src/urosRpcParser.c \
          $(UROS)/src/urosRpcSlave.c \
          $(UROS)/src/urosRpcStreamer.c \
          $(UROS)/src/urosTcpRos.c \
          $(UROS)/src/urosThreading.c

# Required include directories
UROSINC = $(UROS)/include
