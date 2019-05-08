#include "woody.h"

int main(int ac, char **av)
{
	char		*file;
	char		*new_file;
	off_t		file_size;
	Elf64_Shdr	*text;
	char		key[16];

	srand(time(NULL));
	if (ac < 2)
	{
		dprintf(2, "Usage: %s [FILENAME]\n", av[0]);
		return (1);
	}
	file_size = 0;
	file = get_file(av[1], &file_size);
	if (!file || !verif_header(file, file_size))
		return (1);
	printf(" * File type [OK]\n");
	if (is_infected(file, file_size))
		return (1);
	if (!(text = find_sect(file, ".text", file_size)))
		return (1);
	generate_key(key);
	new_file = inject_code(file, &file_size, text, key);
	if (!text || text->sh_offset + text->sh_size > (size_t)file_size)
	{
		dprintf(2, text ? "[KO] No text section\n" : "[KO] Invalid file\n");
		return (1);
	}
	printf(" * Code injection [OK]\n");
	encrypt_section(new_file, text, key);
	printf(" * .text section encrypted [OK]\nFinish !\n");
	return write_to_file(FILE_NAME, new_file, file_size);
}
