

#PREFIX:=riscv64-unknown-elf-
#CFLAGS:=-I/usr/include
#export PATH=/opt/riscv/bin:$PATH
TOOLSET     ?= riscv32-unknown-elf-
CC           = $(TOOLSET)gcc
LD           = $(TOOLSET)ld
AR           = $(TOOLSET)gcc-ar
OBJCOPY      = $(TOOLSET)objcopy
OPTFLAGS    ?= -Og
 

RM = rm -f
fixpath = $(strip $1)

 
CFLAGS      ?=  -march=rv32ima -mabi=ilp32 -ffreestanding -nostartfiles  -Os
INCLUDES     =  -I include -I arch/riscv -I sys -I kernel/inc -I fat32/Inc -I interpreter/monkey/inc
LDSCRIPT     =  arch/riscv/linker.ld

OBJDIR        = obj
START         = $(wildcard arch/riscv/boot_newlib.S) $(wildcard arch/riscv/ctxsw.S) $(wildcard arch/riscv/intr.S) $(wildcard arch/riscv/*.c)
STARTOBJ      = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(START)))))
SOURCES       = $(wildcard sys/*.c) 
OBJECTS       = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SOURCES)))))
SRCLIB        = $(wildcard lib/*.c)
LIBOBJ        = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SRCLIB)))))
FLOATLIB      = $(wildcard floatmath/*.S)
FLOATOBJ      = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(FLOATLIB)))))
KERNEL        = $(wildcard kernel/src/*.c) 
KERNOBJ       = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(KERNEL)))))

SRCDEVICE     = $(wildcard device/nam/*.c) $(wildcard device/tty/*.c) $(wildcard device/telnet/*.c) 
DEVICEOBJ     = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SRCDEVICE)))))
SRCSHELL      = $(wildcard shell/*.c) 
SHELLOBJ      = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SRCSHELL)))))

SRCFAT32         = $(wildcard fat32/Src/*.c)  $(wildcard tinyscript/*.c) $(wildcard cc/*.c) #$(wildcard basic/*.c) 
FAT32OBJ         = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SRCFAT32)))))

SRCBASIC     =  $(wildcard basic/*.c) 
OBJBASIC     =  $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SRCBASIC)))))


SRCMONKEY      =  $(wildcard interpreter/monkey/src/*.c) 
MONKEYOBJ      =  $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SRCMONKEY)))))

SRCPEPPER      = $(wildcard interpreter/pepper-lang/*.c) 
PEPPEROBJ      = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SRCMONKEY)))))#$(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(SRCMONKEY)))))

FSLIB         = #$(wildcard littlefs/*.c)
FSOBJ         = #$(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(basename $(FSLIB)))))





DOUT         = kernel


SRCPATH = $(sort $(dir $(START) $(SOURCES) $(SRCLIB) $(SRCBASIC) $(FLOATLIB) $(SRCFAT32) $(SRCMONKEY) $(FSLIB) $(KERNEL) $(SRCSHELL) $(SRCDEVICE)))
vpath %.c $(SRCPATH)
vpath %.S $(SRCPATH)


 
$(OBJDIR):
	@mkdir $@


kernel:  $(DOUT).bin
		riscv32-unknown-elf-objdump -d kernel.elf > kernel.list


$(DOUT).bin : $(DOUT).elf
	@echo building $@
	@$(OBJCOPY) -O binary $< $@



$(DOUT).elf : $(OBJDIR) $(STARTOBJ) $(LIBOBJ) $(FLOATOBJ) $(OBJBASIC) $(KERNOBJ) $(DEVICEOBJ) $(FAT32OBJ) $(FSOBJ) $(MONKEYOBJ) $(SHELLOBJ) $(OBJECTS) 
	@echo building $@
	@$(CC) $(CFLAGS) $(LDFLAGS) -T $(LDSCRIPT) $(STARTOBJ) $(LIBOBJ) $(FLOATOBJ) $(OBJBASIC) $(FAT32OBJ) $(FSOBJ) $(MONKEYOBJ) $(KERNOBJ) $(SHELLOBJ) $(DEVICEOBJ) $(OBJECTS) -o $@

	
clean: $(OBJDIR)
	clear
	$(MAKE) --version
	@$(RM) $(DOUT).*
	@$(RM) $(call fixpath, $(OBJDIR)/*.*)


$(OBJDIR)/%.o: %.S
	@echo assembling $<
	@$(CC) $(CFLAGS)  $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: %.c
	@echo compiling $<
	@$(CC) $(CFLAGS)  $(INCLUDES) -c $< -o $@




run:
	clear
	#gcc -o mini-rv32ima ../mini-rv32ima/mini-rv32ima.c 
	./rv32ima kernel.bin

riscv:
	gcc -o rv32ima mini-rv32ima/*.c  -I mini-rv32ima


fs:
	gcc -o mfs mkfs/*.c
	#./mfs -binary mkfs/kernel/shell
	./mfs mkfs/kernel


all:
	make kernel
	make riscv
	make run