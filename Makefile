NAME = Datalogger

CC = gcc
LD = gcc

DEFINES = 

C_OPTS = -std=gnu99 -g -Werror

LIBS = -lm

SOURCE_DIR = .
BUILD_DIR = Build

all: build

C_FILES =	AD7608.c \
			AD7608Source.c \
			Config.c \
			CS5467.c \
			CS5467Source.c \
			Evaluator.c \
			I2C.c \
			INA220.c \
			INA220Source.c \
			INA226.c \
			INA226Source.c \
			jsmn.c \
			Main.c \
			Measure.c \
			Registers.c \
			TimeSource.c

C_OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o)

ALL_CFLAGS = $(C_OPTS) $(DEFINES) $(CFLAGS)
ALL_LDFLAGS = $(LD_FLAGS)

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@

build: $(NAME)

.SUFFIXES: .o .c

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

$(NAME): $(C_OBJS)
	$(LD) $(ALL_LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(BUILD_DIR) $(NAME)

-include $(C_OBJS:.o=.d)

