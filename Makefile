include Rules.mk

#######################################
# list of source files
########################################
C_SOURCES =                                         \
src/utils/cmd_option.c                              \
src/utils/thread_queue.c                            \
src/utils/thread.c                                  \
src/utils/evloop_thread.c                           \
src/utils/evloop_timer.c                            \
src/utils/watcher.c                                 \
src/utils/cJSON.c                                   \
src/utils/cJSON_Utils.c                             \
src/utils/bhash.c                                   \
src/utils/circ_buffer.c                             \
src/utils/sbuf.c                                    \
src/utils/config_reader.c                           \
src/utils/utils.c                                   \
src/utils/stream.c                                  \
src/sock/sock_util.c                                \
src/sock/tcp_server.c                               \
src/sock/tcp_server_ipv4.c                          \
src/sock/tcp_server_unix_domain.c                   \
src/sock/tcp_connector.c                            \
src/sock/tcp_auto_connector.c                       \
src/sock/udp_socket.c                               \
src/serial/serial.c                                 \
src/utils/cli.c                                     \
src/utils/cli_telnet.c                              \
src/utils/cli_serial.c                              \
src/protocols/telnet/telnet_reader.c                \
src/debug/hex_dump.c                                \
src/debug/trace.c                                   \
src/debug_log/debug_log.c

MAIN_C_SOURCE =                                     \
src/main.c

TEST_C_SOURCE =                                     \
test/cmd_option_test.c

TEST_MAIN_C_SOURCE =                                \
test/test_main.c

#######################################
#  libev
########################################
LIBEV_DIR=src/libev/install_dir

#######################################
C_DEFS  = -DTRACE_ENABLED

#######################################
# include and lib setup
#######################################
C_INCLUDES =                              \
-Isrc/common                              \
-Isrc/utils                               \
-Isrc/sock                                \
-Isrc/serial                              \
-Isrc/debug_log                           \
-Isrc/protocols/telnet                    \
-I$(LIBEV_DIR)/include

LIBS = -lev -lpthread -lm
LIBDIR = -L$(LIBEV_DIR)/lib

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
TARGET    = macron_bog

#######################################
# compile & link flags
#######################################
CFLAGS += $(C_DEFS) $(C_INCLUDES)

# Generate dependency information
CFLAGS += -MMD -MF .dep/$(*F).d

LDFLAGS +=  $(LIBDIR) $(LIBS)

#######################################
# build target
#######################################
all: $(BUILD_DIR)/$(TARGET)

#######################################
# build rules
#######################################
TARGET_SOURCES := $(C_SOURCES) $(MAIN_C_SOURCE)
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TARGET_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(TARGET_SOURCES)))

TEST_SOURCES := $(C_SOURCES) $(TEST_C_SOURCE) $(TEST_MAIN_C_SOURCE)
TEST_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TEST_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(TEST_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC]         $(notdir $<)"
	$Q$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	@echo "[LD]         $(TARGET)"
	$Q$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/test_$(TARGET): $(TEST_OBJECTS) Makefile
	@echo "[LD]         test_$(TARGET)"
	$Q$(CC) $(TEST_OBJECTS) $(LDFLAGS) -lcunit -o $@
	$(BUILD_DIR)/test_$(TARGET)

$(BUILD_DIR):
	@echo "MKDIR          $(BUILD_DIR)"
	$Qmkdir $@

.PHONY:
test: $(BUILD_DIR)/test_$(TARGET)


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
