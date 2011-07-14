#These must be declared, but can ofcourse be altered.
PROJECT_NAME := APlayer
VERSION := 4.0.1

#build date is not currently used.
BUILDDATE := $(shell date +%b_%d_%Y)

OBJDIR := __OBJ_x86
DEPDIR := __DEP_x86
RULESDIR := __RULES_x86
MAKEFILEDIR := makefiles_x86

CC := gcc -c -pipe
LD := gcc

#Dependency generator
#DEP := gcc -M -DBEOS_BUILD
#Ignores system headers. A bit faster, but might give
#small problems if local headers are used with <> insetad of "".
DEP := gcc -MM -DBEOS_BUILD

BUILD_DEBUG=TRUE
#BUILD_DEBUG=FALSE
ifeq ($(BUILD_DEBUG),TRUE)
	CFLAGS += -g -O0
	CFLAGS += -D_DEBUG
	LDFLAGS +=
else
	CFLAGS += -O3
	LDFLAGS +=
endif

ifeq ($(wildcard /boot/develop/headers/be/bone/arpa/inet.h),)
	NETLIBS = network
else
	NETLIBS = socket bind
	CFLAGS += -DBONE_VERSION
endif

CFLAGS += -DBEOS_BUILD
#CFLAGS += -DBUILD_DATE=\"$(BUILDDATE)\"
CFLAGS += -fno-pic
CFLAGS += -ffast-math
CFLAGS += -march=pentium -mcpu=pentium
CFLAGS += -Wall -W -Wno-multichar -Wpointer-arith
CFLAGS += -Wwrite-strings #Causes problems with ASSERT. :/
CFLAGS += -Wall -Wno-multichar -Wno-ctor-dtor-privacy
CFLAGS += -Woverloaded-virtual
CFLAGS += -Wconversion -Wpointer-arith
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
#CFLAGS += -Wcast-qual -Wshadow

APLAYER_LIBS := ../APlayer/bin/BeOS_x86/lib/APlayerKit.so ../APlayer/bin/BeOS_x86/lib/PolyKit.so
APLAYER_INCLUDES := ../PolyKit/Sources ../PolyKit/Sources/BeOS ../APlayer/APlayerKit

#Restriction: No binary can have the same name as any other binary. :/
BINARY_LIST := \
	PolyKit.so \
	APlayerKit.so \
	APlayer \
	$(foreach f, $(wildcard ../APlayer/Agents/*), $(notdir $(f)) ) \
	$(foreach f, $(wildcard ../APlayer/Clients/*),  $(notdir $(f)) ) \
	$(foreach f, $(wildcard ../APlayer/Converters/*),  $(notdir $(f)) ) \
	$(foreach f, $(wildcard ../APlayer/Players/*),  $(notdir $(f)) ) 
#Don't touch unless you know what you are doing.

BEIDE_OBJECTDIRS := \
	\(Objects.* \
	../PolyKit/Sources/BeOS/\(Objects.* \
	../APlayer/APlayerKit/Master/\(Objects.* \
	../APlayer/Server/BeOS/\(Objects.* \
	$(foreach f, $(wildcard ../APlayer/Agents/*), $(f)/BeOS/?Objects.* ) \
	$(foreach f, $(wildcard ../APlayer/Clients/*), $(f)/BeOS/?Objects.* ) \
	$(foreach f, $(wildcard ../APlayer/Converters/*), $(f)/BeOS/?Objects.* ) \
	$(foreach f, $(wildcard ../APlayer/Players/*), $(f)/BeOS/?Objects.* ) 
	
#Set by included makefiles.
ALL_SOURCES :=
ALL_BINARIES :=
ALL_TARGETS := 

EXTRA_MAKEFILES := $(foreach bin, $(BINARY_LIST), $(addprefix $(MAKEFILEDIR)/makefile., $(bin)) )

.PHONY :default depend build clean dist

default: build

depend: build

include $(EXTRA_MAKEFILES)

build : $(ALL_TARGETS)

dist: build $(INSTALL_FILES)
	@echo $(ALL_TARGETS)


clean:
	@rm -rf $(OBJDIR) $(DEPDIR) $(RULESDIR) $(ALL_TARGETS) $(BEIDE_OBJECTDIRS)

$(RULESDIR)/makefile.%.rules: $(MAKEFILEDIR)/makefile.% Makefile.rules Makefile
	@echo Updating $(notdir $@)
	@mkdir -p $(dir $@)
	@rm -f $@
	@$($*_WRITEOBJCPP)
	@$($*_WRITEOBJC)
	@$($*_WRITEOBJASM)
	@$($*_WRITEDEPCPP)
	@$($*_WRITEDEPC)
	@printf '\n' '' >> $@
