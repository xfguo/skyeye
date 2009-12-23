void dbct_allote_mem(){
#ifdef DBCT
		if (!skyeye_config.no_dbct) {
			//teawater add for arm2x86 2004.12.04-------------------------------------------
			if (mb[bank].len % TB_LEN) {
				fprintf (stderr,
					 "SKYEYE: mem_reset: Bank number %d length error.\n", bank);
				skyeye_exit (-1);
			}

			global_memory.tbp[bank] = MAP_FAILED;
			global_memory.tbt[bank] = NULL;

			/*
			   global_memory.tbp[bank] = mmap(NULL, mb[bank].len / sizeof(ARMword) * TB_INSN_LEN_MAX + mb[bank].len / TB_LEN * op_return.len, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
			   if (global_memory.tbp[bank] == MAP_FAILED) {
			   fprintf(stderr, "SKYEYE: mem_reset: Error allocating mem for bank number %d.\n", bank);
			   exit(-1);
				   }
				   global_memory.tbt[bank] = malloc(mb[bank].len/TB_LEN*sizeof(tb_t));
				   if (!global_memory.tbt[bank]) {
				   fprintf(stderr, "SKYEYE: mem_reset: Error allocating mem for bank number %d.\n", bank);
				   exit(-1);
				   }
				   memset(global_memory.tbt[bank], 0, mb[bank].len/TB_LEN*sizeof(tb_t));
				 */
				//AJ2D--------------------------------------------------------------------------
			}
#endif
}
