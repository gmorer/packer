#include "woody.h"

int verif_header(char *file, off_t size)
{
	if (strncmp(file, ELFMAG, sizeof(ELFMAG) - 1) || size < EI_CLASS)
	{
		dprintf(2, "the file is not an elf\n");
		return (0);
	}
	if ((file[EI_CLASS] == ELFCLASS32 && (size_t)size < sizeof(Elf32_Ehdr))
        || (file[EI_CLASS] == ELFCLASS64 && (size_t)size < sizeof(Elf64_Ehdr)))
	{
		dprintf(2, "The file is too small\n");
		return (0);
	}
	if (file[EI_CLASS] == ELFCLASS32)
	{
		if (((Elf32_Ehdr *)file)->e_type != ET_EXEC && ((Elf32_Ehdr *)file)->e_type != ET_DYN)
		{
			dprintf(2, "the file is not an executable\n");
			return (0);
		}
		return (32);
	}
	else if (file[EI_CLASS] == ELFCLASS64)
	{
		if (((Elf64_Ehdr *)file)->e_type != ET_EXEC && ((Elf64_Ehdr *)file)->e_type != ET_DYN)
		{
			dprintf(2, "the file is not an executable\n");
			return (0);
		}
		return (64);
	}
	else
	{
		dprintf(2, "not a valid format\n");
		return (0);
	}
}
