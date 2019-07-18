#ifndef SYSROOT_H
#define SYSROOT_H

struct sysroot_buffer {
	unsigned short version;
	unsigned short size;
	unsigned int current_firmware_version;
	unsigned int firmware_version_shipped_from_factory;
	unsigned int unk0c[(0x2c - 0x0c) / 4];
	unsigned long long bitfield_flags;
	unsigned int unk34[(0x40 - 0x34) / 4];
	unsigned int devkit_function_address_1;
	unsigned int devkit_uid;
	unsigned int devkit_function_address_2;
	unsigned int aslr_seed;
	unsigned int devkit_config_flags;
	unsigned int devkit_config_flags2;
	unsigned int devkit_config;
	unsigned int devkit_config_flags3;
	unsigned int dram_base_paddr;
	unsigned int dram_size;
	unsigned int unk68;
	unsigned int boot_type_indicator_1;
	unsigned char openpsid[0x10];
	unsigned int secure_kernel_enp_raw_data_paddr;
	unsigned int secure_kernel_enp_size;
	unsigned int unk88;
	unsigned int unk8c;
	unsigned int kprx_auth_sm_self_raw_data_paddr;
	unsigned int kprx_auth_sm_self_size;
	unsigned int prog_rvk_srvk_raw_data_paddr;
	unsigned int prog_rvk_srvk_size;
	unsigned short model;
	unsigned short device_type;
	unsigned short device_config;
	unsigned short type;
	unsigned short unka8[(0xb0 - 0xa8) / 2];
	unsigned char session_id[0x10];
	unsigned int unkc0;
	unsigned int boot_type_indicator_2;
	unsigned int unkc8[(0xd0 - 0xc8) / 4];
	unsigned int suspend_saved_context_paddr;
	unsigned int unkd4[(0xf8 - 0xd4) / 4];
	unsigned int bootloader_revision;
	unsigned int sysroot_magic_value;
	unsigned char encrypted_session_key[0x20];
} __attribute__((packed));

#endif
