#ifndef _FAT12_H
#define _FAT12_H

typedef struct
{
	uint8_t jmp[3];
	char oem[8];
	uint16_t sector_size;
	uint8_t sec_per_cluster;
	uint16_t reserved;
	uint8_t num_fat;
	uint16_t root_ent;
	uint16_t num_sec;
	uint8_t media;
	uint16_t sec_per_fat;
	uint16_t sec_per_track;
	uint16_t num_heads;
	uint32_t hidden_sectors;
	uint32_t num_sec32;
} __attribute__((packed)) bpb;

typedef struct
{
	char filename[8];
	char extension[3];
	uint8_t attrib;
	uint8_t usr_attrib;
	uint8_t create_time_ms;
	uint16_t create_time;
	uint16_t create_date;
	uint16_t last_access_time;
	uint16_t access;
	uint16_t last_time;
	uint16_t last_date;
	uint16_t start_cluster;
	uint32_t size;
} __attribute__((packed)) root_entry;

extern int read_bpb(uint8_t id);
extern uint8_t* read_fat();
extern uint8_t* read_fat_entries(uint8_t* fat);
extern root_entry fat_find_file(char* name);
extern uint8_t* fat_read_file(char* name);
extern void fat_create_file(char* name, uint8_t* data, uint32_t size);
extern void fat_modify_file(char* name, uint8_t* new_data, uint32_t size);
extern void fat_delete_file(char* name);
extern void fat_rename_file(char* name, char* new_name);
extern root_entry* fat_read_dir(char* name);
extern void fat_change_dir(char* name);
extern void fat_create_dir(char* name);
extern int fat_install();

#endif
