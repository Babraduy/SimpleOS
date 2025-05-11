#include <stdint.h>
#include <stddef.h>
#include "fat12.h"
#include "disk_driver.h"
#include "heap.h"
#include "libstr.h"
#include "libio.h"

uint8_t index;

bpb* bpb_data;

int read_bpb(uint8_t id)
{
	bpb_data = (bpb*) read_disk(index, 1, 0);
	if (bpb_data == NULL)
	{
		//kprint("Error while trying to read BPB\n", 0x47);
		return -1;
	}

	index = id;
	return 0;
}

uint8_t* read_fat()
{
	uint8_t* fat = read_disk(index, bpb_data->sec_per_fat, bpb_data->reserved);

	if (fat == NULL)
	{
		kprint("Error while trying to read FAT\n", 0x47);
		return NULL;
	}

	return fat;
}

root_entry fat_find_file(char* name)
{
	root_entry* root_entries = (root_entry*) read_disk(index, (bpb_data->root_ent * 32 + 511) / 512, bpb_data->reserved + bpb_data->num_fat * bpb_data->sec_per_fat);

	if (root_entries == NULL)
	{
		kprint("Error while trying to read the root entries\n", 0x47);
	}

	char* dot = strrchr(name, '.');

	char filename[9];
	char extension[4];

	memset(filename, ' ', 8);
	filename[8] = '\0';

	memset(extension, ' ', 3);
	extension[3] = '\0';

	if (dot != NULL)
	{
		int len = dot - name;
		if (len > 8) len = 8;
		strncpy(filename, name, len);
		strupper(filename);

		strncpy(extension, dot + 1, 3);
		strupper(extension);
	}
	else
	{
		strcpy(filename, name);
		strupper(filename);
	}

	root_entry file;

	int found = 0;

	for (int i=0; i<bpb_data->root_ent; i++)
	{
		if ((uint8_t)root_entries[i].attrib == 0x0f) continue;
		if ((uint8_t)root_entries[i].filename[0] == 0xe5) continue;
		if ((uint8_t)root_entries[i].filename[0] == 0x00) break;

		char tmp1[9];
		strncpy(tmp1, root_entries[i].filename, 8);
		tmp1[8] = '\0';

		char tmp2[4];
		strncpy(tmp2, root_entries[i].extension, 3);
		tmp2[3] = '\0';

		if (strncmp(tmp1, filename, 8) == 0 &&
		    strncmp(tmp2, extension, 3) == 0)
		{
			file = root_entries[i];
			found = 1;
			break;
		}
	}

	if (!found)
	{
		file.start_cluster = 0xfff;
	}

	kfree(root_entries);

	return file;
}

uint8_t* fat_read_file (char* name)
{
	uint8_t* fat_entries = read_fat();
	if (fat_entries == NULL)
	{
		return NULL;
	}

	root_entry file = fat_find_file(name);

	if (file.start_cluster == 0xfff)
	{
		kprintf("File %s does not exist\n", 0x47, name);
		return NULL;
	}

	uint8_t* data = (uint8_t*) kmalloc(file.size + 1);
	if (data == NULL)
	{
		kprint("Could not allocate memory for 'data'\n", 0x47);
		return NULL;
	}
	uint16_t cluster = file.start_cluster;
	int total_size = 0;

	while(cluster < 0xff0)
	{
		uint8_t* tmp_data = read_disk(index, bpb_data->sec_per_cluster * bpb_data->sector_size / 512, bpb_data->reserved + bpb_data->num_fat * bpb_data->sec_per_fat + (bpb_data->root_ent * 32 + bpb_data->sector_size-1) / bpb_data->sector_size + (cluster - 2) * bpb_data->sec_per_cluster * bpb_data->sector_size / 512);

		if (tmp_data == NULL)
		{
			kfree(data);
			kprintf("Could not read file %s data\n", 0x47, name);
			return NULL;
		}

		if (total_size + bpb_data->sector_size * bpb_data->sec_per_cluster > (int)file.size)
		{
			memcpy(data + total_size, tmp_data, file.size - total_size);
			total_size += file.size - total_size;
		}
		else
		{
			memcpy(data + total_size, tmp_data, bpb_data->sector_size * bpb_data->sec_per_cluster);
			total_size += bpb_data->sector_size * bpb_data->sec_per_cluster;
		}

		if (cluster % 2 == 0)
		{
			cluster = fat_entries[cluster] | ((fat_entries[cluster + 1] & 0x0f) << 8);
		}
		else
		{
			cluster = ((fat_entries[cluster + 1] >> 4) & 0x0f) | (fat_entries[cluster + 2] << 4);
		}
	}

	data[file.size] = '\0';

	kfree(fat_entries);

	return data;
}

void fat_create_file(char* name, uint8_t* data, uint32_t size)
{
	if (fat_find_file(name).start_cluster != 0xfff)
	{
		kprintf("File %s already exists!\n", 0x47, name);
		return;
	}

	uint8_t* fat_entries = read_fat();

	int num_clusters = (size + bpb_data->sector_size * bpb_data->sec_per_cluster - 1)/ (bpb_data->sector_size * bpb_data->sec_per_cluster);
	uint16_t cluster = 2;
	uint16_t clusters[num_clusters + 1];
	int i = 1;
	int j = 0;

	int root_sectors = (bpb_data->root_ent * 32 + bpb_data->sector_size-1) / bpb_data->sector_size;
	int data_sectors = bpb_data->num_sec - (bpb_data->reserved + (bpb_data->num_fat * bpb_data->sec_per_fat) + root_sectors);
	int max_clusters = data_sectors / bpb_data->sec_per_cluster;

	uint8_t* new_fat = fat_entries;
	while (i <= num_clusters && j < max_clusters)
	{
		if (j % 2 == 0)
		{
			cluster = fat_entries[j/2 * 3] | ((fat_entries[j/2 * 3 + 1] & 0x0f) << 8);
		}
		else
		{
			cluster = ((fat_entries[j/2 * 3 + 1] >> 4) & 0x0f) | (fat_entries[j/2 * 3 + 2] << 4);
		}

		if (cluster == 0x000)
		{
			clusters[i - 1] = j;
			i++;
		}

		j++;
	}

	if (i <= num_clusters)
	{
		kprintf("There is no more space left on disk nr.%d\n", 0x47, index);
		return;
	}

	clusters[num_clusters] = 0xfff;

	for (int i=0; i<num_clusters; i++)
	{
		if (clusters[i] % 2 == 0)
		{
			new_fat[clusters[i]/2 * 3] = clusters[i+1] & 0xff;
			new_fat[clusters[i]/2 * 3 + 1] = (new_fat[clusters[i]/2 * 3 + 1] & 0xf0) | ((clusters[i + 1] >> 8) & 0x0f);
		}
		else
		{
			new_fat[clusters[i]/2 * 3 + 1] = (new_fat[clusters[i]/2 * 3 + 2] & 0xf0) | ((clusters[i + 1] & 0xff) << 4);
			new_fat[clusters[i]/2 * 3 + 2] = (clusters[i + 1] >> 8) & 0x0f;
		}
	}

	for (int i=0; i<bpb_data->num_fat; i++)
	{
		write_disk(index, bpb_data->sec_per_fat, (bpb_data->reserved + i*bpb_data->sec_per_fat), new_fat, bpb_data->sec_per_fat * 512);
	}

	root_entry* root_entries = (root_entry*) read_disk(index, root_sectors * bpb_data->sector_size / 512, (bpb_data->reserved + (bpb_data->num_fat * bpb_data->sec_per_fat)) * bpb_data->sector_size / 512);

	for (j = 0; j<bpb_data->root_ent; j++)
	{
		if ((uint8_t)root_entries[j].filename[0] == 0x00 || (uint8_t)root_entries[j].filename[0] == 0xe5)
		{
			char* dot = strrchr(name, '.');

			char filename[9];
			char extension[4];

			memset(filename, ' ', 8);
			filename[8] = '\0';

			memset(extension, ' ', 3);
			extension[3] = '\0';

			if (dot != NULL)
			{
				int len = dot - name;
				if (len > 8) len = 8;
				strncpy(filename, name, len);
				strupper(filename);

				strncpy(extension, dot + 1, 3);
				strupper(extension);
			}
			else
			{
				strcpy(filename, name);
				strupper(filename);
			}

			strncpy(root_entries[j].filename, filename, 8);
			strncpy(root_entries[j].extension, extension, 3);

			root_entries[j].attrib = 0x20;
			root_entries[j].create_time_ms = 0;
			root_entries[j].create_time = 0;
			root_entries[j].create_date = 0;
			root_entries[j].last_time = 0;
			root_entries[j].last_date = 0;
			root_entries[j].start_cluster = clusters[0];
			root_entries[j].size = size;

			write_disk(index, root_sectors * bpb_data->sector_size / 512, (bpb_data->reserved + (bpb_data->num_fat * bpb_data->sec_per_fat)) * bpb_data->sector_size / 512, (uint8_t*) root_entries, root_sectors * bpb_data->sector_size / 512 * 512);

			break;
		}
	}


	int size_rem = size % (bpb_data->sector_size * bpb_data->sec_per_cluster);
	for (int i=0; i<num_clusters; i++)
	{
		int cluster_size = i == num_clusters-1 ? size_rem : 512;
		write_disk(index, bpb_data->sec_per_cluster * bpb_data->sector_size / 512, bpb_data->reserved + bpb_data->num_fat * bpb_data->sec_per_fat + (bpb_data->root_ent * 32 + bpb_data->sector_size-1) / bpb_data->sector_size + (clusters[i] - 2) * bpb_data->sec_per_cluster * bpb_data->sector_size / 512, data + i * (bpb_data->sector_size * bpb_data->sec_per_cluster), cluster_size);
	}
	kfree(new_fat);
	kfree(root_entries);
	kfree(fat_entries);
}

void fat_modify_file(char* name, uint8_t* new_data, uint32_t size)
{
	root_entry file = fat_find_file(name);
	if (file.start_cluster == 0xfff)
	{
		fat_create_file(name, new_data, size);
		return;
	}
}

void fat_delete_file(char* name)
{
	root_entry file = fat_find_file(name);
	if (file.start_cluster == 0xfff)
	{
		return;
	}

	root_entry* new_root_entries = (root_entry*) read_disk(index, (bpb_data->root_ent * 32 + 511) / 512, bpb_data->reserved + bpb_data->num_fat * bpb_data->sec_per_fat);

	for (int i=0; i<bpb_data->root_ent; i++)
	{
		if (new_root_entries[i].start_cluster == file.start_cluster)
		{
			new_root_entries[i].filename[0] = (char) 0xe5;
			break;
		}
	}

	write_disk(index, (bpb_data->root_ent * 32 + 511) / 512, bpb_data->reserved + bpb_data->num_fat * bpb_data->sec_per_fat, (uint8_t*) new_root_entries, bpb_data->root_ent * 32);

	uint8_t* fat_entries = read_fat();

	uint16_t cluster = file.start_cluster;
	
	uint8_t* new_fat = fat_entries;
	while(cluster < 0xff0)
	{
		if (cluster % 2 == 0)
		{
			new_fat[cluster/2 * 3] = 0x00;
			new_fat[cluster/2 * 3 + 1] &= 0xf0;
			
			cluster = fat_entries[cluster/2 * 3] | ((fat_entries[cluster/2 * 3 + 1] & 0x0f) << 8);
		}
		else
		{
			new_fat[cluster/2 * 3 + 1] &= 0x0f;
			new_fat[cluster/2 * 3 + 2] = 0x00;
			
			cluster = ((fat_entries[cluster/2 * 3 + 1] >> 4) & 0x0f) | (fat_entries[cluster/2 * 3 + 2] << 4);
		}
	}

	// erase the last entry
	if (cluster % 2 == 0)
	{
		new_fat[cluster/2 * 3] = 0x00;
		new_fat[cluster/2 * 3 + 1] &= 0xf0;
	}
	else
	{
		new_fat[cluster/2 * 3 + 1] &= 0x0f;
		new_fat[cluster/2 * 3 + 2] = 0x00;
	}

	for (int i=0; i<bpb_data->num_fat; i++)
	{
		write_disk(index, bpb_data->sec_per_fat, bpb_data->reserved + i*bpb_data->sec_per_fat, new_fat, bpb_data->sec_per_fat * bpb_data->sector_size);
	}

	kfree(new_fat);
	kfree(fat_entries);
	kfree(new_root_entries);
}

int fat_install()
{
	if (identify_disks()) return -1;
	for (int i=0; i<4; i++)
	{
		if (!read_bpb(i)) break;
		if (i == 3)
		{
			kprint("There are no disks with FAT12!\n", 0x47);
			return -1;
		}
	}

	return 0;
}
