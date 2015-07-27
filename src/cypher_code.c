
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include "packer.h"

extern uint64_t		entry_addr, entry_size, key;

static char*	get_section_name(t_elf *elf, int id)
{
  uint16_t	shname;

  shname = elf->elf_header->e_shstrndx;
  return ((char*)(elf->section_data[shname] + elf->section_header[id].sh_name));
}

static uint16_t	get_section_text(t_elf *elf)
{
  uint16_t	id;

  for (id = 0; id < elf->elf_header->e_shnum; id += 1) {
    if (!strcmp(".text", get_section_name(elf, id))) {
      return (id);
    }
  }
  return -1;
}

static uint64_t	get_random_key(void)
{
  uint64_t	key;

  syscall(__NR_getrandom, &key, 8, 0);
  printf("key: 0x%lX\n", key);
  return (key);
}

static uint64_t	rotate_right(uint64_t key)
{
  return (((key & 0xFF) << 56) | (key >> 8));
}

int32_t		cypher_code(t_elf *elf)
{
  uint16_t	id;

  if ((id = get_section_text(elf)) == (uint16_t)-1) {
    printf("Can't find section '.text'\n");
    return (-1);
  }

  uint64_t	size, offset, lkey;
  uint8_t	*data;

  size = elf->section_header[id].sh_size;
  data = elf->section_data[id];

  entry_addr = elf->section_header[id].sh_addr;
  entry_size = size;
  key = get_random_key();
  lkey = key;

  for (offset = 0; offset < size; offset += 1) {
    data[offset] ^= (uint8_t)lkey;
    lkey = rotate_right(lkey);
  }

  return (0);
}
