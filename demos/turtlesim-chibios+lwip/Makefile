##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -falign-functions=16 -O3 -ggdb
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
COPT       = $(USE_COPT) -mthumb -DTHUMB -fstack-usage -fdump-final-insns
CPPOPT     = $(USE_CPPOPT) -fstack-usage -fdump-final-insns
ifeq ($(USE_LINK_GC),yes)
  OPT += -ffunction-sections -fdata-sections
endif

# Source files groups and paths
SRCPATHS   = $(sort $(dir $(ASMXSRC)) $(dir $(ASMSRC)) $(dir $(CSRC) $(CPPSRC)))

# Various directories
OBJDIR     = $(BUILDDIR)/obj
LSTDIR     = $(BUILDDIR)/lst

# Object files groups
COBJS      = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o)))
CPPOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(CPPSRC:.cpp=.o)))
ASMOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(ASMSRC:.s=.o)))
ASMXOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(ASMXSRC:.S=.o)))
OBJS	     = $(ASMXOBJS) $(ASMOBJS) $(COBJS) $(CPPOBJS)

CGKDOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o.gkd)))
CPPGKDOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(CPPSRC:.cpp=.o.gkd)))
CSUOBJS     = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.su)))
CPPSUOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(CPPSRC:.cpp=.su)))
CNMOBJS     = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o.nm)))
CPPNMOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(CPPSRC:.cpp=.o.nm)))
CSTANOBJS   = $(CGKDOBJS) $(CSUOBJS) $(CNMOBJS)
CPPSTANOBJS = $(CPPGKDOBJS) $(CPPSUOBJS) $(CPPNMOBJS)
STANOBJS    = $(CSTANOBJS) $(CPPSTANOBJS)

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

# Thumb interwork not enabled because it kills performance.
CFLAGS   += -DTHUMB_PRESENT
CPPFLAGS += -DTHUMB_PRESENT
ASFLAGS  += -DTHUMB_PRESENT
CFLAGS   += -mno-thumb-interwork -DTHUMB_NO_INTERWORKING
CPPFLAGS += -mno-thumb-interwork -DTHUMB_NO_INTERWORKING
ASFLAGS  += -mno-thumb-interwork -DTHUMB_NO_INTERWORKING -mthumb
LDFLAGS  += -mno-thumb-interwork -mthumb

# Generate dependency information
CFLAGS     += -MD -MP -MF .dep/$(@F).d
CPPFLAGS   += -MD -MP -MF .dep/$(@F).d

# Paths where to search for sources
VPATH      = $(SRCPATHS)

#
# Make definitions
##############################################################################

##############################################################################
# Make targets
#

# Virtual targets
.PHONY : MAKE_ALL_RULE_HOOK all clean static-analysis \
         list-c-src show-cc-cmd \
         list-cpp-src show-cppc-cmd

# Make all
all : MAKE_ALL_RULE_HOOK $(OBJS) $(OUTFILES)

# Static analysis
static-analysis : MAKE_ALL_RULE_HOOK $(STANOBJS)

# List C source files
list-c-src :
	@for f in $(CSRC); do echo $$f; done

# List C++ source files
list-cpp-src :
	@for f in $(CPPSRC); do echo $$f; done

# Shows the C compiler command (without source/output file options)
show-cc-cmd :
	@echo $(CC) -c $(CFLAGS) $(COPT) -I. $(IINCDIR)

# Shows the C++ compiler command (without source/output file options)
show-cppc-cmd :
	@echo $(CPPC) -c $(CPPFLAGS) $(CPPOPT) -I. $(IINCDIR)

# Pre-make script
MAKE_ALL_RULE_HOOK :
ifneq ($(USE_VERBOSE_COMPILE),yes)
	@echo === CC command example ===
	@echo $(CC) -c $(CFLAGS) $(IINCDIR) main.c -o main.o
	@echo
	@echo === CPPC command example ===
	@echo $(CPPC) -c $(CPPFLAGS) $(IINCDIR) main.cpp -o main.o
	@echo
endif

# Folder constraints
$(OBJS) $(STANOBJS) : | $(BUILDDIR)

# Directories setup
$(BUILDDIR) $(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(LSTDIR)
	@echo

# C++ compiler
$(CPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(COPT) -I. $(IINCDIR) $< -o $@
	-nm $@ > $@.nm
else
	@echo Compiling $<
	@$(CPPC) -c $(CPPFLAGS) $(COPT) -I. $(IINCDIR) $< -o $@
	@nm $@ > $@.nm
endif

# C compiler
$(COBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(COPT) -I. $(IINCDIR) $< -o $@
	-nm $@ > $@.nm
else
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $(COPT) -I. $(IINCDIR) $< -o $@
	@nm $@ > $@.nm
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

# Static analysis objects
$(CGKDOBJS)   : $(OBJDIR)/%.o.gkd : $(OBJDIR)/%.o
$(CPPGKDOBJS) : $(OBJDIR)/%.o.gkd : $(OBJDIR)/%.o
$(CSUOBJS)    : $(OBJDIR)/%.su    : $(OBJDIR)/%.o
$(CPPSUOBJS)  : $(OBJDIR)/%.su    : $(OBJDIR)/%.o
$(CNMOBJS)    : $(OBJDIR)/%.o.nm  : $(OBJDIR)/%.o
$(CPPNMOBJS)  : $(OBJDIR)/%.o.nm  : $(OBJDIR)/%.o

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
