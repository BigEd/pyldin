/* Relocable ELF loader (c) 2011, Alexander Chukov <sash@pdaXrom.org>
 * Based on ELF loader
 * Copyright (C) 2009-2010, Alexander Graf <agraf@znc.in>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <alloca.h>

#include "elf.h"
#include "loadelf.h"

//#define DEBUG

#define min(a, b)	((a) < (b) ? (a) : (b))

#ifndef TEST
/* Execute.  Add ELF program entry,
 * - r0 contains a pointer to the argument command line, or 0
 * - r1 contains the entry point of the ELF
 * - r2 contains a useful stack pointer
 */
int exec_mem(char *elfarg, void *entry, void *sp);
asm (
    "exec_mem:		\n\t"
    "push {r1-r12, lr}	\n\t"
    "mov r3, sp		\n\t"
    "mov sp, r2		\n\t"
    "bic sp, sp, #7	\n\t"
    "push {r3, r4}	\n\t"
    "mov lr, pc		\n\t"
    "bx  r1		\n\t"
    "pop {r3, r4}	\n\t"
    "mov sp, r3		\n\t"
    "pop {r1-r12, lr}	\n\t"
    "bx  lr		\n\t"
);
#endif

int load_elf(void *addr, char *arg, void *entry)
{
    Elf32_Ehdr ehdr;
    Elf32_Phdr *phdr;
    Elf32_Shdr *shdr;
    Elf32_Sym  *dynsym = NULL;
    void *ph = NULL;
    char *strtable;
    char *elffile, *elfarg;
    int inst;
    int r, i;
    unsigned int j;

    if (!arg) {
#ifdef DEBUG
	fprintf(stderr, "No argument given.\n");
#endif
	return -1;
    }


    ph = addr;

    elffile = arg;

    elfarg = strchr(arg, ' ');
    if (elfarg) {
	*elfarg = 0;
    }

    /* open ELF file */
    inst = open(elffile, O_RDONLY);
    if (inst < 0) {
#ifdef DEBUG
	fprintf(stderr, "Error opening ELF file!\n");
#endif
	return -1;
    }

    /* read header */
    r = read(inst, &ehdr, sizeof(Elf32_Ehdr));
    if (r != sizeof(Elf32_Ehdr))
	goto fail;

    /* validate header information */
    if (r != sizeof(Elf32_Ehdr) ||
	strncmp((char *)ehdr.e_ident, ELFMAG, SELFMAG) ||
	ehdr.e_ident[EI_CLASS] != ELFCLASS32 ||
	ehdr.e_ident[EI_DATA] != ELFDATA2LSB ||
	ehdr.e_ident[EI_VERSION] != EV_CURRENT ||
	(/*ehdr.e_type != ET_EXEC &&*/ ehdr.e_type != ET_DYN) ||
	ehdr.e_machine != EM_ARM ||
	ehdr.e_version != EV_CURRENT ||
	ehdr.e_shstrndx >= ehdr.e_shnum ||
	ehdr.e_phentsize != sizeof(Elf32_Phdr) ||
	ehdr.e_shentsize != sizeof(Elf32_Shdr)) {
	    fprintf(stderr, "Not a supported ELF file.\n");
	    goto fail;
    }

#ifdef DEBUG
    fprintf(stderr, "Entry point %08x\n", ehdr.e_entry);
#endif

    /* read & load program headers. */
    phdr = alloca(ehdr.e_phnum * sizeof(Elf32_Phdr));
    r = lseek(inst, ehdr.e_phoff, SEEK_SET);
    if ((size_t)r != ehdr.e_phoff) {
#ifdef DEBUG
	fprintf(stderr, "elf - error lseek()\n");
#endif
	goto fail;
    }
    r = read(inst, phdr, ehdr.e_phnum * sizeof(Elf32_Phdr));
    if (r < 0)
	goto fail;
    for (i = 0; i < ehdr.e_phnum; i++) {
	/* we only need to look at loadable program headers */
	if (phdr[i].p_type != PT_LOAD)
	    continue;


#ifdef DEBUG
	fprintf(stderr, "ELF load address %08x, size %08x (%u)\n", phdr[i].p_paddr, phdr[i].p_memsz, phdr[i].p_memsz);
#endif

	/* now load */
	r = lseek(inst, phdr[i].p_offset, SEEK_SET);
	if ((size_t)r != phdr[i].p_offset) {
#ifdef DEBUG
	    fprintf(stderr, "elf - error lseek()\n");
#endif
	    goto fail;
	}

	r = read(inst, (void *) (ph + phdr[i].p_paddr), min(phdr[i].p_filesz, phdr[i].p_memsz));
	if (r < 0)
	    goto fail;

	if (phdr[i].p_memsz > phdr[i].p_filesz)
	    /* zero out .bss (or whatever needs to be zeroed out
	     * here) */
	    memset((void *) (ph + phdr[i].p_paddr + phdr[i].p_filesz), 0,
	    phdr[i].p_memsz - phdr[i].p_filesz);
    }

    if (ehdr.e_type == ET_DYN) {
	/* dynamic ELF (maybe PIC, as needed for MMU-less systems for
	 * global symbol references). We have to do runtime linking now
	 * to ensure that these references do not cause a data abort
	 * exception. */

	if (ehdr.e_shstrndx != SHN_UNDEF) {
#ifdef DEBUG
	    fprintf(stderr, "shdr_size = %d\n", ehdr.e_shnum * sizeof(Elf32_Shdr));
#endif
	    /* read section table */
	    shdr = alloca(ehdr.e_shnum * sizeof(Elf32_Shdr));

	    r = lseek(inst, ehdr.e_shoff, SEEK_SET);
	    if ((size_t)r != ehdr.e_shoff) {
#ifdef DEBUG
		fprintf(stderr, "elf - error lseek()\n");
#endif
		goto fail;
	    }

	    r = read(inst, shdr, ehdr.e_shnum * sizeof(Elf32_Shdr));
	    if (r < 0) {
#ifdef DEBUG
		fprintf(stderr, "shdr\n");
#endif
		goto fail;
	    }

#ifdef DEBUG
	    fprintf(stderr, "sh_size = %d\n", shdr[ehdr.e_shstrndx].sh_size);
#endif
	    /* read section name string table */
	    strtable = alloca(shdr[ehdr.e_shstrndx].sh_size);

	    r = lseek(inst, shdr[ehdr.e_shstrndx].sh_offset, SEEK_SET);
	    if ((size_t)r != shdr[ehdr.e_shstrndx].sh_offset) {
#ifdef DEBUG
		fprintf(stderr, "elf - error lseek()\n");
#endif
		goto fail;
	    }

	    r = read(inst, strtable, shdr[ehdr.e_shstrndx].sh_size);
	    if (r < 0) {
#ifdef DEBUG
		fprintf(stderr, "strtable\n");
#endif
		goto fail;
	    }

	    /* now search till we find proper section */
	    for (i = 1; i < ehdr.e_shnum; i++) {
		Elf32_Rel *rel;
#ifdef DEBUG
		fprintf(stderr, "section %s\n", strtable + shdr[i].sh_name);
#endif
		if (shdr[i].sh_type == SHT_DYNSYM &&
		    !strcmp(strtable + shdr[i].sh_name, ".dynsym")) {
		    dynsym = ph + shdr[i].sh_addr;
		    continue;
		}
		/* check */
		if (shdr[i].sh_type != SHT_REL ||
		    (strcmp(strtable + shdr[i].sh_name, ".rel.text") &&
		     strcmp(strtable + shdr[i].sh_name, ".rel.rodata") &&
		     strcmp(strtable + shdr[i].sh_name, ".rel.data") &&
		     strcmp(strtable + shdr[i].sh_name, ".rel.got") &&
		     strcmp(strtable + shdr[i].sh_name, ".rel.dyn")))
		    continue;

#ifdef DEBUG
		fprintf(stderr, "Relocating...\n");
#endif

		rel = ph + shdr[i].sh_addr;
		for (j = 0; j < (shdr[i].sh_size / sizeof(Elf32_Rel)); j++) {
		    switch (ELF32_R_TYPE(rel[j].r_info)) {
		    case R_ARM_ABS32:
			*(uint32_t *)(ph + rel[j].r_offset) = (uint32_t)ph + dynsym[ELF32_R_SYM(rel[j].r_info)].st_value;
			break;
		    case R_ARM_RELATIVE:
			*(uint32_t *)(ph + rel[j].r_offset) += (uint32_t)ph;
			break;
		    default:
			fprintf(stderr, "%s: unsipported relocation type %08X\n", __FILE__, ELF32_R_TYPE(rel[j].r_info));
			goto fail;
		    }
/*
#ifdef DEBUG
		    fprintf(stderr, "[%08X](%08X) = %08X\n", rel[j].r_offset, rel[j].r_info, *(int *) (ph + rel[j].r_offset));
#endif
 */
		}
	    }
	}
    }

    /* close file handle */
    close(inst);

    if (elfarg)
	*elfarg = ' ';

    *(uint32_t *)entry = (uint32_t)(ph + ehdr.e_entry);

    return 0;

fail:
    close(inst);

    if (elfarg)
	*elfarg = ' ';

#ifdef DEBUG
    fprintf(stderr, "Error loading ELF file!\n");
#endif
    return -1;
}


#ifndef TEST
int exec_elf(void *addr, size_t stacksize, char *arg)
{
    void *entry = NULL;
    char *elfarg;

    if (load_elf(addr, arg, &entry))
	return ELF_NO_FILE;

    elfarg = alloca(strlen(arg) + 1);
    strcpy(elfarg, arg);

#ifdef DEBUG
    fprintf(stderr, "[%s] entry = %p\n", elfarg, entry);
#endif

    return exec_mem(elfarg, entry, entry + stacksize);
}

#else
int main(int argc, char *argv[])
{
    void *entry = NULL;
    char *mem = malloc(1024 * 1024);

    load_elf(mem, argv[1], &entry);

    fprintf(stderr, "[%s] entry = %p\n", argv[1], entry);

    FILE *f = fopen("mem.bin", "wb");
    if (f) {
	fwrite(mem, 1, 1024*1024, f);
	fclose(f);
    }

    return 0;
}
#endif
