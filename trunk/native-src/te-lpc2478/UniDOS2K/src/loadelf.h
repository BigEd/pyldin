#ifndef _ELF_H_
#define _ELF_H_

#define ELF_NO_FILE	(-1000)

int exec_elf(void *addr, size_t stacksize, char *arg);

#endif
