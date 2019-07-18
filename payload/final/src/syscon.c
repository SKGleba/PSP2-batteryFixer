#include "syscon.h"
#include "pervasive.h"
#include "spi.h"
#include "gpio.h"
#include "utils.h"
#include "libc.h"

#define SYSCON_TX_CMD_LO	0
#define SYSCON_TX_CMD_HI	1
#define SYSCON_TX_LENGTH	2
#define SYSCON_TX_DATA(i)	(3 + (i))

#define SYSCON_RX_STATUS_LO	0
#define SYSCON_RX_STATUS_HI	1
#define SYSCON_RX_LENGTH	2
#define SYSCON_RX_RESULT	3

struct syscon_packet {
	unsigned char tx[32];	/* tx[0..1] = cmd, tx[2] = length */
	unsigned char rx[32];	/* rx[0..1] = status, rx[2] = length, rx[3] = result */
};

static void syscon_packet_start(struct syscon_packet *packet)
{
	int i = 0;
	unsigned char cmd_size = packet->tx[2];
	unsigned char tx_total_size = cmd_size + 3;
	unsigned int offset;
	(void)offset;

	gpio_port_clear(0, GPIO_PORT_SYSCON_OUT);
	spi_write_start(0);

	if (cmd_size <= 29) {
		offset = 2;
	} else {
		/* TODO */
	}

	do {
		spi_write(0, (packet->tx[i + 1] << 8) | packet->tx[i]);
		i += 2;
	} while (i < tx_total_size);

	spi_write_end(0);
	gpio_port_set(0, GPIO_PORT_SYSCON_OUT);
}

static unsigned char syscon_cmd_sync(struct syscon_packet *packet)
{
	int i = 0;

	while (!gpio_query_intr(0, GPIO_PORT_SYSCON_IN))
		;

	gpio_acquire_intr(0, GPIO_PORT_SYSCON_IN);

	while (spi_read_available(0)) {
		unsigned int data = spi_read(0);
		packet->rx[i] = data & 0xFF;
		packet->rx[i + 1] = (data >> 8) & 0xFF;
		i += 2;
	}

	spi_read_end(0);
	gpio_port_clear(0, GPIO_PORT_SYSCON_OUT);

	return packet->rx[SYSCON_RX_RESULT];
}

static void syscon_common_read(unsigned int *buffer, unsigned short cmd)
{
	struct syscon_packet packet;

	packet.tx[SYSCON_TX_CMD_LO] = cmd & 0xFF;
	packet.tx[SYSCON_TX_CMD_HI] = (cmd >> 8) & 0xFF;
	packet.tx[SYSCON_TX_LENGTH] = 1;

	memset(packet.rx, -1, sizeof(packet.rx));

	syscon_packet_start(&packet);
	syscon_cmd_sync(&packet);

	memcpy(buffer, &packet.rx[4], packet.rx[SYSCON_RX_LENGTH] - 2);
}

static void syscon_common_write(unsigned int data, unsigned short cmd, unsigned int length)
{
	int i;
	unsigned char hash, result;
	struct syscon_packet packet;

	packet.tx[SYSCON_TX_CMD_LO] = cmd & 0xFF;
	packet.tx[SYSCON_TX_CMD_HI] = (cmd >> 8) & 0xFF;
	packet.tx[SYSCON_TX_LENGTH] = length;

	packet.tx[SYSCON_TX_DATA(0)] = data & 0xFF;
	packet.tx[SYSCON_TX_DATA(1)] = (data >> 8) & 0xFF;
	packet.tx[SYSCON_TX_DATA(2)] = (data >> 16) & 0xFF;
	packet.tx[SYSCON_TX_DATA(3)] = (data >> 24) & 0xFF;

	/*
	 * Calculate packet hash
	 */
	hash = 0;
	for (i = 0; i < length + 2; i++)
		hash += packet.tx[i];

	packet.tx[2 + length] = ~hash;
	memset(&packet.tx[3 + length], -1, sizeof(packet.rx) - (3 + length));

	do {
		memset(packet.rx, -1, sizeof(packet.rx));
		syscon_packet_start(&packet);

		result = syscon_cmd_sync(&packet);
	} while (result == 0x80 || result == 0x81);
}

int syscon_init(void)
{
	unsigned int syscon_version;

	spi_init(0);

	gpio_set_port_mode(0, GPIO_PORT_SYSCON_OUT, GPIO_PORT_MODE_OUTPUT);
	gpio_set_port_mode(0, GPIO_PORT_SYSCON_IN, GPIO_PORT_MODE_INPUT);
	gpio_set_intr_mode(0, GPIO_PORT_SYSCON_IN, 3);

	syscon_common_read(&syscon_version, 1);

	if (syscon_version > 0x1000003)
		syscon_common_write(0x12, 0x80, 3);
	else if (syscon_version > 0x70501)
		syscon_common_write(2, 0x80, 3);

	return 0;
}

void sc_call(unsigned int arg0, unsigned short cmd, unsigned int len)
{
	syscon_common_write(arg0, cmd, len);
}
