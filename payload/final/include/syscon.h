#ifndef SYSCON_H
#define SYSCON_H

#define SYSCON_RESET_TYPE_POWEROFF	0
#define SYSCON_RESET_TYPE_SUSPEND	1
#define SYSCON_RESET_TYPE_COLD_RESET	2
#define SYSCON_RESET_TYPE_SOFT_RESET	17

int syscon_init(void);
void syscon_reset_device(int type, int mode);
void syscon_set_hdmi_cdc_hpd(int enable);
void syscon_ctrl_read(unsigned int *data);
void syscon_msif_set_power(int enable);
void sc_call(unsigned int data, unsigned short cmd, unsigned int len);

#endif
