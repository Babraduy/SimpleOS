#include <stdint.h>
#include <stddef.h>
#include "disk_driver.h"
#include "libstr.h"
#include "libio.h"
#include "heap.h"

uint16_t ports[] = {0x1f0,0x170 };
ata_disk disks[4];

int wait_bsy(uint8_t index)
{
	while (inb(disks[index].port + 0x07) & BSY);

	if (check_error(index))
	{
		return -1;
	}

	return 0;
}

int wait_drq(uint8_t index)
{
	while (!(inb(disks[index].port + 0x07) & DRQ));

	if (check_error(index))
	{
		return -1;
	}
	return 0;
}

int check_error(uint8_t index)
{
	if (inb(disks[index].port + 0x07) & ERR)
	{
		kprintf("DISK ERROR, CODE: %x\n", 0x47, inb(disks[index].port + 0x01));
		return -1;
	}
	return 0;
}

int identify_disks()
{
	// 0 - primary, 1 - secondary
	for (int i=0; i<2; i++)
	{
		// 0 - master, 1 - slave
		for (int j=0; j<2; j++)
		{
			strcpy(disks[i*2 + j].model, "NOT INSTALLED\0");

			// check if the disk is available
			outb(ports[i] + 0x06, 0xa0 | (j << 4));

			for (int k=0;k<4;k++) inb(ports[i] + 0x07);	// wait 400 ns

			outb(ports[i] + 0x02, 0);
			outb(ports[i] + 0x03, 0);
			outb(ports[i] + 0x04, 0);
			outb(ports[i] + 0x05, 0);
			outb(ports[i] + 0x07, 0xec);

			if (inb(ports[i] + 0x07) == 0x00)
			{
				// disk does not exist
				continue;
			}
			
			if (inb(ports[i] + 0x04) != 0 && inb(ports[i] + 0x05) != 0)
			{
				// disk is not ata
				disks[i*2 + j].port = 0x000;
				continue;
			}

			disks[i*2 + j].port = ports[i];
			if (wait_bsy(i*2 + j) || wait_drq(i*2 + j))
			{
				// disk error, initialize port to 0
				disks[i*2 + j].port = 0x000;
				continue;
			}

			uint16_t buffer[256];

			for (int k=0; k<256; k++)
			{
				buffer[k] = inw(ports[i]);
			}
			
			// set up info about disk
			disks[i*2 + j].is_slave = j;
			disks[i*2 + j].total_sectors = (buffer[61] << 16) |  buffer[60];

			for (int k=0; k<20; k++)
			{
				disks[i*2 + j].model[k * 2] = buffer[27 + k] >> 8;
				disks[i*2 + j].model[k*2+1] = buffer[27 + k] & 0xff;
			}
			disks[i*2 + j].model[40] = '\0';
		}
	}

	return 0;
}

uint8_t* read_disk(uint8_t index, uint32_t num_sectors, uint32_t start)
{
	if (index >= 4)
	{
		kprintf("Invalid disk number: %d\n", 0x47, index);
		return NULL;
	}

	if (disks[index].port == 0x000) return NULL;

	if (num_sectors == 0) num_sectors = disks[index].total_sectors - start;

	uint8_t* read_buffer = (uint8_t*) kmalloc(num_sectors*512);
	if (read_buffer == NULL)
	{
		kprint("Could not allocate memory for 'read_buffer'\n", 0x47);
		return NULL;
	}

	uint32_t lba = start;
	
	int total_sectors_read = 0;
	while (num_sectors > 0)
	{
		int sectors_to_read = num_sectors >= 256 ? 256 : num_sectors;
		
		outb(disks[index].port + 0x06, 0xe0 | (disks[index].is_slave << 4) | ((lba >> 24) & 0x0f));		// choose disk
		if (wait_bsy(index))
		{
			kfree (read_buffer);
			return NULL;
		}
		outb(disks[index].port + 0x02, num_sectors >= 256 ? 0 : num_sectors);	// number of sectors to read
		outb(disks[index].port + 0x03, lba & 0xff);					// lba lower byte
		outb(disks[index].port + 0x04, (lba >> 8) & 0xff);				// lba medium bye
		outb(disks[index].port + 0x05, (lba >> 16) & 0xff);				// lba higher byte
		outb(disks[index].port + 0x07, 0x20);					// read command

		for (int i=0; i<sectors_to_read; i++)
		{
			if (wait_bsy(index) || wait_drq(index))
			{
				kfree(read_buffer);
				return NULL;
			}

			for (int w=0; w<256; w++)
			{
				uint16_t word = inw(disks[index].port);
				read_buffer[(total_sectors_read + i) * 512 + w*2] = word & 0xff;
				read_buffer[(total_sectors_read + i) * 512 + w*2 + 1] = (word >> 8) & 0xff;
			}
		}

		lba += sectors_to_read;
		total_sectors_read += sectors_to_read;
		num_sectors -= sectors_to_read;
	}

	return read_buffer;
}

int write_disk (uint8_t index, uint32_t num_sectors, uint32_t start, uint8_t* data, int size)
{
	if (index >= 4)
	{
		kprintf("Invalid disk number: %d\n", 0x47, index);
		return -1;
	}

	if (disks[index].port == 0x000) return -1;

	uint32_t lba = start;

	uint8_t* buffer = (uint8_t*) kmalloc(512 * num_sectors);
	if ((uint32_t)size > 512 * num_sectors) size = 512 * num_sectors;
	memset(buffer, 0, 512 * num_sectors);
	memcpy(buffer, data, size);

	int total_sectors_write = 0;
	while (num_sectors > 0)
	{
		int sectors_to_write = num_sectors >= 256 ? 0 : num_sectors;

		outb(disks[index].port + 0x06, 0xe0 | (disks[index].is_slave << 4) | ((lba >> 24) & 0x0f));		// choose disk
		if (wait_bsy(index))
		{
			kfree(buffer);
			return -1;
		}
		outb(disks[index].port + 0x02, num_sectors >= 256 ? 0 : num_sectors);	// number of sectors to write
		outb(disks[index].port + 0x03, lba & 0xff);					// lba lower byte
		outb(disks[index].port + 0x04, (lba >> 8) & 0xff);				// lba medium byte
		outb(disks[index].port + 0x05, (lba >> 16) & 0xff);				// lba high byte
		outb(disks[index].port + 0x07, 0x30);					// write command

		for (int i=0; i<sectors_to_write; i++)
		{
			if (wait_bsy(index) || wait_drq(index))
			{
				kfree(buffer);
				return -1;
			}

			for (int w=0; w<256; w++)
			{
				uint8_t low = buffer[(total_sectors_write+ i) * 512 + w*2];
				uint8_t high = buffer[(total_sectors_write + i) * 512 + w*2 + 1];

				uint16_t word = (high << 8) | low;

				outw(disks[index].port, word);

				for (int wait = 0; wait < 4; wait++) outb(0x80, 0x00);
			}
		}

		outb(disks[index].port + 0x07, 0xe7);

		sectors_to_write = sectors_to_write == 0 ? 256 : sectors_to_write;

		lba += sectors_to_write;
		total_sectors_write += sectors_to_write;
		num_sectors -= sectors_to_write;
	}

	kfree(buffer);

	return 0;
}

void list_disks()
{
	for (int i=0; i<4; i++)
	{
		set_stringf(0, VGA_HEIGHT-4+i, "PORT:%x, TOTAL:%d, MODEL:%s", 0x47, disks[i].port, disks[i].total_sectors, disks[i].model);
	}
}
