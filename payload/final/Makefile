TARGET	= bicr
SRCDIR	= src
INCDIR	= include
OBJS	= start.o main.o libc.o utils.o pervasive.o gpio.o \
	  spi.o syscon.o \

PREFIX	= arm-vita-eabi
CC	= $(PREFIX)-gcc
AS	= $(PREFIX)-as
OBJCOPY	= $(PREFIX)-objcopy
CFLAGS	= -I$(INCDIR) -Wall -O0 -mcpu=cortex-a9 -mthumb-interwork \
	  -Wno-unused-const-variable -Wno-main
LDFLAGS	= -T linker.ld -nostartfiles -nostdlib -lgcc -lm
ASFLAGS	=
DEPS	= $(OBJS:.o=.d)
all: $(TARGET).bin
%.bin: %.elf
	$(OBJCOPY) -S -O binary $^ $@
$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
%.o: $(SRCDIR)/%.s
	$(CC) $(ASFLAGS) -MMD -MP -c $< -o $@
