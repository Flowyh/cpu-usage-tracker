# PROJECT STRUCTURE

SRC_DIR := ./src
INC_DIR := ./inc
APP_DIR := ./app
TEST_DIR := ./test

# PROJECT FILES

MODE := app
SRC := $(wildcard $(SRC_DIR)/*.c) # list of all the C source files in the SRC_DIR directory

ifeq ($(MODE), test) # SRC + list of all the C source files in TEST_DIR directory
	APP_SRC := $(SRC) $(wildcard $(TEST_DIR)/*.c)
	TARGET := $(TEST_DIR)/test.out
else # SRC + list of all the C source files in APP_DIR directory
	APP_SRC := $(SRC) $(wildcard $(APP_DIR)/*.c)
	TARGET := main.out
endif

OBJ := $(APP_SRC:%.c=%.o) # replace .c with .o for each element in APP_SRC
DEPS := $(OBJ:%.o=%.d) # replace .o with .d for each element in OBJ

# LIBRARIES

LIBS := pthread

# COMPILATION

CC ?= gcc # default compiler
C_FLAGS := -Wall -Wextra -Werror
DEP_FLAGS := -MMD -MP # automatically generate .d dependenciees

INCS_INC := $(foreach i, $(INC_DIR), -I$i) # foreach var in INC_DIR expand with -I$var
LIBS_INC := $(foreach l, $(LIBS), -l$l)

# If CC is set to clang, add -Weverything flag
ifeq ($(CC), clang)
	C_FLAGS += -Weverything -Wno-vla -Wno-disabled-macro-expansion
endif

# Set -O flag if O= argument is present, default=-O3
ifeq ("$(origin O)", "command line")
	OPT := -O$(O)
else
	OPT := -O3
endif

# Set -G flag if G= argument is present, default=none
ifeq ("$(origin G)", "command line")
	GGDB := -ggdb$(G)
else
	GGDB :=
endif

C_FLAGS += $(OPT) $(GGDB) $(DEP_FLAGS)

# TARGETS

# https://stackoverflow.com/questions/16467718/how-to-print-out-a-variable-in-makefile
print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(C_FLAGS) $(INCS_INC) $(OBJ) -o $@ $(LIBS_INC)

%.o:%.c %.d
	$(CC) $(C_FLAGS) $(INCS_INC) -c $< -o $@

# $@ name of the target
# $< name of the first prerequisite (%.c)
# -c compile without linking

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJ)
	rm -rf $(DEPS)
	rm -rf $(DEPS)

$(DEPS):
include $(wildcard $(DEPS))
