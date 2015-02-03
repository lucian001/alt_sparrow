/*
 * Copyright 2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */
 
/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
 
 
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>                /* strcmp() */
#include <stdlib.h>                /* strtoul() */
#include <unistd.h>                /* close() */
 
unsigned long int g_paddr = LOG_ADDRESS;
uint32_t log_size         = LOG_SIZE; 
uint32_t g_count = 0;

char convert[256] = {
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '\t', '\n', '.', '.', '.', '.', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'};
 
void printmem(void *addr, uint32_t count)
{
	uint32_t offset = 0;
	char *as_char = (char *)addr;
	while(offset < count)
		offset += printf("%c", convert[*(as_char + offset)]);
	printf("\n");
}
 
int main(int argc, char **argv)
{
	int fd;
	void * mem;
	void * aligned_vaddr;
	uint32_t *p_magic;
	uint32_t *p_size;
	unsigned long aligned_paddr;
	uint32_t aligned_size;

	if(argc > 1)
		g_paddr = strtol(argv[1], NULL, 16);
	if(argc > 2)
		log_size = strtol(argv[2], NULL, 16);

	/* Align address to access size */
	aligned_paddr = g_paddr & ~(4096 - 1);
	aligned_size = g_paddr - aligned_paddr + log_size;
	aligned_size = (aligned_size + 4096 - 1) & ~(4096 - 1);

	if ((fd = open("/dev/mem", O_RDWR, 0)) < 0) {
		printf("Open /dev/mem error, maybe check permission.\n");
		return 1;
	}
 
	aligned_vaddr = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, aligned_paddr);
	if (aligned_vaddr == NULL) {
		printf("Error mapping address\n");
		close(fd);
		return 1;
	}
 
	mem = (void *)((void *)aligned_vaddr + (g_paddr - aligned_paddr));
 
	p_magic = (mem + log_size - 4);
	p_size = (mem + log_size - 8);
	if(*p_magic != 0x12348765)
		printf("Error, magic %x not valid\n", *p_magic);

	printf("Reading Log (%d/0x%x bytes) at address 0x%08X\n",
		*p_size, *p_size, (uint)g_paddr);
 
	printmem(mem, *p_size);
 
	munmap(aligned_vaddr, aligned_size);
	close(fd);
	return 0;
}
