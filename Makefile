# Inspired by:
# https://github.com/TheNetAdmin/Makefile-Templates/blob/master/SmallProject/Template/Makefile

CC :=  gcc
CCFLAGS := -Wall
DBGFLAGS := -g
CCOBJFLAGS := $(CCFLAGS) -c

BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug

TARGET_NAME := redesigned-octo-computing-machine
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)

SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
				$(OBJ) \
				$(OBJ_DEBUG)

default: build run

# non-phony targets
$(TARGET): $(OBJ)
	@echo -e "BUILD"
	@echo -e "Building target without debug..."
	$(CC) $(CCFLAGS) -o $@ $(OBJ)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	@echo -e "BUILD"
	@echo -e "Building object files without debug..."
	$(CC) $(CCOBJFLAGS) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c
	@echo -e "BUILD"
	@echo -e "Building target with debug..."
	$(CC) $(CCOBJFLAGS) $(DBGFLAGS) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	@echo -e "BUILD"
	@echo -e "Building object files with debug..."
	$(CC) $(CCFLAGS) $(DBGFLAGS) $(OBJ_DEBUG) -o $@

# phony rules
.PHONY: init
init:
	@echo -e "INIT"
	@echo -e "Creating project tree..."
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH)

.PHONY: build
build: $(TARGET) format

.PHONY: format
format:
	@echo -e "FORMAT"
	@find src -iname *.[h,c]| xargs clang-format -i -style="{BasedOnStyle: llvm, BreakBeforeBraces: Linux}"

.PHONY: run
run: 
	@echo -e "RUN"
	@./$(TARGET)

.PHONY: debug
debug: $(TARGET_DEBUG)
	@echo -e "DEBUG"
	@./$(TARGET_DEBUG)


.PHONY: clean
clean:
	@echo -e "CLEAN"
	@echo -e "Cleaning project structure..."
	@rm -rf $(CLEAN_LIST)

