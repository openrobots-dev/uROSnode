# Makefile include for uROSnode

ifeq ($(UROS),)
  # Set the default uROSnode root directory to "<CHIBIOSOS>/ext/uros"
  UROS = $(CHIBIOS)/ext/uros
  $(info <UROS> folder automatically set to: $(UROS))
endif

# Platform-indipendent definitions
include $(UROS)/common.mk

# Low Level Driver bindings for ChibiOS/RT and LWIP
UROSLLDSRC = $(UROS)/src/lld/chibios/uros_lld_base.c \
             $(UROS)/src/lld/chibios/uros_lld_node.c \
             $(UROS)/src/lld/chibios/uros_lld_threading.c \
             $(UROS)/src/lld/lwip/uros_lld_conn.c
