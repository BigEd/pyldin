#ifndef _ELF_H_
#define _ELF_H_

#define ELF_NO_FILE	(-1000)

int load_elf(void *addr, char *arg, void *entry);

#endif
