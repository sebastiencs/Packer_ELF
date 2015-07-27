#ifndef PACKER_H_
# define PACKER_H_

# include <stdio.h>
# include <elf.h>
# include <string.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/types.h>
# include <unistd.h>

typedef struct	s_elf
{
  Elf64_Ehdr	*elf_header;
  Elf64_Phdr	*prog_header;
  Elf64_Shdr	*section_header;
  uint8_t	**section_data;
}		t_elf;

void		*map_elf(void *, size_t);
int32_t		cypher_code(t_elf *);
void		write_file(t_elf *);
int32_t		insert_section(t_elf *);

#endif /* !PACKER_H_ */
