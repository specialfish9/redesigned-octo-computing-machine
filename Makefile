ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
else
	UMPS3_DIR_PREFIX = /usr/local
endif

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include/umps3

SRC_PATH := src/phase1
OBJ_PATH := obj
OUT_PATH := output

HEADERS := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.h))) 
SOURCES := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c)))
OBJS :=  $(addsuffix .o, $(notdir $(basename $(SOURCES))))

CLEAN_LIST := $(OUT_PATH)/* \
				$(OBJ_PATH)/* \

CFLAGS = -ffreestanding -ansi -Wall -c -mips1 -mabi=32 -mfp32 \
				 -mno-gpopt -G 0 -fno-pic -mno-abicalls -EL -I$(UMPS3_INCLUDE_DIR) -std=c99

LDFLAGS = -G 0 -nostdlib -T $(UMPS3_DATA_DIR)/umpscore.ldscript -m elf32ltsmip

CC = mipsel-linux-gnu-gcc
LD = mipsel-linux-gnu-ld
AS = mipsel-linux-gnu-as -KPIC
EF = umps3-elf2umps
UDEV = umps3-mkdev

KERNEL_NAME = ROCM_kernel
DISK_NAME = disk0

#main target
all: kernel.core.umps disk0.umps
	@echo "--------"
	@echo "Done :D"

# use umps3-mkdev to create the disk0 device
disk0.umps:
	@echo "*** DISK ***"
	@echo "Creating disk " $@ "..."
	$(UDEV) -d $(OUT_PATH)/$(DISK_NAME).umps

# create the kernel.core.umps kernel executable file
kernel.core.umps: kernel
	@echo "*** " $@ " ***"
	@echo "Creating " $@ "..."
	umps3-elf2umps -k $(OUT_PATH)/$(KERNEL_NAME)

kernel: $(OBJS) crtso.o libumps.o  
	@echo "*** KERNEL ***"
	@echo "Linking kernel..."
	$(LD) \
		$(LDFLAGS) \
		-o $(OUT_PATH)/$(KERNEL_NAME) \
		$(addprefix $(OBJ_PATH)/,$^)

d:
	@echo $(UMPS3_DATA_DIR) 


%.o: $(SRC_PATH)/%.c
	@echo -e "Building " $@ "..."
	$(CC) $(CFLAGS) -o $(OBJ_PATH)/$@ $<

crtso.o:
	@echo -e "Building " $@ "..."
	$(CC) $(CFLAGS) -o $(OBJ_PATH)/$@ $(UMPS3_DATA_DIR)/crtso.S

libumps.o:
	@echo -e "Building " $@ "..."
	$(CC) $(CFLAGS) -o $(OBJ_PATH)/$@ $(UMPS3_DATA_DIR)/libumps.S

format:
	@echo -e "*** FORMAT ***"
	@find src -iname *.[h,c]| xargs clang-format -i -style="{BasedOnStyle: llvm, BreakBeforeBraces: Linux}"

clean:
	@echo -e "*** CLEAN ***"
	@echo -e "Cleaning project structure..."
	@rm -rf $(CLEAN_LIST)
	@echo -e "-------------------------------------------"
	@echo -e "Un progetto pulito è un progetto felice :D"
	@echo -e "-------------------------------------------"

