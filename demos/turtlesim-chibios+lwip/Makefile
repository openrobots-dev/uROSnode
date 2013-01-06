##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -falign-functions=16 -O0 -ggdb
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Enables the use of FPU on Cortex-M4.
# Enable this if you really want to use the STM FWLib.
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

# Enable this if you really want to use the STM FWLib.
ifeq ($(USE_FWLIB),)
  USE_FWLIB = no
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Project name
PROJECT = turtlesim-chibios+lwip

# Target executable file
EXENAME = turtlesim

# Imported source files and paths
CHIBIOS = ../../../..
include $(CHIBIOS)/os/hal/platforms/STM32F4xx/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/ports/GCC/ARMCMx/STM32F4xx/port.mk
include $(CHIBIOS)/os/kernel/kernel.mk
include $(CHIBIOS)/os/various/lwip_bindings/lwip.mk
include $(CHIBIOS)/test/test.mk
UROS = ../..
include $(UROS)/mk/uros.mk
include $(UROS)/mk/chibios.mk
include $(UROS)/mk/lwip.mk
include ./project.mk

# Linker script
#LDSCRIPT = $(PORTLD)/STM32F407xG.ld
LDSCRIPT = $(PORTLD)/STM32F407xG_CCM.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(PORTSRC) \
       $(KERNSRC) \
       $(TESTSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(LWSRC) \
       $(CHIBIOS)/os/various/evtimer.c \
       $(CHIBIOS)/os/various/chprintf.c \
       $(CHIBIOS)/os/various/shell.c \
       $(UROSSRC) $(UROSLLDSRC) \
       $(PROJSRC)

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(PORTASM)

# Included directories
INCDIR = $(PORTINC) $(KERNINC) $(TESTINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(LWINC) \
         $(CHIBIOS)/os/various \
         $(UROSINC) \
         $(PROJINC)

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU   = cortex-m4

#TRGT  = arm-elf-
TRGT  = arm-none-eabi-
CC    = $(TRGT)gcc
CPPC  = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD    = $(TRGT)gcc
#LD    = $(TRGT)g++
CP    = $(TRGT)objcopy
AS    = $(TRGT)gcc -x assembler-with-cpp
OD    = $(TRGT)objdump
HEX   = $(CP) -O ihex
BIN   = $(CP) -O binary
CFLOW = CFLOWRC=cflow.rc cflow

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra

#
# Compiler settings
##############################################################################

##############################################################################
# Default definitions
#

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# Default definitions
##############################################################################

##############################################################################
# User definitions
#

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = -lm

#
# User definitions
##############################################################################

##############################################################################
# Make definitions
#

ifeq ($(USE_FPU),yes)
  USE_OPT += -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fsingle-precision-constant
  DDEFS += -DCORTEX_USE_FPU=TRUE
else
  DDEFS += -DCORTEX_USE_FPU=FALSE
endif

ifeq ($(USE_FWLIB),yes)
  include $(CHIBIOS)/ext/stm32lib/stm32lib.mk
  CSRC += $(STM32SRC)
  INCDIR += $(STM32INC)
  USE_OPT += -DUSE_STDPERIPH_DRIVER
endif

# Output directory and files
ifeq ($(BUILDDIR),)
  BUILDDIR = build
endif
ifeq ($(BUILDDIR),.)
  BUILDDIR = build
endif
OUTFILES   = $(BUILDDIR)/$(EXENAME).elf $(BUILDDIR)/$(EXENAME).hex \
             $(BUILDDIR)/$(EXENAME).bin $(BUILDDIR)/$(EXENAME).dmp

# Automatic compiler options
OPT        = $(USE_OPT)
COPT       = $(USE_COPT)
CPPOPT     = $(USE_CPPOPT)
ifeq ($(USE_LINK_GC),yes)
  OPT += -ffunction-sections -fdata-sections
endif

# Source files groups and paths
ifeq ($(USE_THUMB),yes)
  TCSRC    += $(CSRC)
  TCPPSRC  += $(CPPSRC)
else
  ACSRC    += $(CSRC)
  ACPPSRC  += $(CPPSRC)
endif
ASRC	     = $(ACSRC)$(ACPPSRC)
TSRC	     = $(TCSRC)$(TCPPSRC)
SRCPATHS   = $(sort $(dir $(ASMXSRC)) $(dir $(ASMSRC)) $(dir $(ASRC)) $(dir $(TSRC)))

# Various directories
OBJDIR     = $(BUILDDIR)/obj
LSTDIR     = $(BUILDDIR)/lst
STANDIR    = $(BUILDDIR)/stan

# Object files groups
ACOBJS     = $(addprefix $(OBJDIR)/, $(notdir $(ACSRC:.c=.o)))
ACPPOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(ACPPSRC:.cpp=.o)))
TCOBJS     = $(addprefix $(OBJDIR)/, $(notdir $(TCSRC:.c=.o)))
TCPPOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(TCPPSRC:.cpp=.o)))
ASMOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(ASMSRC:.s=.o)))
ASMXOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(ASMXSRC:.S=.o)))
OBJS	    = $(ASMXOBJS) $(ASMOBJS) $(ACOBJS) $(TCOBJS) $(ACPPOBJS) $(TCPPOBJS)

ACFLOWOBJS = $(addprefix $(STANDIR)/, $(notdir $(ACSRC:.c=.cflow)))
TCFLOWOBJS = $(addprefix $(STANDIR)/, $(notdir $(TCSRC:.c=.cflow)))
ASUOBJS    = $(addprefix $(STANDIR)/, $(notdir $(ACSRC:.c=.su)))
TSUOBJS    = $(addprefix $(STANDIR)/, $(notdir $(TCSRC:.c=.su)))
STANOBJS   = $(ACFLOWOBJS) $(TCFLOWOBJS) $(ASUOBJS) $(TSUOBJS)

# Paths
IINCDIR    = $(patsubst %,-I%,$(INCDIR) $(DINCDIR) $(UINCDIR))
LLIBDIR    = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

# Macros
DEFS       = $(DDEFS) $(UDEFS)
ADEFS 	   = $(DADEFS) $(UADEFS)

# Libs
LIBS       = $(DLIBS) $(ULIBS)

# Various settings
MCFLAGS    = -mcpu=$(MCU)
ODFLAGS	   = -x --syms
ASFLAGS    = $(MCFLAGS) -Wa,-amhls=$(LSTDIR)/$(notdir $(<:.s=.lst)) $(ADEFS)
ASXFLAGS   = $(MCFLAGS) -Wa,-amhls=$(LSTDIR)/$(notdir $(<:.S=.lst)) $(ADEFS)
CFLAGS     = $(MCFLAGS) $(OPT) $(COPT) $(CWARN) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.c=.lst)) $(DEFS)
CPPFLAGS   = $(MCFLAGS) $(OPT) $(CPPOPT) $(CPPWARN) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.cpp=.lst)) $(DEFS)
ifeq ($(USE_LINK_GC),yes)
  LDFLAGS  = $(MCFLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(BUILDDIR)/$(PROJECT).map,--cref,--no-warn-mismatch,--gc-sections $(LLIBDIR)
else
  LDFLAGS  = $(MCFLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(BUILDDIR)/$(PROJECT).map,--cref,--no-warn-mismatch $(LLIBDIR)
endif

# Thumb interwork enabled only if needed because it kills performance.
ifneq ($(TSRC),)
  CFLAGS   += -DTHUMB_PRESENT
  CPPFLAGS += -DTHUMB_PRESENT
  ASFLAGS  += -DTHUMB_PRESENT
  ifneq ($(ASRC),)
    # Mixed ARM and THUMB mode.
    CFLAGS   += -mthumb-interwork
    CPPFLAGS += -mthumb-interwork
    ASFLAGS  += -mthumb-interwork
    LDFLAGS  += -mthumb-interwork
  else
    # Pure THUMB mode, THUMB C code cannot be called by ARM asm code directly.
    CFLAGS   += -mno-thumb-interwork -DTHUMB_NO_INTERWORKING
    CPPFLAGS += -mno-thumb-interwork -DTHUMB_NO_INTERWORKING
    ASFLAGS  += -mno-thumb-interwork -DTHUMB_NO_INTERWORKING -mthumb
    LDFLAGS  += -mno-thumb-interwork -mthumb
  endif
else
  # Pure ARM mode
  CFLAGS   += -mno-thumb-interwork
  CPPFLAGS += -mno-thumb-interwork
  ASFLAGS  += -mno-thumb-interwork
  LDFLAGS  += -mno-thumb-interwork
endif

# Generate dependency information
CFLAGS     += -MD -MP -MF .dep/$(@F).d
CPPFLAGS   += -MD -MP -MF .dep/$(@F).d
CFLOWFLAGS = $(IINCDIR)

# Paths where to search for sources
VPATH      = $(SRCPATHS)

#
# Make definitions
##############################################################################

##############################################################################
# Make targets
#

# Make all
all : $(OBJS) $(OUTFILES) MAKE_ALL_RULE_HOOK

# Static analysis
static-analysis : $(ACFLOWOBJS) $(TCFLOWOBJS) \
                  $(ASUOBJS) $(TSUOBJS) \
                  MAKE_ALL_RULE_HOOK

# Pre-make script
MAKE_ALL_RULE_HOOK :

# Folder constraints
$(OBJS) $(STANOBJS) : | $(BUILDDIR)

# Directories setup
$(BUILDDIR) $(OBJDIR) $(STANDIR):
ifneq ($(USE_VERBOSE_COMPILE),yes)
	@echo === C compiler options ===
	@echo $(CC) -c $(CFLAGS) $(IINCDIR) main.c -o main.o
	@echo
	@echo === cflow options ===
	@echo $(CFLOW) $(CFLOWFLAGS)
	@echo
endif
	mkdir -p $(OBJDIR)
	mkdir -p $(LSTDIR)
	mkdir -p $(STANDIR)

# C++ compiler
$(ACPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CPPC) -c $(CPPFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
endif

$(TCPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CPPC) -c $(CPPFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

# C compiler
$(ACOBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
endif

$(TCOBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

# ASM compiler
$(ASMOBJS) : $(OBJDIR)/%.o : %.s Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(AS) -c $(ASFLAGS) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(AS) -c $(ASFLAGS) -I. $(IINCDIR) $< -o $@
endif

$(ASMXOBJS) : $(OBJDIR)/%.o : %.S Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo 
	$(CC) -c $(ASXFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CC) -c $(ASXFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

# Target executables and linker
%.elf: $(OBJS) $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
else
	@echo Linking $@
	@$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
endif

%.hex: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(HEX) $< $@
else
	@echo Creating $@
	@$(HEX) $< $@
endif

%.bin: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(BIN) $< $@
else
	@echo Creating $@
	@$(BIN) $< $@
endif

%.dmp: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(OD) $(ODFLAGS) $< > $@
else
	@echo Creating $@
	@$(OD) $(ODFLAGS) $< > $@
	@echo Done
endif

# Call analysis by cflow
$(ACFLOWOBJS) : $(STANDIR)/%.cflow : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CFLOW) $(CFLOWFLAGS) $< -o $@
else
	@echo Call graph $<
	@$(CFLOW) $(CFLOWFLAGS) $< -o $@
endif

$(TCFLOWOBJS) : $(STANDIR)/%.cflow : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CFLOW) $(CFLOWFLAGS) $< -o $@
else
	@echo Call graph $<
	@$(CFLOW) $(CFLOWFLAGS) $< -o $@
endif

# Stack analysis by GCC
$(ASUOBJS) : $(STANDIR)/%.su : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) -fstack-usage $(AOPT) -I. $(IINCDIR) $< -o $(STANDIR)/__PHONY__.o
	-mv $(STANDIR)/__PHONY__.su $@
	-rm -f $(STANDIR)/__PHONY__.o
else
	@echo Stack usage  $<
	@$(CC) -c $(CFLAGS) -fstack-usage $(AOPT) -I. $(IINCDIR) $< -o $(STANDIR)/__PHONY__.o
	@mv $(STANDIR)/__PHONY__.su $@
	@rm -f $(STANDIR)/__PHONY__.o
endif

$(TSUOBJS) : $(STANDIR)/%.su : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) -fstack-usage $(TOPT) -I. $(IINCDIR) $< -o $(STANDIR)/__PHONY__.o
	-mv $(STANDIR)/__PHONY__.su $@
	-rm -f $(STANDIR)/__PHONY__.o
else
	@echo Stack usage  $<
	@$(CC) -c $(CFLAGS) -fstack-usage $(TOPT) -I. $(IINCDIR) $< -o $(STANDIR)/__PHONY__.o
	@mv $(STANDIR)/__PHONY__.su $@
	@rm -f $(STANDIR)/__PHONY__.o
endif

# Project cleaner
clean:
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo Cleaning
	-rm -fRv .dep $(BUILDDIR)
	@echo Done
else
	@echo -n Cleaning...\ 
	@rm -fR .dep $(BUILDDIR)
	@echo done
endif

# Include the dependency files, should be the last of the makefile
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

#
# Make targets
##############################################################################
