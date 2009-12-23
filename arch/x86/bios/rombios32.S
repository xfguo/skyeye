
rombios32.out:     file format elf32-i386

Disassembly of section .text:

000e0000 <_start>:
   e0000:	31 c0                	xor    %eax,%eax
   e0002:	bf 08 07 00 00       	mov    $0x708,%edi
   e0007:	b9 64 07 00 00       	mov    $0x764,%ecx
   e000c:	29 f9                	sub    %edi,%ecx
   e000e:	f3 aa                	rep stos %al,%es:(%edi)
   e0010:	be 08 31 0e 00       	mov    $0xe3108,%esi
   e0015:	bf 00 07 00 00       	mov    $0x700,%edi
   e001a:	b9 04 07 00 00       	mov    $0x704,%ecx
   e001f:	29 f9                	sub    %edi,%ecx
   e0021:	f3 a4                	rep movsb %ds:(%esi),%es:(%edi)
   e0023:	e9 c8 1e 00 00       	jmp    e1ef0 <rombios32_init>

000e0028 <smp_ap_boot_code_start>:
   e0028:	fa                   	cli    
   e0029:	31 c0                	xor    %eax,%eax
   e002b:	8e d8                	mov    %eax,%ds
   e002d:	66 bb 10 05          	mov    $0x510,%bx
   e0031:	00 00                	add    %al,(%eax)
   e0033:	67 66 8b 0b          	addr16 mov (%bp,%di),%cx
   e0037:	66 85 c9             	test   %cx,%cx
   e003a:	74 12                	je     e004e <smp_ap_boot_code_start+0x26>
   e003c:	67 66 8b 43 04       	addr16 mov 0x4(%bp,%di),%ax
   e0041:	67 66 8b 53 08       	addr16 mov 0x8(%bp,%di),%dx
   e0046:	0f 30                	wrmsr  
   e0048:	66 83 c3 0c          	add    $0xc,%bx
   e004c:	eb e5                	jmp    e0033 <smp_ap_boot_code_start+0xb>
   e004e:	f0 ff 06             	lock incl (%esi)
   e0051:	3c 07                	cmp    $0x7,%al
   e0053:	f4                   	hlt    
   e0054:	eb fd                	jmp    e0053 <smp_ap_boot_code_start+0x2b>

000e0056 <smm_relocation_start>:
   e0056:	66 bb fc fe          	mov    $0xfefc,%bx
   e005a:	03 00                	add    (%eax),%eax
   e005c:	67 8a 03             	addr16 mov (%bp,%di),%al
   e005f:	3c 64                	cmp    $0x64,%al
   e0061:	74 08                	je     e006b <smm_relocation_start+0x15>
   e0063:	66 bb f8 fe          	mov    $0xfef8,%bx
   e0067:	03 00                	add    (%eax),%eax
   e0069:	eb 06                	jmp    e0071 <smm_relocation_start+0x1b>
   e006b:	66 bb 00 ff          	mov    $0xff00,%bx
   e006f:	03 00                	add    (%eax),%eax
   e0071:	66 b8 00 00          	mov    $0x0,%ax
   e0075:	0a 00                	or     (%eax),%al
   e0077:	67 66 89 03          	addr16 mov %ax,(%bp,%di)
   e007b:	b0 00                	mov    $0x0,%al
   e007d:	ba b3 00 ee 0f       	mov    $0xfee00b3,%edx
   e0082:	aa                   	stos   %al,%es:(%edi)

000e0083 <smm_code_start>:
   e0083:	ba b2 00 ec 3c       	mov    $0x3cec00b2,%edx
   e0088:	f0 75 0a             	lock jne e0095 <smm_code_start+0x12>
   e008b:	ba 04 b0 ed 83       	mov    $0x83edb004,%edx
   e0090:	e0 fe                	loopne e0090 <smm_code_start+0xd>
   e0092:	ef                   	out    %eax,(%dx)
   e0093:	eb 0c                	jmp    e00a1 <smm_code_start+0x1e>
   e0095:	3c f1                	cmp    $0xf1,%al
   e0097:	75 08                	jne    e00a1 <smm_code_start+0x1e>
   e0099:	ba 04 b0 ed 83       	mov    $0x83edb004,%edx
   e009e:	c8 01 ef 0f          	enter  $0xef01,$0xf
   e00a2:	aa                   	stos   %al,%es:(%edi)

000e00a3 <smm_code_end>:
	...

000e00b0 <memset>:
   e00b0:	55                   	push   %ebp
   e00b1:	89 e5                	mov    %esp,%ebp
   e00b3:	8b 45 10             	mov    0x10(%ebp),%eax
   e00b6:	53                   	push   %ebx
   e00b7:	8b 5d 08             	mov    0x8(%ebp),%ebx
   e00ba:	85 c0                	test   %eax,%eax
   e00bc:	74 10                	je     e00ce <memset+0x1e>
   e00be:	0f b6 4d 0c          	movzbl 0xc(%ebp),%ecx
   e00c2:	31 d2                	xor    %edx,%edx
   e00c4:	88 0c 1a             	mov    %cl,(%edx,%ebx,1)
   e00c7:	83 c2 01             	add    $0x1,%edx
   e00ca:	39 c2                	cmp    %eax,%edx
   e00cc:	75 f6                	jne    e00c4 <memset+0x14>
   e00ce:	89 d8                	mov    %ebx,%eax
   e00d0:	5b                   	pop    %ebx
   e00d1:	5d                   	pop    %ebp
   e00d2:	c3                   	ret    
   e00d3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e00d9:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e00e0 <memcpy>:
   e00e0:	55                   	push   %ebp
   e00e1:	89 e5                	mov    %esp,%ebp
   e00e3:	8b 4d 10             	mov    0x10(%ebp),%ecx
   e00e6:	56                   	push   %esi
   e00e7:	8b 75 08             	mov    0x8(%ebp),%esi
   e00ea:	53                   	push   %ebx
   e00eb:	8b 5d 0c             	mov    0xc(%ebp),%ebx
   e00ee:	85 c9                	test   %ecx,%ecx
   e00f0:	74 10                	je     e0102 <memcpy+0x22>
   e00f2:	31 d2                	xor    %edx,%edx
   e00f4:	0f b6 04 1a          	movzbl (%edx,%ebx,1),%eax
   e00f8:	88 04 32             	mov    %al,(%edx,%esi,1)
   e00fb:	83 c2 01             	add    $0x1,%edx
   e00fe:	39 ca                	cmp    %ecx,%edx
   e0100:	75 f2                	jne    e00f4 <memcpy+0x14>
   e0102:	89 f0                	mov    %esi,%eax
   e0104:	5b                   	pop    %ebx
   e0105:	5e                   	pop    %esi
   e0106:	5d                   	pop    %ebp
   e0107:	c3                   	ret    
   e0108:	90                   	nop    
   e0109:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi

000e0110 <memmove>:
   e0110:	55                   	push   %ebp
   e0111:	89 e5                	mov    %esp,%ebp
   e0113:	56                   	push   %esi
   e0114:	53                   	push   %ebx
   e0115:	8b 75 08             	mov    0x8(%ebp),%esi
   e0118:	8b 4d 0c             	mov    0xc(%ebp),%ecx
   e011b:	8b 5d 10             	mov    0x10(%ebp),%ebx
   e011e:	39 ce                	cmp    %ecx,%esi
   e0120:	77 1a                	ja     e013c <memmove+0x2c>
   e0122:	85 db                	test   %ebx,%ebx
   e0124:	74 10                	je     e0136 <memmove+0x26>
   e0126:	31 d2                	xor    %edx,%edx
   e0128:	0f b6 04 0a          	movzbl (%edx,%ecx,1),%eax
   e012c:	88 04 32             	mov    %al,(%edx,%esi,1)
   e012f:	83 c2 01             	add    $0x1,%edx
   e0132:	39 da                	cmp    %ebx,%edx
   e0134:	75 f2                	jne    e0128 <memmove+0x18>
   e0136:	89 f0                	mov    %esi,%eax
   e0138:	5b                   	pop    %ebx
   e0139:	5e                   	pop    %esi
   e013a:	5d                   	pop    %ebp
   e013b:	c3                   	ret    
   e013c:	85 db                	test   %ebx,%ebx
   e013e:	74 f6                	je     e0136 <memmove+0x26>
   e0140:	8d 14 1e             	lea    (%esi,%ebx,1),%edx
   e0143:	01 d9                	add    %ebx,%ecx
   e0145:	89 d0                	mov    %edx,%eax
   e0147:	29 d8                	sub    %ebx,%eax
   e0149:	89 c3                	mov    %eax,%ebx
   e014b:	90                   	nop    
   e014c:	8d 74 26 00          	lea    0x0(%esi),%esi
   e0150:	0f b6 41 ff          	movzbl -0x1(%ecx),%eax
   e0154:	83 e9 01             	sub    $0x1,%ecx
   e0157:	88 42 ff             	mov    %al,-0x1(%edx)
   e015a:	83 ea 01             	sub    $0x1,%edx
   e015d:	39 da                	cmp    %ebx,%edx
   e015f:	75 ef                	jne    e0150 <memmove+0x40>
   e0161:	89 f0                	mov    %esi,%eax
   e0163:	5b                   	pop    %ebx
   e0164:	5e                   	pop    %esi
   e0165:	5d                   	pop    %ebp
   e0166:	c3                   	ret    
   e0167:	89 f6                	mov    %esi,%esi
   e0169:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e0170 <memcmp>:
   e0170:	55                   	push   %ebp
   e0171:	89 e5                	mov    %esp,%ebp
   e0173:	56                   	push   %esi
   e0174:	53                   	push   %ebx
   e0175:	8b 5d 10             	mov    0x10(%ebp),%ebx
   e0178:	8b 75 08             	mov    0x8(%ebp),%esi
   e017b:	8b 4d 0c             	mov    0xc(%ebp),%ecx
   e017e:	eb 06                	jmp    e0186 <memcmp+0x16>
   e0180:	83 c6 01             	add    $0x1,%esi
   e0183:	83 c1 01             	add    $0x1,%ecx
   e0186:	83 eb 01             	sub    $0x1,%ebx
   e0189:	83 fb ff             	cmp    $0xffffffff,%ebx
   e018c:	74 10                	je     e019e <memcmp+0x2e>
   e018e:	0f be 01             	movsbl (%ecx),%eax
   e0191:	0f be 16             	movsbl (%esi),%edx
   e0194:	29 c2                	sub    %eax,%edx
   e0196:	89 d0                	mov    %edx,%eax
   e0198:	74 e6                	je     e0180 <memcmp+0x10>
   e019a:	5b                   	pop    %ebx
   e019b:	5e                   	pop    %esi
   e019c:	5d                   	pop    %ebp
   e019d:	c3                   	ret    
   e019e:	5b                   	pop    %ebx
   e019f:	31 c0                	xor    %eax,%eax
   e01a1:	5e                   	pop    %esi
   e01a2:	5d                   	pop    %ebp
   e01a3:	c3                   	ret    
   e01a4:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e01aa:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

000e01b0 <strlen>:
   e01b0:	55                   	push   %ebp
   e01b1:	31 c0                	xor    %eax,%eax
   e01b3:	89 e5                	mov    %esp,%ebp
   e01b5:	8b 55 08             	mov    0x8(%ebp),%edx
   e01b8:	80 3a 00             	cmpb   $0x0,(%edx)
   e01bb:	74 0d                	je     e01ca <strlen+0x1a>
   e01bd:	89 d0                	mov    %edx,%eax
   e01bf:	90                   	nop    
   e01c0:	83 c0 01             	add    $0x1,%eax
   e01c3:	80 38 00             	cmpb   $0x0,(%eax)
   e01c6:	75 f8                	jne    e01c0 <strlen+0x10>
   e01c8:	29 d0                	sub    %edx,%eax
   e01ca:	5d                   	pop    %ebp
   e01cb:	c3                   	ret    
   e01cc:	8d 74 26 00          	lea    0x0(%esi),%esi

000e01d0 <vsnprintf>:
   e01d0:	55                   	push   %ebp
   e01d1:	89 e5                	mov    %esp,%ebp
   e01d3:	57                   	push   %edi
   e01d4:	56                   	push   %esi
   e01d5:	53                   	push   %ebx
   e01d6:	83 ec 4c             	sub    $0x4c,%esp
   e01d9:	8b 45 0c             	mov    0xc(%ebp),%eax
   e01dc:	83 e8 01             	sub    $0x1,%eax
   e01df:	85 c0                	test   %eax,%eax
   e01e1:	89 45 d0             	mov    %eax,-0x30(%ebp)
   e01e4:	0f 8e 7d 03 00 00    	jle    e0567 <vsnprintf+0x397>
   e01ea:	8b 4d 08             	mov    0x8(%ebp),%ecx
   e01ed:	8d 45 f3             	lea    -0xd(%ebp),%eax
   e01f0:	8b 5d 10             	mov    0x10(%ebp),%ebx
   e01f3:	89 45 b4             	mov    %eax,-0x4c(%ebp)
   e01f6:	89 4d c8             	mov    %ecx,-0x38(%ebp)
   e01f9:	8b 55 10             	mov    0x10(%ebp),%edx
   e01fc:	0f b6 02             	movzbl (%edx),%eax
   e01ff:	3c 25                	cmp    $0x25,%al
   e0201:	74 08                	je     e020b <vsnprintf+0x3b>
   e0203:	84 c0                	test   %al,%al
   e0205:	0f 85 15 01 00 00    	jne    e0320 <vsnprintf+0x150>
   e020b:	39 5d 10             	cmp    %ebx,0x10(%ebp)
   e020e:	0f 82 23 01 00 00    	jb     e0337 <vsnprintf+0x167>
   e0214:	8b 45 10             	mov    0x10(%ebp),%eax
   e0217:	80 38 00             	cmpb   $0x0,(%eax)
   e021a:	0f 84 a5 00 00 00    	je     e02c5 <vsnprintf+0xf5>
   e0220:	8b 4d 10             	mov    0x10(%ebp),%ecx
   e0223:	8b 5d 10             	mov    0x10(%ebp),%ebx
   e0226:	0f b6 41 01          	movzbl 0x1(%ecx),%eax
   e022a:	83 c3 01             	add    $0x1,%ebx
   e022d:	c7 45 bc 20 00 00 00 	movl   $0x20,-0x44(%ebp)
   e0234:	3c 30                	cmp    $0x30,%al
   e0236:	0f be c8             	movsbl %al,%ecx
   e0239:	0f 84 ad 02 00 00    	je     e04ec <vsnprintf+0x31c>
   e023f:	83 f9 2a             	cmp    $0x2a,%ecx
   e0242:	0f 84 c1 02 00 00    	je     e0509 <vsnprintf+0x339>
   e0248:	8d 51 d0             	lea    -0x30(%ecx),%edx
   e024b:	83 fa 09             	cmp    $0x9,%edx
   e024e:	c7 45 b8 00 00 00 00 	movl   $0x0,-0x48(%ebp)
   e0255:	77 23                	ja     e027a <vsnprintf+0xaa>
   e0257:	c7 45 b8 00 00 00 00 	movl   $0x0,-0x48(%ebp)
   e025e:	66 90                	xchg   %ax,%ax
   e0260:	8b 4d b8             	mov    -0x48(%ebp),%ecx
   e0263:	83 c3 01             	add    $0x1,%ebx
   e0266:	8d 04 89             	lea    (%ecx,%ecx,4),%eax
   e0269:	8d 04 42             	lea    (%edx,%eax,2),%eax
   e026c:	89 45 b8             	mov    %eax,-0x48(%ebp)
   e026f:	0f be 0b             	movsbl (%ebx),%ecx
   e0272:	8d 51 d0             	lea    -0x30(%ecx),%edx
   e0275:	83 fa 09             	cmp    $0x9,%edx
   e0278:	76 e6                	jbe    e0260 <vsnprintf+0x90>
   e027a:	83 f9 2e             	cmp    $0x2e,%ecx
   e027d:	74 60                	je     e02df <vsnprintf+0x10f>
   e027f:	31 f6                	xor    %esi,%esi
   e0281:	83 f9 6c             	cmp    $0x6c,%ecx
   e0284:	75 06                	jne    e028c <vsnprintf+0xbc>
   e0286:	83 c3 01             	add    $0x1,%ebx
   e0289:	0f be 0b             	movsbl (%ebx),%ecx
   e028c:	8d 41 a8             	lea    -0x58(%ecx),%eax
   e028f:	83 c3 01             	add    $0x1,%ebx
   e0292:	83 f8 20             	cmp    $0x20,%eax
   e0295:	89 5d 10             	mov    %ebx,0x10(%ebp)
   e0298:	0f 86 ca 00 00 00    	jbe    e0368 <vsnprintf+0x198>
   e029e:	8b 45 c8             	mov    -0x38(%ebp),%eax
   e02a1:	c6 00 25             	movb   $0x25,(%eax)
   e02a4:	31 c0                	xor    %eax,%eax
   e02a6:	83 f9 25             	cmp    $0x25,%ecx
   e02a9:	0f 95 c0             	setne  %al
   e02ac:	83 45 c8 01          	addl   $0x1,-0x38(%ebp)
   e02b0:	29 45 10             	sub    %eax,0x10(%ebp)
   e02b3:	83 6d d0 01          	subl   $0x1,-0x30(%ebp)
   e02b7:	8b 45 d0             	mov    -0x30(%ebp),%eax
   e02ba:	8b 5d 10             	mov    0x10(%ebp),%ebx
   e02bd:	85 c0                	test   %eax,%eax
   e02bf:	0f 8f 34 ff ff ff    	jg     e01f9 <vsnprintf+0x29>
   e02c5:	8b 55 c8             	mov    -0x38(%ebp),%edx
   e02c8:	8b 45 c8             	mov    -0x38(%ebp),%eax
   e02cb:	2b 45 08             	sub    0x8(%ebp),%eax
   e02ce:	89 55 08             	mov    %edx,0x8(%ebp)
   e02d1:	8b 4d 08             	mov    0x8(%ebp),%ecx
   e02d4:	c6 01 00             	movb   $0x0,(%ecx)
   e02d7:	83 c4 4c             	add    $0x4c,%esp
   e02da:	5b                   	pop    %ebx
   e02db:	5e                   	pop    %esi
   e02dc:	5f                   	pop    %edi
   e02dd:	5d                   	pop    %ebp
   e02de:	c3                   	ret    
   e02df:	83 c3 01             	add    $0x1,%ebx
   e02e2:	0f b6 03             	movzbl (%ebx),%eax
   e02e5:	3c 2a                	cmp    $0x2a,%al
   e02e7:	0f 84 4c 02 00 00    	je     e0539 <vsnprintf+0x369>
   e02ed:	0f be c8             	movsbl %al,%ecx
   e02f0:	31 f6                	xor    %esi,%esi
   e02f2:	8d 51 d0             	lea    -0x30(%ecx),%edx
   e02f5:	83 fa 09             	cmp    $0x9,%edx
   e02f8:	77 85                	ja     e027f <vsnprintf+0xaf>
   e02fa:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e0300:	83 c3 01             	add    $0x1,%ebx
   e0303:	0f be 0b             	movsbl (%ebx),%ecx
   e0306:	8d 04 b6             	lea    (%esi,%esi,4),%eax
   e0309:	8d 34 42             	lea    (%edx,%eax,2),%esi
   e030c:	8d 51 d0             	lea    -0x30(%ecx),%edx
   e030f:	83 fa 09             	cmp    $0x9,%edx
   e0312:	76 ec                	jbe    e0300 <vsnprintf+0x130>
   e0314:	e9 68 ff ff ff       	jmp    e0281 <vsnprintf+0xb1>
   e0319:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi
   e0320:	83 c3 01             	add    $0x1,%ebx
   e0323:	0f b6 03             	movzbl (%ebx),%eax
   e0326:	3c 25                	cmp    $0x25,%al
   e0328:	0f 85 d5 fe ff ff    	jne    e0203 <vsnprintf+0x33>
   e032e:	39 5d 10             	cmp    %ebx,0x10(%ebp)
   e0331:	0f 83 dd fe ff ff    	jae    e0214 <vsnprintf+0x44>
   e0337:	89 de                	mov    %ebx,%esi
   e0339:	2b 75 10             	sub    0x10(%ebp),%esi
   e033c:	3b 75 d0             	cmp    -0x30(%ebp),%esi
   e033f:	7e 03                	jle    e0344 <vsnprintf+0x174>
   e0341:	8b 75 d0             	mov    -0x30(%ebp),%esi
   e0344:	8b 55 10             	mov    0x10(%ebp),%edx
   e0347:	8b 4d c8             	mov    -0x38(%ebp),%ecx
   e034a:	89 74 24 08          	mov    %esi,0x8(%esp)
   e034e:	89 54 24 04          	mov    %edx,0x4(%esp)
   e0352:	89 0c 24             	mov    %ecx,(%esp)
   e0355:	e8 86 fd ff ff       	call   e00e0 <memcpy>
   e035a:	01 75 c8             	add    %esi,-0x38(%ebp)
   e035d:	29 75 d0             	sub    %esi,-0x30(%ebp)
   e0360:	89 5d 10             	mov    %ebx,0x10(%ebp)
   e0363:	e9 ac fe ff ff       	jmp    e0214 <vsnprintf+0x44>
   e0368:	ff 24 85 c0 25 0e 00 	jmp    *0xe25c0(,%eax,4)
   e036f:	8b 4d 14             	mov    0x14(%ebp),%ecx
   e0372:	8b 19                	mov    (%ecx),%ebx
   e0374:	83 c1 04             	add    $0x4,%ecx
   e0377:	89 4d 14             	mov    %ecx,0x14(%ebp)
   e037a:	89 1c 24             	mov    %ebx,(%esp)
   e037d:	e8 2e fe ff ff       	call   e01b0 <strlen>
   e0382:	85 f6                	test   %esi,%esi
   e0384:	89 c7                	mov    %eax,%edi
   e0386:	7e 06                	jle    e038e <vsnprintf+0x1be>
   e0388:	39 f0                	cmp    %esi,%eax
   e038a:	7e 02                	jle    e038e <vsnprintf+0x1be>
   e038c:	89 f7                	mov    %esi,%edi
   e038e:	8b 55 b8             	mov    -0x48(%ebp),%edx
   e0391:	85 d2                	test   %edx,%edx
   e0393:	7e 31                	jle    e03c6 <vsnprintf+0x1f6>
   e0395:	8b 55 b8             	mov    -0x48(%ebp),%edx
   e0398:	3b 55 d0             	cmp    -0x30(%ebp),%edx
   e039b:	7e 03                	jle    e03a0 <vsnprintf+0x1d0>
   e039d:	8b 55 d0             	mov    -0x30(%ebp),%edx
   e03a0:	89 d6                	mov    %edx,%esi
   e03a2:	29 fe                	sub    %edi,%esi
   e03a4:	85 f6                	test   %esi,%esi
   e03a6:	7e 1e                	jle    e03c6 <vsnprintf+0x1f6>
   e03a8:	0f b6 45 bc          	movzbl -0x44(%ebp),%eax
   e03ac:	89 f1                	mov    %esi,%ecx
   e03ae:	8b 55 c8             	mov    -0x38(%ebp),%edx
   e03b1:	88 02                	mov    %al,(%edx)
   e03b3:	83 c2 01             	add    $0x1,%edx
   e03b6:	83 e9 01             	sub    $0x1,%ecx
   e03b9:	75 f6                	jne    e03b1 <vsnprintf+0x1e1>
   e03bb:	8b 4d c8             	mov    -0x38(%ebp),%ecx
   e03be:	29 75 d0             	sub    %esi,-0x30(%ebp)
   e03c1:	01 f1                	add    %esi,%ecx
   e03c3:	89 4d c8             	mov    %ecx,-0x38(%ebp)
   e03c6:	8b 75 d0             	mov    -0x30(%ebp),%esi
   e03c9:	39 fe                	cmp    %edi,%esi
   e03cb:	7e 02                	jle    e03cf <vsnprintf+0x1ff>
   e03cd:	89 fe                	mov    %edi,%esi
   e03cf:	8b 45 c8             	mov    -0x38(%ebp),%eax
   e03d2:	89 74 24 08          	mov    %esi,0x8(%esp)
   e03d6:	89 5c 24 04          	mov    %ebx,0x4(%esp)
   e03da:	89 04 24             	mov    %eax,(%esp)
   e03dd:	e8 fe fc ff ff       	call   e00e0 <memcpy>
   e03e2:	01 75 c8             	add    %esi,-0x38(%ebp)
   e03e5:	29 75 d0             	sub    %esi,-0x30(%ebp)
   e03e8:	e9 ca fe ff ff       	jmp    e02b7 <vsnprintf+0xe7>
   e03ed:	8b 55 14             	mov    0x14(%ebp),%edx
   e03f0:	8d 7d d6             	lea    -0x2a(%ebp),%edi
   e03f3:	8b 4d 14             	mov    0x14(%ebp),%ecx
   e03f6:	c7 45 c0 10 00 00 00 	movl   $0x10,-0x40(%ebp)
   e03fd:	c7 45 c4 02 00 00 00 	movl   $0x2,-0x3c(%ebp)
   e0404:	8b 02                	mov    (%edx),%eax
   e0406:	83 c1 04             	add    $0x4,%ecx
   e0409:	89 4d cc             	mov    %ecx,-0x34(%ebp)
   e040c:	c6 45 f3 00          	movb   $0x0,-0xd(%ebp)
   e0410:	8d 5d f3             	lea    -0xd(%ebp),%ebx
   e0413:	39 fb                	cmp    %edi,%ebx
   e0415:	76 20                	jbe    e0437 <vsnprintf+0x267>
   e0417:	31 d2                	xor    %edx,%edx
   e0419:	83 ee 01             	sub    $0x1,%esi
   e041c:	f7 75 c0             	divl   -0x40(%ebp)
   e041f:	83 eb 01             	sub    $0x1,%ebx
   e0422:	85 f6                	test   %esi,%esi
   e0424:	89 c1                	mov    %eax,%ecx
   e0426:	0f b6 82 71 2e 0e 00 	movzbl 0xe2e71(%edx),%eax
   e042d:	88 03                	mov    %al,(%ebx)
   e042f:	89 c8                	mov    %ecx,%eax
   e0431:	7f e0                	jg     e0413 <vsnprintf+0x243>
   e0433:	85 c9                	test   %ecx,%ecx
   e0435:	75 dc                	jne    e0413 <vsnprintf+0x243>
   e0437:	83 7d c4 01          	cmpl   $0x1,-0x3c(%ebp)
   e043b:	0f 84 ed 00 00 00    	je     e052e <vsnprintf+0x35e>
   e0441:	83 7d c4 02          	cmpl   $0x2,-0x3c(%ebp)
   e0445:	75 0a                	jne    e0451 <vsnprintf+0x281>
   e0447:	c6 43 ff 78          	movb   $0x78,-0x1(%ebx)
   e044b:	83 eb 02             	sub    $0x2,%ebx
   e044e:	c6 03 30             	movb   $0x30,(%ebx)
   e0451:	8b 7d b4             	mov    -0x4c(%ebp),%edi
   e0454:	8b 55 cc             	mov    -0x34(%ebp),%edx
   e0457:	29 df                	sub    %ebx,%edi
   e0459:	89 55 14             	mov    %edx,0x14(%ebp)
   e045c:	e9 2d ff ff ff       	jmp    e038e <vsnprintf+0x1be>
   e0461:	8b 45 14             	mov    0x14(%ebp),%eax
   e0464:	8d 7d d4             	lea    -0x2c(%ebp),%edi
   e0467:	8b 55 14             	mov    0x14(%ebp),%edx
   e046a:	c7 45 c0 08 00 00 00 	movl   $0x8,-0x40(%ebp)
   e0471:	c7 45 c4 00 00 00 00 	movl   $0x0,-0x3c(%ebp)
   e0478:	83 c0 04             	add    $0x4,%eax
   e047b:	89 45 cc             	mov    %eax,-0x34(%ebp)
   e047e:	8b 02                	mov    (%edx),%eax
   e0480:	eb 8a                	jmp    e040c <vsnprintf+0x23c>
   e0482:	8b 4d 14             	mov    0x14(%ebp),%ecx
   e0485:	8b 55 14             	mov    0x14(%ebp),%edx
   e0488:	8b 01                	mov    (%ecx),%eax
   e048a:	83 c2 04             	add    $0x4,%edx
   e048d:	89 55 cc             	mov    %edx,-0x34(%ebp)
   e0490:	85 c0                	test   %eax,%eax
   e0492:	0f 88 b7 00 00 00    	js     e054f <vsnprintf+0x37f>
   e0498:	8d 7d d4             	lea    -0x2c(%ebp),%edi
   e049b:	c7 45 c0 0a 00 00 00 	movl   $0xa,-0x40(%ebp)
   e04a2:	c7 45 c4 00 00 00 00 	movl   $0x0,-0x3c(%ebp)
   e04a9:	e9 5e ff ff ff       	jmp    e040c <vsnprintf+0x23c>
   e04ae:	8b 55 14             	mov    0x14(%ebp),%edx
   e04b1:	8d 5d d4             	lea    -0x2c(%ebp),%ebx
   e04b4:	8b 02                	mov    (%edx),%eax
   e04b6:	83 c2 04             	add    $0x4,%edx
   e04b9:	c6 45 d5 00          	movb   $0x0,-0x2b(%ebp)
   e04bd:	89 55 14             	mov    %edx,0x14(%ebp)
   e04c0:	88 45 d4             	mov    %al,-0x2c(%ebp)
   e04c3:	e9 b2 fe ff ff       	jmp    e037a <vsnprintf+0x1aa>
   e04c8:	8b 55 14             	mov    0x14(%ebp),%edx
   e04cb:	8d 7d d4             	lea    -0x2c(%ebp),%edi
   e04ce:	8b 4d 14             	mov    0x14(%ebp),%ecx
   e04d1:	c7 45 c0 10 00 00 00 	movl   $0x10,-0x40(%ebp)
   e04d8:	c7 45 c4 00 00 00 00 	movl   $0x0,-0x3c(%ebp)
   e04df:	8b 02                	mov    (%edx),%eax
   e04e1:	83 c1 04             	add    $0x4,%ecx
   e04e4:	89 4d cc             	mov    %ecx,-0x34(%ebp)
   e04e7:	e9 20 ff ff ff       	jmp    e040c <vsnprintf+0x23c>
   e04ec:	8b 45 10             	mov    0x10(%ebp),%eax
   e04ef:	8b 5d 10             	mov    0x10(%ebp),%ebx
   e04f2:	0f be 48 02          	movsbl 0x2(%eax),%ecx
   e04f6:	83 c3 02             	add    $0x2,%ebx
   e04f9:	c7 45 bc 30 00 00 00 	movl   $0x30,-0x44(%ebp)
   e0500:	83 f9 2a             	cmp    $0x2a,%ecx
   e0503:	0f 85 3f fd ff ff    	jne    e0248 <vsnprintf+0x78>
   e0509:	8b 55 14             	mov    0x14(%ebp),%edx
   e050c:	83 c3 01             	add    $0x1,%ebx
   e050f:	8b 45 14             	mov    0x14(%ebp),%eax
   e0512:	8b 12                	mov    (%edx),%edx
   e0514:	83 c0 04             	add    $0x4,%eax
   e0517:	89 55 b8             	mov    %edx,-0x48(%ebp)
   e051a:	0f be 0b             	movsbl (%ebx),%ecx
   e051d:	89 45 14             	mov    %eax,0x14(%ebp)
   e0520:	83 f9 2e             	cmp    $0x2e,%ecx
   e0523:	0f 85 56 fd ff ff    	jne    e027f <vsnprintf+0xaf>
   e0529:	e9 b1 fd ff ff       	jmp    e02df <vsnprintf+0x10f>
   e052e:	83 eb 01             	sub    $0x1,%ebx
   e0531:	c6 03 2d             	movb   $0x2d,(%ebx)
   e0534:	e9 18 ff ff ff       	jmp    e0451 <vsnprintf+0x281>
   e0539:	8b 45 14             	mov    0x14(%ebp),%eax
   e053c:	83 c3 01             	add    $0x1,%ebx
   e053f:	0f be 0b             	movsbl (%ebx),%ecx
   e0542:	8b 30                	mov    (%eax),%esi
   e0544:	83 c0 04             	add    $0x4,%eax
   e0547:	89 45 14             	mov    %eax,0x14(%ebp)
   e054a:	e9 32 fd ff ff       	jmp    e0281 <vsnprintf+0xb1>
   e054f:	f7 d8                	neg    %eax
   e0551:	8d 7d d5             	lea    -0x2b(%ebp),%edi
   e0554:	c7 45 c0 0a 00 00 00 	movl   $0xa,-0x40(%ebp)
   e055b:	c7 45 c4 01 00 00 00 	movl   $0x1,-0x3c(%ebp)
   e0562:	e9 a5 fe ff ff       	jmp    e040c <vsnprintf+0x23c>
   e0567:	31 c0                	xor    %eax,%eax
   e0569:	e9 63 fd ff ff       	jmp    e02d1 <vsnprintf+0x101>
   e056e:	66 90                	xchg   %ax,%ax

000e0570 <delay_ms>:
   e0570:	55                   	push   %ebp
   e0571:	89 e5                	mov    %esp,%ebp
   e0573:	57                   	push   %edi
   e0574:	8b 7d 08             	mov    0x8(%ebp),%edi
   e0577:	56                   	push   %esi
   e0578:	53                   	push   %ebx
   e0579:	85 ff                	test   %edi,%edi
   e057b:	7e 30                	jle    e05ad <delay_ms+0x3d>
   e057d:	31 f6                	xor    %esi,%esi
   e057f:	90                   	nop    
   e0580:	ba 61 00 00 00       	mov    $0x61,%edx
   e0585:	ec                   	in     (%dx),%al
   e0586:	89 c3                	mov    %eax,%ebx
   e0588:	b9 42 00 00 00       	mov    $0x42,%ecx
   e058d:	83 e3 10             	and    $0x10,%ebx
   e0590:	ba 61 00 00 00       	mov    $0x61,%edx
   e0595:	ec                   	in     (%dx),%al
   e0596:	83 e0 10             	and    $0x10,%eax
   e0599:	39 c3                	cmp    %eax,%ebx
   e059b:	74 05                	je     e05a2 <delay_ms+0x32>
   e059d:	83 e9 01             	sub    $0x1,%ecx
   e05a0:	89 c3                	mov    %eax,%ebx
   e05a2:	85 c9                	test   %ecx,%ecx
   e05a4:	7f ea                	jg     e0590 <delay_ms+0x20>
   e05a6:	83 c6 01             	add    $0x1,%esi
   e05a9:	39 fe                	cmp    %edi,%esi
   e05ab:	75 d3                	jne    e0580 <delay_ms+0x10>
   e05ad:	5b                   	pop    %ebx
   e05ae:	5e                   	pop    %esi
   e05af:	5f                   	pop    %edi
   e05b0:	5d                   	pop    %ebp
   e05b1:	c3                   	ret    
   e05b2:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi
   e05b9:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e05c0 <wrmsr_smp>:
   e05c0:	55                   	push   %ebp
   e05c1:	89 e5                	mov    %esp,%ebp
   e05c3:	8b 4d 08             	mov    0x8(%ebp),%ecx
   e05c6:	53                   	push   %ebx
   e05c7:	8b 45 0c             	mov    0xc(%ebp),%eax
   e05ca:	8b 55 10             	mov    0x10(%ebp),%edx
   e05cd:	0f 30                	wrmsr  
   e05cf:	8b 1d 00 07 00 00    	mov    0x700,%ebx
   e05d5:	89 43 04             	mov    %eax,0x4(%ebx)
   e05d8:	89 d0                	mov    %edx,%eax
   e05da:	31 d2                	xor    %edx,%edx
   e05dc:	89 43 08             	mov    %eax,0x8(%ebx)
   e05df:	8d 43 0c             	lea    0xc(%ebx),%eax
   e05e2:	89 0b                	mov    %ecx,(%ebx)
   e05e4:	c7 43 0c 00 00 00 00 	movl   $0x0,0xc(%ebx)
   e05eb:	5b                   	pop    %ebx
   e05ec:	5d                   	pop    %ebp
   e05ed:	a3 00 07 00 00       	mov    %eax,0x700
   e05f2:	c3                   	ret    
   e05f3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e05f9:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e0600 <uuid_probe>:
   e0600:	55                   	push   %ebp
   e0601:	89 e5                	mov    %esp,%ebp
   e0603:	5d                   	pop    %ebp
   e0604:	c7 05 44 07 00 00 00 	movl   $0x0,0x744
   e060b:	00 00 00 
   e060e:	c7 05 48 07 00 00 00 	movl   $0x0,0x748
   e0615:	00 00 00 
   e0618:	c7 05 4c 07 00 00 00 	movl   $0x0,0x74c
   e061f:	00 00 00 
   e0622:	c7 05 50 07 00 00 00 	movl   $0x0,0x750
   e0629:	00 00 00 
   e062c:	c3                   	ret    
   e062d:	8d 76 00             	lea    0x0(%esi),%esi

000e0630 <cpu_probe>:
   e0630:	55                   	push   %ebp
   e0631:	b8 01 00 00 00       	mov    $0x1,%eax
   e0636:	89 e5                	mov    %esp,%ebp
   e0638:	53                   	push   %ebx
   e0639:	0f a2                	cpuid  
   e063b:	5b                   	pop    %ebx
   e063c:	5d                   	pop    %ebp
   e063d:	a3 58 07 00 00       	mov    %eax,0x758
   e0642:	89 15 30 07 00 00    	mov    %edx,0x730
   e0648:	89 0d 40 07 00 00    	mov    %ecx,0x740
   e064e:	c3                   	ret    
   e064f:	90                   	nop    

000e0650 <cmos_readb>:
   e0650:	55                   	push   %ebp
   e0651:	ba 70 00 00 00       	mov    $0x70,%edx
   e0656:	89 e5                	mov    %esp,%ebp
   e0658:	ee                   	out    %al,(%dx)
   e0659:	b2 71                	mov    $0x71,%dl
   e065b:	ec                   	in     (%dx),%al
   e065c:	5d                   	pop    %ebp
   e065d:	0f b6 c0             	movzbl %al,%eax
   e0660:	c3                   	ret    
   e0661:	eb 0d                	jmp    e0670 <setup_mtrr>
   e0663:	90                   	nop    
   e0664:	90                   	nop    
   e0665:	90                   	nop    
   e0666:	90                   	nop    
   e0667:	90                   	nop    
   e0668:	90                   	nop    
   e0669:	90                   	nop    
   e066a:	90                   	nop    
   e066b:	90                   	nop    
   e066c:	90                   	nop    
   e066d:	90                   	nop    
   e066e:	90                   	nop    
   e066f:	90                   	nop    

000e0670 <setup_mtrr>:
   e0670:	55                   	push   %ebp
   e0671:	89 e5                	mov    %esp,%ebp
   e0673:	53                   	push   %ebx
   e0674:	83 ec 1c             	sub    $0x1c,%esp
   e0677:	a1 30 07 00 00       	mov    0x730,%eax
   e067c:	c7 05 10 05 00 00 00 	movl   $0x0,0x510
   e0683:	00 00 00 
   e0686:	f6 c4 10             	test   $0x10,%ah
   e0689:	0f 84 08 02 00 00    	je     e0897 <setup_mtrr+0x227>
   e068f:	a8 20                	test   $0x20,%al
   e0691:	0f 84 00 02 00 00    	je     e0897 <setup_mtrr+0x227>
   e0697:	b9 fe 00 00 00       	mov    $0xfe,%ecx
   e069c:	0f 32                	rdmsr  
   e069e:	84 c0                	test   %al,%al
   e06a0:	0f 84 f1 01 00 00    	je     e0897 <setup_mtrr+0x227>
   e06a6:	f6 c4 01             	test   $0x1,%ah
   e06a9:	0f 84 e8 01 00 00    	je     e0897 <setup_mtrr+0x227>
   e06af:	8b 0d 34 07 00 00    	mov    0x734,%ecx
   e06b5:	31 d2                	xor    %edx,%edx
   e06b7:	c7 45 f4 00 00 00 00 	movl   $0x0,-0xc(%ebp)
   e06be:	8d 5d f4             	lea    -0xc(%ebp),%ebx
   e06c1:	c7 45 f8 00 00 00 00 	movl   $0x0,-0x8(%ebp)
   e06c8:	83 c2 01             	add    $0x1,%edx
   e06cb:	89 d0                	mov    %edx,%eax
   e06cd:	c1 e0 10             	shl    $0x10,%eax
   e06d0:	39 c1                	cmp    %eax,%ecx
   e06d2:	72 05                	jb     e06d9 <setup_mtrr+0x69>
   e06d4:	c6 44 1a ff 06       	movb   $0x6,-0x1(%edx,%ebx,1)
   e06d9:	83 fa 08             	cmp    $0x8,%edx
   e06dc:	75 ea                	jne    e06c8 <setup_mtrr+0x58>
   e06de:	8b 45 f4             	mov    -0xc(%ebp),%eax
   e06e1:	8d 5d f4             	lea    -0xc(%ebp),%ebx
   e06e4:	8b 55 f8             	mov    -0x8(%ebp),%edx
   e06e7:	c7 04 24 50 02 00 00 	movl   $0x250,(%esp)
   e06ee:	89 44 24 04          	mov    %eax,0x4(%esp)
   e06f2:	89 54 24 08          	mov    %edx,0x8(%esp)
   e06f6:	e8 c5 fe ff ff       	call   e05c0 <wrmsr_smp>
   e06fb:	8b 0d 34 07 00 00    	mov    0x734,%ecx
   e0701:	31 c0                	xor    %eax,%eax
   e0703:	c7 45 f4 00 00 00 00 	movl   $0x0,-0xc(%ebp)
   e070a:	ba 00 40 08 00       	mov    $0x84000,%edx
   e070f:	c7 45 f8 00 00 00 00 	movl   $0x0,-0x8(%ebp)
   e0716:	83 c0 01             	add    $0x1,%eax
   e0719:	39 d1                	cmp    %edx,%ecx
   e071b:	72 05                	jb     e0722 <setup_mtrr+0xb2>
   e071d:	c6 44 18 ff 06       	movb   $0x6,-0x1(%eax,%ebx,1)
   e0722:	81 c2 00 40 00 00    	add    $0x4000,%edx
   e0728:	83 f8 08             	cmp    $0x8,%eax
   e072b:	75 e9                	jne    e0716 <setup_mtrr+0xa6>
   e072d:	8b 45 f4             	mov    -0xc(%ebp),%eax
   e0730:	8b 55 f8             	mov    -0x8(%ebp),%edx
   e0733:	c7 04 24 58 02 00 00 	movl   $0x258,(%esp)
   e073a:	89 44 24 04          	mov    %eax,0x4(%esp)
   e073e:	89 54 24 08          	mov    %edx,0x8(%esp)
   e0742:	e8 79 fe ff ff       	call   e05c0 <wrmsr_smp>
   e0747:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e074e:	00 
   e074f:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e0756:	00 
   e0757:	c7 04 24 59 02 00 00 	movl   $0x259,(%esp)
   e075e:	e8 5d fe ff ff       	call   e05c0 <wrmsr_smp>
   e0763:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e076a:	00 
   e076b:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e0772:	00 
   e0773:	c7 04 24 68 02 00 00 	movl   $0x268,(%esp)
   e077a:	e8 41 fe ff ff       	call   e05c0 <wrmsr_smp>
   e077f:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e0786:	00 
   e0787:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e078e:	00 
   e078f:	c7 04 24 69 02 00 00 	movl   $0x269,(%esp)
   e0796:	e8 25 fe ff ff       	call   e05c0 <wrmsr_smp>
   e079b:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e07a2:	00 
   e07a3:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e07aa:	00 
   e07ab:	c7 04 24 6a 02 00 00 	movl   $0x26a,(%esp)
   e07b2:	e8 09 fe ff ff       	call   e05c0 <wrmsr_smp>
   e07b7:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e07be:	00 
   e07bf:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e07c6:	00 
   e07c7:	c7 04 24 6b 02 00 00 	movl   $0x26b,(%esp)
   e07ce:	e8 ed fd ff ff       	call   e05c0 <wrmsr_smp>
   e07d3:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e07da:	00 
   e07db:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e07e2:	00 
   e07e3:	c7 04 24 6c 02 00 00 	movl   $0x26c,(%esp)
   e07ea:	e8 d1 fd ff ff       	call   e05c0 <wrmsr_smp>
   e07ef:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e07f6:	00 
   e07f7:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e07fe:	00 
   e07ff:	c7 04 24 6d 02 00 00 	movl   $0x26d,(%esp)
   e0806:	e8 b5 fd ff ff       	call   e05c0 <wrmsr_smp>
   e080b:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e0812:	00 
   e0813:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e081a:	00 
   e081b:	c7 04 24 6e 02 00 00 	movl   $0x26e,(%esp)
   e0822:	e8 99 fd ff ff       	call   e05c0 <wrmsr_smp>
   e0827:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e082e:	00 
   e082f:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e0836:	00 
   e0837:	c7 04 24 6f 02 00 00 	movl   $0x26f,(%esp)
   e083e:	e8 7d fd ff ff       	call   e05c0 <wrmsr_smp>
   e0843:	c7 44 24 04 00 00 00 	movl   $0xe0000000,0x4(%esp)
   e084a:	e0 
   e084b:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e0852:	00 
   e0853:	c7 04 24 00 02 00 00 	movl   $0x200,(%esp)
   e085a:	e8 61 fd ff ff       	call   e05c0 <wrmsr_smp>
   e085f:	c7 44 24 04 00 08 00 	movl   $0xe0000800,0x4(%esp)
   e0866:	e0 
   e0867:	c7 44 24 08 ff ff ff 	movl   $0xffffffff,0x8(%esp)
   e086e:	ff 
   e086f:	c7 04 24 01 02 00 00 	movl   $0x201,(%esp)
   e0876:	e8 45 fd ff ff       	call   e05c0 <wrmsr_smp>
   e087b:	c7 44 24 04 06 0c 00 	movl   $0xc06,0x4(%esp)
   e0882:	00 
   e0883:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%esp)
   e088a:	00 
   e088b:	c7 04 24 ff 02 00 00 	movl   $0x2ff,(%esp)
   e0892:	e8 29 fd ff ff       	call   e05c0 <wrmsr_smp>
   e0897:	83 c4 1c             	add    $0x1c,%esp
   e089a:	5b                   	pop    %ebx
   e089b:	5d                   	pop    %ebp
   e089c:	c3                   	ret    
   e089d:	8d 76 00             	lea    0x0(%esi),%esi

000e08a0 <pci_config_writel>:
   e08a0:	55                   	push   %ebp
   e08a1:	81 e2 fc 00 00 00    	and    $0xfc,%edx
   e08a7:	89 e5                	mov    %esp,%ebp
   e08a9:	83 ec 08             	sub    $0x8,%esp
   e08ac:	89 1c 24             	mov    %ebx,(%esp)
   e08af:	bb f8 0c 00 00       	mov    $0xcf8,%ebx
   e08b4:	89 74 24 04          	mov    %esi,0x4(%esp)
   e08b8:	89 ce                	mov    %ecx,%esi
   e08ba:	8b 08                	mov    (%eax),%ecx
   e08bc:	8b 40 04             	mov    0x4(%eax),%eax
   e08bf:	c1 e1 10             	shl    $0x10,%ecx
   e08c2:	c1 e0 08             	shl    $0x8,%eax
   e08c5:	09 c1                	or     %eax,%ecx
   e08c7:	81 c9 00 00 00 80    	or     $0x80000000,%ecx
   e08cd:	09 d1                	or     %edx,%ecx
   e08cf:	89 da                	mov    %ebx,%edx
   e08d1:	89 c8                	mov    %ecx,%eax
   e08d3:	ef                   	out    %eax,(%dx)
   e08d4:	b2 fc                	mov    $0xfc,%dl
   e08d6:	89 f0                	mov    %esi,%eax
   e08d8:	ef                   	out    %eax,(%dx)
   e08d9:	8b 1c 24             	mov    (%esp),%ebx
   e08dc:	8b 74 24 04          	mov    0x4(%esp),%esi
   e08e0:	89 ec                	mov    %ebp,%esp
   e08e2:	5d                   	pop    %ebp
   e08e3:	c3                   	ret    
   e08e4:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e08ea:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

000e08f0 <pci_config_writew>:
   e08f0:	55                   	push   %ebp
   e08f1:	89 e5                	mov    %esp,%ebp
   e08f3:	83 ec 0c             	sub    $0xc,%esp
   e08f6:	89 1c 24             	mov    %ebx,(%esp)
   e08f9:	bb f8 0c 00 00       	mov    $0xcf8,%ebx
   e08fe:	89 74 24 04          	mov    %esi,0x4(%esp)
   e0902:	89 ce                	mov    %ecx,%esi
   e0904:	89 7c 24 08          	mov    %edi,0x8(%esp)
   e0908:	8b 08                	mov    (%eax),%ecx
   e090a:	89 d7                	mov    %edx,%edi
   e090c:	8b 40 04             	mov    0x4(%eax),%eax
   e090f:	c1 e1 10             	shl    $0x10,%ecx
   e0912:	c1 e0 08             	shl    $0x8,%eax
   e0915:	09 c1                	or     %eax,%ecx
   e0917:	89 d0                	mov    %edx,%eax
   e0919:	25 fc 00 00 00       	and    $0xfc,%eax
   e091e:	81 c9 00 00 00 80    	or     $0x80000000,%ecx
   e0924:	09 c1                	or     %eax,%ecx
   e0926:	89 da                	mov    %ebx,%edx
   e0928:	89 c8                	mov    %ecx,%eax
   e092a:	ef                   	out    %eax,(%dx)
   e092b:	83 e7 02             	and    $0x2,%edi
   e092e:	89 f0                	mov    %esi,%eax
   e0930:	8d 97 fc 0c 00 00    	lea    0xcfc(%edi),%edx
   e0936:	66 ef                	out    %ax,(%dx)
   e0938:	8b 1c 24             	mov    (%esp),%ebx
   e093b:	8b 74 24 04          	mov    0x4(%esp),%esi
   e093f:	8b 7c 24 08          	mov    0x8(%esp),%edi
   e0943:	89 ec                	mov    %ebp,%esp
   e0945:	5d                   	pop    %ebp
   e0946:	c3                   	ret    
   e0947:	89 f6                	mov    %esi,%esi
   e0949:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e0950 <pci_config_writeb>:
   e0950:	55                   	push   %ebp
   e0951:	89 e5                	mov    %esp,%ebp
   e0953:	83 ec 0c             	sub    $0xc,%esp
   e0956:	89 1c 24             	mov    %ebx,(%esp)
   e0959:	bb f8 0c 00 00       	mov    $0xcf8,%ebx
   e095e:	89 74 24 04          	mov    %esi,0x4(%esp)
   e0962:	89 ce                	mov    %ecx,%esi
   e0964:	89 7c 24 08          	mov    %edi,0x8(%esp)
   e0968:	8b 08                	mov    (%eax),%ecx
   e096a:	89 d7                	mov    %edx,%edi
   e096c:	8b 40 04             	mov    0x4(%eax),%eax
   e096f:	c1 e1 10             	shl    $0x10,%ecx
   e0972:	c1 e0 08             	shl    $0x8,%eax
   e0975:	09 c1                	or     %eax,%ecx
   e0977:	89 d0                	mov    %edx,%eax
   e0979:	25 fc 00 00 00       	and    $0xfc,%eax
   e097e:	81 c9 00 00 00 80    	or     $0x80000000,%ecx
   e0984:	09 c1                	or     %eax,%ecx
   e0986:	89 da                	mov    %ebx,%edx
   e0988:	89 c8                	mov    %ecx,%eax
   e098a:	ef                   	out    %eax,(%dx)
   e098b:	83 e7 03             	and    $0x3,%edi
   e098e:	89 f0                	mov    %esi,%eax
   e0990:	8d 97 fc 0c 00 00    	lea    0xcfc(%edi),%edx
   e0996:	ee                   	out    %al,(%dx)
   e0997:	8b 1c 24             	mov    (%esp),%ebx
   e099a:	8b 74 24 04          	mov    0x4(%esp),%esi
   e099e:	8b 7c 24 08          	mov    0x8(%esp),%edi
   e09a2:	89 ec                	mov    %ebp,%esp
   e09a4:	5d                   	pop    %ebp
   e09a5:	c3                   	ret    
   e09a6:	8d 76 00             	lea    0x0(%esi),%esi
   e09a9:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e09b0 <pci_config_readl>:
   e09b0:	55                   	push   %ebp
   e09b1:	81 e2 fc 00 00 00    	and    $0xfc,%edx
   e09b7:	89 e5                	mov    %esp,%ebp
   e09b9:	53                   	push   %ebx
   e09ba:	8b 08                	mov    (%eax),%ecx
   e09bc:	bb f8 0c 00 00       	mov    $0xcf8,%ebx
   e09c1:	8b 40 04             	mov    0x4(%eax),%eax
   e09c4:	c1 e1 10             	shl    $0x10,%ecx
   e09c7:	c1 e0 08             	shl    $0x8,%eax
   e09ca:	09 c1                	or     %eax,%ecx
   e09cc:	81 c9 00 00 00 80    	or     $0x80000000,%ecx
   e09d2:	09 d1                	or     %edx,%ecx
   e09d4:	89 da                	mov    %ebx,%edx
   e09d6:	89 c8                	mov    %ecx,%eax
   e09d8:	ef                   	out    %eax,(%dx)
   e09d9:	b2 fc                	mov    $0xfc,%dl
   e09db:	ed                   	in     (%dx),%eax
   e09dc:	5b                   	pop    %ebx
   e09dd:	5d                   	pop    %ebp
   e09de:	c3                   	ret    
   e09df:	90                   	nop    

000e09e0 <pci_config_readw>:
   e09e0:	55                   	push   %ebp
   e09e1:	89 e5                	mov    %esp,%ebp
   e09e3:	83 ec 08             	sub    $0x8,%esp
   e09e6:	89 1c 24             	mov    %ebx,(%esp)
   e09e9:	bb f8 0c 00 00       	mov    $0xcf8,%ebx
   e09ee:	89 74 24 04          	mov    %esi,0x4(%esp)
   e09f2:	8b 08                	mov    (%eax),%ecx
   e09f4:	89 d6                	mov    %edx,%esi
   e09f6:	8b 40 04             	mov    0x4(%eax),%eax
   e09f9:	c1 e1 10             	shl    $0x10,%ecx
   e09fc:	c1 e0 08             	shl    $0x8,%eax
   e09ff:	09 c1                	or     %eax,%ecx
   e0a01:	89 d0                	mov    %edx,%eax
   e0a03:	25 fc 00 00 00       	and    $0xfc,%eax
   e0a08:	81 c9 00 00 00 80    	or     $0x80000000,%ecx
   e0a0e:	09 c1                	or     %eax,%ecx
   e0a10:	89 da                	mov    %ebx,%edx
   e0a12:	89 c8                	mov    %ecx,%eax
   e0a14:	ef                   	out    %eax,(%dx)
   e0a15:	83 e6 02             	and    $0x2,%esi
   e0a18:	8d 96 fc 0c 00 00    	lea    0xcfc(%esi),%edx
   e0a1e:	66 ed                	in     (%dx),%ax
   e0a20:	8b 1c 24             	mov    (%esp),%ebx
   e0a23:	0f b7 c0             	movzwl %ax,%eax
   e0a26:	8b 74 24 04          	mov    0x4(%esp),%esi
   e0a2a:	89 ec                	mov    %ebp,%esp
   e0a2c:	5d                   	pop    %ebp
   e0a2d:	c3                   	ret    
   e0a2e:	66 90                	xchg   %ax,%ax

000e0a30 <pci_config_readb>:
   e0a30:	55                   	push   %ebp
   e0a31:	89 e5                	mov    %esp,%ebp
   e0a33:	83 ec 08             	sub    $0x8,%esp
   e0a36:	89 1c 24             	mov    %ebx,(%esp)
   e0a39:	bb f8 0c 00 00       	mov    $0xcf8,%ebx
   e0a3e:	89 74 24 04          	mov    %esi,0x4(%esp)
   e0a42:	8b 08                	mov    (%eax),%ecx
   e0a44:	89 d6                	mov    %edx,%esi
   e0a46:	8b 40 04             	mov    0x4(%eax),%eax
   e0a49:	c1 e1 10             	shl    $0x10,%ecx
   e0a4c:	c1 e0 08             	shl    $0x8,%eax
   e0a4f:	09 c1                	or     %eax,%ecx
   e0a51:	89 d0                	mov    %edx,%eax
   e0a53:	25 fc 00 00 00       	and    $0xfc,%eax
   e0a58:	81 c9 00 00 00 80    	or     $0x80000000,%ecx
   e0a5e:	09 c1                	or     %eax,%ecx
   e0a60:	89 da                	mov    %ebx,%edx
   e0a62:	89 c8                	mov    %ecx,%eax
   e0a64:	ef                   	out    %eax,(%dx)
   e0a65:	83 e6 03             	and    $0x3,%esi
   e0a68:	8d 96 fc 0c 00 00    	lea    0xcfc(%esi),%edx
   e0a6e:	ec                   	in     (%dx),%al
   e0a6f:	8b 1c 24             	mov    (%esp),%ebx
   e0a72:	0f b6 c0             	movzbl %al,%eax
   e0a75:	8b 74 24 04          	mov    0x4(%esp),%esi
   e0a79:	89 ec                	mov    %ebp,%esp
   e0a7b:	5d                   	pop    %ebp
   e0a7c:	c3                   	ret    
   e0a7d:	8d 76 00             	lea    0x0(%esi),%esi

000e0a80 <bios_lock_shadow_ram>:
   e0a80:	55                   	push   %ebp
   e0a81:	89 e5                	mov    %esp,%ebp
   e0a83:	0f 09                	wbinvd 
   e0a85:	ba 59 00 00 00       	mov    $0x59,%edx
   e0a8a:	b8 14 07 00 00       	mov    $0x714,%eax
   e0a8f:	e8 9c ff ff ff       	call   e0a30 <pci_config_readb>
   e0a94:	ba 59 00 00 00       	mov    $0x59,%edx
   e0a99:	5d                   	pop    %ebp
   e0a9a:	89 c1                	mov    %eax,%ecx
   e0a9c:	b8 14 07 00 00       	mov    $0x714,%eax
   e0aa1:	83 e1 0f             	and    $0xf,%ecx
   e0aa4:	83 c9 10             	or     $0x10,%ecx
   e0aa7:	e9 a4 fe ff ff       	jmp    e0950 <pci_config_writeb>
   e0aac:	8d 74 26 00          	lea    0x0(%esi),%esi

000e0ab0 <piix4_pm_enable>:
   e0ab0:	55                   	push   %ebp
   e0ab1:	b9 01 b0 00 00       	mov    $0xb001,%ecx
   e0ab6:	89 e5                	mov    %esp,%ebp
   e0ab8:	ba 40 00 00 00       	mov    $0x40,%edx
   e0abd:	56                   	push   %esi
   e0abe:	89 c6                	mov    %eax,%esi
   e0ac0:	53                   	push   %ebx
   e0ac1:	83 ec 10             	sub    $0x10,%esp
   e0ac4:	e8 d7 fd ff ff       	call   e08a0 <pci_config_writel>
   e0ac9:	b9 01 00 00 00       	mov    $0x1,%ecx
   e0ace:	ba 80 00 00 00       	mov    $0x80,%edx
   e0ad3:	89 f0                	mov    %esi,%eax
   e0ad5:	e8 76 fe ff ff       	call   e0950 <pci_config_writeb>
   e0ada:	b9 01 b1 00 00       	mov    $0xb101,%ecx
   e0adf:	ba 90 00 00 00       	mov    $0x90,%edx
   e0ae4:	89 f0                	mov    %esi,%eax
   e0ae6:	e8 b5 fd ff ff       	call   e08a0 <pci_config_writel>
   e0aeb:	b9 09 00 00 00       	mov    $0x9,%ecx
   e0af0:	ba d2 00 00 00       	mov    $0xd2,%edx
   e0af5:	89 f0                	mov    %esi,%eax
   e0af7:	e8 54 fe ff ff       	call   e0950 <pci_config_writeb>
   e0afc:	ba 58 00 00 00       	mov    $0x58,%edx
   e0b01:	89 f0                	mov    %esi,%eax
   e0b03:	e8 a8 fe ff ff       	call   e09b0 <pci_config_readl>
   e0b08:	a9 00 00 00 02       	test   $0x2000000,%eax
   e0b0d:	89 c3                	mov    %eax,%ebx
   e0b0f:	74 07                	je     e0b18 <piix4_pm_enable+0x68>
   e0b11:	83 c4 10             	add    $0x10,%esp
   e0b14:	5b                   	pop    %ebx
   e0b15:	5e                   	pop    %esi
   e0b16:	5d                   	pop    %ebp
   e0b17:	c3                   	ret    
   e0b18:	b9 4a 00 00 00       	mov    $0x4a,%ecx
   e0b1d:	ba 72 00 00 00       	mov    $0x72,%edx
   e0b22:	b8 14 07 00 00       	mov    $0x714,%eax
   e0b27:	81 cb 00 00 00 02    	or     $0x2000000,%ebx
   e0b2d:	e8 1e fe ff ff       	call   e0950 <pci_config_writeb>
   e0b32:	c7 44 24 08 00 80 00 	movl   $0x8000,0x8(%esp)
   e0b39:	00 
   e0b3a:	c7 44 24 04 00 80 03 	movl   $0x38000,0x4(%esp)
   e0b41:	00 
   e0b42:	c7 04 24 00 80 0a 00 	movl   $0xa8000,(%esp)
   e0b49:	e8 92 f5 ff ff       	call   e00e0 <memcpy>
   e0b4e:	b8 83 00 0e 00       	mov    $0xe0083,%eax
   e0b53:	2d 56 00 0e 00       	sub    $0xe0056,%eax
   e0b58:	89 44 24 08          	mov    %eax,0x8(%esp)
   e0b5c:	c7 44 24 04 56 00 0e 	movl   $0xe0056,0x4(%esp)
   e0b63:	00 
   e0b64:	c7 04 24 00 80 03 00 	movl   $0x38000,(%esp)
   e0b6b:	e8 70 f5 ff ff       	call   e00e0 <memcpy>
   e0b70:	89 f0                	mov    %esi,%eax
   e0b72:	89 d9                	mov    %ebx,%ecx
   e0b74:	ba 58 00 00 00       	mov    $0x58,%edx
   e0b79:	e8 22 fd ff ff       	call   e08a0 <pci_config_writel>
   e0b7e:	ba b3 00 00 00       	mov    $0xb3,%edx
   e0b83:	b8 01 00 00 00       	mov    $0x1,%eax
   e0b88:	ee                   	out    %al,(%dx)
   e0b89:	b2 b2                	mov    $0xb2,%dl
   e0b8b:	30 c0                	xor    %al,%al
   e0b8d:	ee                   	out    %al,(%dx)
   e0b8e:	66 90                	xchg   %ax,%ax
   e0b90:	ba b3 00 00 00       	mov    $0xb3,%edx
   e0b95:	ec                   	in     (%dx),%al
   e0b96:	84 c0                	test   %al,%al
   e0b98:	75 f6                	jne    e0b90 <piix4_pm_enable+0xe0>
   e0b9a:	c7 44 24 08 00 80 00 	movl   $0x8000,0x8(%esp)
   e0ba1:	00 
   e0ba2:	c7 44 24 04 00 80 0a 	movl   $0xa8000,0x4(%esp)
   e0ba9:	00 
   e0baa:	c7 04 24 00 80 03 00 	movl   $0x38000,(%esp)
   e0bb1:	e8 2a f5 ff ff       	call   e00e0 <memcpy>
   e0bb6:	b8 a3 00 0e 00       	mov    $0xe00a3,%eax
   e0bbb:	2d 83 00 0e 00       	sub    $0xe0083,%eax
   e0bc0:	89 44 24 08          	mov    %eax,0x8(%esp)
   e0bc4:	c7 44 24 04 83 00 0e 	movl   $0xe0083,0x4(%esp)
   e0bcb:	00 
   e0bcc:	c7 04 24 00 80 0a 00 	movl   $0xa8000,(%esp)
   e0bd3:	e8 08 f5 ff ff       	call   e00e0 <memcpy>
   e0bd8:	0f 09                	wbinvd 
   e0bda:	83 c4 10             	add    $0x10,%esp
   e0bdd:	b9 0a 00 00 00       	mov    $0xa,%ecx
   e0be2:	5b                   	pop    %ebx
   e0be3:	ba 72 00 00 00       	mov    $0x72,%edx
   e0be8:	5e                   	pop    %esi
   e0be9:	b8 14 07 00 00       	mov    $0x714,%eax
   e0bee:	5d                   	pop    %ebp
   e0bef:	e9 5c fd ff ff       	jmp    e0950 <pci_config_writeb>
   e0bf4:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e0bfa:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

000e0c00 <pci_for_each_device>:
   e0c00:	55                   	push   %ebp
   e0c01:	89 e5                	mov    %esp,%ebp
   e0c03:	57                   	push   %edi
   e0c04:	56                   	push   %esi
   e0c05:	31 f6                	xor    %esi,%esi
   e0c07:	53                   	push   %ebx
   e0c08:	83 ec 1c             	sub    $0x1c,%esp
   e0c0b:	8d 7d ec             	lea    -0x14(%ebp),%edi
   e0c0e:	eb 11                	jmp    e0c21 <pci_for_each_device+0x21>
   e0c10:	83 c6 01             	add    $0x1,%esi
   e0c13:	89 3c 24             	mov    %edi,(%esp)
   e0c16:	ff 55 08             	call   *0x8(%ebp)
   e0c19:	81 fe 00 01 00 00    	cmp    $0x100,%esi
   e0c1f:	74 38                	je     e0c59 <pci_for_each_device+0x59>
   e0c21:	31 d2                	xor    %edx,%edx
   e0c23:	89 f8                	mov    %edi,%eax
   e0c25:	c7 45 ec 00 00 00 00 	movl   $0x0,-0x14(%ebp)
   e0c2c:	89 75 f0             	mov    %esi,-0x10(%ebp)
   e0c2f:	e8 ac fd ff ff       	call   e09e0 <pci_config_readw>
   e0c34:	ba 02 00 00 00       	mov    $0x2,%edx
   e0c39:	89 c3                	mov    %eax,%ebx
   e0c3b:	89 f8                	mov    %edi,%eax
   e0c3d:	e8 9e fd ff ff       	call   e09e0 <pci_config_readw>
   e0c42:	66 83 c3 01          	add    $0x1,%bx
   e0c46:	75 c8                	jne    e0c10 <pci_for_each_device+0x10>
   e0c48:	66 83 c0 01          	add    $0x1,%ax
   e0c4c:	75 c2                	jne    e0c10 <pci_for_each_device+0x10>
   e0c4e:	83 c6 01             	add    $0x1,%esi
   e0c51:	81 fe 00 01 00 00    	cmp    $0x100,%esi
   e0c57:	75 c8                	jne    e0c21 <pci_for_each_device+0x21>
   e0c59:	83 c4 1c             	add    $0x1c,%esp
   e0c5c:	5b                   	pop    %ebx
   e0c5d:	5e                   	pop    %esi
   e0c5e:	5f                   	pop    %edi
   e0c5f:	5d                   	pop    %ebp
   e0c60:	c3                   	ret    
   e0c61:	eb 0d                	jmp    e0c70 <pci_bios_init>
   e0c63:	90                   	nop    
   e0c64:	90                   	nop    
   e0c65:	90                   	nop    
   e0c66:	90                   	nop    
   e0c67:	90                   	nop    
   e0c68:	90                   	nop    
   e0c69:	90                   	nop    
   e0c6a:	90                   	nop    
   e0c6b:	90                   	nop    
   e0c6c:	90                   	nop    
   e0c6d:	90                   	nop    
   e0c6e:	90                   	nop    
   e0c6f:	90                   	nop    

000e0c70 <pci_bios_init>:
   e0c70:	55                   	push   %ebp
   e0c71:	89 e5                	mov    %esp,%ebp
   e0c73:	83 ec 08             	sub    $0x8,%esp
   e0c76:	a1 34 07 00 00       	mov    0x734,%eax
   e0c7b:	c7 05 08 07 00 00 00 	movl   $0xc000,0x708
   e0c82:	c0 00 00 
   e0c85:	c7 05 0c 07 00 00 00 	movl   $0xc0000000,0x70c
   e0c8c:	00 00 c0 
   e0c8f:	3d ff ff ff 8f       	cmp    $0x8fffffff,%eax
   e0c94:	a3 10 07 00 00       	mov    %eax,0x710
   e0c99:	77 0a                	ja     e0ca5 <pci_bios_init+0x35>
   e0c9b:	c7 05 10 07 00 00 00 	movl   $0x90000000,0x710
   e0ca2:	00 00 90 
   e0ca5:	c7 04 24 60 24 0e 00 	movl   $0xe2460,(%esp)
   e0cac:	e8 4f ff ff ff       	call   e0c00 <pci_for_each_device>
   e0cb1:	c7 04 24 f0 19 0e 00 	movl   $0xe19f0,(%esp)
   e0cb8:	e8 43 ff ff ff       	call   e0c00 <pci_for_each_device>
   e0cbd:	c9                   	leave  
   e0cbe:	c3                   	ret    
   e0cbf:	90                   	nop    

000e0cc0 <putb>:
   e0cc0:	8b 08                	mov    (%eax),%ecx
   e0cc2:	55                   	push   %ebp
   e0cc3:	89 e5                	mov    %esp,%ebp
   e0cc5:	88 11                	mov    %dl,(%ecx)
   e0cc7:	83 c1 01             	add    $0x1,%ecx
   e0cca:	5d                   	pop    %ebp
   e0ccb:	89 08                	mov    %ecx,(%eax)
   e0ccd:	c3                   	ret    
   e0cce:	66 90                	xchg   %ax,%ax

000e0cd0 <putstr>:
   e0cd0:	0f b6 0a             	movzbl (%edx),%ecx
   e0cd3:	55                   	push   %ebp
   e0cd4:	89 e5                	mov    %esp,%ebp
   e0cd6:	53                   	push   %ebx
   e0cd7:	89 c3                	mov    %eax,%ebx
   e0cd9:	8b 00                	mov    (%eax),%eax
   e0cdb:	84 c9                	test   %cl,%cl
   e0cdd:	74 11                	je     e0cf0 <putstr+0x20>
   e0cdf:	90                   	nop    
   e0ce0:	88 08                	mov    %cl,(%eax)
   e0ce2:	0f b6 4a 01          	movzbl 0x1(%edx),%ecx
   e0ce6:	83 c0 01             	add    $0x1,%eax
   e0ce9:	83 c2 01             	add    $0x1,%edx
   e0cec:	84 c9                	test   %cl,%cl
   e0cee:	75 f0                	jne    e0ce0 <putstr+0x10>
   e0cf0:	89 03                	mov    %eax,(%ebx)
   e0cf2:	5b                   	pop    %ebx
   e0cf3:	5d                   	pop    %ebp
   e0cf4:	c3                   	ret    
   e0cf5:	8d 74 26 00          	lea    0x0(%esi),%esi
   e0cf9:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e0d00 <putle16>:
   e0d00:	8b 08                	mov    (%eax),%ecx
   e0d02:	55                   	push   %ebp
   e0d03:	89 e5                	mov    %esp,%ebp
   e0d05:	88 11                	mov    %dl,(%ecx)
   e0d07:	c1 fa 08             	sar    $0x8,%edx
   e0d0a:	88 51 01             	mov    %dl,0x1(%ecx)
   e0d0d:	83 c1 02             	add    $0x2,%ecx
   e0d10:	5d                   	pop    %ebp
   e0d11:	89 08                	mov    %ecx,(%eax)
   e0d13:	c3                   	ret    
   e0d14:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e0d1a:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

000e0d20 <putle32>:
   e0d20:	8b 08                	mov    (%eax),%ecx
   e0d22:	55                   	push   %ebp
   e0d23:	89 e5                	mov    %esp,%ebp
   e0d25:	88 11                	mov    %dl,(%ecx)
   e0d27:	c1 fa 08             	sar    $0x8,%edx
   e0d2a:	88 51 01             	mov    %dl,0x1(%ecx)
   e0d2d:	c1 fa 08             	sar    $0x8,%edx
   e0d30:	88 51 02             	mov    %dl,0x2(%ecx)
   e0d33:	c1 fa 08             	sar    $0x8,%edx
   e0d36:	88 51 03             	mov    %dl,0x3(%ecx)
   e0d39:	83 c1 04             	add    $0x4,%ecx
   e0d3c:	5d                   	pop    %ebp
   e0d3d:	89 08                	mov    %ecx,(%eax)
   e0d3f:	c3                   	ret    

000e0d40 <mpf_checksum>:
   e0d40:	55                   	push   %ebp
   e0d41:	89 e5                	mov    %esp,%ebp
   e0d43:	56                   	push   %esi
   e0d44:	89 c6                	mov    %eax,%esi
   e0d46:	31 c0                	xor    %eax,%eax
   e0d48:	85 d2                	test   %edx,%edx
   e0d4a:	53                   	push   %ebx
   e0d4b:	89 d3                	mov    %edx,%ebx
   e0d4d:	7e 14                	jle    e0d63 <mpf_checksum+0x23>
   e0d4f:	31 d2                	xor    %edx,%edx
   e0d51:	31 c9                	xor    %ecx,%ecx
   e0d53:	0f b6 04 31          	movzbl (%ecx,%esi,1),%eax
   e0d57:	83 c1 01             	add    $0x1,%ecx
   e0d5a:	01 c2                	add    %eax,%edx
   e0d5c:	39 d9                	cmp    %ebx,%ecx
   e0d5e:	75 f3                	jne    e0d53 <mpf_checksum+0x13>
   e0d60:	0f b6 c2             	movzbl %dl,%eax
   e0d63:	5b                   	pop    %ebx
   e0d64:	5e                   	pop    %esi
   e0d65:	5d                   	pop    %ebp
   e0d66:	c3                   	ret    
   e0d67:	89 f6                	mov    %esi,%esi
   e0d69:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e0d70 <align>:
   e0d70:	55                   	push   %ebp
   e0d71:	89 d1                	mov    %edx,%ecx
   e0d73:	8d 44 10 ff          	lea    -0x1(%eax,%edx,1),%eax
   e0d77:	89 e5                	mov    %esp,%ebp
   e0d79:	f7 d9                	neg    %ecx
   e0d7b:	5d                   	pop    %ebp
   e0d7c:	21 c1                	and    %eax,%ecx
   e0d7e:	89 c8                	mov    %ecx,%eax
   e0d80:	c3                   	ret    
   e0d81:	eb 0d                	jmp    e0d90 <acpi_checksum>
   e0d83:	90                   	nop    
   e0d84:	90                   	nop    
   e0d85:	90                   	nop    
   e0d86:	90                   	nop    
   e0d87:	90                   	nop    
   e0d88:	90                   	nop    
   e0d89:	90                   	nop    
   e0d8a:	90                   	nop    
   e0d8b:	90                   	nop    
   e0d8c:	90                   	nop    
   e0d8d:	90                   	nop    
   e0d8e:	90                   	nop    
   e0d8f:	90                   	nop    

000e0d90 <acpi_checksum>:
   e0d90:	55                   	push   %ebp
   e0d91:	89 e5                	mov    %esp,%ebp
   e0d93:	56                   	push   %esi
   e0d94:	89 c6                	mov    %eax,%esi
   e0d96:	31 c0                	xor    %eax,%eax
   e0d98:	85 d2                	test   %edx,%edx
   e0d9a:	53                   	push   %ebx
   e0d9b:	89 d3                	mov    %edx,%ebx
   e0d9d:	7e 16                	jle    e0db5 <acpi_checksum+0x25>
   e0d9f:	31 d2                	xor    %edx,%edx
   e0da1:	31 c9                	xor    %ecx,%ecx
   e0da3:	0f b6 04 31          	movzbl (%ecx,%esi,1),%eax
   e0da7:	83 c1 01             	add    $0x1,%ecx
   e0daa:	01 c2                	add    %eax,%edx
   e0dac:	39 d9                	cmp    %ebx,%ecx
   e0dae:	75 f3                	jne    e0da3 <acpi_checksum+0x13>
   e0db0:	f7 da                	neg    %edx
   e0db2:	0f b6 c2             	movzbl %dl,%eax
   e0db5:	5b                   	pop    %ebx
   e0db6:	5e                   	pop    %esi
   e0db7:	5d                   	pop    %ebp
   e0db8:	c3                   	ret    
   e0db9:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi

000e0dc0 <acpi_build_table_header>:
   e0dc0:	55                   	push   %ebp
   e0dc1:	89 e5                	mov    %esp,%ebp
   e0dc3:	53                   	push   %ebx
   e0dc4:	89 c3                	mov    %eax,%ebx
   e0dc6:	8b 02                	mov    (%edx),%eax
   e0dc8:	89 4b 04             	mov    %ecx,0x4(%ebx)
   e0dcb:	c7 43 0a 42 4f 43 48 	movl   $0x48434f42,0xa(%ebx)
   e0dd2:	89 03                	mov    %eax,(%ebx)
   e0dd4:	8b 45 08             	mov    0x8(%ebp),%eax
   e0dd7:	66 c7 43 0e 53 20    	movw   $0x2053,0xe(%ebx)
   e0ddd:	c7 43 10 42 58 50 43 	movl   $0x43505842,0x10(%ebx)
   e0de4:	88 43 08             	mov    %al,0x8(%ebx)
   e0de7:	8b 02                	mov    (%edx),%eax
   e0de9:	89 ca                	mov    %ecx,%edx
   e0deb:	c7 43 18 01 00 00 00 	movl   $0x1,0x18(%ebx)
   e0df2:	c7 43 1c 42 58 50 43 	movl   $0x43505842,0x1c(%ebx)
   e0df9:	c7 43 20 01 00 00 00 	movl   $0x1,0x20(%ebx)
   e0e00:	89 43 14             	mov    %eax,0x14(%ebx)
   e0e03:	89 d8                	mov    %ebx,%eax
   e0e05:	e8 86 ff ff ff       	call   e0d90 <acpi_checksum>
   e0e0a:	88 43 09             	mov    %al,0x9(%ebx)
   e0e0d:	5b                   	pop    %ebx
   e0e0e:	5d                   	pop    %ebp
   e0e0f:	c3                   	ret    

000e0e10 <acpi_build_processor_ssdt>:
   e0e10:	55                   	push   %ebp
   e0e11:	89 e5                	mov    %esp,%ebp
   e0e13:	56                   	push   %esi
   e0e14:	53                   	push   %ebx
   e0e15:	83 ec 04             	sub    $0x4,%esp
   e0e18:	0f b7 05 3c 07 00 00 	movzwl 0x73c,%eax
   e0e1f:	8b 75 08             	mov    0x8(%ebp),%esi
   e0e22:	66 3d ff 00          	cmp    $0xff,%ax
   e0e26:	77 2b                	ja     e0e53 <acpi_build_processor_ssdt+0x43>
   e0e28:	0f b7 d8             	movzwl %ax,%ebx
   e0e2b:	8d 04 5b             	lea    (%ebx,%ebx,2),%eax
   e0e2e:	8d 0c 83             	lea    (%ebx,%eax,4),%ecx
   e0e31:	8d 41 04             	lea    0x4(%ecx),%eax
   e0e34:	83 f8 3e             	cmp    $0x3e,%eax
   e0e37:	c6 46 09 00          	movb   $0x0,0x9(%esi)
   e0e3b:	8d 56 25             	lea    0x25(%esi),%edx
   e0e3e:	c6 46 24 10          	movb   $0x10,0x24(%esi)
   e0e42:	0f 8f e0 00 00 00    	jg     e0f28 <acpi_build_processor_ssdt+0x118>
   e0e48:	83 c0 01             	add    $0x1,%eax
   e0e4b:	88 46 25             	mov    %al,0x25(%esi)
   e0e4e:	8d 46 26             	lea    0x26(%esi),%eax
   e0e51:	eb 1e                	jmp    e0e71 <acpi_build_processor_ssdt+0x61>
   e0e53:	c6 46 09 00          	movb   $0x0,0x9(%esi)
   e0e57:	8d 56 25             	lea    0x25(%esi),%edx
   e0e5a:	bb ff 00 00 00       	mov    $0xff,%ebx
   e0e5f:	c6 46 24 10          	movb   $0x10,0x24(%esi)
   e0e63:	b8 33 00 00 00       	mov    $0x33,%eax
   e0e68:	88 42 01             	mov    %al,0x1(%edx)
   e0e6b:	8d 42 02             	lea    0x2(%edx),%eax
   e0e6e:	c6 02 7f             	movb   $0x7f,(%edx)
   e0e71:	85 db                	test   %ebx,%ebx
   e0e73:	c6 00 5f             	movb   $0x5f,(%eax)
   e0e76:	8d 50 04             	lea    0x4(%eax),%edx
   e0e79:	c6 40 01 50          	movb   $0x50,0x1(%eax)
   e0e7d:	c6 40 02 52          	movb   $0x52,0x2(%eax)
   e0e81:	c6 40 03 5f          	movb   $0x5f,0x3(%eax)
   e0e85:	0f 84 7b 00 00 00    	je     e0f06 <acpi_build_processor_ssdt+0xf6>
   e0e8b:	83 c0 0a             	add    $0xa,%eax
   e0e8e:	31 c9                	xor    %ecx,%ecx
   e0e90:	eb 43                	jmp    e0ed5 <acpi_build_processor_ssdt+0xc5>
   e0e92:	89 ca                	mov    %ecx,%edx
   e0e94:	c1 fa 04             	sar    $0x4,%edx
   e0e97:	83 fa 09             	cmp    $0x9,%edx
   e0e9a:	7f 65                	jg     e0f01 <acpi_build_processor_ssdt+0xf1>
   e0e9c:	83 c2 30             	add    $0x30,%edx
   e0e9f:	88 50 ff             	mov    %dl,-0x1(%eax)
   e0ea2:	89 ca                	mov    %ecx,%edx
   e0ea4:	83 e2 0f             	and    $0xf,%edx
   e0ea7:	83 fa 09             	cmp    $0x9,%edx
   e0eaa:	7f 50                	jg     e0efc <acpi_build_processor_ssdt+0xec>
   e0eac:	83 c2 30             	add    $0x30,%edx
   e0eaf:	88 48 01             	mov    %cl,0x1(%eax)
   e0eb2:	83 c1 01             	add    $0x1,%ecx
   e0eb5:	88 10                	mov    %dl,(%eax)
   e0eb7:	8d 50 07             	lea    0x7(%eax),%edx
   e0eba:	c6 40 02 10          	movb   $0x10,0x2(%eax)
   e0ebe:	c6 40 03 b0          	movb   $0xb0,0x3(%eax)
   e0ec2:	c6 40 04 00          	movb   $0x0,0x4(%eax)
   e0ec6:	c6 40 05 00          	movb   $0x0,0x5(%eax)
   e0eca:	c6 40 06 06          	movb   $0x6,0x6(%eax)
   e0ece:	83 c0 0d             	add    $0xd,%eax
   e0ed1:	39 d9                	cmp    %ebx,%ecx
   e0ed3:	74 31                	je     e0f06 <acpi_build_processor_ssdt+0xf6>
   e0ed5:	f6 c1 f0             	test   $0xf0,%cl
   e0ed8:	c6 40 fa 5b          	movb   $0x5b,-0x6(%eax)
   e0edc:	c6 40 fb 83          	movb   $0x83,-0x5(%eax)
   e0ee0:	c6 40 fc 0b          	movb   $0xb,-0x4(%eax)
   e0ee4:	c6 40 fd 43          	movb   $0x43,-0x3(%eax)
   e0ee8:	c6 40 fe 50          	movb   $0x50,-0x2(%eax)
   e0eec:	75 a4                	jne    e0e92 <acpi_build_processor_ssdt+0x82>
   e0eee:	89 ca                	mov    %ecx,%edx
   e0ef0:	83 e2 0f             	and    $0xf,%edx
   e0ef3:	83 fa 09             	cmp    $0x9,%edx
   e0ef6:	c6 40 ff 55          	movb   $0x55,-0x1(%eax)
   e0efa:	7e b0                	jle    e0eac <acpi_build_processor_ssdt+0x9c>
   e0efc:	83 c2 37             	add    $0x37,%edx
   e0eff:	eb ae                	jmp    e0eaf <acpi_build_processor_ssdt+0x9f>
   e0f01:	83 c2 37             	add    $0x37,%edx
   e0f04:	eb 99                	jmp    e0e9f <acpi_build_processor_ssdt+0x8f>
   e0f06:	89 d3                	mov    %edx,%ebx
   e0f08:	89 f0                	mov    %esi,%eax
   e0f0a:	29 f3                	sub    %esi,%ebx
   e0f0c:	ba 82 2e 0e 00       	mov    $0xe2e82,%edx
   e0f11:	89 d9                	mov    %ebx,%ecx
   e0f13:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
   e0f1a:	e8 a1 fe ff ff       	call   e0dc0 <acpi_build_table_header>
   e0f1f:	83 c4 04             	add    $0x4,%esp
   e0f22:	89 d8                	mov    %ebx,%eax
   e0f24:	5b                   	pop    %ebx
   e0f25:	5e                   	pop    %esi
   e0f26:	5d                   	pop    %ebp
   e0f27:	c3                   	ret    
   e0f28:	8d 41 06             	lea    0x6(%ecx),%eax
   e0f2b:	c1 e8 06             	shr    $0x6,%eax
   e0f2e:	e9 35 ff ff ff       	jmp    e0e68 <acpi_build_processor_ssdt+0x58>
   e0f33:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e0f39:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e0f40 <find_440fx>:
   e0f40:	55                   	push   %ebp
   e0f41:	31 d2                	xor    %edx,%edx
   e0f43:	89 e5                	mov    %esp,%ebp
   e0f45:	83 ec 08             	sub    $0x8,%esp
   e0f48:	89 1c 24             	mov    %ebx,(%esp)
   e0f4b:	89 74 24 04          	mov    %esi,0x4(%esp)
   e0f4f:	8b 75 08             	mov    0x8(%ebp),%esi
   e0f52:	89 f0                	mov    %esi,%eax
   e0f54:	e8 87 fa ff ff       	call   e09e0 <pci_config_readw>
   e0f59:	ba 02 00 00 00       	mov    $0x2,%edx
   e0f5e:	89 c3                	mov    %eax,%ebx
   e0f60:	89 f0                	mov    %esi,%eax
   e0f62:	e8 79 fa ff ff       	call   e09e0 <pci_config_readw>
   e0f67:	66 81 fb 86 80       	cmp    $0x8086,%bx
   e0f6c:	74 12                	je     e0f80 <find_440fx+0x40>
   e0f6e:	8b 1c 24             	mov    (%esp),%ebx
   e0f71:	8b 74 24 04          	mov    0x4(%esp),%esi
   e0f75:	89 ec                	mov    %ebp,%esp
   e0f77:	5d                   	pop    %ebp
   e0f78:	c3                   	ret    
   e0f79:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi
   e0f80:	66 3d 37 12          	cmp    $0x1237,%ax
   e0f84:	75 e8                	jne    e0f6e <find_440fx+0x2e>
   e0f86:	8b 16                	mov    (%esi),%edx
   e0f88:	8b 46 04             	mov    0x4(%esi),%eax
   e0f8b:	89 15 14 07 00 00    	mov    %edx,0x714
   e0f91:	a3 18 07 00 00       	mov    %eax,0x718
   e0f96:	8b 1c 24             	mov    (%esp),%ebx
   e0f99:	8b 74 24 04          	mov    0x4(%esp),%esi
   e0f9d:	89 ec                	mov    %ebp,%esp
   e0f9f:	5d                   	pop    %ebp
   e0fa0:	c3                   	ret    
   e0fa1:	eb 0d                	jmp    e0fb0 <reinit_piix4_pm>
   e0fa3:	90                   	nop    
   e0fa4:	90                   	nop    
   e0fa5:	90                   	nop    
   e0fa6:	90                   	nop    
   e0fa7:	90                   	nop    
   e0fa8:	90                   	nop    
   e0fa9:	90                   	nop    
   e0faa:	90                   	nop    
   e0fab:	90                   	nop    
   e0fac:	90                   	nop    
   e0fad:	90                   	nop    
   e0fae:	90                   	nop    
   e0faf:	90                   	nop    

000e0fb0 <reinit_piix4_pm>:
   e0fb0:	55                   	push   %ebp
   e0fb1:	31 d2                	xor    %edx,%edx
   e0fb3:	89 e5                	mov    %esp,%ebp
   e0fb5:	83 ec 08             	sub    $0x8,%esp
   e0fb8:	89 1c 24             	mov    %ebx,(%esp)
   e0fbb:	89 74 24 04          	mov    %esi,0x4(%esp)
   e0fbf:	8b 75 08             	mov    0x8(%ebp),%esi
   e0fc2:	89 f0                	mov    %esi,%eax
   e0fc4:	e8 17 fa ff ff       	call   e09e0 <pci_config_readw>
   e0fc9:	ba 02 00 00 00       	mov    $0x2,%edx
   e0fce:	89 c3                	mov    %eax,%ebx
   e0fd0:	89 f0                	mov    %esi,%eax
   e0fd2:	e8 09 fa ff ff       	call   e09e0 <pci_config_readw>
   e0fd7:	66 81 fb 86 80       	cmp    $0x8086,%bx
   e0fdc:	74 12                	je     e0ff0 <reinit_piix4_pm+0x40>
   e0fde:	8b 1c 24             	mov    (%esp),%ebx
   e0fe1:	8b 74 24 04          	mov    0x4(%esp),%esi
   e0fe5:	89 ec                	mov    %ebp,%esp
   e0fe7:	5d                   	pop    %ebp
   e0fe8:	c3                   	ret    
   e0fe9:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi
   e0ff0:	66 3d 13 71          	cmp    $0x7113,%ax
   e0ff4:	75 e8                	jne    e0fde <reinit_piix4_pm+0x2e>
   e0ff6:	89 f0                	mov    %esi,%eax
   e0ff8:	8b 1c 24             	mov    (%esp),%ebx
   e0ffb:	8b 74 24 04          	mov    0x4(%esp),%esi
   e0fff:	89 ec                	mov    %ebp,%esp
   e1001:	5d                   	pop    %ebp
   e1002:	e9 a9 fa ff ff       	jmp    e0ab0 <piix4_pm_enable>
   e1007:	89 f6                	mov    %esi,%esi
   e1009:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e1010 <snprintf>:
   e1010:	55                   	push   %ebp
   e1011:	89 e5                	mov    %esp,%ebp
   e1013:	83 ec 28             	sub    $0x28,%esp
   e1016:	8d 45 14             	lea    0x14(%ebp),%eax
   e1019:	89 45 fc             	mov    %eax,-0x4(%ebp)
   e101c:	89 44 24 0c          	mov    %eax,0xc(%esp)
   e1020:	8b 45 10             	mov    0x10(%ebp),%eax
   e1023:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1027:	8b 45 0c             	mov    0xc(%ebp),%eax
   e102a:	89 44 24 04          	mov    %eax,0x4(%esp)
   e102e:	8b 45 08             	mov    0x8(%ebp),%eax
   e1031:	89 04 24             	mov    %eax,(%esp)
   e1034:	e8 97 f1 ff ff       	call   e01d0 <vsnprintf>
   e1039:	c9                   	leave  
   e103a:	c3                   	ret    
   e103b:	90                   	nop    
   e103c:	8d 74 26 00          	lea    0x0(%esi),%esi

000e1040 <bios_printf>:
   e1040:	55                   	push   %ebp
   e1041:	89 e5                	mov    %esp,%ebp
   e1043:	53                   	push   %ebx
   e1044:	81 ec 24 04 00 00    	sub    $0x424,%esp
   e104a:	8b 45 08             	mov    0x8(%ebp),%eax
   e104d:	83 e0 07             	and    $0x7,%eax
   e1050:	83 f8 07             	cmp    $0x7,%eax
   e1053:	74 58                	je     e10ad <bios_printf+0x6d>
   e1055:	8d 45 10             	lea    0x10(%ebp),%eax
   e1058:	89 45 f8             	mov    %eax,-0x8(%ebp)
   e105b:	8d 9d f8 fb ff ff    	lea    -0x408(%ebp),%ebx
   e1061:	89 44 24 0c          	mov    %eax,0xc(%esp)
   e1065:	8b 45 0c             	mov    0xc(%ebp),%eax
   e1068:	c7 44 24 04 00 04 00 	movl   $0x400,0x4(%esp)
   e106f:	00 
   e1070:	89 1c 24             	mov    %ebx,(%esp)
   e1073:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1077:	e8 54 f1 ff ff       	call   e01d0 <vsnprintf>
   e107c:	0f b6 85 f8 fb ff ff 	movzbl -0x408(%ebp),%eax
   e1083:	84 c0                	test   %al,%al
   e1085:	74 1d                	je     e10a4 <bios_printf+0x64>
   e1087:	89 d9                	mov    %ebx,%ecx
   e1089:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi
   e1090:	ba 02 04 00 00       	mov    $0x402,%edx
   e1095:	0f be c0             	movsbl %al,%eax
   e1098:	ee                   	out    %al,(%dx)
   e1099:	0f b6 41 01          	movzbl 0x1(%ecx),%eax
   e109d:	83 c1 01             	add    $0x1,%ecx
   e10a0:	84 c0                	test   %al,%al
   e10a2:	75 ec                	jne    e1090 <bios_printf+0x50>
   e10a4:	81 c4 24 04 00 00    	add    $0x424,%esp
   e10aa:	5b                   	pop    %ebx
   e10ab:	5d                   	pop    %ebp
   e10ac:	c3                   	ret    
   e10ad:	ba 01 04 00 00       	mov    $0x401,%edx
   e10b2:	30 c0                	xor    %al,%al
   e10b4:	ee                   	out    %al,(%dx)
   e10b5:	eb 9e                	jmp    e1055 <bios_printf+0x15>
   e10b7:	89 f6                	mov    %esi,%esi
   e10b9:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e10c0 <smbios_init>:
   e10c0:	55                   	push   %ebp
   e10c1:	89 e5                	mov    %esp,%ebp
   e10c3:	57                   	push   %edi
   e10c4:	56                   	push   %esi
   e10c5:	31 f6                	xor    %esi,%esi
   e10c7:	53                   	push   %ebx
   e10c8:	83 ec 4c             	sub    $0x4c,%esp
   e10cb:	8b 3d 34 07 00 00    	mov    0x734,%edi
   e10d1:	8b 15 20 07 00 00    	mov    0x720,%edx
   e10d7:	8b 0d 24 07 00 00    	mov    0x724,%ecx
   e10dd:	89 f8                	mov    %edi,%eax
   e10df:	31 d0                	xor    %edx,%eax
   e10e1:	09 c8                	or     %ecx,%eax
   e10e3:	0f 84 73 05 00 00    	je     e165c <smbios_init+0x59c>
   e10e9:	83 c2 00             	add    $0x0,%edx
   e10ec:	83 d1 ff             	adc    $0xffffffff,%ecx
   e10ef:	01 fa                	add    %edi,%edx
   e10f1:	11 f1                	adc    %esi,%ecx
   e10f3:	0f ac ca 14          	shrd   $0x14,%ecx,%edx
   e10f7:	89 55 c8             	mov    %edx,-0x38(%ebp)
   e10fa:	c1 e9 14             	shr    $0x14,%ecx
   e10fd:	a1 54 07 00 00       	mov    0x754,%eax
   e1102:	ba 10 00 00 00       	mov    $0x10,%edx
   e1107:	e8 64 fc ff ff       	call   e0d70 <align>
   e110c:	89 45 e8             	mov    %eax,-0x18(%ebp)
   e110f:	a3 54 07 00 00       	mov    %eax,0x754
   e1114:	83 c0 1f             	add    $0x1f,%eax
   e1117:	89 45 c4             	mov    %eax,-0x3c(%ebp)
   e111a:	8b 45 e8             	mov    -0x18(%ebp),%eax
   e111d:	c6 40 1f 00          	movb   $0x0,0x1f(%eax)
   e1121:	8b 55 c4             	mov    -0x3c(%ebp),%edx
   e1124:	c6 42 04 01          	movb   $0x1,0x4(%edx)
   e1128:	c6 42 05 01          	movb   $0x1,0x5(%edx)
   e112c:	c6 42 08 02          	movb   $0x2,0x8(%edx)
   e1130:	c6 42 09 00          	movb   $0x0,0x9(%edx)
   e1134:	c6 42 01 18          	movb   $0x18,0x1(%edx)
   e1138:	66 c7 42 06 00 e8    	movw   $0xe800,0x6(%edx)
   e113e:	66 c7 42 02 00 00    	movw   $0x0,0x2(%edx)
   e1144:	c7 40 29 00 00 00 00 	movl   $0x0,0x29(%eax)
   e114b:	c7 40 2d 00 00 00 00 	movl   $0x0,0x2d(%eax)
   e1152:	c6 42 0a 08          	movb   $0x8,0xa(%edx)
   e1156:	c6 42 12 00          	movb   $0x0,0x12(%edx)
   e115a:	c6 42 13 00          	movb   $0x0,0x13(%edx)
   e115e:	c6 42 14 01          	movb   $0x1,0x14(%edx)
   e1162:	c6 42 15 00          	movb   $0x0,0x15(%edx)
   e1166:	c6 42 16 ff          	movb   $0xff,0x16(%edx)
   e116a:	c6 42 17 ff          	movb   $0xff,0x17(%edx)
   e116e:	c7 40 37 42 6f 63 68 	movl   $0x68636f42,0x37(%eax)
   e1175:	66 c7 40 3b 73 00    	movw   $0x73,0x3b(%eax)
   e117b:	8b 4d e8             	mov    -0x18(%ebp),%ecx
   e117e:	8b 45 e8             	mov    -0x18(%ebp),%eax
   e1181:	89 ca                	mov    %ecx,%edx
   e1183:	83 c2 49             	add    $0x49,%edx
   e1186:	83 c0 3d             	add    $0x3d,%eax
   e1189:	c7 41 3d 30 31 2f 30 	movl   $0x302f3130,0x3d(%ecx)
   e1190:	89 d3                	mov    %edx,%ebx
   e1192:	c7 40 04 31 2f 32 30 	movl   $0x30322f31,0x4(%eax)
   e1199:	66 c7 40 08 30 37    	movw   $0x3730,0x8(%eax)
   e119f:	c6 40 0a 00          	movb   $0x0,0xa(%eax)
   e11a3:	c6 41 48 00          	movb   $0x0,0x48(%ecx)
   e11a7:	2b 5d c4             	sub    -0x3c(%ebp),%ebx
   e11aa:	c6 41 49 01          	movb   $0x1,0x49(%ecx)
   e11ae:	c6 42 01 1b          	movb   $0x1b,0x1(%edx)
   e11b2:	c6 42 04 00          	movb   $0x0,0x4(%edx)
   e11b6:	c6 42 05 00          	movb   $0x0,0x5(%edx)
   e11ba:	c6 42 06 00          	movb   $0x0,0x6(%edx)
   e11be:	c6 42 07 00          	movb   $0x0,0x7(%edx)
   e11c2:	8b 4d e8             	mov    -0x18(%ebp),%ecx
   e11c5:	66 c7 42 02 00 01    	movw   $0x100,0x2(%edx)
   e11cb:	a1 44 07 00 00       	mov    0x744,%eax
   e11d0:	83 c1 51             	add    $0x51,%ecx
   e11d3:	89 42 08             	mov    %eax,0x8(%edx)
   e11d6:	a1 48 07 00 00       	mov    0x748,%eax
   e11db:	89 41 04             	mov    %eax,0x4(%ecx)
   e11de:	a1 4c 07 00 00       	mov    0x74c,%eax
   e11e3:	89 41 08             	mov    %eax,0x8(%ecx)
   e11e6:	a1 50 07 00 00       	mov    0x750,%eax
   e11eb:	c6 42 18 06          	movb   $0x6,0x18(%edx)
   e11ef:	c6 42 19 00          	movb   $0x0,0x19(%edx)
   e11f3:	c6 42 1a 00          	movb   $0x0,0x1a(%edx)
   e11f7:	89 41 0c             	mov    %eax,0xc(%ecx)
   e11fa:	8b 45 e8             	mov    -0x18(%ebp),%eax
   e11fd:	66 c7 42 1b 00 00    	movw   $0x0,0x1b(%edx)
   e1203:	c6 42 1d 03          	movb   $0x3,0x1d(%edx)
   e1207:	83 c0 66             	add    $0x66,%eax
   e120a:	c6 40 01 14          	movb   $0x14,0x1(%eax)
   e120e:	66 c7 40 02 00 03    	movw   $0x300,0x2(%eax)
   e1214:	c6 40 04 00          	movb   $0x0,0x4(%eax)
   e1218:	c6 40 05 01          	movb   $0x1,0x5(%eax)
   e121c:	c6 40 06 00          	movb   $0x0,0x6(%eax)
   e1220:	c6 40 07 00          	movb   $0x0,0x7(%eax)
   e1224:	c6 40 08 00          	movb   $0x0,0x8(%eax)
   e1228:	c6 40 09 03          	movb   $0x3,0x9(%eax)
   e122c:	c6 40 0a 03          	movb   $0x3,0xa(%eax)
   e1230:	c6 40 0b 03          	movb   $0x3,0xb(%eax)
   e1234:	c6 40 0c 02          	movb   $0x2,0xc(%eax)
   e1238:	c7 40 0d 00 00 00 00 	movl   $0x0,0xd(%eax)
   e123f:	c6 40 11 00          	movb   $0x0,0x11(%eax)
   e1243:	c6 40 12 00          	movb   $0x0,0x12(%eax)
   e1247:	c6 40 13 00          	movb   $0x0,0x13(%eax)
   e124b:	8b 75 e8             	mov    -0x18(%ebp),%esi
   e124e:	66 c7 42 31 00 00    	movw   $0x0,0x31(%edx)
   e1254:	83 c6 7c             	add    $0x7c,%esi
   e1257:	89 f1                	mov    %esi,%ecx
   e1259:	29 c1                	sub    %eax,%ecx
   e125b:	29 d0                	sub    %edx,%eax
   e125d:	39 c1                	cmp    %eax,%ecx
   e125f:	73 02                	jae    e1263 <smbios_init+0x1a3>
   e1261:	89 c1                	mov    %eax,%ecx
   e1263:	39 d9                	cmp    %ebx,%ecx
   e1265:	89 cf                	mov    %ecx,%edi
   e1267:	73 02                	jae    e126b <smbios_init+0x1ab>
   e1269:	89 df                	mov    %ebx,%edi
   e126b:	66 83 3d 3c 07 00 00 	cmpw   $0x0,0x73c
   e1272:	00 
   e1273:	c7 45 f0 04 00 00 00 	movl   $0x4,-0x10(%ebp)
   e127a:	0f 84 c7 00 00 00    	je     e1347 <smbios_init+0x287>
   e1280:	8b 4d e8             	mov    -0x18(%ebp),%ecx
   e1283:	bb 03 00 00 00       	mov    $0x3,%ebx
   e1288:	81 c1 a3 00 00 00    	add    $0xa3,%ecx
   e128e:	66 90                	xchg   %ax,%ax
   e1290:	8d 83 fe 03 00 00    	lea    0x3fe(%ebx),%eax
   e1296:	89 ce                	mov    %ecx,%esi
   e1298:	c6 41 d9 04          	movb   $0x4,-0x27(%ecx)
   e129c:	8d 51 f9             	lea    -0x7(%ecx),%edx
   e129f:	c6 41 da 20          	movb   $0x20,-0x26(%ecx)
   e12a3:	c6 41 dd 01          	movb   $0x1,-0x23(%ecx)
   e12a7:	c6 41 de 03          	movb   $0x3,-0x22(%ecx)
   e12ab:	c6 41 df 01          	movb   $0x1,-0x21(%ecx)
   e12af:	c6 41 e0 00          	movb   $0x0,-0x20(%ecx)
   e12b3:	66 89 41 db          	mov    %ax,-0x25(%ecx)
   e12b7:	a1 58 07 00 00       	mov    0x758,%eax
   e12bc:	66 c7 41 eb 00 00    	movw   $0x0,-0x15(%ecx)
   e12c2:	66 c7 41 ed 00 00    	movw   $0x0,-0x13(%ecx)
   e12c8:	66 c7 41 ef 00 00    	movw   $0x0,-0x11(%ecx)
   e12ce:	89 41 e1             	mov    %eax,-0x1f(%ecx)
   e12d1:	a1 30 07 00 00       	mov    0x730,%eax
   e12d6:	66 c7 41 f3 ff ff    	movw   $0xffff,-0xd(%ecx)
   e12dc:	c6 41 e9 00          	movb   $0x0,-0x17(%ecx)
   e12e0:	c6 41 ea 00          	movb   $0x0,-0x16(%ecx)
   e12e4:	89 41 e5             	mov    %eax,-0x1b(%ecx)
   e12e7:	c6 41 f1 41          	movb   $0x41,-0xf(%ecx)
   e12eb:	c6 41 f2 01          	movb   $0x1,-0xe(%ecx)
   e12ef:	66 c7 41 f5 ff ff    	movw   $0xffff,-0xb(%ecx)
   e12f5:	66 c7 41 f7 ff ff    	movw   $0xffff,-0x9(%ecx)
   e12fb:	a1 44 26 0e 00       	mov    0xe2644,%eax
   e1300:	89 41 f9             	mov    %eax,-0x7(%ecx)
   e1303:	0f b7 05 48 26 0e 00 	movzwl 0xe2648,%eax
   e130a:	66 89 42 04          	mov    %ax,0x4(%edx)
   e130e:	0f b6 05 4a 26 0e 00 	movzbl 0xe264a,%eax
   e1315:	88 42 06             	mov    %al,0x6(%edx)
   e1318:	8d 43 2e             	lea    0x2e(%ebx),%eax
   e131b:	83 c3 01             	add    $0x1,%ebx
   e131e:	88 41 fd             	mov    %al,-0x3(%ecx)
   e1321:	b8 27 00 00 00       	mov    $0x27,%eax
   e1326:	39 c7                	cmp    %eax,%edi
   e1328:	73 02                	jae    e132c <smbios_init+0x26c>
   e132a:	89 c7                	mov    %eax,%edi
   e132c:	0f b7 05 3c 07 00 00 	movzwl 0x73c,%eax
   e1333:	8d 53 fe             	lea    -0x2(%ebx),%edx
   e1336:	8d 4e 27             	lea    0x27(%esi),%ecx
   e1339:	39 c2                	cmp    %eax,%edx
   e133b:	0f 86 4f ff ff ff    	jbe    e1290 <smbios_init+0x1d0>
   e1341:	83 c3 01             	add    $0x1,%ebx
   e1344:	89 5d f0             	mov    %ebx,-0x10(%ebp)
   e1347:	8b 5d c8             	mov    -0x38(%ebp),%ebx
   e134a:	66 c7 46 02 00 10    	movw   $0x1000,0x2(%esi)
   e1350:	66 c7 46 0b fe ff    	movw   $0xfffe,0xb(%esi)
   e1356:	66 c7 46 0f 00 00    	movw   $0x0,0xf(%esi)
   e135c:	81 c3 ff 3f 00 00    	add    $0x3fff,%ebx
   e1362:	c1 fb 0e             	sar    $0xe,%ebx
   e1365:	89 5d d0             	mov    %ebx,-0x30(%ebp)
   e1368:	c6 06 10             	movb   $0x10,(%esi)
   e136b:	c6 46 01 0f          	movb   $0xf,0x1(%esi)
   e136f:	c6 46 04 01          	movb   $0x1,0x4(%esi)
   e1373:	c6 46 05 03          	movb   $0x3,0x5(%esi)
   e1377:	c6 46 06 01          	movb   $0x1,0x6(%esi)
   e137b:	8b 45 c8             	mov    -0x38(%ebp),%eax
   e137e:	66 89 5e 0d          	mov    %bx,0xd(%esi)
   e1382:	8d 5e 11             	lea    0x11(%esi),%ebx
   e1385:	c1 e0 0a             	shl    $0xa,%eax
   e1388:	89 46 07             	mov    %eax,0x7(%esi)
   e138b:	89 d8                	mov    %ebx,%eax
   e138d:	29 f0                	sub    %esi,%eax
   e138f:	39 f8                	cmp    %edi,%eax
   e1391:	89 45 e4             	mov    %eax,-0x1c(%ebp)
   e1394:	73 03                	jae    e1399 <smbios_init+0x2d9>
   e1396:	89 7d e4             	mov    %edi,-0x1c(%ebp)
   e1399:	8b 4d d0             	mov    -0x30(%ebp),%ecx
   e139c:	85 c9                	test   %ecx,%ecx
   e139e:	0f 8e 8e 01 00 00    	jle    e1532 <smbios_init+0x472>
   e13a4:	8b 4d c8             	mov    -0x38(%ebp),%ecx
   e13a7:	8b 55 d0             	mov    -0x30(%ebp),%edx
   e13aa:	89 5d ec             	mov    %ebx,-0x14(%ebp)
   e13ad:	c7 45 cc 00 00 00 00 	movl   $0x0,-0x34(%ebp)
   e13b4:	83 e9 01             	sub    $0x1,%ecx
   e13b7:	83 ea 01             	sub    $0x1,%edx
   e13ba:	81 e1 ff 3f 00 00    	and    $0x3fff,%ecx
   e13c0:	89 55 d8             	mov    %edx,-0x28(%ebp)
   e13c3:	89 4d c0             	mov    %ecx,-0x40(%ebp)
   e13c6:	e9 36 01 00 00       	jmp    e1501 <smbios_init+0x441>
   e13cb:	90                   	nop    
   e13cc:	8d 74 26 00          	lea    0x0(%esi),%esi
   e13d0:	c6 03 11             	movb   $0x11,(%ebx)
   e13d3:	8d 73 15             	lea    0x15(%ebx),%esi
   e13d6:	c6 43 01 15          	movb   $0x15,0x1(%ebx)
   e13da:	0f b7 45 cc          	movzwl -0x34(%ebp),%eax
   e13de:	66 89 53 0c          	mov    %dx,0xc(%ebx)
   e13e2:	66 c7 43 04 00 10    	movw   $0x1000,0x4(%ebx)
   e13e8:	66 c7 43 08 40 00    	movw   $0x40,0x8(%ebx)
   e13ee:	66 89 45 e2          	mov    %ax,-0x1e(%ebp)
   e13f2:	66 05 00 11          	add    $0x1100,%ax
   e13f6:	66 89 45 d6          	mov    %ax,-0x2a(%ebp)
   e13fa:	c6 43 0e 09          	movb   $0x9,0xe(%ebx)
   e13fe:	c6 43 0f 00          	movb   $0x0,0xf(%ebx)
   e1402:	c6 43 10 01          	movb   $0x1,0x10(%ebx)
   e1406:	c6 43 11 00          	movb   $0x0,0x11(%ebx)
   e140a:	c6 43 12 07          	movb   $0x7,0x12(%ebx)
   e140e:	8b 55 cc             	mov    -0x34(%ebp),%edx
   e1411:	66 89 43 02          	mov    %ax,0x2(%ebx)
   e1415:	66 c7 43 0a 40 00    	movw   $0x40,0xa(%ebx)
   e141b:	66 c7 43 13 00 00    	movw   $0x0,0x13(%ebx)
   e1421:	83 c3 16             	add    $0x16,%ebx
   e1424:	89 54 24 0c          	mov    %edx,0xc(%esp)
   e1428:	89 34 24             	mov    %esi,(%esp)
   e142b:	c7 44 24 08 87 2e 0e 	movl   $0xe2e87,0x8(%esp)
   e1432:	00 
   e1433:	c7 44 24 04 08 00 00 	movl   $0x8,0x4(%esp)
   e143a:	00 
   e143b:	e8 d0 fb ff ff       	call   e1010 <snprintf>
   e1440:	89 34 24             	mov    %esi,(%esp)
   e1443:	e8 68 ed ff ff       	call   e01b0 <strlen>
   e1448:	01 c3                	add    %eax,%ebx
   e144a:	8d 7b 01             	lea    0x1(%ebx),%edi
   e144d:	c6 03 00             	movb   $0x0,(%ebx)
   e1450:	c6 43 01 13          	movb   $0x13,0x1(%ebx)
   e1454:	c6 47 01 0f          	movb   $0xf,0x1(%edi)
   e1458:	8b 45 cc             	mov    -0x34(%ebp),%eax
   e145b:	8b 75 dc             	mov    -0x24(%ebp),%esi
   e145e:	0f b7 4d e2          	movzwl -0x1e(%ebp),%ecx
   e1462:	66 c7 47 0c 00 10    	movw   $0x1000,0xc(%edi)
   e1468:	c1 e0 18             	shl    $0x18,%eax
   e146b:	8d 74 06 ff          	lea    -0x1(%esi,%eax,1),%esi
   e146f:	89 75 bc             	mov    %esi,-0x44(%ebp)
   e1472:	66 81 c1 00 13       	add    $0x1300,%cx
   e1477:	89 77 08             	mov    %esi,0x8(%edi)
   e147a:	8d 73 12             	lea    0x12(%ebx),%esi
   e147d:	c6 47 0e 01          	movb   $0x1,0xe(%edi)
   e1481:	66 89 4f 02          	mov    %cx,0x2(%edi)
   e1485:	89 47 04             	mov    %eax,0x4(%edi)
   e1488:	66 c7 43 10 00 00    	movw   $0x0,0x10(%ebx)
   e148e:	c6 43 12 14          	movb   $0x14,0x12(%ebx)
   e1492:	c6 46 01 13          	movb   $0x13,0x1(%esi)
   e1496:	0f b7 55 e2          	movzwl -0x1e(%ebp),%edx
   e149a:	89 46 04             	mov    %eax,0x4(%esi)
   e149d:	8b 45 bc             	mov    -0x44(%ebp),%eax
   e14a0:	66 89 4e 0e          	mov    %cx,0xe(%esi)
   e14a4:	66 81 c2 00 14       	add    $0x1400,%dx
   e14a9:	66 89 56 02          	mov    %dx,0x2(%esi)
   e14ad:	0f b7 55 d6          	movzwl -0x2a(%ebp),%edx
   e14b1:	89 46 08             	mov    %eax,0x8(%esi)
   e14b4:	89 f8                	mov    %edi,%eax
   e14b6:	c6 46 10 01          	movb   $0x1,0x10(%esi)
   e14ba:	c6 46 11 00          	movb   $0x0,0x11(%esi)
   e14be:	c6 46 12 00          	movb   $0x0,0x12(%esi)
   e14c2:	66 89 56 0c          	mov    %dx,0xc(%esi)
   e14c6:	2b 45 ec             	sub    -0x14(%ebp),%eax
   e14c9:	66 c7 43 25 00 00    	movw   $0x0,0x25(%ebx)
   e14cf:	83 c3 27             	add    $0x27,%ebx
   e14d2:	3b 45 e4             	cmp    -0x1c(%ebp),%eax
   e14d5:	73 03                	jae    e14da <smbios_init+0x41a>
   e14d7:	8b 45 e4             	mov    -0x1c(%ebp),%eax
   e14da:	89 f2                	mov    %esi,%edx
   e14dc:	29 fa                	sub    %edi,%edx
   e14de:	39 d0                	cmp    %edx,%eax
   e14e0:	73 02                	jae    e14e4 <smbios_init+0x424>
   e14e2:	89 d0                	mov    %edx,%eax
   e14e4:	89 da                	mov    %ebx,%edx
   e14e6:	29 f2                	sub    %esi,%edx
   e14e8:	39 d0                	cmp    %edx,%eax
   e14ea:	89 45 e4             	mov    %eax,-0x1c(%ebp)
   e14ed:	73 03                	jae    e14f2 <smbios_init+0x432>
   e14ef:	89 55 e4             	mov    %edx,-0x1c(%ebp)
   e14f2:	83 45 cc 01          	addl   $0x1,-0x34(%ebp)
   e14f6:	8b 4d d0             	mov    -0x30(%ebp),%ecx
   e14f9:	39 4d cc             	cmp    %ecx,-0x34(%ebp)
   e14fc:	89 5d ec             	mov    %ebx,-0x14(%ebp)
   e14ff:	74 2b                	je     e152c <smbios_init+0x46c>
   e1501:	8b 75 d8             	mov    -0x28(%ebp),%esi
   e1504:	ba 00 40 00 00       	mov    $0x4000,%edx
   e1509:	39 75 cc             	cmp    %esi,-0x34(%ebp)
   e150c:	c7 45 dc 00 00 00 01 	movl   $0x1000000,-0x24(%ebp)
   e1513:	0f 85 b7 fe ff ff    	jne    e13d0 <smbios_init+0x310>
   e1519:	8b 45 c0             	mov    -0x40(%ebp),%eax
   e151c:	83 c0 01             	add    $0x1,%eax
   e151f:	89 c2                	mov    %eax,%edx
   e1521:	c1 e0 0a             	shl    $0xa,%eax
   e1524:	89 45 dc             	mov    %eax,-0x24(%ebp)
   e1527:	e9 a4 fe ff ff       	jmp    e13d0 <smbios_init+0x310>
   e152c:	8d 04 49             	lea    (%ecx,%ecx,2),%eax
   e152f:	01 45 f0             	add    %eax,-0x10(%ebp)
   e1532:	8d 53 0d             	lea    0xd(%ebx),%edx
   e1535:	89 d0                	mov    %edx,%eax
   e1537:	c7 43 04 00 00 00 00 	movl   $0x0,0x4(%ebx)
   e153e:	29 d8                	sub    %ebx,%eax
   e1540:	66 c7 43 08 00 00    	movw   $0x0,0x8(%ebx)
   e1546:	8d 73 13             	lea    0x13(%ebx),%esi
   e1549:	c6 43 0a 00          	movb   $0x0,0xa(%ebx)
   e154d:	c6 43 01 0b          	movb   $0xb,0x1(%ebx)
   e1551:	66 c7 43 0b 00 00    	movw   $0x0,0xb(%ebx)
   e1557:	c6 03 20             	movb   $0x20,(%ebx)
   e155a:	c6 42 01 04          	movb   $0x4,0x1(%edx)
   e155e:	c6 43 0d 7f          	movb   $0x7f,0xd(%ebx)
   e1562:	3b 45 e4             	cmp    -0x1c(%ebp),%eax
   e1565:	66 c7 43 02 00 20    	movw   $0x2000,0x2(%ebx)
   e156b:	66 c7 42 02 00 7f    	movw   $0x7f00,0x2(%edx)
   e1571:	66 c7 43 11 00 00    	movw   $0x0,0x11(%ebx)
   e1577:	73 03                	jae    e157c <smbios_init+0x4bc>
   e1579:	8b 45 e4             	mov    -0x1c(%ebp),%eax
   e157c:	8b 5d e8             	mov    -0x18(%ebp),%ebx
   e157f:	89 f1                	mov    %esi,%ecx
   e1581:	29 d1                	sub    %edx,%ecx
   e1583:	39 c8                	cmp    %ecx,%eax
   e1585:	c7 03 5f 53 4d 5f    	movl   $0x5f4d535f,(%ebx)
   e158b:	c6 43 05 1f          	movb   $0x1f,0x5(%ebx)
   e158f:	c6 43 06 02          	movb   $0x2,0x6(%ebx)
   e1593:	c6 43 07 04          	movb   $0x4,0x7(%ebx)
   e1597:	73 02                	jae    e159b <smbios_init+0x4db>
   e1599:	89 c8                	mov    %ecx,%eax
   e159b:	8b 5d e8             	mov    -0x18(%ebp),%ebx
   e159e:	c7 43 0b 00 00 00 00 	movl   $0x0,0xb(%ebx)
   e15a5:	c6 43 0f 00          	movb   $0x0,0xf(%ebx)
   e15a9:	c6 43 0a 00          	movb   $0x0,0xa(%ebx)
   e15ad:	c7 43 10 5f 44 4d 49 	movl   $0x494d445f,0x10(%ebx)
   e15b4:	c6 43 14 5f          	movb   $0x5f,0x14(%ebx)
   e15b8:	8b 55 e8             	mov    -0x18(%ebp),%edx
   e15bb:	66 89 43 08          	mov    %ax,0x8(%ebx)
   e15bf:	89 f0                	mov    %esi,%eax
   e15c1:	8b 4d c4             	mov    -0x3c(%ebp),%ecx
   e15c4:	66 29 d8             	sub    %bx,%ax
   e15c7:	83 e8 1f             	sub    $0x1f,%eax
   e15ca:	66 89 42 16          	mov    %ax,0x16(%edx)
   e15ce:	8b 45 f0             	mov    -0x10(%ebp),%eax
   e15d1:	c6 42 1e 24          	movb   $0x24,0x1e(%edx)
   e15d5:	c6 42 04 00          	movb   $0x0,0x4(%edx)
   e15d9:	c6 42 15 00          	movb   $0x0,0x15(%edx)
   e15dd:	8b 5d e8             	mov    -0x18(%ebp),%ebx
   e15e0:	83 c0 02             	add    $0x2,%eax
   e15e3:	66 89 42 1c          	mov    %ax,0x1c(%edx)
   e15e7:	b8 02 00 00 00       	mov    $0x2,%eax
   e15ec:	89 4a 18             	mov    %ecx,0x18(%edx)
   e15ef:	0f b6 13             	movzbl (%ebx),%edx
   e15f2:	8b 4d e8             	mov    -0x18(%ebp),%ecx
   e15f5:	02 54 01 ff          	add    -0x1(%ecx,%eax,1),%dl
   e15f9:	83 c0 01             	add    $0x1,%eax
   e15fc:	83 f8 11             	cmp    $0x11,%eax
   e15ff:	75 f1                	jne    e15f2 <smbios_init+0x532>
   e1601:	f7 da                	neg    %edx
   e1603:	31 c0                	xor    %eax,%eax
   e1605:	88 51 04             	mov    %dl,0x4(%ecx)
   e1608:	8b 5d e8             	mov    -0x18(%ebp),%ebx
   e160b:	0f b6 4b 05          	movzbl 0x5(%ebx),%ecx
   e160f:	83 f9 10             	cmp    $0x10,%ecx
   e1612:	7e 1a                	jle    e162e <smbios_init+0x56e>
   e1614:	89 d8                	mov    %ebx,%eax
   e1616:	31 d2                	xor    %edx,%edx
   e1618:	8d 4c 0b f0          	lea    -0x10(%ebx,%ecx,1),%ecx
   e161c:	8d 74 26 00          	lea    0x0(%esi),%esi
   e1620:	02 50 10             	add    0x10(%eax),%dl
   e1623:	83 c0 01             	add    $0x1,%eax
   e1626:	39 c8                	cmp    %ecx,%eax
   e1628:	75 f6                	jne    e1620 <smbios_init+0x560>
   e162a:	89 d0                	mov    %edx,%eax
   e162c:	f7 d8                	neg    %eax
   e162e:	8b 55 e8             	mov    -0x18(%ebp),%edx
   e1631:	88 42 15             	mov    %al,0x15(%edx)
   e1634:	29 d6                	sub    %edx,%esi
   e1636:	89 54 24 08          	mov    %edx,0x8(%esp)
   e163a:	01 35 54 07 00 00    	add    %esi,0x754
   e1640:	c7 44 24 04 8f 2e 0e 	movl   $0xe2e8f,0x4(%esp)
   e1647:	00 
   e1648:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e164f:	e8 ec f9 ff ff       	call   e1040 <bios_printf>
   e1654:	83 c4 4c             	add    $0x4c,%esp
   e1657:	5b                   	pop    %ebx
   e1658:	5e                   	pop    %esi
   e1659:	5f                   	pop    %edi
   e165a:	5d                   	pop    %ebp
   e165b:	c3                   	ret    
   e165c:	c1 ef 14             	shr    $0x14,%edi
   e165f:	89 7d c8             	mov    %edi,-0x38(%ebp)
   e1662:	e9 96 fa ff ff       	jmp    e10fd <smbios_init+0x3d>
   e1667:	89 f6                	mov    %esi,%esi
   e1669:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e1670 <acpi_bios_init>:
   e1670:	55                   	push   %ebp
   e1671:	ba 10 00 00 00       	mov    $0x10,%edx
   e1676:	89 e5                	mov    %esp,%ebp
   e1678:	57                   	push   %edi
   e1679:	56                   	push   %esi
   e167a:	53                   	push   %ebx
   e167b:	83 ec 2c             	sub    $0x2c,%esp
   e167e:	a1 54 07 00 00       	mov    0x754,%eax
   e1683:	e8 e8 f6 ff ff       	call   e0d70 <align>
   e1688:	8b 35 34 07 00 00    	mov    0x734,%esi
   e168e:	81 ee 00 00 01 00    	sub    $0x10000,%esi
   e1694:	8d 5e 30             	lea    0x30(%esi),%ebx
   e1697:	89 45 dc             	mov    %eax,-0x24(%ebp)
   e169a:	83 c0 24             	add    $0x24,%eax
   e169d:	a3 54 07 00 00       	mov    %eax,0x754
   e16a2:	8d 86 e3 00 00 00    	lea    0xe3(%esi),%eax
   e16a8:	83 e0 c0             	and    $0xffffffc0,%eax
   e16ab:	89 45 e4             	mov    %eax,-0x1c(%ebp)
   e16ae:	8b 55 e4             	mov    -0x1c(%ebp),%edx
   e16b1:	83 c0 40             	add    $0x40,%eax
   e16b4:	89 45 e8             	mov    %eax,-0x18(%ebp)
   e16b7:	81 c2 4d 08 00 00    	add    $0x84d,%edx
   e16bd:	89 55 ec             	mov    %edx,-0x14(%ebp)
   e16c0:	89 14 24             	mov    %edx,(%esp)
   e16c3:	e8 48 f7 ff ff       	call   e0e10 <acpi_build_processor_ssdt>
   e16c8:	8b 7d e4             	mov    -0x1c(%ebp),%edi
   e16cb:	89 74 24 0c          	mov    %esi,0xc(%esp)
   e16cf:	c7 44 24 04 a4 2f 0e 	movl   $0xe2fa4,0x4(%esp)
   e16d6:	00 
   e16d7:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e16de:	8d 84 07 54 08 00 00 	lea    0x854(%edi,%eax,1),%eax
   e16e5:	83 e0 f8             	and    $0xfffffff8,%eax
   e16e8:	89 45 f0             	mov    %eax,-0x10(%ebp)
   e16eb:	0f b7 05 3c 07 00 00 	movzwl 0x73c,%eax
   e16f2:	8d 04 c5 38 00 00 00 	lea    0x38(,%eax,8),%eax
   e16f9:	89 45 e0             	mov    %eax,-0x20(%ebp)
   e16fc:	29 f0                	sub    %esi,%eax
   e16fe:	03 45 f0             	add    -0x10(%ebp),%eax
   e1701:	89 44 24 10          	mov    %eax,0x10(%esp)
   e1705:	8b 45 dc             	mov    -0x24(%ebp),%eax
   e1708:	89 44 24 08          	mov    %eax,0x8(%esp)
   e170c:	e8 2f f9 ff ff       	call   e1040 <bios_printf>
   e1711:	8b 7d dc             	mov    -0x24(%ebp),%edi
   e1714:	31 c0                	xor    %eax,%eax
   e1716:	fc                   	cld    
   e1717:	b9 09 00 00 00       	mov    $0x9,%ecx
   e171c:	ba 14 00 00 00       	mov    $0x14,%edx
   e1721:	f3 ab                	rep stos %eax,%es:(%edi)
   e1723:	89 f7                	mov    %esi,%edi
   e1725:	8b 45 dc             	mov    -0x24(%ebp),%eax
   e1728:	89 70 10             	mov    %esi,0x10(%eax)
   e172b:	c7 00 52 53 44 20    	movl   $0x20445352,(%eax)
   e1731:	c7 40 04 50 54 52 20 	movl   $0x20525450,0x4(%eax)
   e1738:	c7 40 09 42 4f 43 48 	movl   $0x48434f42,0x9(%eax)
   e173f:	66 c7 40 0d 53 20    	movw   $0x2053,0xd(%eax)
   e1745:	8b 45 dc             	mov    -0x24(%ebp),%eax
   e1748:	e8 43 f6 ff ff       	call   e0d90 <acpi_checksum>
   e174d:	8b 55 dc             	mov    -0x24(%ebp),%edx
   e1750:	b9 0c 00 00 00       	mov    $0xc,%ecx
   e1755:	fc                   	cld    
   e1756:	88 42 08             	mov    %al,0x8(%edx)
   e1759:	31 c0                	xor    %eax,%eax
   e175b:	f3 ab                	rep stos %eax,%es:(%edi)
   e175d:	89 f0                	mov    %esi,%eax
   e175f:	89 5e 24             	mov    %ebx,0x24(%esi)
   e1762:	8b 55 f0             	mov    -0x10(%ebp),%edx
   e1765:	8b 7d ec             	mov    -0x14(%ebp),%edi
   e1768:	b1 30                	mov    $0x30,%cl
   e176a:	89 56 28             	mov    %edx,0x28(%esi)
   e176d:	ba aa 2e 0e 00       	mov    $0xe2eaa,%edx
   e1772:	89 7e 2c             	mov    %edi,0x2c(%esi)
   e1775:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
   e177c:	e8 3f f6 ff ff       	call   e0dc0 <acpi_build_table_header>
   e1781:	89 1c 24             	mov    %ebx,(%esp)
   e1784:	c7 44 24 08 74 00 00 	movl   $0x74,0x8(%esp)
   e178b:	00 
   e178c:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e1793:	00 
   e1794:	e8 17 e9 ff ff       	call   e00b0 <memset>
   e1799:	8b 45 e4             	mov    -0x1c(%ebp),%eax
   e179c:	b9 74 00 00 00       	mov    $0x74,%ecx
   e17a1:	8b 55 e8             	mov    -0x18(%ebp),%edx
   e17a4:	c6 43 2c 01          	movb   $0x1,0x2c(%ebx)
   e17a8:	c6 43 2d 00          	movb   $0x0,0x2d(%ebx)
   e17ac:	89 43 24             	mov    %eax,0x24(%ebx)
   e17af:	a1 28 07 00 00       	mov    0x728,%eax
   e17b4:	c6 43 34 f1          	movb   $0xf1,0x34(%ebx)
   e17b8:	c6 43 35 f0          	movb   $0xf0,0x35(%ebx)
   e17bc:	89 53 28             	mov    %edx,0x28(%ebx)
   e17bf:	66 89 43 2e          	mov    %ax,0x2e(%ebx)
   e17c3:	a1 60 07 00 00       	mov    0x760,%eax
   e17c8:	c7 43 30 b2 00 00 00 	movl   $0xb2,0x30(%ebx)
   e17cf:	c6 43 58 04          	movb   $0x4,0x58(%ebx)
   e17d3:	c6 43 59 02          	movb   $0x2,0x59(%ebx)
   e17d7:	89 43 38             	mov    %eax,0x38(%ebx)
   e17da:	8d 50 04             	lea    0x4(%eax),%edx
   e17dd:	83 c0 08             	add    $0x8,%eax
   e17e0:	89 53 40             	mov    %edx,0x40(%ebx)
   e17e3:	ba af 2e 0e 00       	mov    $0xe2eaf,%edx
   e17e8:	89 43 4c             	mov    %eax,0x4c(%ebx)
   e17eb:	89 d8                	mov    %ebx,%eax
   e17ed:	c6 43 5b 04          	movb   $0x4,0x5b(%ebx)
   e17f1:	66 c7 43 60 ff 0f    	movw   $0xfff,0x60(%ebx)
   e17f7:	66 c7 43 62 ff 0f    	movw   $0xfff,0x62(%ebx)
   e17fd:	c7 43 70 75 00 00 00 	movl   $0x75,0x70(%ebx)
   e1804:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
   e180b:	e8 b0 f5 ff ff       	call   e0dc0 <acpi_build_table_header>
   e1810:	8b 7d e4             	mov    -0x1c(%ebp),%edi
   e1813:	c7 44 24 08 40 00 00 	movl   $0x40,0x8(%esp)
   e181a:	00 
   e181b:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e1822:	00 
   e1823:	89 3c 24             	mov    %edi,(%esp)
   e1826:	e8 85 e8 ff ff       	call   e00b0 <memset>
   e182b:	89 f8                	mov    %edi,%eax
   e182d:	c7 07 46 41 43 53    	movl   $0x53434146,(%edi)
   e1833:	83 c0 0c             	add    $0xc,%eax
   e1836:	c7 47 04 40 00 00 00 	movl   $0x40,0x4(%edi)
   e183d:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1841:	c7 44 24 04 b4 2e 0e 	movl   $0xe2eb4,0x4(%esp)
   e1848:	00 
   e1849:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e1850:	e8 eb f7 ff ff       	call   e1040 <bios_printf>
   e1855:	8b 45 e8             	mov    -0x18(%ebp),%eax
   e1858:	c7 44 24 08 0d 08 00 	movl   $0x80d,0x8(%esp)
   e185f:	00 
   e1860:	c7 44 24 04 60 26 0e 	movl   $0xe2660,0x4(%esp)
   e1867:	00 
   e1868:	89 04 24             	mov    %eax,(%esp)
   e186b:	e8 70 e8 ff ff       	call   e00e0 <memcpy>
   e1870:	8b 7d f0             	mov    -0x10(%ebp),%edi
   e1873:	8b 55 e0             	mov    -0x20(%ebp),%edx
   e1876:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
   e187d:	00 
   e187e:	89 3c 24             	mov    %edi,(%esp)
   e1881:	89 54 24 08          	mov    %edx,0x8(%esp)
   e1885:	e8 26 e8 ff ff       	call   e00b0 <memset>
   e188a:	0f b7 35 3c 07 00 00 	movzwl 0x73c,%esi
   e1891:	89 fa                	mov    %edi,%edx
   e1893:	83 c2 2c             	add    $0x2c,%edx
   e1896:	c7 47 24 00 00 e0 fe 	movl   $0xfee00000,0x24(%edi)
   e189d:	c7 47 28 01 00 00 00 	movl   $0x1,0x28(%edi)
   e18a4:	66 85 f6             	test   %si,%si
   e18a7:	0f b7 de             	movzwl %si,%ebx
   e18aa:	74 28                	je     e18d4 <acpi_bios_init+0x264>
   e18ac:	89 f8                	mov    %edi,%eax
   e18ae:	31 c9                	xor    %ecx,%ecx
   e18b0:	83 c0 34             	add    $0x34,%eax
   e18b3:	88 48 fa             	mov    %cl,-0x6(%eax)
   e18b6:	89 c2                	mov    %eax,%edx
   e18b8:	88 48 fb             	mov    %cl,-0x5(%eax)
   e18bb:	83 c1 01             	add    $0x1,%ecx
   e18be:	39 cb                	cmp    %ecx,%ebx
   e18c0:	c6 40 f8 00          	movb   $0x0,-0x8(%eax)
   e18c4:	c6 40 f9 08          	movb   $0x8,-0x7(%eax)
   e18c8:	c7 40 fc 01 00 00 00 	movl   $0x1,-0x4(%eax)
   e18cf:	8d 40 08             	lea    0x8(%eax),%eax
   e18d2:	75 df                	jne    e18b3 <acpi_bios_init+0x243>
   e18d4:	89 f0                	mov    %esi,%eax
   e18d6:	c6 02 01             	movb   $0x1,(%edx)
   e18d9:	c6 42 01 0c          	movb   $0xc,0x1(%edx)
   e18dd:	88 42 02             	mov    %al,0x2(%edx)
   e18e0:	8b 4d e0             	mov    -0x20(%ebp),%ecx
   e18e3:	8b 45 f0             	mov    -0x10(%ebp),%eax
   e18e6:	c7 42 04 00 00 c0 fe 	movl   $0xfec00000,0x4(%edx)
   e18ed:	c7 42 08 00 00 00 00 	movl   $0x0,0x8(%edx)
   e18f4:	ba cf 2e 0e 00       	mov    $0xe2ecf,%edx
   e18f9:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
   e1900:	e8 bb f4 ff ff       	call   e0dc0 <acpi_build_table_header>
   e1905:	83 c4 2c             	add    $0x2c,%esp
   e1908:	5b                   	pop    %ebx
   e1909:	5e                   	pop    %esi
   e190a:	5f                   	pop    %edi
   e190b:	5d                   	pop    %ebp
   e190c:	c3                   	ret    
   e190d:	8d 76 00             	lea    0x0(%esi),%esi

000e1910 <pci_set_io_region_addr>:
   e1910:	55                   	push   %ebp
   e1911:	89 e5                	mov    %esp,%ebp
   e1913:	83 ec 28             	sub    $0x28,%esp
   e1916:	83 fa 06             	cmp    $0x6,%edx
   e1919:	89 75 f8             	mov    %esi,-0x8(%ebp)
   e191c:	89 c6                	mov    %eax,%esi
   e191e:	89 7d fc             	mov    %edi,-0x4(%ebp)
   e1921:	89 d7                	mov    %edx,%edi
   e1923:	89 5d f4             	mov    %ebx,-0xc(%ebp)
   e1926:	89 4d ec             	mov    %ecx,-0x14(%ebp)
   e1929:	74 75                	je     e19a0 <pci_set_io_region_addr+0x90>
   e192b:	8d 1c 95 10 00 00 00 	lea    0x10(,%edx,4),%ebx
   e1932:	89 da                	mov    %ebx,%edx
   e1934:	e8 77 f0 ff ff       	call   e09b0 <pci_config_readl>
   e1939:	8b 4d ec             	mov    -0x14(%ebp),%ecx
   e193c:	89 da                	mov    %ebx,%edx
   e193e:	89 45 f0             	mov    %eax,-0x10(%ebp)
   e1941:	89 f0                	mov    %esi,%eax
   e1943:	e8 58 ef ff ff       	call   e08a0 <pci_config_writel>
   e1948:	8b 45 ec             	mov    -0x14(%ebp),%eax
   e194b:	89 7c 24 08          	mov    %edi,0x8(%esp)
   e194f:	c7 44 24 04 d4 2e 0e 	movl   $0xe2ed4,0x4(%esp)
   e1956:	00 
   e1957:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e195e:	89 44 24 0c          	mov    %eax,0xc(%esp)
   e1962:	e8 d9 f6 ff ff       	call   e1040 <bios_printf>
   e1967:	ba 04 00 00 00       	mov    $0x4,%edx
   e196c:	89 f0                	mov    %esi,%eax
   e196e:	e8 6d f0 ff ff       	call   e09e0 <pci_config_readw>
   e1973:	f6 45 f0 01          	testb  $0x1,-0x10(%ebp)
   e1977:	75 1e                	jne    e1997 <pci_set_io_region_addr+0x87>
   e1979:	83 c8 02             	or     $0x2,%eax
   e197c:	8b 5d f4             	mov    -0xc(%ebp),%ebx
   e197f:	0f b7 c8             	movzwl %ax,%ecx
   e1982:	89 f0                	mov    %esi,%eax
   e1984:	8b 7d fc             	mov    -0x4(%ebp),%edi
   e1987:	ba 04 00 00 00       	mov    $0x4,%edx
   e198c:	8b 75 f8             	mov    -0x8(%ebp),%esi
   e198f:	89 ec                	mov    %ebp,%esp
   e1991:	5d                   	pop    %ebp
   e1992:	e9 59 ef ff ff       	jmp    e08f0 <pci_config_writew>
   e1997:	83 c8 01             	or     $0x1,%eax
   e199a:	eb e0                	jmp    e197c <pci_set_io_region_addr+0x6c>
   e199c:	8d 74 26 00          	lea    0x0(%esi),%esi
   e19a0:	ba 30 00 00 00       	mov    $0x30,%edx
   e19a5:	e8 06 f0 ff ff       	call   e09b0 <pci_config_readl>
   e19aa:	8b 4d ec             	mov    -0x14(%ebp),%ecx
   e19ad:	ba 30 00 00 00       	mov    $0x30,%edx
   e19b2:	89 f0                	mov    %esi,%eax
   e19b4:	e8 e7 ee ff ff       	call   e08a0 <pci_config_writel>
   e19b9:	8b 45 ec             	mov    -0x14(%ebp),%eax
   e19bc:	c7 44 24 08 06 00 00 	movl   $0x6,0x8(%esp)
   e19c3:	00 
   e19c4:	c7 44 24 04 d4 2e 0e 	movl   $0xe2ed4,0x4(%esp)
   e19cb:	00 
   e19cc:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e19d3:	89 44 24 0c          	mov    %eax,0xc(%esp)
   e19d7:	e8 64 f6 ff ff       	call   e1040 <bios_printf>
   e19dc:	ba 04 00 00 00       	mov    $0x4,%edx
   e19e1:	89 f0                	mov    %esi,%eax
   e19e3:	e8 f8 ef ff ff       	call   e09e0 <pci_config_readw>
   e19e8:	83 c8 02             	or     $0x2,%eax
   e19eb:	eb 8f                	jmp    e197c <pci_set_io_region_addr+0x6c>
   e19ed:	8d 76 00             	lea    0x0(%esi),%esi

000e19f0 <pci_bios_init_device>:
   e19f0:	55                   	push   %ebp
   e19f1:	ba 0a 00 00 00       	mov    $0xa,%edx
   e19f6:	89 e5                	mov    %esp,%ebp
   e19f8:	57                   	push   %edi
   e19f9:	56                   	push   %esi
   e19fa:	53                   	push   %ebx
   e19fb:	83 ec 2c             	sub    $0x2c,%esp
   e19fe:	8b 45 08             	mov    0x8(%ebp),%eax
   e1a01:	e8 da ef ff ff       	call   e09e0 <pci_config_readw>
   e1a06:	31 d2                	xor    %edx,%edx
   e1a08:	89 c3                	mov    %eax,%ebx
   e1a0a:	8b 45 08             	mov    0x8(%ebp),%eax
   e1a0d:	e8 ce ef ff ff       	call   e09e0 <pci_config_readw>
   e1a12:	ba 02 00 00 00       	mov    $0x2,%edx
   e1a17:	89 45 e8             	mov    %eax,-0x18(%ebp)
   e1a1a:	8b 45 08             	mov    0x8(%ebp),%eax
   e1a1d:	e8 be ef ff ff       	call   e09e0 <pci_config_readw>
   e1a22:	89 45 ec             	mov    %eax,-0x14(%ebp)
   e1a25:	89 5c 24 18          	mov    %ebx,0x18(%esp)
   e1a29:	89 44 24 14          	mov    %eax,0x14(%esp)
   e1a2d:	8b 45 e8             	mov    -0x18(%ebp),%eax
   e1a30:	89 44 24 10          	mov    %eax,0x10(%esp)
   e1a34:	8b 55 08             	mov    0x8(%ebp),%edx
   e1a37:	8b 42 04             	mov    0x4(%edx),%eax
   e1a3a:	89 44 24 0c          	mov    %eax,0xc(%esp)
   e1a3e:	8b 02                	mov    (%edx),%eax
   e1a40:	c7 44 24 04 e8 2f 0e 	movl   $0xe2fe8,0x4(%esp)
   e1a47:	00 
   e1a48:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e1a4f:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1a53:	e8 e8 f5 ff ff       	call   e1040 <bios_printf>
   e1a58:	81 fb 00 08 00 00    	cmp    $0x800,%ebx
   e1a5e:	0f 84 cd 01 00 00    	je     e1c31 <pci_bios_init_device+0x241>
   e1a64:	81 fb 00 ff 00 00    	cmp    $0xff00,%ebx
   e1a6a:	0f 84 90 01 00 00    	je     e1c00 <pci_bios_init_device+0x210>
   e1a70:	81 fb 01 01 00 00    	cmp    $0x101,%ebx
   e1a76:	0f 84 a0 00 00 00    	je     e1b1c <pci_bios_init_device+0x12c>
   e1a7c:	31 ff                	xor    %edi,%edi
   e1a7e:	c7 45 f0 10 00 00 00 	movl   $0x10,-0x10(%ebp)
   e1a85:	83 ff 06             	cmp    $0x6,%edi
   e1a88:	bb 30 00 00 00       	mov    $0x30,%ebx
   e1a8d:	74 03                	je     e1a92 <pci_bios_init_device+0xa2>
   e1a8f:	8b 5d f0             	mov    -0x10(%ebp),%ebx
   e1a92:	8b 45 08             	mov    0x8(%ebp),%eax
   e1a95:	b9 ff ff ff ff       	mov    $0xffffffff,%ecx
   e1a9a:	89 da                	mov    %ebx,%edx
   e1a9c:	e8 ff ed ff ff       	call   e08a0 <pci_config_writel>
   e1aa1:	8b 45 08             	mov    0x8(%ebp),%eax
   e1aa4:	89 da                	mov    %ebx,%edx
   e1aa6:	e8 05 ef ff ff       	call   e09b0 <pci_config_readl>
   e1aab:	85 c0                	test   %eax,%eax
   e1aad:	74 39                	je     e1ae8 <pci_bios_init_device+0xf8>
   e1aaf:	89 c2                	mov    %eax,%edx
   e1ab1:	be 08 07 00 00       	mov    $0x708,%esi
   e1ab6:	83 e2 f0             	and    $0xfffffff0,%edx
   e1ab9:	89 d3                	mov    %edx,%ebx
   e1abb:	f7 db                	neg    %ebx
   e1abd:	a8 01                	test   $0x1,%al
   e1abf:	75 12                	jne    e1ad3 <pci_bios_init_device+0xe3>
   e1ac1:	81 fb ff ff ff 03    	cmp    $0x3ffffff,%ebx
   e1ac7:	be 10 07 00 00       	mov    $0x710,%esi
   e1acc:	77 05                	ja     e1ad3 <pci_bios_init_device+0xe3>
   e1ace:	be 0c 07 00 00       	mov    $0x70c,%esi
   e1ad3:	8d 4b ff             	lea    -0x1(%ebx),%ecx
   e1ad6:	8b 45 08             	mov    0x8(%ebp),%eax
   e1ad9:	03 0e                	add    (%esi),%ecx
   e1adb:	21 d1                	and    %edx,%ecx
   e1add:	89 fa                	mov    %edi,%edx
   e1adf:	89 0e                	mov    %ecx,(%esi)
   e1ae1:	e8 2a fe ff ff       	call   e1910 <pci_set_io_region_addr>
   e1ae6:	01 1e                	add    %ebx,(%esi)
   e1ae8:	83 c7 01             	add    $0x1,%edi
   e1aeb:	83 45 f0 04          	addl   $0x4,-0x10(%ebp)
   e1aef:	83 ff 07             	cmp    $0x7,%edi
   e1af2:	75 91                	jne    e1a85 <pci_bios_init_device+0x95>
   e1af4:	8b 45 08             	mov    0x8(%ebp),%eax
   e1af7:	ba 3d 00 00 00       	mov    $0x3d,%edx
   e1afc:	e8 2f ef ff ff       	call   e0a30 <pci_config_readb>
   e1b01:	85 c0                	test   %eax,%eax
   e1b03:	89 c2                	mov    %eax,%edx
   e1b05:	75 69                	jne    e1b70 <pci_bios_init_device+0x180>
   e1b07:	81 7d e8 86 80 00 00 	cmpl   $0x8086,-0x18(%ebp)
   e1b0e:	0f 84 8d 00 00 00    	je     e1ba1 <pci_bios_init_device+0x1b1>
   e1b14:	83 c4 2c             	add    $0x2c,%esp
   e1b17:	5b                   	pop    %ebx
   e1b18:	5e                   	pop    %esi
   e1b19:	5f                   	pop    %edi
   e1b1a:	5d                   	pop    %ebp
   e1b1b:	c3                   	ret    
   e1b1c:	81 7d e8 86 80 00 00 	cmpl   $0x8086,-0x18(%ebp)
   e1b23:	0f 84 41 01 00 00    	je     e1c6a <pci_bios_init_device+0x27a>
   e1b29:	8b 45 08             	mov    0x8(%ebp),%eax
   e1b2c:	31 d2                	xor    %edx,%edx
   e1b2e:	b9 f0 01 00 00       	mov    $0x1f0,%ecx
   e1b33:	e8 d8 fd ff ff       	call   e1910 <pci_set_io_region_addr>
   e1b38:	8b 45 08             	mov    0x8(%ebp),%eax
   e1b3b:	b9 f4 03 00 00       	mov    $0x3f4,%ecx
   e1b40:	ba 01 00 00 00       	mov    $0x1,%edx
   e1b45:	e8 c6 fd ff ff       	call   e1910 <pci_set_io_region_addr>
   e1b4a:	8b 45 08             	mov    0x8(%ebp),%eax
   e1b4d:	b9 70 01 00 00       	mov    $0x170,%ecx
   e1b52:	ba 02 00 00 00       	mov    $0x2,%edx
   e1b57:	e8 b4 fd ff ff       	call   e1910 <pci_set_io_region_addr>
   e1b5c:	8b 45 08             	mov    0x8(%ebp),%eax
   e1b5f:	b9 74 03 00 00       	mov    $0x374,%ecx
   e1b64:	ba 03 00 00 00       	mov    $0x3,%edx
   e1b69:	e8 a2 fd ff ff       	call   e1910 <pci_set_io_region_addr>
   e1b6e:	eb 84                	jmp    e1af4 <pci_bios_init_device+0x104>
   e1b70:	8b 4d 08             	mov    0x8(%ebp),%ecx
   e1b73:	8b 41 04             	mov    0x4(%ecx),%eax
   e1b76:	c1 f8 03             	sar    $0x3,%eax
   e1b79:	8d 44 02 fe          	lea    -0x2(%edx,%eax,1),%eax
   e1b7d:	ba 3c 00 00 00       	mov    $0x3c,%edx
   e1b82:	83 e0 03             	and    $0x3,%eax
   e1b85:	0f b6 88 6d 2e 0e 00 	movzbl 0xe2e6d(%eax),%ecx
   e1b8c:	8b 45 08             	mov    0x8(%ebp),%eax
   e1b8f:	e8 bc ed ff ff       	call   e0950 <pci_config_writeb>
   e1b94:	81 7d e8 86 80 00 00 	cmpl   $0x8086,-0x18(%ebp)
   e1b9b:	0f 85 73 ff ff ff    	jne    e1b14 <pci_bios_init_device+0x124>
   e1ba1:	81 7d ec 13 71 00 00 	cmpl   $0x7113,-0x14(%ebp)
   e1ba8:	0f 85 66 ff ff ff    	jne    e1b14 <pci_bios_init_device+0x124>
   e1bae:	8b 45 08             	mov    0x8(%ebp),%eax
   e1bb1:	b9 09 00 00 00       	mov    $0x9,%ecx
   e1bb6:	ba 3c 00 00 00       	mov    $0x3c,%edx
   e1bbb:	c7 05 60 07 00 00 00 	movl   $0xb000,0x760
   e1bc2:	b0 00 00 
   e1bc5:	c7 05 2c 07 00 00 00 	movl   $0xb100,0x72c
   e1bcc:	b1 00 00 
   e1bcf:	e8 7c ed ff ff       	call   e0950 <pci_config_writeb>
   e1bd4:	8b 45 08             	mov    0x8(%ebp),%eax
   e1bd7:	ba 3c 00 00 00       	mov    $0x3c,%edx
   e1bdc:	e8 4f ee ff ff       	call   e0a30 <pci_config_readb>
   e1be1:	a3 28 07 00 00       	mov    %eax,0x728
   e1be6:	8b 45 08             	mov    0x8(%ebp),%eax
   e1be9:	e8 c2 ee ff ff       	call   e0ab0 <piix4_pm_enable>
   e1bee:	c7 05 5c 07 00 00 01 	movl   $0x1,0x75c
   e1bf5:	00 00 00 
   e1bf8:	83 c4 2c             	add    $0x2c,%esp
   e1bfb:	5b                   	pop    %ebx
   e1bfc:	5e                   	pop    %esi
   e1bfd:	5f                   	pop    %edi
   e1bfe:	5d                   	pop    %ebp
   e1bff:	c3                   	ret    
   e1c00:	81 7d e8 6b 10 00 00 	cmpl   $0x106b,-0x18(%ebp)
   e1c07:	0f 85 e7 fe ff ff    	jne    e1af4 <pci_bios_init_device+0x104>
   e1c0d:	83 7d ec 17          	cmpl   $0x17,-0x14(%ebp)
   e1c11:	74 0a                	je     e1c1d <pci_bios_init_device+0x22d>
   e1c13:	83 7d ec 22          	cmpl   $0x22,-0x14(%ebp)
   e1c17:	0f 85 d7 fe ff ff    	jne    e1af4 <pci_bios_init_device+0x104>
   e1c1d:	8b 45 08             	mov    0x8(%ebp),%eax
   e1c20:	b9 00 00 80 80       	mov    $0x80800000,%ecx
   e1c25:	31 d2                	xor    %edx,%edx
   e1c27:	e8 e4 fc ff ff       	call   e1910 <pci_set_io_region_addr>
   e1c2c:	e9 c3 fe ff ff       	jmp    e1af4 <pci_bios_init_device+0x104>
   e1c31:	81 7d e8 14 10 00 00 	cmpl   $0x1014,-0x18(%ebp)
   e1c38:	0f 85 b6 fe ff ff    	jne    e1af4 <pci_bios_init_device+0x104>
   e1c3e:	83 7d ec 46          	cmpl   $0x46,-0x14(%ebp)
   e1c42:	74 12                	je     e1c56 <pci_bios_init_device+0x266>
   e1c44:	81 7d ec ff ff 00 00 	cmpl   $0xffff,-0x14(%ebp)
   e1c4b:	90                   	nop    
   e1c4c:	8d 74 26 00          	lea    0x0(%esi),%esi
   e1c50:	0f 85 9e fe ff ff    	jne    e1af4 <pci_bios_init_device+0x104>
   e1c56:	8b 45 08             	mov    0x8(%ebp),%eax
   e1c59:	b9 00 00 84 80       	mov    $0x80840000,%ecx
   e1c5e:	31 d2                	xor    %edx,%edx
   e1c60:	e8 ab fc ff ff       	call   e1910 <pci_set_io_region_addr>
   e1c65:	e9 8a fe ff ff       	jmp    e1af4 <pci_bios_init_device+0x104>
   e1c6a:	81 7d ec 10 70 00 00 	cmpl   $0x7010,-0x14(%ebp)
   e1c71:	74 13                	je     e1c86 <pci_bios_init_device+0x296>
   e1c73:	81 7d ec 11 71 00 00 	cmpl   $0x7111,-0x14(%ebp)
   e1c7a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
   e1c80:	0f 85 a3 fe ff ff    	jne    e1b29 <pci_bios_init_device+0x139>
   e1c86:	8b 45 08             	mov    0x8(%ebp),%eax
   e1c89:	b9 00 80 00 00       	mov    $0x8000,%ecx
   e1c8e:	ba 40 00 00 00       	mov    $0x40,%edx
   e1c93:	e8 58 ec ff ff       	call   e08f0 <pci_config_writew>
   e1c98:	8b 45 08             	mov    0x8(%ebp),%eax
   e1c9b:	b9 00 80 00 00       	mov    $0x8000,%ecx
   e1ca0:	ba 42 00 00 00       	mov    $0x42,%edx
   e1ca5:	e8 46 ec ff ff       	call   e08f0 <pci_config_writew>
   e1caa:	e9 cd fd ff ff       	jmp    e1a7c <pci_bios_init_device+0x8c>
   e1caf:	90                   	nop    

000e1cb0 <smp_probe>:
   e1cb0:	55                   	push   %ebp
   e1cb1:	89 e5                	mov    %esp,%ebp
   e1cb3:	83 ec 18             	sub    $0x18,%esp
   e1cb6:	f6 05 31 07 00 00 02 	testb  $0x2,0x731
   e1cbd:	66 c7 05 3c 07 00 00 	movw   $0x1,0x73c
   e1cc4:	01 00 
   e1cc6:	74 4f                	je     e1d17 <smp_probe+0x67>
   e1cc8:	a1 f0 00 e0 fe       	mov    0xfee000f0,%eax
   e1ccd:	80 cc 01             	or     $0x1,%ah
   e1cd0:	a3 f0 00 e0 fe       	mov    %eax,0xfee000f0
   e1cd5:	b8 56 00 0e 00       	mov    $0xe0056,%eax
   e1cda:	2d 28 00 0e 00       	sub    $0xe0028,%eax
   e1cdf:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1ce3:	c7 44 24 04 28 00 0e 	movl   $0xe0028,0x4(%esp)
   e1cea:	00 
   e1ceb:	c7 04 24 00 f0 09 00 	movl   $0x9f000,(%esp)
   e1cf2:	e8 e9 e3 ff ff       	call   e00e0 <memcpy>
   e1cf7:	c7 05 00 03 e0 fe 00 	movl   $0xc4500,0xfee00300
   e1cfe:	45 0c 00 
   e1d01:	c7 05 00 03 e0 fe 9f 	movl   $0xc469f,0xfee00300
   e1d08:	46 0c 00 
   e1d0b:	c7 04 24 0a 00 00 00 	movl   $0xa,(%esp)
   e1d12:	e8 59 e8 ff ff       	call   e0570 <delay_ms>
   e1d17:	0f b7 05 3c 07 00 00 	movzwl 0x73c,%eax
   e1d1e:	c7 44 24 04 e7 2e 0e 	movl   $0xe2ee7,0x4(%esp)
   e1d25:	00 
   e1d26:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e1d2d:	0f b7 c0             	movzwl %ax,%eax
   e1d30:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1d34:	e8 07 f3 ff ff       	call   e1040 <bios_printf>
   e1d39:	c9                   	leave  
   e1d3a:	c3                   	ret    
   e1d3b:	90                   	nop    
   e1d3c:	8d 74 26 00          	lea    0x0(%esi),%esi

000e1d40 <ram_probe>:
   e1d40:	55                   	push   %ebp
   e1d41:	b8 34 00 00 00       	mov    $0x34,%eax
   e1d46:	89 e5                	mov    %esp,%ebp
   e1d48:	56                   	push   %esi
   e1d49:	53                   	push   %ebx
   e1d4a:	83 ec 20             	sub    $0x20,%esp
   e1d4d:	e8 fe e8 ff ff       	call   e0650 <cmos_readb>
   e1d52:	89 c3                	mov    %eax,%ebx
   e1d54:	b8 35 00 00 00       	mov    $0x35,%eax
   e1d59:	e8 f2 e8 ff ff       	call   e0650 <cmos_readb>
   e1d5e:	09 d8                	or     %ebx,%eax
   e1d60:	0f 84 bc 00 00 00    	je     e1e22 <ram_probe+0xe2>
   e1d66:	b8 34 00 00 00       	mov    $0x34,%eax
   e1d6b:	e8 e0 e8 ff ff       	call   e0650 <cmos_readb>
   e1d70:	89 c3                	mov    %eax,%ebx
   e1d72:	b8 35 00 00 00       	mov    $0x35,%eax
   e1d77:	e8 d4 e8 ff ff       	call   e0650 <cmos_readb>
   e1d7c:	c1 e0 08             	shl    $0x8,%eax
   e1d7f:	09 d8                	or     %ebx,%eax
   e1d81:	c1 e0 10             	shl    $0x10,%eax
   e1d84:	05 00 00 00 01       	add    $0x1000000,%eax
   e1d89:	a3 34 07 00 00       	mov    %eax,0x734
   e1d8e:	a1 34 07 00 00       	mov    0x734,%eax
   e1d93:	c7 44 24 04 f8 2e 0e 	movl   $0xe2ef8,0x4(%esp)
   e1d9a:	00 
   e1d9b:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e1da2:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1da6:	e8 95 f2 ff ff       	call   e1040 <bios_printf>
   e1dab:	b8 5b 00 00 00       	mov    $0x5b,%eax
   e1db0:	e8 9b e8 ff ff       	call   e0650 <cmos_readb>
   e1db5:	89 c6                	mov    %eax,%esi
   e1db7:	b8 5c 00 00 00       	mov    $0x5c,%eax
   e1dbc:	e8 8f e8 ff ff       	call   e0650 <cmos_readb>
   e1dc1:	89 c3                	mov    %eax,%ebx
   e1dc3:	b8 5d 00 00 00       	mov    $0x5d,%eax
   e1dc8:	e8 83 e8 ff ff       	call   e0650 <cmos_readb>
   e1dcd:	09 f3                	or     %esi,%ebx
   e1dcf:	09 c3                	or     %eax,%ebx
   e1dd1:	0f 85 79 00 00 00    	jne    e1e50 <ram_probe+0x110>
   e1dd7:	8b 15 34 07 00 00    	mov    0x734,%edx
   e1ddd:	c7 05 24 07 00 00 00 	movl   $0x0,0x724
   e1de4:	00 00 00 
   e1de7:	89 15 20 07 00 00    	mov    %edx,0x720
   e1ded:	8b 15 24 07 00 00    	mov    0x724,%edx
   e1df3:	a1 20 07 00 00       	mov    0x720,%eax
   e1df8:	c7 44 24 04 0a 2f 0e 	movl   $0xe2f0a,0x4(%esp)
   e1dff:	00 
   e1e00:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e1e07:	0f ac d0 14          	shrd   $0x14,%edx,%eax
   e1e0b:	c1 ea 14             	shr    $0x14,%edx
   e1e0e:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1e12:	89 54 24 0c          	mov    %edx,0xc(%esp)
   e1e16:	e8 25 f2 ff ff       	call   e1040 <bios_printf>
   e1e1b:	83 c4 20             	add    $0x20,%esp
   e1e1e:	5b                   	pop    %ebx
   e1e1f:	5e                   	pop    %esi
   e1e20:	5d                   	pop    %ebp
   e1e21:	c3                   	ret    
   e1e22:	b8 30 00 00 00       	mov    $0x30,%eax
   e1e27:	e8 24 e8 ff ff       	call   e0650 <cmos_readb>
   e1e2c:	89 c3                	mov    %eax,%ebx
   e1e2e:	b8 31 00 00 00       	mov    $0x31,%eax
   e1e33:	e8 18 e8 ff ff       	call   e0650 <cmos_readb>
   e1e38:	c1 e0 08             	shl    $0x8,%eax
   e1e3b:	09 d8                	or     %ebx,%eax
   e1e3d:	c1 e0 0a             	shl    $0xa,%eax
   e1e40:	05 00 00 10 00       	add    $0x100000,%eax
   e1e45:	a3 34 07 00 00       	mov    %eax,0x734
   e1e4a:	e9 3f ff ff ff       	jmp    e1d8e <ram_probe+0x4e>
   e1e4f:	90                   	nop    
   e1e50:	b8 5b 00 00 00       	mov    $0x5b,%eax
   e1e55:	e8 f6 e7 ff ff       	call   e0650 <cmos_readb>
   e1e5a:	89 c6                	mov    %eax,%esi
   e1e5c:	b8 5c 00 00 00       	mov    $0x5c,%eax
   e1e61:	e8 ea e7 ff ff       	call   e0650 <cmos_readb>
   e1e66:	89 c3                	mov    %eax,%ebx
   e1e68:	b8 5d 00 00 00       	mov    $0x5d,%eax
   e1e6d:	e8 de e7 ff ff       	call   e0650 <cmos_readb>
   e1e72:	89 5d f0             	mov    %ebx,-0x10(%ebp)
   e1e75:	89 c1                	mov    %eax,%ecx
   e1e77:	89 d8                	mov    %ebx,%eax
   e1e79:	c1 f8 1f             	sar    $0x1f,%eax
   e1e7c:	89 45 f4             	mov    %eax,-0xc(%ebp)
   e1e7f:	8b 45 f0             	mov    -0x10(%ebp),%eax
   e1e82:	8b 55 f4             	mov    -0xc(%ebp),%edx
   e1e85:	89 4d e8             	mov    %ecx,-0x18(%ebp)
   e1e88:	0f a4 c2 18          	shld   $0x18,%eax,%edx
   e1e8c:	c1 e0 18             	shl    $0x18,%eax
   e1e8f:	89 45 f0             	mov    %eax,-0x10(%ebp)
   e1e92:	8b 5d f0             	mov    -0x10(%ebp),%ebx
   e1e95:	89 f0                	mov    %esi,%eax
   e1e97:	89 55 f4             	mov    %edx,-0xc(%ebp)
   e1e9a:	89 f2                	mov    %esi,%edx
   e1e9c:	8b 75 f4             	mov    -0xc(%ebp),%esi
   e1e9f:	c1 fa 1f             	sar    $0x1f,%edx
   e1ea2:	0f a4 c2 10          	shld   $0x10,%eax,%edx
   e1ea6:	c1 e0 10             	shl    $0x10,%eax
   e1ea9:	09 c3                	or     %eax,%ebx
   e1eab:	8b 45 e8             	mov    -0x18(%ebp),%eax
   e1eae:	09 d6                	or     %edx,%esi
   e1eb0:	89 ca                	mov    %ecx,%edx
   e1eb2:	c1 fa 1f             	sar    $0x1f,%edx
   e1eb5:	89 55 ec             	mov    %edx,-0x14(%ebp)
   e1eb8:	8b 55 ec             	mov    -0x14(%ebp),%edx
   e1ebb:	89 c2                	mov    %eax,%edx
   e1ebd:	b8 00 00 00 00       	mov    $0x0,%eax
   e1ec2:	89 55 ec             	mov    %edx,-0x14(%ebp)
   e1ec5:	8b 4d ec             	mov    -0x14(%ebp),%ecx
   e1ec8:	89 45 e8             	mov    %eax,-0x18(%ebp)
   e1ecb:	8b 55 e8             	mov    -0x18(%ebp),%edx
   e1ece:	09 f1                	or     %esi,%ecx
   e1ed0:	09 da                	or     %ebx,%edx
   e1ed2:	83 c2 00             	add    $0x0,%edx
   e1ed5:	83 d1 01             	adc    $0x1,%ecx
   e1ed8:	89 15 20 07 00 00    	mov    %edx,0x720
   e1ede:	89 0d 24 07 00 00    	mov    %ecx,0x724
   e1ee4:	e9 04 ff ff ff       	jmp    e1ded <ram_probe+0xad>
   e1ee9:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi

000e1ef0 <rombios32_init>:
   e1ef0:	55                   	push   %ebp
   e1ef1:	89 e5                	mov    %esp,%ebp
   e1ef3:	57                   	push   %edi
   e1ef4:	56                   	push   %esi
   e1ef5:	53                   	push   %ebx
   e1ef6:	83 ec 3c             	sub    $0x3c,%esp
   e1ef9:	8b 5d 0c             	mov    0xc(%ebp),%ebx
   e1efc:	c7 44 24 04 19 2f 0e 	movl   $0xe2f19,0x4(%esp)
   e1f03:	00 
   e1f04:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e1f0b:	e8 30 f1 ff ff       	call   e1040 <bios_printf>
   e1f10:	0f b6 03             	movzbl (%ebx),%eax
   e1f13:	c7 44 24 04 2d 2f 0e 	movl   $0xe2f2d,0x4(%esp)
   e1f1a:	00 
   e1f1b:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e1f22:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1f26:	e8 15 f1 ff ff       	call   e1040 <bios_printf>
   e1f2b:	e8 10 fe ff ff       	call   e1d40 <ram_probe>
   e1f30:	e8 fb e6 ff ff       	call   e0630 <cpu_probe>
   e1f35:	e8 36 e7 ff ff       	call   e0670 <setup_mtrr>
   e1f3a:	e8 71 fd ff ff       	call   e1cb0 <smp_probe>
   e1f3f:	ba 00 00 0f 00       	mov    $0xf0000,%edx
   e1f44:	eb 0c                	jmp    e1f52 <rombios32_init+0x62>
   e1f46:	83 c2 10             	add    $0x10,%edx
   e1f49:	81 fa 00 00 10 00    	cmp    $0x100000,%edx
   e1f4f:	90                   	nop    
   e1f50:	74 38                	je     e1f8a <rombios32_init+0x9a>
   e1f52:	81 3a 42 44 fb aa    	cmpl   $0xaafb4442,(%edx)
   e1f58:	75 ec                	jne    e1f46 <rombios32_init+0x56>
   e1f5a:	8b 4a 04             	mov    0x4(%edx),%ecx
   e1f5d:	8d 42 08             	lea    0x8(%edx),%eax
   e1f60:	a3 54 07 00 00       	mov    %eax,0x754
   e1f65:	89 44 24 08          	mov    %eax,0x8(%esp)
   e1f69:	c7 44 24 04 34 30 0e 	movl   $0xe3034,0x4(%esp)
   e1f70:	00 
   e1f71:	8d 14 08             	lea    (%eax,%ecx,1),%edx
   e1f74:	89 15 38 07 00 00    	mov    %edx,0x738
   e1f7a:	89 54 24 0c          	mov    %edx,0xc(%esp)
   e1f7e:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e1f85:	e8 b6 f0 ff ff       	call   e1040 <bios_printf>
   e1f8a:	80 3b fe             	cmpb   $0xfe,(%ebx)
   e1f8d:	0f 84 f3 03 00 00    	je     e2386 <rombios32_init+0x496>
   e1f93:	e8 d8 ec ff ff       	call   e0c70 <pci_bios_init>
   e1f98:	a1 54 07 00 00       	mov    0x754,%eax
   e1f9d:	85 c0                	test   %eax,%eax
   e1f9f:	75 08                	jne    e1fa9 <rombios32_init+0xb9>
   e1fa1:	83 c4 3c             	add    $0x3c,%esp
   e1fa4:	5b                   	pop    %ebx
   e1fa5:	5e                   	pop    %esi
   e1fa6:	5f                   	pop    %edi
   e1fa7:	5d                   	pop    %ebp
   e1fa8:	c3                   	ret    
   e1fa9:	ba 10 00 00 00       	mov    $0x10,%edx
   e1fae:	e8 bd ed ff ff       	call   e0d70 <align>
   e1fb3:	8d 7d f0             	lea    -0x10(%ebp),%edi
   e1fb6:	ba 5d 2f 0e 00       	mov    $0xe2f5d,%edx
   e1fbb:	89 45 e0             	mov    %eax,-0x20(%ebp)
   e1fbe:	a3 54 07 00 00       	mov    %eax,0x754
   e1fc3:	89 45 f0             	mov    %eax,-0x10(%ebp)
   e1fc6:	89 f8                	mov    %edi,%eax
   e1fc8:	e8 03 ed ff ff       	call   e0cd0 <putstr>
   e1fcd:	31 d2                	xor    %edx,%edx
   e1fcf:	89 f8                	mov    %edi,%eax
   e1fd1:	e8 2a ed ff ff       	call   e0d00 <putle16>
   e1fd6:	ba 04 00 00 00       	mov    $0x4,%edx
   e1fdb:	89 f8                	mov    %edi,%eax
   e1fdd:	e8 de ec ff ff       	call   e0cc0 <putb>
   e1fe2:	31 d2                	xor    %edx,%edx
   e1fe4:	89 f8                	mov    %edi,%eax
   e1fe6:	e8 d5 ec ff ff       	call   e0cc0 <putb>
   e1feb:	ba 62 2f 0e 00       	mov    $0xe2f62,%edx
   e1ff0:	89 f8                	mov    %edi,%eax
   e1ff2:	e8 d9 ec ff ff       	call   e0cd0 <putstr>
   e1ff7:	ba 6b 2f 0e 00       	mov    $0xe2f6b,%edx
   e1ffc:	89 f8                	mov    %edi,%eax
   e1ffe:	e8 cd ec ff ff       	call   e0cd0 <putstr>
   e2003:	31 d2                	xor    %edx,%edx
   e2005:	89 f8                	mov    %edi,%eax
   e2007:	e8 14 ed ff ff       	call   e0d20 <putle32>
   e200c:	31 d2                	xor    %edx,%edx
   e200e:	89 f8                	mov    %edi,%eax
   e2010:	e8 eb ec ff ff       	call   e0d00 <putle16>
   e2015:	0f b7 15 3c 07 00 00 	movzwl 0x73c,%edx
   e201c:	89 f8                	mov    %edi,%eax
   e201e:	83 c2 12             	add    $0x12,%edx
   e2021:	e8 da ec ff ff       	call   e0d00 <putle16>
   e2026:	ba 00 00 e0 fe       	mov    $0xfee00000,%edx
   e202b:	89 f8                	mov    %edi,%eax
   e202d:	e8 ee ec ff ff       	call   e0d20 <putle32>
   e2032:	31 d2                	xor    %edx,%edx
   e2034:	89 f8                	mov    %edi,%eax
   e2036:	e8 c5 ec ff ff       	call   e0d00 <putle16>
   e203b:	31 d2                	xor    %edx,%edx
   e203d:	89 f8                	mov    %edi,%eax
   e203f:	e8 7c ec ff ff       	call   e0cc0 <putb>
   e2044:	31 d2                	xor    %edx,%edx
   e2046:	89 f8                	mov    %edi,%eax
   e2048:	e8 73 ec ff ff       	call   e0cc0 <putb>
   e204d:	66 83 3d 3c 07 00 00 	cmpw   $0x0,0x73c
   e2054:	00 
   e2055:	0f 84 d9 00 00 00    	je     e2134 <rombios32_init+0x244>
   e205b:	31 db                	xor    %ebx,%ebx
   e205d:	eb 5c                	jmp    e20bb <rombios32_init+0x1cb>
   e205f:	ba 03 00 00 00       	mov    $0x3,%edx
   e2064:	89 f8                	mov    %edi,%eax
   e2066:	e8 55 ec ff ff       	call   e0cc0 <putb>
   e206b:	8b 15 58 07 00 00    	mov    0x758,%edx
   e2071:	85 d2                	test   %edx,%edx
   e2073:	74 7e                	je     e20f3 <rombios32_init+0x203>
   e2075:	89 f8                	mov    %edi,%eax
   e2077:	e8 a4 ec ff ff       	call   e0d20 <putle32>
   e207c:	8b 15 30 07 00 00    	mov    0x730,%edx
   e2082:	89 f8                	mov    %edi,%eax
   e2084:	e8 97 ec ff ff       	call   e0d20 <putle32>
   e2089:	31 d2                	xor    %edx,%edx
   e208b:	89 f8                	mov    %edi,%eax
   e208d:	e8 6e ec ff ff       	call   e0d00 <putle16>
   e2092:	31 d2                	xor    %edx,%edx
   e2094:	89 f8                	mov    %edi,%eax
   e2096:	e8 65 ec ff ff       	call   e0d00 <putle16>
   e209b:	31 d2                	xor    %edx,%edx
   e209d:	89 f8                	mov    %edi,%eax
   e209f:	e8 5c ec ff ff       	call   e0d00 <putle16>
   e20a4:	31 d2                	xor    %edx,%edx
   e20a6:	89 f8                	mov    %edi,%eax
   e20a8:	e8 53 ec ff ff       	call   e0d00 <putle16>
   e20ad:	0f b7 05 3c 07 00 00 	movzwl 0x73c,%eax
   e20b4:	83 c3 01             	add    $0x1,%ebx
   e20b7:	39 d8                	cmp    %ebx,%eax
   e20b9:	7e 79                	jle    e2134 <rombios32_init+0x244>
   e20bb:	31 d2                	xor    %edx,%edx
   e20bd:	89 f8                	mov    %edi,%eax
   e20bf:	e8 fc eb ff ff       	call   e0cc0 <putb>
   e20c4:	89 da                	mov    %ebx,%edx
   e20c6:	89 f8                	mov    %edi,%eax
   e20c8:	e8 f3 eb ff ff       	call   e0cc0 <putb>
   e20cd:	ba 11 00 00 00       	mov    $0x11,%edx
   e20d2:	89 f8                	mov    %edi,%eax
   e20d4:	e8 e7 eb ff ff       	call   e0cc0 <putb>
   e20d9:	85 db                	test   %ebx,%ebx
   e20db:	74 82                	je     e205f <rombios32_init+0x16f>
   e20dd:	ba 01 00 00 00       	mov    $0x1,%edx
   e20e2:	89 f8                	mov    %edi,%eax
   e20e4:	e8 d7 eb ff ff       	call   e0cc0 <putb>
   e20e9:	8b 15 58 07 00 00    	mov    0x758,%edx
   e20ef:	85 d2                	test   %edx,%edx
   e20f1:	75 82                	jne    e2075 <rombios32_init+0x185>
   e20f3:	31 d2                	xor    %edx,%edx
   e20f5:	89 f8                	mov    %edi,%eax
   e20f7:	e8 c4 eb ff ff       	call   e0cc0 <putb>
   e20fc:	89 f8                	mov    %edi,%eax
   e20fe:	ba 06 00 00 00       	mov    $0x6,%edx
   e2103:	e8 b8 eb ff ff       	call   e0cc0 <putb>
   e2108:	31 d2                	xor    %edx,%edx
   e210a:	89 f8                	mov    %edi,%eax
   e210c:	e8 af eb ff ff       	call   e0cc0 <putb>
   e2111:	31 d2                	xor    %edx,%edx
   e2113:	89 f8                	mov    %edi,%eax
   e2115:	e8 a6 eb ff ff       	call   e0cc0 <putb>
   e211a:	89 f8                	mov    %edi,%eax
   e211c:	ba 01 02 00 00       	mov    $0x201,%edx
   e2121:	e8 da eb ff ff       	call   e0d00 <putle16>
   e2126:	31 d2                	xor    %edx,%edx
   e2128:	89 f8                	mov    %edi,%eax
   e212a:	e8 d1 eb ff ff       	call   e0d00 <putle16>
   e212f:	e9 55 ff ff ff       	jmp    e2089 <rombios32_init+0x199>
   e2134:	ba 01 00 00 00       	mov    $0x1,%edx
   e2139:	89 f8                	mov    %edi,%eax
   e213b:	e8 80 eb ff ff       	call   e0cc0 <putb>
   e2140:	31 d2                	xor    %edx,%edx
   e2142:	89 f8                	mov    %edi,%eax
   e2144:	e8 77 eb ff ff       	call   e0cc0 <putb>
   e2149:	ba 78 2f 0e 00       	mov    $0xe2f78,%edx
   e214e:	89 f8                	mov    %edi,%eax
   e2150:	e8 7b eb ff ff       	call   e0cd0 <putstr>
   e2155:	0f b7 35 3c 07 00 00 	movzwl 0x73c,%esi
   e215c:	ba 02 00 00 00       	mov    $0x2,%edx
   e2161:	89 f8                	mov    %edi,%eax
   e2163:	bb 01 00 00 00       	mov    $0x1,%ebx
   e2168:	e8 53 eb ff ff       	call   e0cc0 <putb>
   e216d:	89 f8                	mov    %edi,%eax
   e216f:	89 f2                	mov    %esi,%edx
   e2171:	e8 4a eb ff ff       	call   e0cc0 <putb>
   e2176:	ba 11 00 00 00       	mov    $0x11,%edx
   e217b:	89 f8                	mov    %edi,%eax
   e217d:	e8 3e eb ff ff       	call   e0cc0 <putb>
   e2182:	ba 01 00 00 00       	mov    $0x1,%edx
   e2187:	89 f8                	mov    %edi,%eax
   e2189:	e8 32 eb ff ff       	call   e0cc0 <putb>
   e218e:	ba 00 00 c0 fe       	mov    $0xfec00000,%edx
   e2193:	89 f8                	mov    %edi,%eax
   e2195:	e8 86 eb ff ff       	call   e0d20 <putle32>
   e219a:	ba 03 00 00 00       	mov    $0x3,%edx
   e219f:	89 f8                	mov    %edi,%eax
   e21a1:	e8 1a eb ff ff       	call   e0cc0 <putb>
   e21a6:	31 d2                	xor    %edx,%edx
   e21a8:	89 f8                	mov    %edi,%eax
   e21aa:	e8 11 eb ff ff       	call   e0cc0 <putb>
   e21af:	31 d2                	xor    %edx,%edx
   e21b1:	89 f8                	mov    %edi,%eax
   e21b3:	e8 08 eb ff ff       	call   e0cc0 <putb>
   e21b8:	31 d2                	xor    %edx,%edx
   e21ba:	89 f8                	mov    %edi,%eax
   e21bc:	e8 ff ea ff ff       	call   e0cc0 <putb>
   e21c1:	31 d2                	xor    %edx,%edx
   e21c3:	89 f8                	mov    %edi,%eax
   e21c5:	e8 f6 ea ff ff       	call   e0cc0 <putb>
   e21ca:	31 d2                	xor    %edx,%edx
   e21cc:	89 f8                	mov    %edi,%eax
   e21ce:	e8 ed ea ff ff       	call   e0cc0 <putb>
   e21d3:	89 f2                	mov    %esi,%edx
   e21d5:	89 f8                	mov    %edi,%eax
   e21d7:	e8 e4 ea ff ff       	call   e0cc0 <putb>
   e21dc:	31 d2                	xor    %edx,%edx
   e21de:	89 f8                	mov    %edi,%eax
   e21e0:	e8 db ea ff ff       	call   e0cc0 <putb>
   e21e5:	ba 03 00 00 00       	mov    $0x3,%edx
   e21ea:	89 f8                	mov    %edi,%eax
   e21ec:	e8 cf ea ff ff       	call   e0cc0 <putb>
   e21f1:	31 d2                	xor    %edx,%edx
   e21f3:	89 f8                	mov    %edi,%eax
   e21f5:	e8 c6 ea ff ff       	call   e0cc0 <putb>
   e21fa:	31 d2                	xor    %edx,%edx
   e21fc:	89 f8                	mov    %edi,%eax
   e21fe:	e8 bd ea ff ff       	call   e0cc0 <putb>
   e2203:	31 d2                	xor    %edx,%edx
   e2205:	89 f8                	mov    %edi,%eax
   e2207:	e8 b4 ea ff ff       	call   e0cc0 <putb>
   e220c:	31 d2                	xor    %edx,%edx
   e220e:	89 f8                	mov    %edi,%eax
   e2210:	e8 ab ea ff ff       	call   e0cc0 <putb>
   e2215:	89 da                	mov    %ebx,%edx
   e2217:	89 f8                	mov    %edi,%eax
   e2219:	e8 a2 ea ff ff       	call   e0cc0 <putb>
   e221e:	89 f2                	mov    %esi,%edx
   e2220:	89 f8                	mov    %edi,%eax
   e2222:	e8 99 ea ff ff       	call   e0cc0 <putb>
   e2227:	89 da                	mov    %ebx,%edx
   e2229:	89 f8                	mov    %edi,%eax
   e222b:	83 c3 01             	add    $0x1,%ebx
   e222e:	e8 8d ea ff ff       	call   e0cc0 <putb>
   e2233:	83 fb 10             	cmp    $0x10,%ebx
   e2236:	75 ad                	jne    e21e5 <rombios32_init+0x2f5>
   e2238:	8b 75 e0             	mov    -0x20(%ebp),%esi
   e223b:	8b 45 f0             	mov    -0x10(%ebp),%eax
   e223e:	2b 45 e0             	sub    -0x20(%ebp),%eax
   e2241:	88 46 04             	mov    %al,0x4(%esi)
   e2244:	c1 f8 08             	sar    $0x8,%eax
   e2247:	88 46 05             	mov    %al,0x5(%esi)
   e224a:	8b 55 f0             	mov    -0x10(%ebp),%edx
   e224d:	89 f0                	mov    %esi,%eax
   e224f:	29 f2                	sub    %esi,%edx
   e2251:	e8 ea ea ff ff       	call   e0d40 <mpf_checksum>
   e2256:	ba 10 00 00 00       	mov    $0x10,%edx
   e225b:	f7 d8                	neg    %eax
   e225d:	88 46 07             	mov    %al,0x7(%esi)
   e2260:	8b 45 f0             	mov    -0x10(%ebp),%eax
   e2263:	29 f0                	sub    %esi,%eax
   e2265:	89 45 dc             	mov    %eax,-0x24(%ebp)
   e2268:	03 05 54 07 00 00    	add    0x754,%eax
   e226e:	e8 fd ea ff ff       	call   e0d70 <align>
   e2273:	ba 7f 2f 0e 00       	mov    $0xe2f7f,%edx
   e2278:	89 c3                	mov    %eax,%ebx
   e227a:	a3 54 07 00 00       	mov    %eax,0x754
   e227f:	89 45 f0             	mov    %eax,-0x10(%ebp)
   e2282:	89 f8                	mov    %edi,%eax
   e2284:	e8 47 ea ff ff       	call   e0cd0 <putstr>
   e2289:	89 f2                	mov    %esi,%edx
   e228b:	89 f8                	mov    %edi,%eax
   e228d:	e8 8e ea ff ff       	call   e0d20 <putle32>
   e2292:	ba 01 00 00 00       	mov    $0x1,%edx
   e2297:	89 f8                	mov    %edi,%eax
   e2299:	e8 22 ea ff ff       	call   e0cc0 <putb>
   e229e:	ba 04 00 00 00       	mov    $0x4,%edx
   e22a3:	89 f8                	mov    %edi,%eax
   e22a5:	e8 16 ea ff ff       	call   e0cc0 <putb>
   e22aa:	31 d2                	xor    %edx,%edx
   e22ac:	89 f8                	mov    %edi,%eax
   e22ae:	e8 0d ea ff ff       	call   e0cc0 <putb>
   e22b3:	31 d2                	xor    %edx,%edx
   e22b5:	89 f8                	mov    %edi,%eax
   e22b7:	e8 04 ea ff ff       	call   e0cc0 <putb>
   e22bc:	31 d2                	xor    %edx,%edx
   e22be:	89 f8                	mov    %edi,%eax
   e22c0:	e8 fb e9 ff ff       	call   e0cc0 <putb>
   e22c5:	31 d2                	xor    %edx,%edx
   e22c7:	89 f8                	mov    %edi,%eax
   e22c9:	e8 f2 e9 ff ff       	call   e0cc0 <putb>
   e22ce:	31 d2                	xor    %edx,%edx
   e22d0:	89 f8                	mov    %edi,%eax
   e22d2:	e8 e9 e9 ff ff       	call   e0cc0 <putb>
   e22d7:	31 d2                	xor    %edx,%edx
   e22d9:	89 f8                	mov    %edi,%eax
   e22db:	e8 e0 e9 ff ff       	call   e0cc0 <putb>
   e22e0:	8b 55 f0             	mov    -0x10(%ebp),%edx
   e22e3:	89 d8                	mov    %ebx,%eax
   e22e5:	29 da                	sub    %ebx,%edx
   e22e7:	e8 54 ea ff ff       	call   e0d40 <mpf_checksum>
   e22ec:	f7 d8                	neg    %eax
   e22ee:	88 43 0a             	mov    %al,0xa(%ebx)
   e22f1:	8b 55 dc             	mov    -0x24(%ebp),%edx
   e22f4:	8b 45 f0             	mov    -0x10(%ebp),%eax
   e22f7:	89 5c 24 08          	mov    %ebx,0x8(%esp)
   e22fb:	89 74 24 0c          	mov    %esi,0xc(%esp)
   e22ff:	89 54 24 10          	mov    %edx,0x10(%esp)
   e2303:	29 d8                	sub    %ebx,%eax
   e2305:	c7 44 24 04 8c 30 0e 	movl   $0xe308c,0x4(%esp)
   e230c:	00 
   e230d:	01 05 54 07 00 00    	add    %eax,0x754
   e2313:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e231a:	e8 21 ed ff ff       	call   e1040 <bios_printf>
   e231f:	e8 dc e2 ff ff       	call   e0600 <uuid_probe>
   e2324:	e8 97 ed ff ff       	call   e10c0 <smbios_init>
   e2329:	8b 1d 5c 07 00 00    	mov    0x75c,%ebx
   e232f:	85 db                	test   %ebx,%ebx
   e2331:	0f 85 8e 00 00 00    	jne    e23c5 <rombios32_init+0x4d5>
   e2337:	e8 44 e7 ff ff       	call   e0a80 <bios_lock_shadow_ram>
   e233c:	a1 54 07 00 00       	mov    0x754,%eax
   e2341:	c7 44 24 04 84 2f 0e 	movl   $0xe2f84,0x4(%esp)
   e2348:	00 
   e2349:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e2350:	89 44 24 08          	mov    %eax,0x8(%esp)
   e2354:	e8 e7 ec ff ff       	call   e1040 <bios_printf>
   e2359:	a1 54 07 00 00       	mov    0x754,%eax
   e235e:	3b 05 38 07 00 00    	cmp    0x738,%eax
   e2364:	0f 86 37 fc ff ff    	jbe    e1fa1 <rombios32_init+0xb1>
   e236a:	c7 44 24 04 c4 30 0e 	movl   $0xe30c4,0x4(%esp)
   e2371:	00 
   e2372:	c7 04 24 07 00 00 00 	movl   $0x7,(%esp)
   e2379:	e8 c2 ec ff ff       	call   e1040 <bios_printf>
   e237e:	83 c4 3c             	add    $0x3c,%esp
   e2381:	5b                   	pop    %ebx
   e2382:	5e                   	pop    %esi
   e2383:	5f                   	pop    %edi
   e2384:	5d                   	pop    %ebp
   e2385:	c3                   	ret    
   e2386:	c7 04 24 40 0f 0e 00 	movl   $0xe0f40,(%esp)
   e238d:	e8 6e e8 ff ff       	call   e0c00 <pci_for_each_device>
   e2392:	e8 e9 e6 ff ff       	call   e0a80 <bios_lock_shadow_ram>
   e2397:	a1 54 07 00 00       	mov    0x754,%eax
   e239c:	85 c0                	test   %eax,%eax
   e239e:	75 2f                	jne    e23cf <rombios32_init+0x4df>
   e23a0:	8b 4d 08             	mov    0x8(%ebp),%ecx
   e23a3:	c7 01 00 00 00 00    	movl   $0x0,(%ecx)
   e23a9:	c7 44 24 04 5c 30 0e 	movl   $0xe305c,0x4(%esp)
   e23b0:	00 
   e23b1:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e23b8:	e8 83 ec ff ff       	call   e1040 <bios_printf>
   e23bd:	83 c4 3c             	add    $0x3c,%esp
   e23c0:	5b                   	pop    %ebx
   e23c1:	5e                   	pop    %esi
   e23c2:	5f                   	pop    %edi
   e23c3:	5d                   	pop    %ebp
   e23c4:	c3                   	ret    
   e23c5:	e8 a6 f2 ff ff       	call   e1670 <acpi_bios_init>
   e23ca:	e9 68 ff ff ff       	jmp    e2337 <rombios32_init+0x447>
   e23cf:	ba 10 00 00 00       	mov    $0x10,%edx
   e23d4:	e8 97 e9 ff ff       	call   e0d70 <align>
   e23d9:	8b 1d 38 07 00 00    	mov    0x738,%ebx
   e23df:	39 d8                	cmp    %ebx,%eax
   e23e1:	89 45 d8             	mov    %eax,-0x28(%ebp)
   e23e4:	72 0b                	jb     e23f1 <rombios32_init+0x501>
   e23e6:	eb 5c                	jmp    e2444 <rombios32_init+0x554>
   e23e8:	83 45 d8 10          	addl   $0x10,-0x28(%ebp)
   e23ec:	3b 5d d8             	cmp    -0x28(%ebp),%ebx
   e23ef:	76 53                	jbe    e2444 <rombios32_init+0x554>
   e23f1:	8b 75 d8             	mov    -0x28(%ebp),%esi
   e23f4:	b9 08 00 00 00       	mov    $0x8,%ecx
   e23f9:	bf 3f 2f 0e 00       	mov    $0xe2f3f,%edi
   e23fe:	fc                   	cld    
   e23ff:	f3 a6                	repz cmpsb %es:(%edi),%ds:(%esi)
   e2401:	75 e5                	jne    e23e8 <rombios32_init+0x4f8>
   e2403:	8b 55 d8             	mov    -0x28(%ebp),%edx
   e2406:	8b 4d 08             	mov    0x8(%ebp),%ecx
   e2409:	8b 42 10             	mov    0x10(%edx),%eax
   e240c:	8b 40 24             	mov    0x24(%eax),%eax
   e240f:	8b 40 24             	mov    0x24(%eax),%eax
   e2412:	8b 40 0c             	mov    0xc(%eax),%eax
   e2415:	85 c0                	test   %eax,%eax
   e2417:	89 01                	mov    %eax,(%ecx)
   e2419:	74 8e                	je     e23a9 <rombios32_init+0x4b9>
   e241b:	89 44 24 08          	mov    %eax,0x8(%esp)
   e241f:	c7 44 24 04 48 2f 0e 	movl   $0xe2f48,0x4(%esp)
   e2426:	00 
   e2427:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e242e:	e8 0d ec ff ff       	call   e1040 <bios_printf>
   e2433:	c7 04 24 b0 0f 0e 00 	movl   $0xe0fb0,(%esp)
   e243a:	e8 c1 e7 ff ff       	call   e0c00 <pci_for_each_device>
   e243f:	e9 5d fb ff ff       	jmp    e1fa1 <rombios32_init+0xb1>
   e2444:	8b 75 08             	mov    0x8(%ebp),%esi
   e2447:	c7 06 00 00 00 00    	movl   $0x0,(%esi)
   e244d:	e9 57 ff ff ff       	jmp    e23a9 <rombios32_init+0x4b9>
   e2452:	8d b4 26 00 00 00 00 	lea    0x0(%esi),%esi
   e2459:	8d bc 27 00 00 00 00 	lea    0x0(%edi),%edi

000e2460 <pci_bios_init_bridges>:
   e2460:	55                   	push   %ebp
   e2461:	31 d2                	xor    %edx,%edx
   e2463:	89 e5                	mov    %esp,%ebp
   e2465:	83 ec 38             	sub    $0x38,%esp
   e2468:	89 7d fc             	mov    %edi,-0x4(%ebp)
   e246b:	8b 7d 08             	mov    0x8(%ebp),%edi
   e246e:	89 5d f4             	mov    %ebx,-0xc(%ebp)
   e2471:	89 75 f8             	mov    %esi,-0x8(%ebp)
   e2474:	89 f8                	mov    %edi,%eax
   e2476:	e8 65 e5 ff ff       	call   e09e0 <pci_config_readw>
   e247b:	ba 02 00 00 00       	mov    $0x2,%edx
   e2480:	89 c3                	mov    %eax,%ebx
   e2482:	89 f8                	mov    %edi,%eax
   e2484:	e8 57 e5 ff ff       	call   e09e0 <pci_config_readw>
   e2489:	66 81 fb 86 80       	cmp    $0x8086,%bx
   e248e:	74 10                	je     e24a0 <pci_bios_init_bridges+0x40>
   e2490:	8b 5d f4             	mov    -0xc(%ebp),%ebx
   e2493:	8b 75 f8             	mov    -0x8(%ebp),%esi
   e2496:	8b 7d fc             	mov    -0x4(%ebp),%edi
   e2499:	89 ec                	mov    %ebp,%esp
   e249b:	5d                   	pop    %ebp
   e249c:	c3                   	ret    
   e249d:	8d 76 00             	lea    0x0(%esi),%esi
   e24a0:	66 3d 00 70          	cmp    $0x7000,%ax
   e24a4:	0f 84 9a 00 00 00    	je     e2544 <pci_bios_init_bridges+0xe4>
   e24aa:	66 3d 10 71          	cmp    $0x7110,%ax
   e24ae:	0f 84 90 00 00 00    	je     e2544 <pci_bios_init_bridges+0xe4>
   e24b4:	66 3d 37 12          	cmp    $0x1237,%ax
   e24b8:	75 d6                	jne    e2490 <pci_bios_init_bridges+0x30>
   e24ba:	8b 35 54 07 00 00    	mov    0x754,%esi
   e24c0:	85 f6                	test   %esi,%esi
   e24c2:	74 cc                	je     e2490 <pci_bios_init_bridges+0x30>
   e24c4:	ba 59 00 00 00       	mov    $0x59,%edx
   e24c9:	89 f8                	mov    %edi,%eax
   e24cb:	e8 60 e5 ff ff       	call   e0a30 <pci_config_readb>
   e24d0:	ba 59 00 00 00       	mov    $0x59,%edx
   e24d5:	89 c3                	mov    %eax,%ebx
   e24d7:	89 f8                	mov    %edi,%eax
   e24d9:	81 e3 cf 00 00 00    	and    $0xcf,%ebx
   e24df:	89 d9                	mov    %ebx,%ecx
   e24e1:	83 cb 30             	or     $0x30,%ebx
   e24e4:	e8 67 e4 ff ff       	call   e0950 <pci_config_writeb>
   e24e9:	c7 44 24 08 00 00 01 	movl   $0x10000,0x8(%esp)
   e24f0:	00 
   e24f1:	c7 44 24 04 00 00 0f 	movl   $0xf0000,0x4(%esp)
   e24f8:	00 
   e24f9:	c7 04 24 00 00 03 00 	movl   $0x30000,(%esp)
   e2500:	e8 db db ff ff       	call   e00e0 <memcpy>
   e2505:	ba 59 00 00 00       	mov    $0x59,%edx
   e250a:	89 d9                	mov    %ebx,%ecx
   e250c:	89 f8                	mov    %edi,%eax
   e250e:	e8 3d e4 ff ff       	call   e0950 <pci_config_writeb>
   e2513:	c7 44 24 08 00 00 01 	movl   $0x10000,0x8(%esp)
   e251a:	00 
   e251b:	c7 44 24 04 00 00 03 	movl   $0x30000,0x4(%esp)
   e2522:	00 
   e2523:	c7 04 24 00 00 0f 00 	movl   $0xf0000,(%esp)
   e252a:	e8 b1 db ff ff       	call   e00e0 <memcpy>
   e252f:	8b 17                	mov    (%edi),%edx
   e2531:	8b 47 04             	mov    0x4(%edi),%eax
   e2534:	89 15 14 07 00 00    	mov    %edx,0x714
   e253a:	a3 18 07 00 00       	mov    %eax,0x718
   e253f:	e9 4c ff ff ff       	jmp    e2490 <pci_bios_init_bridges+0x30>
   e2544:	c6 45 f2 00          	movb   $0x0,-0xe(%ebp)
   e2548:	be 60 00 00 00       	mov    $0x60,%esi
   e254d:	c6 45 f3 00          	movb   $0x0,-0xd(%ebp)
   e2551:	0f b6 86 0d 2e 0e 00 	movzbl 0xe2e0d(%esi),%eax
   e2558:	89 c2                	mov    %eax,%edx
   e255a:	89 c3                	mov    %eax,%ebx
   e255c:	83 e2 07             	and    $0x7,%edx
   e255f:	89 45 e0             	mov    %eax,-0x20(%ebp)
   e2562:	89 d1                	mov    %edx,%ecx
   e2564:	b8 01 00 00 00       	mov    $0x1,%eax
   e2569:	c1 fb 03             	sar    $0x3,%ebx
   e256c:	89 f2                	mov    %esi,%edx
   e256e:	d3 e0                	shl    %cl,%eax
   e2570:	83 c6 01             	add    $0x1,%esi
   e2573:	08 44 1d f2          	or     %al,-0xe(%ebp,%ebx,1)
   e2577:	89 f8                	mov    %edi,%eax
   e2579:	8b 4d e0             	mov    -0x20(%ebp),%ecx
   e257c:	e8 cf e3 ff ff       	call   e0950 <pci_config_writeb>
   e2581:	83 fe 64             	cmp    $0x64,%esi
   e2584:	75 cb                	jne    e2551 <pci_bios_init_bridges+0xf1>
   e2586:	0f b6 4d f2          	movzbl -0xe(%ebp),%ecx
   e258a:	ba d0 04 00 00       	mov    $0x4d0,%edx
   e258f:	89 c8                	mov    %ecx,%eax
   e2591:	ee                   	out    %al,(%dx)
   e2592:	0f b6 45 f3          	movzbl -0xd(%ebp),%eax
   e2596:	b2 d1                	mov    $0xd1,%dl
   e2598:	ee                   	out    %al,(%dx)
   e2599:	89 44 24 0c          	mov    %eax,0xc(%esp)
   e259d:	89 4c 24 08          	mov    %ecx,0x8(%esp)
   e25a1:	c7 44 24 04 e4 30 0e 	movl   $0xe30e4,0x4(%esp)
   e25a8:	00 
   e25a9:	c7 04 24 04 00 00 00 	movl   $0x4,(%esp)
   e25b0:	e8 8b ea ff ff       	call   e1040 <bios_printf>
   e25b5:	e9 d6 fe ff ff       	jmp    e2490 <pci_bios_init_bridges+0x30>
