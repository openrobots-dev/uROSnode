# Makefile include for uROSnode

ifeq ($(UROS),)
ifeq ($(CHIBIOS),)
  # Set the default uROSnode root directory to "<CHIBIOS>/ext/uros"
  UROS = $(CHIBIOS)/ext/uros
  $(info <UROS> folder automatically set to: $(UROS))
else
  $(error Please define <UROS> so that it points to the uROSnode root folder!)
endif
endif

# Low Level Driver bindings for LWIP
UROSLLDSRC += $(UROS)/src/lld/lwip/uros_lld_conn.c
