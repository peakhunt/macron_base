include ../Rules.mk

#######################################
# list of source files
########################################
APP_SOURCES =                                       \
../src/app/app_stdlib/control_lib.c

TEST_C_SOURCE =                                     \
./test_main.c \
./control_lib_test.c

#######################################
# include and lib setup
#######################################
C_INCLUDES =                                  \
-I../src/utils                                \
-I../src/app/app_stdlib

#######################################
# for verbose output
#######################################
# Prettify output
V = 0
ifeq ($V, 0)
  Q = @
  P = > /dev/null
else
  Q =
  P =
endif

#######################################
# build directory and target setup
#######################################
BUILD_DIR = build
TARGET    = control_lib_test

#######################################
# compile & link flags
#######################################
CFLAGS += -g $(C_DEFS) $(C_INCLUDES) -DCONTROL_LIB_TEST

# Generate dependency information
CFLAGS += -MMD -MF .dep/$(*F).d

LDFLAGS +=  $(LIBDIR) $(LIBS)

#######################################
# build target
#######################################
all: $(BUILD_DIR)/$(TARGET)

#######################################
# target source setup
#######################################
TARGET_SOURCES := $(APP_SOURCES)
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TARGET_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(TARGET_SOURCES)))

TEST_SOURCES := $(TEST_C_SOURCE)
TEST_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TEST_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(TEST_SOURCES)))

#######################################
# C source build rule
#######################################
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC]         $(notdir $<)"
	$Q$(CC) -c $(CFLAGS) $< -o $@

#######################################
# main target
#######################################
$(BUILD_DIR)/$(TARGET): $(OBJECTS) $(TEST_OBJECTS) Makefile
	@echo "[LD]         $@"
	$Q$(CC) $(OBJECTS) $(TEST_OBJECTS) $(LDFLAGS) -lcunit -o $@

$(BUILD_DIR):
	@echo "MKDIR          $(BUILD_DIR)"
	$Qmkdir $@

#######################################
# clean up
#######################################
clean:
	@echo "[CLEAN]          $(TARGET) $(BUILD_DIR) .dep"
	$Q-rm -fR .dep $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
