# Makefile include for uROSnode

ifeq ($(UROS),)
    # Set the default uROSnode root directory to "<CHIBIOS>/ext/uros"
    UROS = $(CHIBIOS)/ext/uros
    $(info <UROS> folder automatically set to: $(UROS))
endif

# Low Level Driver bindings for ChibiOS/RT
UROSLLDSRC += $(UROS)/src/lld/chibios/uros_lld_base.c \
              $(UROS)/src/lld/chibios/uros_lld_node.c \
              $(UROS)/src/lld/chibios/uros_lld_threading.c
