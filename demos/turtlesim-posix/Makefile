##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O0 -ggdb
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = -ansi -pedantic
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
PROJECT = turtlesim-posix

# Target executable file
EXEOBJ = turtlesim

# Imported source files and paths
UROS = ../..
include $(UROS)/mk/uros.mk
include $(UROS)/mk/posix.mk
include ./project.mk

# C sources
CSRC = $(UROSSRC) $(UROSLLDSRC) \
       $(PROJSRC)

# C++ sources
CPPSRC =

# Included directories
INCDIR = $(UROSINC) \
         $(PROJINC)

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

CC    = gcc
CPPC  = g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD    = gcc
#LD    = g++
CP    = objcopy
OD    = objdump
CFLOW = CFLOWRC=cflow.rc cflow

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

# List all default directories to look for include files here
DINCDIR = .

# List the default directory to look for the libraries here
DLIBDIR = .

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

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = -lpthread -lm

#
# User definitions
##############################################################################

##############################################################################
# Make definitions
#

# Output directory and files
ifeq ($(BUILDDIR),)
  BUILDDIR = build
endif
ifeq ($(BUILDDIR),.)
  BUILDDIR = build
endif
OUTFILES   = $(BUILDDIR)/$(EXEOBJ)

# Automatic compiler options
OPT        = $(USE_OPT)
COPT       = $(USE_COPT)
CPPOPT     = $(USE_CPPOPT)

# Source files groups and paths
SRCPATHS   = $(sort $(dir $(CSRC)$(CPPSRC)))

# Various directories
OBJDIR     = $(BUILDDIR)/obj
STANDIR    = $(BUILDDIR)/stan

# Object files groups
COBJS      = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o)))
CPPOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(CPPSRC:.cpp=.o)))
OBJS       = $(ASMOBJS) $(COBJS) $(CPPOBJS)

CFLOWOBJS  = $(addprefix $(STANDIR)/, $(notdir $(CSRC:.c=.cflow)))
SUOBJS     = $(addprefix $(STANDIR)/, $(notdir $(CSRC:.c=.su)))
STANOBJS   = $(CFLOWOBJS) $(SUOBJS)

# Paths
IINCDIR    = $(patsubst %,-I%,$(INCDIR) $(DINCDIR) $(UINCDIR))
LLIBDIR    = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

# Macros
DEFS       = $(DDEFS) $(UDEFS)

# Libs
LIBS       = $(DLIBS) $(ULIBS)

# Various settings
CFLAGS     = $(OPT) $(COPT) $(CWARN) $(DEFS)
CPPFLAGS   = $(OPT) $(CPPOPT) $(CPPWARN) $(DEFS)
LDFLAGS    = 

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
static-analysis : $(CFLOWOBJS) $(SUOBJS) MAKE_ALL_RULE_HOOK

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
	mkdir -p $(STANDIR)

# C++ compiler
$(CPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CPPC) -c $(CPPFLAGS) $(IINCDIR) $< -o $@
endif

# C compiler
$(COBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $(IINCDIR) $< -o $@
endif

# Target executable linker
$(BUILDDIR)/$(EXEOBJ) : $(OBJS) $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
else
	@echo Linking $@
	@$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
endif

# Call analysis by cflow
$(CFLOWOBJS) : $(STANDIR)/%.cflow : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CFLOW) $(CFLOWFLAGS) $< -o $@
else
	@echo Call graph $<
	@$(CFLOW) $(CFLOWFLAGS) $< -o $@
endif

# Stack analysis by GCC
$(SUOBJS) : $(STANDIR)/%.su : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) -fstack-usage $(IINCDIR) $< -o $(STANDIR)/__PHONY__.o
	-mv $(STANDIR)/__PHONY__.su $@
	-rm -f $(STANDIR)/__PHONY__.o
else
	@echo Stack usage  $<
	@$(CC) -c $(CFLAGS) -fstack-usage $(IINCDIR) $< -o $(STANDIR)/__PHONY__.o
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
