/* Copyright (C) 2009-2010, Alexander Graf <agraf@znc.in>
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

#define min(a, b)	((a) < (b) ? (a) : (b))

#define EI_NIDENT	16	/* size of e_ident in elf_ehdr */
#define EI_MAG		0	/* magic */
#define EI_CLASS	4	/* class (32 / 64 bit ) */
#define EI_DATA		5	/* endianness */
#define EI_VERSION	6	/* ELF version */

/* right values */
#define E_MAG		("\x7f" "ELF")
#define ELFCLASS32	1
#define ELFDATA2LSB	1
#define EV_CURRENT	1
#define EM_ARM		40
#define ET_EXEC		2
#define ET_DYN		3
#define PT_LOAD		1
#define PF_X		(1 << 0)
#define PF_W		(1 << 1)
#define PF_R		(1 << 2)
#define SHN_UNDEF	0
#define SHT_PROGBITS	1
#define SHT_SYMTAB	2

/* types used in ELF structures */
typedef uint32_t	elf_addr;	/* memory address */
typedef uint32_t	elf_off;	/* file offset */
typedef uint16_t	elf_section;	/* section index */
typedef uint16_t	elf_versym;	/* version symbol information */
typedef	int8_t		elf_byte;
typedef uint16_t	elf_half;
typedef int32_t		elf_sword;
typedef uint32_t	elf_word;
typedef int64_t		elf_sxword;
typedef uint64_t	elf_xword;

/* start of every ELF file */
struct elf_ehdr {
	elf_byte e_ident[EI_NIDENT];
	elf_half e_type;	/* type of ELF (executable, ...) */
	elf_half e_machine;	/* machine type */
	elf_word e_version;	/* ELF version */
	elf_addr e_entry;	/* entry point */
	elf_off  e_phoff;	/* offset of program header table */
	elf_off  e_shoff;	/* offset of section header table */
	elf_word e_flags;	/* machine flags (unused on ARM) */
	elf_half e_ehsize;	/* ELF header size */
	elf_half e_phentsize;	/* program header entry size */
	elf_half e_phnum;	/* number of program headers */
	elf_half e_shentsize;	/* section header entry size */
	elf_half e_shnum;	/* number of section headers */
	elf_half e_shstrndx;	/* section string table index */
};

/* program header table entry */
struct elf_phdr {
	elf_word p_type;	/* type (loadable, ...) */
	elf_off  p_offset;	/* offset of data */
	elf_addr p_vaddr;	/* address when running (for MMU systems) */
	elf_addr p_paddr;	/* address for loading (for MMU systems) */
	elf_word p_filesz;	/* size in ELF file */
	elf_word p_memsz;	/* size in memory */
	elf_word p_flags;	/* flags (exec, ...) (for MMU systems) */
	elf_word p_align;	/* alignment */
};

/* Execute.  Add ELF program entry,
 * - r0 contains a pointer to the argument command line, or 0
 * - lr contains a pointer to the reset vector, so that the device is
 *   resetted in case the _start() function of the ELF tries to return
 * - sp contains a useful stack pointer
 * - pc contains, of course, the entry point of the ELF */
int exec(char *elfarg, unsigned long entry, unsigned long sp)
{
    volatile long ret;
#if 0
    asm volatile ("push {r1-r12, lr}\n\t"\
		  "mov r0, %1	\n\t"	\
		  "mov r1, %2	\n\t"	\
		  "mov r2, %3	\n\t"	\
		  "mov r3, sp	\n\t"	\
		  "mov sp, r2	\n\t"	\
		  "push {r3}	\n\t"	\
		  "mov lr, pc	\n\t"	\
		  "bx  r1	\n\t"	\
		  "pop {r3}	\n\t"	\
		  "mov sp, r3	\n\t"	\
		  "pop {r1-r12, lr}\n\t"\
		  "mov %0, r0	\n\t"
		    : "=r" (ret) 
		    : "r" (elfarg), "r" (entry), "r" (sp)
		    : "r0"
		 );
#else
    asm volatile ("push {r1-r12, lr}\n\t"\
		  "mov r0, %1	\n\t"	\
		  "mov r1, %2	\n\t"	\
		  "mov r2, %3	\n\t"	\
		  "mov lr, pc	\n\t"	\
		  "bx  r1	\n\t"	\
		  "pop {r1-r12, lr}\n\t"\
		  "mov %0, r0	\n\t"
		    : "=r" (ret) 
		    : "r" (elfarg), "r" (entry), "r" (sp)
		    : "r0"
		 );
#endif
    return ret;
}

int exec_elf(char *arg)
{
    struct elf_ehdr ehdr;
    struct elf_phdr *phdr;
    char *elffile, *elfarg;
    int inst;
    int r, i;

    if (!arg) {
	fprintf(stderr, "No argument given.\n");
	return -1;
    }

    elffile = arg;

    elfarg = strchr(arg, ' ');
    if (elfarg) {
	*elfarg = 0;
	elfarg++;
    }

    /* open ELF file */
    inst = open(elffile, O_RDONLY);
    if (!inst) {
	fprintf(stderr, "Error opening ELF file!\n");
	return -1;
    }

    /* read header */
    r = read(inst, &ehdr, sizeof(struct elf_ehdr));
    if (r != sizeof(struct elf_ehdr))
	goto fail;

    /* validate header information */
    if (r != sizeof(struct elf_ehdr) ||
	strncmp((char *)ehdr.e_ident, E_MAG, 4) ||
	ehdr.e_ident[EI_CLASS] != ELFCLASS32 ||
	ehdr.e_ident[EI_DATA] != ELFDATA2LSB ||
	ehdr.e_ident[EI_VERSION] != EV_CURRENT ||
	ehdr.e_type != ET_EXEC ||
	ehdr.e_machine != EM_ARM ||
	ehdr.e_version != EV_CURRENT ||
	ehdr.e_shstrndx >= ehdr.e_shnum ||
	ehdr.e_phentsize != sizeof(struct elf_phdr)) {
//	    fprintf(stderr, "Not a supported ELF file.\n");
	    goto fail;
    }

    fprintf(stderr, "Entry point %08x\n", ehdr.e_entry);

    /* read & load program headers. */
    phdr = alloca(ehdr.e_phnum * sizeof(struct elf_phdr));
    r = lseek(inst, ehdr.e_phoff, SEEK_SET);
    if (r != ehdr.e_phoff) {
	fprintf(stderr, "elf - error lseek()\n");
	goto fail;
    }
    r = read(inst, phdr, ehdr.e_phnum * sizeof(struct elf_phdr));
    if (r < 0)
	goto fail;
    for (i = 0; i < ehdr.e_phnum; i++) {
	/* we only need to look at loadable program headers */
	if (phdr[i].p_type != PT_LOAD)
	    continue;


	fprintf(stderr, "ELF load address %08x, size %08x (%u)\n", phdr[i].p_paddr, phdr[i].p_memsz, phdr[i].p_memsz);
//	return -1;
#if 0
	/* check whether sections fits in external ram */
	if (phdr[i].p_paddr < EXTRAM_START ||
	    phdr[i].p_paddr + phdr[i].p_memsz >=
	    EXTRAM_START + EXTRAM_SIZE) {
	    fprintf(stderr, "ELF does not fit in external RAM.\n");
	    goto fail;
	}
#endif

	/* now load */
	r = lseek(inst, phdr[i].p_offset, SEEK_SET);
	if (r != phdr[i].p_offset) {
	    fprintf(stderr, "elf - error lseek()\n");
	    goto fail;
	}

	r = read(inst, (void *) (phdr[i].p_paddr), min(phdr[i].p_filesz, phdr[i].p_memsz));
	if (r < 0)
	    goto fail;

	if (phdr[i].p_memsz > phdr[i].p_filesz)
	    /* zero out .bss (or whatever needs to be zeroed out
	     * here) */
	    memset((void *) (phdr[i].p_paddr + phdr[i].p_filesz), 0,
	    phdr[i].p_memsz - phdr[i].p_filesz);
    }

    /* close file handle */
    close(inst);

    /* Now we execute the ELF. */
    exec(elfarg, ehdr.e_entry, ehdr.e_entry + 1024*1024);

    return 0;

fail:
    close(inst);
    fprintf(stderr, "Error loading ELF file!\n");
    return -1;
}

#ifdef TEST

int main(int argc, char *argv[])
{
    cmd_load(argv[1]);
    return 0;
}

#endif
