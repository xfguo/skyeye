#ifndef __X86_REGFORMAT_H__
#define __X86_REGFORMAT_H__
#ifdef __cplusplus
 extern "C" {
#endif
enum x86_regno {
	Eax = 0,
	Ebx,
	Ecx,
	Edx,
	Esp,
	Ebp,
	Esi,
	Edi,
	Eip,
	Cs,
	Ds,
	Es,
	Ss,
	Fs,
	Gs,
};
char* x86_regstr[] = {
	"EAX",
	"EBX",
	"ECX",
	"EDX",
	"ESP",
	"EBP",
	"ESI",
	"EDI",
	"EIP",
	"CS",
	"DS",
	"ES",
	"SS",
	"FS",
	"GS",
	NULL
};

#ifdef __cplusplus
}
#endif

#endif
