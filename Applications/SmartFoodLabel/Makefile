# Makefile for Smart Food Label Application
# CH592 RISC-V Toolchain

#######################################
# Project Name
#######################################
PROJECT = SmartFoodLabel

#######################################
# Target
#######################################
TARGET = $(PROJECT)

#######################################
# Building variables
#######################################
# Debug build?
DEBUG = 1
# Optimization
OPT = -Os

#######################################
# Paths
#######################################
# Build path
BUILD_DIR = build

# EVT path (CH592 SDK)
EVT_PATH = ../../EVT

#######################################
# Source files
#######################################
# C sources
C_SOURCES =  \
APP/main.c \
HAL/shtxx.c \
HAL/gdey029t94.c \
HAL/button.c \
HAL/data_storage.c \
UI/food_label_ui.c

# Add CH592 SDK sources (adjust paths as needed)
C_SOURCES += \
$(EVT_PATH)/EXAM/StdPeriphDriver/CH59x_uart.c \
$(EVT_PATH)/EXAM/StdPeriphDriver/CH59x_gpio.c \
$(EVT_PATH)/EXAM/StdPeriphDriver/CH59x_i2c.c \
$(EVT_PATH)/EXAM/StdPeriphDriver/CH59x_spi.c \
$(EVT_PATH)/EXAM/StdPeriphDriver/CH59x_flash.c

# ASM sources
ASM_SOURCES = \
$(EVT_PATH)/EXAM/Startup/startup_CH592.S

#######################################
# Binaries
#######################################
PREFIX = riscv-none-embed-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
AR = $(PREFIX)ar
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# CPU
CPU = -march=rv32imac -mabi=ilp32

# MCU
MCU = $(CPU) -msmall-data-limit=8 -msave-restore -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections

# C includes
C_INCLUDES =  \
-I. \
-IAPP \
-IHAL \
-IUI \
-ICONFIG \
-I$(EVT_PATH)/EXAM/StdPeriphDriver/inc \
-I$(EVT_PATH)/EXAM/RVMSIS

# Compile gcc flags
ASFLAGS = $(MCU) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# Link script
LDSCRIPT = $(EVT_PATH)/EXAM/Ld/Link.ld

# Libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -nostartfiles

# Default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# Build the application
#######################################
# List of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# List of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "CC $<"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "LD $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "HEX $@"
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "BIN $@"
	@$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p $@

#######################################
# Clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# Flash
#######################################
flash: all
	@echo "Flashing $(BUILD_DIR)/$(TARGET).hex"
	# Add your flash command here (WCH-Link tool)
	# wchlink flash $(BUILD_DIR)/$(TARGET).hex

#######################################
# Dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean flash

#######################################
# EOF
#######################################
