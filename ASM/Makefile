CC = mips64-gcc
LD = mips64-ld
OBJDUMP = mips64-objdump

CFLAGS = -O1 -fno-reorder-blocks -march=vr4300 -mtune=vr4300 -mabi=32 -mno-gpopt -mdivide-breaks \
	-mexplicit-relocs -std=gnu11 -ffunction-sections -fdata-sections
CPPFLAGS = -DF3DEX_GBI_2 -DZ64_VERSION=Z64_OOT10

OUTDIR := build
OBJDIR := build/bin
SRCDIR := c
vpath %.c c
vpath %.h c
vpath %.c c/gz
vpath %.h c/gz

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(sort $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)))

.PHONY: all clean bundle symbols

all: clean bundle symbols

$(OBJDIR)/%.o: %.c 
	$(CC) -o $@ -c $< $(CFLAGS) $(CPPFLAGS)
ifdef RUN_OBJDUMP
		$(OBJDUMP) -d $@ | tr -d '\015' > $@_d.txt
		$(OBJDUMP) -r $@ | tr -d '\015' > $@_r.txt
endif

$(OBJDIR): 
	mkdir -p $@/gz

$(OBJECTS): | $(OBJDIR)

bundle: $(OBJECTS)
	$(LD) -T /usr/mips64-ultra-elf/n64-sysroot/usr/lib/gl-n64.ld -o $(OUTDIR)/bundle.o -i --force-dynamic -L. -L/usr/lib/gcc/mips64-ultra-elf/11.2.0/dynamic -L/usr/mips64-ultra-elf/lib/dynamic $(patsubst %.o,-l:%.o,$(OBJECTS)) -lc -lm -lg -lgcc -lnosys
#	$(LD) -o $(OUTDIR)/bundle.o -i -L. -static -lc $(patsubst %.o,-l:%.o,$(OBJECTS))

symbols: bundle
	$(OBJDUMP) -t $(OUTDIR)/bundle.o | tr -d '\015' > $(OUTDIR)/c_symbols.txt
ifdef RUN_OBJDUMP
		$(OBJDUMP) -d $(OUTDIR)/bundle.o | tr -d '\015' > $(OUTDIR)/bundle_d.txt
		$(OBJDUMP) -r $(OUTDIR)/bundle.o | tr -d '\015' > $(OUTDIR)/bundle_r.txt
endif

clean:
	rm -rf ${OBJDIR}/*.o
	rm -rf ${OBJDIR}/gz/*.o
