# Makefile include for uROSnode

ifeq ($(UROS),)
    $(error Please define <UROS> so that it points to the uROSnode root folder!)
endif
ifneq ($(UROSLLDSRC),)
    $(error <UROSLLDSRC> not empty; please include urosnode.mk BEFORE makefile scripts for low-level sources)
endif

# List of all the related source files
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

# Remember to append to <UROSLLDSRC> to list low-level source files
UROSLLDSRC = # Do not append here
