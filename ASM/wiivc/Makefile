ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPro/devkitPPC)
endif

include $(DEVKITPPC)/wii_rules

PREFIX      	= $(DEVKITPPC)/bin/powerpc-eabi-
CC				= $(PREFIX)gcc
LD				= $(CC)
OBJCOPY			= $(PREFIX)objcopy
TARGET      	= mwserial
CFILES			= *.c
SRCDIR	 		= src
INCDIR			= include
OBJDIR			= obj
BINDIR			= bin
OBJCOPYFLAGS	:= -S -O binary --set-section-flags .bss=alloc,load,contents $(OBJCOPYFLAGS)
ADDRESS			= 0x817F0000
VC_VERSIONS 	= usa jpn

#------------------------------------------------------------------------------
# Stub binaries for patching into each supported region release of VC
#------------------------------------------------------------------------------

define bin_template
OBJDIR-$(1)		= $$(OBJDIR)/$(1)
BINDIR-$(1)		= $$(BINDIR)/$(1)
OUTDIR-$(1)		= $$(OBJDIR-$(1)) $$(BINDIR-$(1))
SRCDIR-$(1)		= src
CSRC-$(1)		:= $$(foreach s,$$(CFILES),$$(wildcard $$(SRCDIR-$(1))/$$(s)))
COBJ-$(1)		= $$(patsubst $$(SRCDIR-$(1))/%.c,$$(OBJDIR-$(1))/%.o,$$(CSRC-$(1)))
ELF-$(1)		= $$(BINDIR-$(1))/$$(TARGET)-$(1).elf
BIN-$(1)		= $$(BINDIR-$(1))/$$(TARGET)-$(1).bin
CFLAGS-$(1)		= -c -std=gnu11 -G 0 -O0 -Wall -Wextra \
					-mrvl -mcpu=750 -meabi -mhard-float \
					-ffunction-sections -fdata-sections \
					-I$$(INCDIR) -DIS_OOT -DIS_WII -DVC_VERSION=$(1)
LDFLAGS-$(1)	= -Wl,-Map,$$(BINDIR-$(1))/$$(TARGET)-$(1).map \
					-T build.ld -G 0 -nostartfiles -specs=nosys.specs \
					-Wl,--gc-sections,--section-start,.init=$$(ADDRESS),--defsym,init=$$(ADDRESS)

$$(BIN-$(1)): $$(ELF-$(1)) | $$(BINDIR-$(1))
	$$(OBJCOPY) $$(OBJCOPYFLAGS) $$< $$@

$$(ELF-$(1)): $$(COBJ-$(1)) | $$(BINDIR-$(1))
	$$(LD) -T lib/vcSymbols-$(1).ld $$(LDFLAGS-$(1)) $$(COBJ-$(1)) -o $$@

$$(COBJ-$(1)): $$(OBJDIR-$(1))/%.o : $$(SRCDIR-$(1))/%.c | $$(OBJDIR-$(1))
	$$(CC) $$(CFLAGS-$(1)) $$< -o $$@

$$(OUTDIR-$(1)):
	mkdir -p $$@
endef

$(foreach v,$(VC_VERSIONS),$(eval $(call bin_template,$(v))))

#------------------------------------------------------------------------------
# Build rules
#------------------------------------------------------------------------------

MWSERIAL 		= $(foreach v,$(VC_VERSIONS),$(BIN-$(v)))

.PHONY: all clean

all: clean $(MWSERIAL)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
