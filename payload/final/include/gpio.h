#ifndef GPIO_H
#define GPIO_H

#define GPIO_PORT_MODE_INPUT	0
#define GPIO_PORT_MODE_OUTPUT	1

#define GPIO_PORT_OLED		0
#define GPIO_PORT_SYSCON_OUT	3
#define GPIO_PORT_SYSCON_IN	4
#define GPIO_PORT_GAMECARD_LED	6
#define GPIO_PORT_PS_LED	7
#define GPIO_PORT_HDMI_BRIDGE	15

void gpio_set_port_mode(int bus, int port, int mode);
int gpio_port_read(int bus, int port);
void gpio_port_set(int bus, int port);
void gpio_port_clear(int bus, int port);
void gpio_set_intr_mode(int bus, int port, int mode);
int gpio_query_intr(int bus, int port);
int gpio_acquire_intr(int bus, int port);

#endif
