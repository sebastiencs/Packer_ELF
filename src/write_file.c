
#include "packer.h"

#define FILENAME	("exec")

static uint64_t	off = 0;

static inline void	write_to_file(int fd, void *data, uint64_t size)
{
  if (write(fd, data, size) != (ssize_t)size) {
    perror("write");
    exit(-1);
  }
  off += size;
}

static void		pad_zero(int fd, uint64_t end)
{
  static const char	c = 0;

  while (off < end) {
    write_to_file(fd, (void *)&c, 1);
  }
}

void			write_file(t_elf *elf)
{
  int			fd;

  if ((fd = open(FILENAME, O_CREAT | O_WRONLY, 0744)) < 0) {
    perror("open");
    return ;
  }

  write_to_file(fd, elf->elf_header, sizeof(Elf64_Ehdr));
  pad_zero(fd, elf->elf_header->e_phoff);
  write_to_file(fd, elf->prog_header, sizeof(Elf64_Phdr) * elf->elf_header->e_phnum);

  for (uint16_t id = 0; id < elf->elf_header->e_shnum; id += 1) {
    if (elf->section_header[id].sh_type != SHT_NOBITS) {
      pad_zero(fd, elf->section_header[id].sh_offset);
      write_to_file(fd, elf->section_data[id], elf->section_header[id].sh_size);
    }
  }
  pad_zero(fd, elf->elf_header->e_shoff);

  for (uint16_t id = 0; id < elf->elf_header->e_shnum; id += 1) {
    write_to_file(fd, &elf->section_header[id], sizeof(Elf64_Shdr));
  }

  close(fd);

  printf("file created: '%s'\n", FILENAME);
}
