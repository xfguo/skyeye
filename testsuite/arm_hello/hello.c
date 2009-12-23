/*
 * hello.c
 * just a function used to output "helloworld" to uart 
 * 
 * author: SU Hang
 * date:   2004-08-28	
 */
#define BOGO_MIPS 1000000
void hello(void)
{	
	int i;
	char * hellostr="helloworld";
	long* paddr=(long*)0xfffd001c;
	int timeout ;
	i = *hellostr - 'h';
	while(1){
		timeout = 0;
		while(++timeout != BOGO_MIPS);
		for(i=0;i<10;i++)
		{
			* paddr=hellostr[i];
		}
	}
	return;	
}

