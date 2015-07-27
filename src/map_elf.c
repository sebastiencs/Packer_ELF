
#include "packer.h"

static int32_t	map_elf_header(t_elf *elf, void *data, size_t size)
{
  if (size < sizeof(Elf64_Ehdr)) {
    printf("Wrong file\n");
    return (-1);
  }

  if (!(elf->elf_header = malloc(sizeof(Elf64_Ehdr)))) {
    return (-1);
  }

  memcpy(elf->elf_header, data, sizeof(Elf64_Ehdr));

  if (strncmp((char *)elf->elf_header->e_ident, ELFMAG, SELFMAG)) {
    printf("Wrong file\n");
    return (-1);
  }

  if (elf->elf_header->e_machine != EM_X86_64) {
    printf("File architecture not supported. x86_64 only\n");
    return (-1);
  }

  return (0);
}

static int32_t	map_prog_header(t_elf *elf, void *data, size_t size)
{
  size_t	size_headers;

  size_headers = sizeof(Elf64_Phdr) * elf->elf_header->e_phnum;

  if (size < sizeof(Elf64_Ehdr) + size_headers) {
    printf("Wrong file\n");
    return (-1);
  }

  if (!(elf->prog_header = malloc(size_headers))) {
    perror("malloc");
    return (-1);
  }

  memcpy(elf->prog_header, (uint8_t *)data + elf->elf_header->e_phoff, size_headers);

  return (0);
}

static int32_t	map_sections_header(t_elf *elf, void *data, size_t size)
{
  uint16_t	id;

  if (!(elf->section_header = malloc(sizeof(Elf64_Shdr) * elf->elf_header->e_shnum))) {
    perror("malloc");
    return (-1);
  }

  memset(elf->section_header, 0, sizeof(Elf64_Shdr) * elf->elf_header->e_shnum);

  for (id = 0; id < elf->elf_header->e_shnum; id += 1) {

    if (size < elf->elf_header->e_shoff + id * sizeof(Elf64_Shdr)) {
      printf("Wrong file\n");
      return (-1);
    }

    memcpy(&(elf->section_header[id]), (uint8_t *)data + elf->elf_header->e_shoff + id * sizeof(Elf64_Shdr), sizeof(Elf64_Shdr));

  }

  return (0);
}

static int32_t	map_sections_data(t_elf *elf, void *data, size_t size)
{
  uint16_t	id;
  uint64_t	size_section;

  if (!(elf->section_data = malloc(sizeof(uint8_t *) * elf->elf_header->e_shnum))) {
    perror("malloc");
    return (-1);
  }

  memset(elf->section_data, 0, sizeof(uint8_t *) * elf->elf_header->e_shnum);

  for (id = 0; id < elf->elf_header->e_shnum; id += 1) {

    if (elf->section_header[id].sh_type == SHT_NOBITS) {
      elf->section_data[id] = (uint8_t *)0;
    }
    else {

      if (size < elf->section_header[id].sh_offset) {
	printf("Wrong file\n");
	return (-1);
      }

      size_section = elf->section_header[id].sh_size;
      if (!(elf->section_data[id] = malloc(size_section))) {
	perror("malloc");
	return (-1);
      }
      memset(elf->section_data[id], 0, size_section);
      memcpy(elf->section_data[id], (uint8_t *)data + elf->section_header[id].sh_offset, size_section);
    }

  }

  return (0);
}

void		*map_elf(void *data, size_t size)
{
  t_elf		*elf;

  if (!(elf = malloc(sizeof(t_elf)))) {
    perror("malloc");
    return ((t_elf *)0);
  }
  memset(elf, 0, sizeof(t_elf));

  if (map_elf_header(elf, data, size)) {
    return ((t_elf *)0);
  }

  if (map_prog_header(elf, data, size)) {
    return ((t_elf *)0);
  }

  if (map_sections_header(elf, data, size)) {
    return ((t_elf *)0);
  }

  if (map_sections_data(elf, data, size)) {
    return ((t_elf *)0);
  }

  return (elf);
}
