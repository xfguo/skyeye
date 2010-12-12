/*
        loader_elf.c - load elf file to the memory 
        Copyright (C) 2003-2007 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 06/22/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */
#include "portable/mman.h"
#include <stdint.h>
#include <stdio.h>
#include <skyeye_config.h>
#include "bank_defs.h"
#include "skyeye_pref.h"
#include "skyeye_arch.h"
#include "elf.h"
/** 
 * add by michael.Kang, to load elf file to another address 
 */
//unsigned long load_base = 0x0;
//unsigned long load_mask = 0xffffffff;

static inline void
write_phys (uint32 addr, uint8_t * buffer, int size)
{
	int i,fault;
	sky_pref_t* pref = get_skyeye_pref();
	unsigned long load_base = pref->exec_load_base;
	unsigned long load_mask = pref->exec_load_mask;
	//skyeye_log(Info_log, __FUNCTION__, "load_base=0x%x,load_mask=0x%x", load_base, load_mask);
	addr = (addr & load_mask)|load_base;
	for (i = 0; i < size; i++) {
		/*
		if(arch_instance->ICE_write_byte)
			fault = arch_instance->ICE_write_byte (addr + i, buffer[i]);
		else
			fault = -1;
		*/
		/* byte write */
		fault = mem_write (8, addr + i, buffer[i]);

		if(fault) {
			printf("SKYEYE: write physical address 0x%x error!!!\n", addr + i);
			//return -1;
		}
	}
}

static inline void
write_virt (uint32 addr, uint8_t * buffer, int size)
{
	int i,fault;
	//skyeye_log(Info_log, __FUNCTION__, "load_base=0x%x,load_mask=0x%x", load_base, load_mask);
	skyeye_config_t* config = get_current_config();
	generic_arch_t *arch_instance = get_arch_instance(config->arch->arch_name);

	sky_pref_t* pref = get_skyeye_pref();
	unsigned long load_base = pref->exec_load_base;
	unsigned long load_mask = pref->exec_load_mask;
	addr = (addr & load_mask)|load_base;
	for (i = 0; i < size; i++) {
		/*
		if(arch_instance->ICE_write_byte)
			fault = arch_instance->ICE_write_byte (addr + i, buffer[i]);
		else
			fault = -1;
		*/
		/* byte write */
		fault = arch_instance->mmu_write(8, addr + i, buffer[i]);

		if(fault) {
			fprintf(stderr, "SKYEYE: write virtual address 0x%x error!!!\n", addr + i);
			//return -1;
		}
	}
}

#if 0
#include <elf32.h>

#ifdef __MINGW32__
#include <io.h>
#undef O_RDONLY
#define O_RDONLY			(_O_RDONLY | _O_BINARY)
#define open(file, flags)		_open(file, flags)
#define close(fd)			_close(fd)
#endif /* __MINGW32__ */


static inline void
tea_set(uint32 addr, uint8_t value, int size)
{
   int i,fault;

   addr = (addr & load_mask)|load_base;
   for (i = 0; i < size; i++) {
	   	fault=arch_instance->ICE_write_byte (addr + i, value);
		if(fault) {printf("SKYEYE: tea_set error!!!\n");skyeye_exit(-1);}
   }
}

/* These function convert little-endian ELF datatypes
   into host endianess values. */

#ifdef HOST_IS_BIG_ENDIAN
uint16_t
e2h16(uint16_t x)
{
	if (big_endian) return x;
	return ((x & 0xff) << 8) | (x >> 8);
}

uint32
e2h32(uint32 x)
{
	if (big_endian) return x;
	return ((x & 0xff) << 24) | 
		(((x >> 8) & 0xff) << 16)  |
		(((x >> 16) & 0xff) << 8)  |
		(((x >> 24) & 0xff));
}
#else
uint16_t
e2h16(uint16_t x) {
	if (!big_endian) return x;
	return ((x & 0xff) << 8) | (x >> 8);
}

uint32
e2h32(uint32 x) {
	if (!big_endian) return x;
	return ((x & 0xff) << 24) | 
		(((x >> 8) & 0xff) << 16)  |
		(((x >> 16) & 0xff) << 8)  |
		(((x >> 24) & 0xff));
}
#endif /* HOST_IS_BIG_ENDIAN */

static int
elf32_checkFile(struct Elf32_Header *file)
{
   if (file->e_ident[EI_MAG0] != ELFMAG0
       || file->e_ident[EI_MAG1] != ELFMAG1
       || file->e_ident[EI_MAG2] != ELFMAG2
       || file->e_ident[EI_MAG3] != ELFMAG3)
      return -1;  /* not an elf file */
   if (file->e_ident[EI_CLASS] != ELFCLASS32)
      return -2;  /* not 32-bit file */
   switch (e2h16(file->e_machine)) {
      case EM_ARM:
      case EM_BLACKFIN:
      case EM_COLDFIRE:
      case EM_MIPS:
      case EM_PPC:
        break;
      default:
        return -3;
   }
   return 0;      /* elf file looks OK */
}

static int
tea_load_exec(const char *file, int only_check_big_endian)
{
   int ret = -1;
   int i;
   int tmp_fd;
   int r;
   struct Elf32_Header *elfFile;
   struct stat stat;
   struct Elf32_Phdr *segments;

   tmp_fd = open(file, O_RDONLY);
   if (tmp_fd == -1) {
      fprintf (stderr, "open %s error: %s\n", file, strerror(errno));
      goto out;
   }

   fstat(tmp_fd, &stat);

   /* malloc */
   elfFile = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, tmp_fd, 0);
   if (elfFile == NULL || elfFile == MAP_FAILED) {
      fprintf (stderr, "mmap error: %s\n", strerror(errno));
      goto out;
   }

   big_endian = (elfFile->e_ident[EI_DATA] == ELFDATA2MSB);
   if (only_check_big_endian) goto out;

   r = elf32_checkFile(elfFile);
   if (r != 0) {
       fprintf (stderr, "elf_checkFile failed: %d\n", r);
       goto out;
   }

   segments = (struct Elf32_Phdr*) (uintptr_t) (((uintptr_t) elfFile) + e2h32(elfFile->e_phoff));

   for(i=0; i < e2h16(elfFile->e_phnum); i++) {
      /* Load that section */
      uint32 dest;
      char *src;
      size_t filesz = e2h32(segments[i].p_filesz);
      size_t memsz = e2h32(segments[i].p_memsz);
      dest = e2h32(segments[i].p_paddr);
      src = ((char*) elfFile) + e2h32(segments[i].p_offset);
      tea_write(dest, src, filesz);
      dest += filesz;
      tea_set(dest, 0, memsz - filesz);
   }
   skyeye_config_t* config = get_current_config();
   if (config->start_address == 0) {
       config->start_address = e2h32(elfFile->e_entry);
   }

   ret = 0;
out:
   if (tmp_fd != -1)
      close(tmp_fd);
   if (elfFile)
      munmap(elfFile, stat.st_size);
   return(ret);
}
#else //#ifndef HAVE_LIBBFD

//teawater add for load elf 2005.07.31------------------------------------------
#define ELF_LOADING_INFO 0
#include <bfd.h>

static exception_t
load_exec (const char *file, addr_type_t addr_type)
{
	int ret = -1;
	bfd *tmp_bfd = NULL;
	asection *s;
	char *tmp_str = NULL;

	/* open */
	tmp_bfd = bfd_openr (file, NULL);
	if (tmp_bfd == NULL) {
		fprintf (stderr, "open %s error: %s\n", file,
			 bfd_errmsg (bfd_get_error ()));
		goto out;
	}
	if (!bfd_check_format (tmp_bfd, bfd_object)) {
		/* FIXME:In freebsd, if bfd_errno is bfd_error_file_ambiguously_recognized,
		 * though bfd can't recognize this format, we should try to load file.*/
		if (bfd_get_error () != bfd_error_file_ambiguously_recognized) {
			fprintf (stderr, "check format of %s error: %s\n",
				 file, bfd_errmsg (bfd_get_error ()));
			goto out;
		}
	}

	//big_endian = bfd_big_endian(tmp_bfd);

	printf ("exec file \"%s\"'s format is %s.\n", file,
		tmp_bfd->xvec->name);

	/* load the corresponding section to memory */
	for (s = tmp_bfd->sections; s; s = s->next) {
		if (bfd_get_section_flags (tmp_bfd, s) & (SEC_LOAD)) {
            		if (bfd_section_lma (tmp_bfd, s) != bfd_section_vma (tmp_bfd, s)) {
#if ELF_LOADING_INFO	
                printf ("load section %s: lma = 0x%08x (vma = 0x%08x)  size = 0x%08x.\n", bfd_section_name (tmp_bfd, s), (unsigned int) bfd_section_lma (tmp_bfd, s), (unsigned int) bfd_section_vma (tmp_bfd, s), (unsigned int) bfd_section_size (tmp_bfd, s));
#endif
            } else {
#if ELF_LOADING_INFO	
                printf ("load section %s: addr = 0x%08x  size = 0x%08x.\n", bfd_section_name (tmp_bfd, s), (unsigned int) bfd_section_lma (tmp_bfd, s), (unsigned int) bfd_section_size (tmp_bfd, s));
#endif
            }
	if (bfd_section_size (tmp_bfd, s) > 0) {
		tmp_str = (char *)malloc (bfd_section_size
						(tmp_bfd, s));
		if (!tmp_str) {
			fprintf (stderr, "alloc memory to load session %s error.\n",
			 bfd_section_name (tmp_bfd, s));
			goto out;
		}
		if (!bfd_get_section_contents(tmp_bfd, s, tmp_str, 0,
			bfd_section_size (tmp_bfd, s))) {
				fprintf (stderr,
					 "get session %s content error: %s\n",
					 bfd_section_name (tmp_bfd, s),
					 bfd_errmsg (bfd_get_error()));
					goto out;
				}
				if(addr_type == Phys_addr){
					write_phys(bfd_section_vma (tmp_bfd, s),
                                           tmp_str, bfd_section_size (tmp_bfd,
                                                                      s));

				}
				else if(addr_type == Virt_addr){
					write_virt (bfd_section_vma (tmp_bfd, s),
					   tmp_str, bfd_section_size (tmp_bfd,
								      s));
				}
				else{
					fprintf(stderr, "wrong address type %d\n", addr_type);
					goto out;
				}
				free (tmp_str);
				tmp_str = NULL;
			}
		}
		else {
#if ELF_LOADING_INFO	
			printf ("not load section %s: addr = 0x%08x  size = 0x%08x .\n", bfd_section_name (tmp_bfd, s), (unsigned int) bfd_section_vma (tmp_bfd, s), (unsigned int) bfd_section_size (tmp_bfd, s));
#endif
		}
	}

	//set start address
	skyeye_config_t* config = get_current_config();
	config->start_address = bfd_get_start_address (tmp_bfd);

	ret = 0;
      out:
	if (tmp_str)
		free (tmp_str);
	if (tmp_bfd)
		bfd_close (tmp_bfd);
	return (ret);
}

#endif //#ifndef HAVE_LIBBFD
endian_t get_elf_endian(const char* elf_filename){
	endian_t endian;
	Elf32_Ehdr elf_header;
	FILE* file = fopen(elf_filename, "r");
	if(file == NULL){
		fprintf(stderr, "In %s, can not open file %s\n", __FUNCTION__, elf_filename);
		exit(-1);
	}

	if (fread (elf_header.e_ident, EI_NIDENT, 1, file) != 1)
		return 0;

	/* Determine how to read the rest of the header.  */
	switch (elf_header.e_ident[EI_DATA])
	{
		default: /* fall through */
		case ELFDATANONE: /* fall through */
		case ELFDATA2LSB:
			endian = Little_endian;
			break;
		case ELFDATA2MSB:
			endian = Big_endian;
			break;
	}
	fclose(file);
	return endian;
}
exception_t load_elf(const char* elf_filename, addr_type_t addr_type){
	return load_exec(elf_filename, addr_type);
}
