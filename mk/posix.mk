# Makefile include for uROSnode

ifeq ($(UROS),)
  $(error Please define <UROS> so that it points to the uROSnode root folder!)
endif

# Low Level Driver bindings for POSIX
UROSLLDSRC += $(UROS)/src/lld/posix/uros_lld_base.c \
              $(UROS)/src/lld/posix/uros_lld_conn.c \
              $(UROS)/src/lld/posix/uros_lld_threading.c
