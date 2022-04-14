ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
else
	UMPS3_DIR_PREFIX = /usr/local
endif

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include

SRC_PATH := src
OBJ_PATH := obj
OUT_PATH := output
DOC_PATH := doc

HEADERS := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.h))) 
SOURCES := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c)))
OBJS :=  $(addsuffix .o, $(notdir $(basename $(SOURCES))))

CLEAN_LIST := $(OUT_PATH)/* \
				$(OBJ_PATH)/* \
				$(DOC_PATH)/* \

CFLAGS = -ffreestanding -ansi -Wall -c -mips1 -mabi=32 -mfp32 \
				 -mno-gpopt -G 0 -fno-pic -mno-abicalls -EL  -std=gnu99

LDFLAGS = -G 0 -nostdlib -T $(UMPS3_DATA_DIR)/umpscore.ldscript -m elf32ltsmip

CC = mipsel-linux-gnu-gcc
LD = mipsel-linux-gnu-ld
AS = mipsel-linux-gnu-as -KPIC
EF = umps3-elf2umps
UDEV = umps3-mkdev

KERNEL_NAME = ROCM_kernel
DISK_NAME = disk0

#main target
all: figlet structure kernel.core.umps disk0.umps docs
	@echo -e "Done :D"

structure:
	@echo -e "*** STRUCTURE ***"
	@echo -e "Generating project structure...":
	@mkdir doc
	@mkdir out 

# use umps3-mkdev to create the disk0 device
disk0.umps:
	@echo -e "*** DISK ***"
	@echo -e "Creating disk " $@ "..."
	@$(UDEV) -d $(OUT_PATH)/$(DISK_NAME).umps

# create the kernel.core.umps kernel executable file
kernel.core.umps: kernel
	@echo -e "*** " $@ " ***"
	@echo -e "Creating " $@ "..."
	@umps3-elf2umps -k $(OUT_PATH)/$(KERNEL_NAME)

kernel: $(OBJS) crtso.o libumps.o  
	@echo -e "*** KERNEL ***"
	@echo -e "Linking kernel..."
	@$(LD) $(LDFLAGS) -o $(OUT_PATH)/$(KERNEL_NAME) $(addprefix $(OBJ_PATH)/,$^)

%.o: $(SRC_PATH)/%.c
	@echo -e "Building " $@ "..."
	@$(CC) $(CFLAGS) -I$(UMPS3_INCLUDE_DIR) -o $(OBJ_PATH)/$@ $<

crtso.o:
	@echo -e "Building " $@ "..."
	@$(CC) $(CFLAGS) -I$(UMPS3_INCLUDE_DIR)/umps3 -o $(OBJ_PATH)/$@ $(UMPS3_DATA_DIR)/crtso.S

libumps.o:
	@echo -e "Building " $@ "..."
	@$(CC) $(CFLAGS) -I$(UMPS3_INCLUDE_DIR)/umps3 -o $(OBJ_PATH)/$@ $(UMPS3_DATA_DIR)/libumps.S

format:
	@echo -e "*** FORMAT ***"
	@find src -iname *.[h,c]| xargs clang-format -i -style="{BasedOnStyle: llvm, BreakBeforeBraces: Linux}"

clean:
	@echo -e "*** CLEAN ***"
	@echo -e "Cleaning project structure..."
	@rm -rf $(CLEAN_LIST)

docs:
	@echo -e "*** DOCUMENTATION ***"
	@echo -e "Creating documentation..."
	@doxygen 
	@echo -e "Done"
	@echo -e "Building pdf"
	@make -C doc/latex
	@echo -e "Done"
	@cp doc/latex/refman.pdf doc/
	@echo -e "Outputs in doc folder"

figlet:
	@echo -e "\e[0;32moooooooooo    ooooooo     oooooooo8 oooo     oooo "
	@echo -e "\e[0;32m 888    888 o888   888o o888     88  8888o   888  "
	@echo -e "\e[0;32m 888oooo88  888     888 888          88 888o8 88  "
	@echo -e "\e[0;32m 888  88o   888o   o888 888o     oo  88  888  88  "
	@echo -e "\e[0;32mo888o  88o8   88ooo88    888oooo88  o88o  8  o88o "
	@echo -e "\e[0m"
