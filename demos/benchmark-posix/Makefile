##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb
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
PROJECT = benchmark-posix

# Target executable file
EXEOBJ = benchmark

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
SRCPATHS   = $(sort $(dir $(CSRC) $(CPPSRC)))

# Various directories
OBJDIR     = $(BUILDDIR)/obj

# Object files groups
COBJS      = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o)))
CPPOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(CPPSRC:.cpp=.o)))
OBJS       = $(COBJS) $(CPPOBJS)

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
$(OBJS) : | $(BUILDDIR)

# Directories setup
$(BUILDDIR) $(OBJDIR):
	mkdir -p $(OBJDIR)
	@echo

# C++ compiler
$(CPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(COPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CPPC) -c $(CPPFLAGS) $(COPT) -I. $(IINCDIR) $< -o $@
endif

# C compiler
$(COBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(COPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $(COPT) -I. $(IINCDIR) $< -o $@
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
