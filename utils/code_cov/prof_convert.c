/*
        prof_convert.c - a tools that can convert data of code coverage to text 	output.

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
 * 12/16/2006   Michael.Kang  <blackfin.kang@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#define MAX_DESC_STR 32
typedef struct prof_header_s{
        /* the version of header file */
        int ver;
        /* The length of header */
        int header_length;
        int prof_start;
        int prof_end;
        /* The description info for profiling file */
        char desc[MAX_DESC_STR];
}prof_header_t;

/*
 * dump header of data file
 */
static void dump_header(prof_header_t *header, FILE * fd){
	fprintf(fd, "Version:0x%x\n", header->ver);
	fprintf(fd, "Length of header: 0x%x bytes.\n", header->header_length);
	fprintf(fd, "From 0x%x to 0x%x for code coverage.\n", header->prof_start, header->prof_end);
	fprintf(fd, "Desc: %s\n", header->desc);
}
int main(int argc, char** argv){
	if(argc == 1 || argc > 3){
		printf("Purpose: print the header info of data file or convert  raw data file to text file.\n");
		printf("Usage: %s data_file output_file.\n", argv[0]);
		printf("Or: %s data_file\n", argv[0]);
		exit(-1);
	}
	FILE * in_fp = fopen(argv[1], "r");
	if(!in_fp){
	}
	int header_size = sizeof(prof_header_t);
	prof_header_t *header = malloc(header_size);
	size_t count = fread(header, 1, header_size, in_fp);
	if(count < header_size){
		printf("count = 0x%x, header_size=0x%x\n", count, header_size);
		fprintf(stderr, "The format of data file is not correctly.\n");
		exit(-1);
	}
	if(argc == 2){
		dump_header(header, stdout);	
		goto dump_exit;
	}

	FILE * out_fp = fopen(argv[2], "w");
	if(!out_fp){
		fprintf(stderr, "Can not open file %s to write.\n", argv[2]);
		exit(-1);
	}
	dump_header(header, out_fp);
	fprintf(out_fp, "address : flag\n");
	unsigned char read_flag;
	unsigned long addr = header->prof_start;
	/* 
	 * Every time , read only one byte from data file. One byte can 
	 * record the exeutable flag for two 32 bit words.
	 */
	while(fread(&read_flag, 1, 1, in_fp)){
		if(read_flag & 0xf)/* if executed */
			fprintf(out_fp, "0x%x : x\n", addr);
		else
			fprintf(out_fp, "0x%x : \n", addr);

		if(read_flag & 0xf0) /* if executed */
		       fprintf(out_fp, "0x%x : x\n", addr + 4);
                else
                       fprintf(out_fp, "0x%x : \n", addr + 4);
		addr += 8;
	}
	fclose(out_fp);
dump_exit:
	free(header);
	fclose(in_fp);
	return;
}
