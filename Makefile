include Rules.mk

#######################################
# list of source files
########################################
LIB_UTILS_SOURCES =                                 \
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
src/utils/serial.c                                  \
src/utils/sock_util.c                               \
src/utils/tcp_server.c                              \
src/utils/tcp_server_ipv4.c                         \
src/utils/tcp_server_unix_domain.c                  \
src/utils/tcp_connector.c                           \
src/utils/tcp_auto_connector.c                      \
src/utils/udp_socket.c                              \
src/utils/cli.c                                     \
src/utils/cli_telnet.c                              \
src/utils/cli_serial.c                              \
src/utils/telnet_reader.c                           \
src/utils/hex_dump.c                                \
src/utils/trace.c                                   \
src/utils/lookup_table.c                            \
src/utils/publish_observe.c                         \
src/utils/atomic_file_update.c                      \
src/utils/debug_log.c

LIB_MODBUS_SOURCES =                                \
src/modbus/modbus_util.c                            \
src/modbus/mbap_reader.c                            \
src/modbus/modbus_rtu_request_handler.c             \
src/modbus/modbus_func_coils.c                      \
src/modbus/modbus_func_discrete.c                   \
src/modbus/modbus_func_holding.c                    \
src/modbus/modbus_func_input.c                      \
src/modbus/modbus_tcp_slave.c                       \
src/modbus/modbus_rtu_slave.c                       \
src/modbus/modbus_master.c                          \
src/modbus/modbus_rtu_master.c                      \
src/modbus/modbus_tcp_master.c                      \
src/modbus/modbus_rtu_response_handler.c            \
src/modbus/modbus_regs.c                            \
src/modbus/modbus_crc.c

MONGOOSE_SOURCES =                                  \
src/mongoose/mongoose.c                             \
src/mongoose/mongoose_util.c

LIB_CORE_SOURCES =                                  \
src/core/channel.c                                  \
src/core/alarm.c                                    \
src/core/channel_manager.c                          \
src/core/alarm_manager.c

APP_SOURCES =                                       \
src/main/main.c                                     \
src/main/core_driver.c                              \
src/main/cli_driver.c                               \
src/main/modbus_slave_driver.c                      \
src/main/modbus_master_driver.c                     \
src/main/webserver_driver.c                         \
src/main/webserver_api.c                            \
src/main/cfg_mgr.c

TEST_C_SOURCE =                                     \
test/cmd_option_test.c                              \
test/mb_reg_test.c                                  \
test/lookup_table_test.c                            \
test/alarm_test.c

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
-Isrc/utils                               \
-Isrc/modbus                              \
-Isrc/core                                \
-Isrc/main                                \
-Isrc/mongoose                            \
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
MIGRATION = config_migration

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
all: $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(MIGRATION)

#######################################
# target source setup
#######################################
TARGET_SOURCES := $(LIB_UTILS_SOURCES) $(LIB_MODBUS_SOURCES) $(LIB_CORE_SOURCES) $(MONGOOSE_SOURCES) $(APP_SOURCES)
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TARGET_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(TARGET_SOURCES)))

TEST_SOURCES := $(LIB_UTILS_SOURCES) $(TEST_C_SOURCE) $(TEST_MAIN_C_SOURCE) $(LIB_MODBUS_SOURCES) $(LIB_CORE_SOURCES)
TEST_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TEST_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(TEST_SOURCES)))

MIGRATION_SOURCES := $(LIB_UTILS_SOURCES) src/app/config_migration.c
MIGRATION_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(MIGRATION_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(MIGRATION_SOURCES)))

#######################################
# C source build rule
#######################################
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC]         $(notdir $<)"
	$Q$(CC) -c $(CFLAGS) $< -o $@

#######################################
# main target
#######################################
$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	@echo "[LD]         $@"
	$Q$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR):
	@echo "MKDIR          $(BUILD_DIR)"
	$Qmkdir $@

#######################################
# test target
#######################################
$(BUILD_DIR)/test_$(TARGET): $(TEST_OBJECTS) Makefile
	@echo "[LD]         $@"
	$Q$(CC) $(TEST_OBJECTS) $(LDFLAGS) -lcunit -o $@

.PHONY: test
test: $(BUILD_DIR)/test_$(TARGET)
	$(BUILD_DIR)/test_$(TARGET)
	
# separation mark. \t is above there. be careful

#######################################
# config_migration
#######################################
$(BUILD_DIR)/config_migration: $(MIGRATION_OBJECTS) Makefile
	@echo "[LD]         $@"
	$Q$(CC) $(MIGRATION_OBJECTS) $(LDFLAGS) -o $@

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
