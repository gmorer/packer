#include "woody.h"

char *get_new_file(char *old_file, off_t file_size)
{
	char *result;

	if (!(result = malloc(file_size)))
	{
		dprintf(2, "malloc failed\n");
		return (NULL);
	}
	memcpy(result, old_file, (size_t)file_size);
	return (result);
}

int write_to_file(char *file_name, char *content, off_t content_size)
{
	int fd;
	size_t writed;

	fd = open(file_name, O_WRONLY | O_CREAT, S_IRWXU);
	if (fd == -1)
	{
		dprintf(2, "Cannot create the new binary \"%s\":(\n", file_name);
		return (0);
	}
	writed = write(fd, content, content_size);
	if (writed != (size_t)content_size)
	{
		dprintf(2, "incomplete write to the new binary\n");
		close(fd);
		unlink(file_name);
		return (0);
	}
	close(fd);
	return (1);
}

Elf64_Addr get_virt_addr(char *file, off_t file_size, int *error, off_t *offset_max)
{
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	size_t headers_length;
	off_t index;

	ehdr = (void *)file;
	headers_length = ehdr->e_phnum;
	if ((off_t)(headers_length * sizeof(Elf64_Phdr) + ehdr->e_phoff) > file_size)
	{
		dprintf(2, "corrupted binary\n");
		return 0;
	}
	phdr = (void *)(file + ehdr->e_phoff);
	index = 0;
	*error = 1;
	if (!headers_length)
	{
		dprintf(2, "There is no program header\n");
		return (0);
	}
	while ((size_t)index < headers_length)
	{
		if (phdr[index].p_type == 1) // check the p_type for 1 which is PT_LOAD
		{
			*offset_max = phdr[index].p_memsz;
			*error = 0;
			return (phdr[index].p_vaddr);
		}
		index += 1;
	}
	dprintf(2, "unamble to find the PT_LOAD program header\n");
	return (0);
}

int build_payload(char *file, char *new_file, char *code, off_t code_len, Elf64_Shdr *section, off_t virt_addr, char *key)
{
	off_t offset;
	int jmp_adr;
	int	offset_virt;
	char jmp_code[] = "\xe9\xde\xad\xbe\xef";

	offset = ((Elf64_Ehdr *)new_file)->e_entry;
	jmp_adr = ((Elf64_Ehdr *)file)->e_entry - (offset + code_len);
	jmp_adr -= 5;
	memcpy(jmp_code + 1, &jmp_adr, sizeof(int));
	offset += (WOODY_DEBUG ? 4 : 0);
	offset_virt = section->sh_offset + virt_addr ; 
	memcpy(code + NEW_EP_OFFSET, &offset, sizeof(int));
	memcpy(code + TEXT_LENGTH_OFFSET, &(section->sh_size), sizeof(int));
	memcpy(code + TEXT_OFFSET_OFFSET, &(offset_virt), sizeof(int));
	memcpy(code + KEY1_OFFSET_OFFSET, key, sizeof(uint64_t));
	memcpy(code + KEY2_OFFSET_OFFSET, key + 8, sizeof(uint64_t));
	offset -= ((WOODY_DEBUG ? 4 : 0) + virt_addr + OFFSET_PLACE);
	memcpy(new_file + offset, code, code_len);
	offset += code_len;
	memcpy(new_file + offset, jmp_code, sizeof(jmp_code) - 1);
	return 1;
}

char *inject_code(char *file, off_t *file_size, Elf64_Shdr *section, char *key)
{
	off_t virt_addr;
	int error;
	char *new_file;
	char code[] = PAYLOAD;
	off_t cave_entry;
	off_t offset_max;


	virt_addr = get_virt_addr(file, *file_size, &error, &offset_max);
	if (error)
		return (NULL);
	new_file = get_new_file(file, *file_size);
	cave_entry = make_place(&new_file, file_size, sizeof(code) - 1);
	metamorph_segment(new_file, cave_entry, sizeof(code) - 1, virt_addr);
	cave_entry += virt_addr + OFFSET_PLACE;
	((Elf64_Ehdr *)new_file)->e_entry = cave_entry;
	build_payload(file, new_file, code, sizeof(code) - 1, section, virt_addr, key);
	return (new_file);
}
