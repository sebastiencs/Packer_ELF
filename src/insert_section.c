
#include "packer.h"

extern uint64_t		loader_size;
extern uint64_t		infos_size;
extern void		entry_loader(void);

static Elf64_Shdr	new_section = {
  .sh_name = (uint32_t)0,
  .sh_type = (uint32_t)SHT_PROGBITS,
  .sh_flags = (uint64_t)SHF_EXECINSTR | SHF_ALLOC,
  .sh_addr = (Elf64_Addr)0,
  .sh_offset = (Elf64_Off)0,
  .sh_size = (uint64_t)0,
  .sh_link = (uint32_t)0,
  .sh_info = (uint32_t)0,
  .sh_addralign = (uint64_t)16,
  .sh_entsize = (uint64_t)0,
};

uint64_t		entry_addr, entry_size, key;

static int32_t	create_section(t_elf *elf, uint16_t last_section, uint16_t last_ptload)
{
  Elf64_Shdr	*new_shdrs;
  uint8_t	**new_sects;
  uint8_t	*loader;

  elf->elf_header->e_shnum += 1;

  if (!(new_shdrs = realloc(elf->section_header, sizeof(Elf64_Shdr) * elf->elf_header->e_shnum))) {
    perror("realloc");
    return (-1);
  }

  if (!(new_sects = realloc(elf->section_data, sizeof(uint8_t *) * elf->elf_header->e_shnum))) {
    perror("realloc");
    return (-1);
  }

  elf->section_header = new_shdrs;
  elf->section_data = new_sects;

  new_section.sh_offset = elf->prog_header[last_ptload].p_offset + elf->prog_header[last_ptload].p_memsz;
  new_section.sh_addr = elf->prog_header[last_ptload].p_vaddr + elf->prog_header[last_ptload].p_memsz;
  new_section.sh_size = loader_size;

  if (!(loader = malloc(loader_size))) {
    perror("malloc");
    return (-1);
  }

  memcpy(loader, (void *)entry_loader, loader_size);

  memcpy(loader + loader_size - 24, &key, sizeof(uint64_t));
  memcpy(loader + loader_size - 16, &entry_addr, sizeof(uint64_t));
  memcpy(loader + loader_size - 8, &entry_size, sizeof(uint64_t));

  memmove(new_shdrs + last_section + 2, new_shdrs + last_section + 1, sizeof(Elf64_Shdr) * (elf->elf_header->e_shnum - last_section - 2));
  memmove(new_sects + last_section + 2, new_sects + last_section + 1, sizeof(uint8_t *) * (elf->elf_header->e_shnum - last_section - 2));

  memcpy(new_shdrs + last_section + 1, &new_section, sizeof(Elf64_Shdr));
  new_sects[last_section + 1] = loader;

  return (0);
}

static void	change_entry(t_elf *elf, uint16_t last_section)
{
  Elf64_Addr	last_entry;
  int32_t	jump;

  last_entry = elf->elf_header->e_entry;
  elf->elf_header->e_entry = elf->section_header[last_section].sh_addr;
  jump = last_entry - (elf->elf_header->e_entry + loader_size - infos_size);
  memcpy(elf->section_data[last_section] + loader_size - (infos_size + 4), &jump, 4);
}

int32_t		insert_section(t_elf *elf)
{
  uint16_t	last_section = (uint16_t)-1;
  uint16_t	last_ptload = (uint16_t)-1;

  for (uint16_t id = 0; id < elf->elf_header->e_phnum; id += 1) {
      if (elf->prog_header[id].p_type == PT_LOAD) {
	last_ptload = id;
      }
  }
  if (last_ptload == (uint16_t)-1) {
    printf("can't find PT_LOAD section\n");
    return (-1);
  }

  for (uint16_t id = 0; id < elf->elf_header->e_shnum; id += 1) {

    Elf64_Phdr	*phdr = elf->prog_header + last_ptload;
    Elf64_Shdr	*shdr = elf->section_header + id;

    if (shdr->sh_addr + shdr->sh_size >= phdr->p_vaddr + phdr->p_memsz) {
      last_section = id;
    }
  }
  if (last_section == (uint16_t)-1) {
    printf("can't find last section\n");
    return (-1);
  }

  create_section(elf, last_section, last_ptload);

  last_section += 1;

  uint64_t	size = elf->prog_header[last_ptload].p_memsz + loader_size;
  elf->prog_header[last_ptload].p_memsz = size;
  elf->prog_header[last_ptload].p_filesz = size;

  for (uint16_t i = 0; i < elf->elf_header->e_phnum; i++) {
    if(elf->prog_header[i].p_type == PT_LOAD) {
      elf->prog_header[i].p_flags = PF_X | PF_W | PF_R;
    }
  }

  change_entry(elf, last_section);

  for (uint16_t i = last_section; i < elf->elf_header->e_shnum - 1; i += 1) {
    elf->section_header[i + 1].sh_offset = elf->section_header[i].sh_offset + elf->section_header[i].sh_size;
  }

  if (elf->elf_header->e_shstrndx > last_section) {
    elf->elf_header->e_shstrndx += 1;
  }

  uint16_t	shnum = elf->elf_header->e_shnum;
  elf->elf_header->e_shoff = elf->section_header[shnum - 1].sh_offset + elf->section_header[shnum - 1].sh_size;

  return (0);
}
