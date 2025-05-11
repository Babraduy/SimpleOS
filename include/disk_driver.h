#ifndef _DISK_DRIVER_H
#define _DISK_DRIVER_H

#define BSY 0x80
#define DRQ 0x08
#define ERR 0x01

typedef struct
{
	uint16_t port;
	uint8_t is_slave;
	char model[41];
	uint32_t total_sectors;
} ata_disk;

extern int wait_bsy(uint8_t index);
extern int wait_drq(uint8_t index);
extern int check_error(uint8_t index);
extern int identify_disks();
extern uint8_t* read_disk(uint8_t index, uint32_t num_sectors, uint32_t start);
extern int write_disk(uint8_t index, uint32_t num_sectors, uint32_t start, uint8_t* data, int size);
extern void list_disks();

#endif
