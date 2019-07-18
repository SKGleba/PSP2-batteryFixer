BOOTSTRAP_OBJS = payload_bootstrap.o
PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
OBJCOPY = $(PREFIX)-objcopy
payload_bootstrap.elf: $(BOOTSTRAP_OBJS)
	$(CC) -T payload_bootstrap.ld -nostartfiles -nostdlib $^ -o $@ -lgcc
payload_bootstrap.bin: payload_bootstrap.elf
	$(OBJCOPY) -S -O binary $^ $@
payload_bootstrap_bin.o: payload_bootstrap.bin
	$(OBJCOPY) -I binary -O elf32-littlearm --binary-architecture arm $^ $@