
#include "packer.h"

void		*map_file(char *filename, size_t *size_ptr)
{
  void		*data;
  off_t		size;
  int32_t	fd;

  if ((fd = open(filename, O_RDONLY)) < 0) {
      perror("open");
      return ((void *)0);
  }

  if ((size = lseek(fd, 0, SEEK_END)) < 0) {
    perror("lseek");
    return ((void *)0);
  }

  if ((data = mmap((void *)0, (size_t)size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
    perror("mmap");
    return ((void *)0);
  }

  close(fd);
  *size_ptr = (size_t)size;

  return (data);
}

int		main(int argc, char **argv)
{
  void		*data;
  t_elf		*elf;
  size_t	size;

  if (argc < 2) {
    printf("usage: %s FILE\n", argv[0]);
    return (0);
  }

  if (!(data = map_file(argv[1], &size))) {
    return (-1);
  }

  if (!(elf = map_elf(data, size))) {
    return (-1);
  }

  munmap(data, size);

  if (cypher_code(elf)) {
    return (-1);
  }

  if (insert_section(elf)) {
    return (-1);
  }

  write_file(elf);

  for (uint16_t id = 0; id < elf->elf_header->e_shnum; id += 1) {
    free(elf->section_data[id]);
  }
  free(elf->elf_header);
  free(elf->prog_header);
  free(elf->section_data);
  free(elf->section_header);
  free(elf);

  return (0);
}
