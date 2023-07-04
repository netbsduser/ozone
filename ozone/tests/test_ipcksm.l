GAS LISTING test_ipcksm.s 			page 1


   1              		.file	"test_ipcksm.c"
   2              		.version	"01.01"
   3              	gcc2_compiled.:
   4              			.section	.rodata
   5              		.align 32
   6              	.LC0:
   7 0000 73746172 		.string	"start: %4.4x  nwords: %3u  old: %4u new: %4u    %3d%%\n"
   7      743A2025 
   7      342E3478 
   7      20206E77 
   7      6F726473 
   8 0037 00000000 		.align 32
   8      00000000 
   8      00
   9              	.LC1:
  10 0040 0A657272 		.string	"\nerror: seq %d: nwords %u, start %4.4x, cs1 %4.4x, cs2 %4.4x\n"
  10      6F723A20 
  10      73657120 
  10      25643A20 
  10      6E776F72 
  11              	.LC2:
  12 007e 2E00     		.string	"."
  13              	.text
  14              		.align 4
  15              	.globl main
  16              		.type	 main,@function
  17              	main:
  18 0000 55       		pushl	%ebp
  19 0001 89E5     		movl	%esp, %ebp
  20 0003 57       		pushl	%edi
  21 0004 56       		pushl	%esi
  22 0005 53       		pushl	%ebx
  23 0006 81EC4002 		subl	$576, %esp
  23      0000
  24 000c 68000200 		pushl	$512
  24      00
  25 0011 6A00     		pushl	$0
  26 0013 8D85E8FD 		leal	-536(%ebp), %eax
  26      FFFF
  27 0019 50       		pushl	%eax
  28 001a C785E4FD 		movl	$0, -540(%ebp)
  28      FFFF0000 
  28      0000
  29 0024 E8FCFFFF 		call	memset
  29      FF
  30              	.L206:
  31 0029 83C410   		addl	$16, %esp
  32              	.L196:
  33 002c 8DB5E8FD 		leal	-536(%ebp), %esi
  33      FFFF
  34 0032 31DB     		xorl	%ebx, %ebx
  35 0034 89F7     		movl	%esi, %edi
  36 0036 89F6     		.p2align 2
  37              	.L200:
  38 0038 50       		pushl	%eax
  39 0039 8D83F800 		leal	248(%ebx), %eax
  39      0000
GAS LISTING test_ipcksm.s 			page 2


  40 003f 85C0     		testl	%eax, %eax
  41 0041 89C2     		movl	%eax, %edx
  42 0043 7906     		jns	.L201
  43 0045 8D90FF00 		leal	255(%eax), %edx
  43      0000
  44              	.L201:
  45 004b 30D2     		xorb	%dl, %dl
  46 004d 29D0     		subl	%edx, %eax
  47 004f 8D0446   		leal	(%esi,%eax,2), %eax
  48 0052 50       		pushl	%eax
  49 0053 57       		pushl	%edi
  50 0054 6A10     		pushl	$16
  51 0056 83C308   		addl	$8, %ebx
  52 0059 E8FCFFFF 		call	sc_onewayhash
  52      FF
  53 005e 83C410   		addl	$16, %esp
  54 0061 83C710   		addl	$16, %edi
  55 0064 81FBFF00 		cmpl	$255, %ebx
  55      0000
  56 006a 7ECC     		jle	.L200
  57 006c 31C9     		xorl	%ecx, %ecx
  58 006e 668B75E6 		movw	-26(%ebp), %si
  59 0072 89CA     		movl	%ecx, %edx
  60 0074 BFFF0000 		movl	$255, %edi
  60      00
  61 0079 89F0     		movl	%esi, %eax
  62 007b 66F7F7   		divw	%di
  63 007e 8B5DE4   		movl	-28(%ebp), %ebx
  64 0081 89D6     		movl	%edx, %esi
  65 0083 E8FCFFFF 		call	rdtsc
  65      FF
  66 0088 57       		pushl	%edi
  67 0089 0FB7DB   		movzwl	%bx, %ebx
  68 008c 53       		pushl	%ebx
  69 008d 899DD0FD 		movl	%ebx, -560(%ebp)
  69      FFFF
  70 0093 8D9DE8FD 		leal	-536(%ebp), %ebx
  70      FFFF
  71 0099 53       		pushl	%ebx
  72 009a 0FB7F6   		movzwl	%si, %esi
  73 009d 56       		pushl	%esi
  74 009e 8995DCFD 		movl	%edx, -548(%ebp)
  74      FFFF
  75 00a4 89B5CCFD 		movl	%esi, -564(%ebp)
  75      FFFF
  76 00aa 8985D8FD 		movl	%eax, -552(%ebp)
  76      FFFF
  77 00b0 E8FCFFFF 		call	gencksm
  77      FF
  78 00b5 668985D6 		movw	%ax, -554(%ebp)
  78      FDFFFF
  79 00bc E8FCFFFF 		call	rdtsc
  79      FF
  80 00c1 83C40C   		addl	$12, %esp
  81 00c4 FFB5D0FD 		pushl	-560(%ebp)
  81      FFFF
  82 00ca 53       		pushl	%ebx
GAS LISTING test_ipcksm.s 			page 3


  83 00cb 89C6     		movl	%eax, %esi
  84 00cd FFB5CCFD 		pushl	-564(%ebp)
  84      FFFF
  85 00d3 E8FCFFFF 		call	oz_dev_ip_gencksm
  85      FF
  86 00d8 668985D4 		movw	%ax, -556(%ebp)
  86      FDFFFF
  87 00df E8FCFFFF 		call	rdtsc
  87      FF
  88 00e4 89C1     		movl	%eax, %ecx
  89 00e6 29F1     		subl	%esi, %ecx
  90 00e8 8D0489   		leal	(%ecx,%ecx,4), %eax
  91 00eb 89F3     		movl	%esi, %ebx
  92 00ed 8D0480   		leal	(%eax,%eax,4), %eax
  93 00f0 2B9DD8FD 		subl	-552(%ebp), %ebx
  93      FFFF
  94 00f6 5A       		popl	%edx
  95 00f7 C1E002   		sall	$2, %eax
  96 00fa 31D2     		xorl	%edx, %edx
  97 00fc 5E       		popl	%esi
  98 00fd F7F3     		divl	%ebx
  99 00ff 50       		pushl	%eax
 100 0100 51       		pushl	%ecx
 101 0101 53       		pushl	%ebx
 102 0102 FFB5CCFD 		pushl	-564(%ebp)
 102      FFFF
 103 0108 FFB5D0FD 		pushl	-560(%ebp)
 103      FFFF
 104 010e 68000000 		pushl	$.LC0
 104      00
 105 0113 E8FCFFFF 		call	printf
 105      FF
 106 0118 668B8DD6 		movw	-554(%ebp), %cx
 106      FDFFFF
 107 011f 83C420   		addl	$32, %esp
 108 0122 66398DD4 		cmpw	%cx, -556(%ebp)
 108      FDFFFF
 109 0129 742E     		je	.L203
 110 012b 83EC08   		subl	$8, %esp
 111 012e 0FB785D4 		movzwl	-556(%ebp), %eax
 111      FDFFFF
 112 0135 50       		pushl	%eax
 113 0136 0FB7C1   		movzwl	%cx, %eax
 114 0139 50       		pushl	%eax
 115 013a FFB5D0FD 		pushl	-560(%ebp)
 115      FFFF
 116 0140 FFB5CCFD 		pushl	-564(%ebp)
 116      FFFF
 117 0146 FFB5E4FD 		pushl	-540(%ebp)
 117      FFFF
 118 014c 68400000 		pushl	$.LC1
 118      00
 119 0151 E8FCFFFF 		call	printf
 119      FF
 120 0156 83C420   		addl	$32, %esp
 121              	.L203:
 122 0159 FF85E4FD 		incl	-540(%ebp)
GAS LISTING test_ipcksm.s 			page 4


 122      FFFF
 123 015f BA640000 		movl	$100, %edx
 123      00
 124 0164 8B85E4FD 		movl	-540(%ebp), %eax
 124      FFFF
 125 016a 89D1     		movl	%edx, %ecx
 126 016c 99       		cltd
 127 016d F7F9     		idivl	%ecx
 128 016f 85D2     		testl	%edx, %edx
 129 0171 0F85B5FE 		jne	.L196
 129      FFFF
 130 0177 83EC0C   		subl	$12, %esp
 131 017a 687E0000 		pushl	$.LC2
 131      00
 132 017f E8FCFFFF 		call	printf
 132      FF
 133 0184 58       		popl	%eax
 134 0185 FF350000 		pushl	stdout
 134      0000
 135 018b E8FCFFFF 		call	fflush
 135      FF
 136 0190 E994FEFF 		jmp	.L206
 136      FF
 137              	.Lfe1:
 138              		.size	 main,.Lfe1-main
 139 0195 8D7600   		.align 4
 140              		.type	 transform,@function
 141              	transform:
 142 0198 55       		pushl	%ebp
 143 0199 89E5     		movl	%esp, %ebp
 144 019b 57       		pushl	%edi
 145 019c 56       		pushl	%esi
 146 019d 53       		pushl	%ebx
 147 019e 81EC6C01 		subl	$364, %esp
 147      0000
 148 01a4 31DB     		xorl	%ebx, %ebx
 149 01a6 8B4D08   		movl	8(%ebp), %ecx
 150 01a9 C7859CFE 		movl	$15, -356(%ebp)
 150      FFFF0F00 
 150      0000
 151 01b3 90       		.p2align 2
 152              	.L211:
 153 01b4 0FB601   		movzbl	(%ecx), %eax
 154 01b7 41       		incl	%ecx
 155 01b8 0FB611   		movzbl	(%ecx), %edx
 156 01bb C1E008   		sall	$8, %eax
 157 01be 01D0     		addl	%edx, %eax
 158 01c0 41       		incl	%ecx
 159 01c1 0FB611   		movzbl	(%ecx), %edx
 160 01c4 C1E008   		sall	$8, %eax
 161 01c7 01D0     		addl	%edx, %eax
 162 01c9 41       		incl	%ecx
 163 01ca 0FB611   		movzbl	(%ecx), %edx
 164 01cd C1E008   		sall	$8, %eax
 165 01d0 01D0     		addl	%edx, %eax
 166 01d2 89841DA8 		movl	%eax, -344(%ebp,%ebx)
 166      FEFFFF
GAS LISTING test_ipcksm.s 			page 5


 167 01d9 41       		incl	%ecx
 168 01da 83C304   		addl	$4, %ebx
 169 01dd FF8D9CFE 		decl	-356(%ebp)
 169      FFFF
 170 01e3 79CF     		jns	.L211
 171 01e5 C7859CFE 		movl	$16, -356(%ebp)
 171      FFFF1000 
 171      0000
 172 01ef BA400000 		movl	$64, %edx
 172      00
 173              		.p2align 2
 174              	.L216:
 175 01f4 8B841588 		movl	-376(%ebp,%edx), %eax
 175      FEFFFF
 176 01fb 3384159C 		xorl	-356(%ebp,%edx), %eax
 176      FEFFFF
 177 0202 33841570 		xorl	-400(%ebp,%edx), %eax
 177      FEFFFF
 178 0209 33841568 		xorl	-408(%ebp,%edx), %eax
 178      FEFFFF
 179 0210 898415A8 		movl	%eax, -344(%ebp,%edx)
 179      FEFFFF
 180 0217 FF859CFE 		incl	-356(%ebp)
 180      FFFF
 181 021d 83C204   		addl	$4, %edx
 182 0220 83BD9CFE 		cmpl	$79, -356(%ebp)
 182      FFFF4F
 183 0227 7ECB     		jle	.L216
 184 0229 8B450C   		movl	12(%ebp), %eax
 185 022c 8B00     		movl	(%eax), %eax
 186 022e 8B550C   		movl	12(%ebp), %edx
 187 0231 8985A4FE 		movl	%eax, -348(%ebp)
 187      FFFF
 188 0237 898590FE 		movl	%eax, -368(%ebp)
 188      FFFF
 189 023d 8B5A04   		movl	4(%edx), %ebx
 190 0240 8B7208   		movl	8(%edx), %esi
 191 0243 8B7A0C   		movl	12(%edx), %edi
 192 0246 8B4210   		movl	16(%edx), %eax
 193 0249 8D95A8FE 		leal	-344(%ebp), %edx
 193      FFFF
 194 024f 8985A0FE 		movl	%eax, -352(%ebp)
 194      FFFF
 195 0255 C7859CFE 		movl	$0, -356(%ebp)
 195      FFFF0000 
 195      0000
 196 025f 899598FE 		movl	%edx, -360(%ebp)
 196      FFFF
 197 0265 8D7600   		.p2align 2
 198              	.L221:
 199 0268 89D8     		movl	%ebx, %eax
 200 026a 89DA     		movl	%ebx, %edx
 201 026c F7D0     		notl	%eax
 202 026e 21F8     		andl	%edi, %eax
 203 0270 21F2     		andl	%esi, %edx
 204 0272 8B8DA4FE 		movl	-348(%ebp), %ecx
 204      FFFF
GAS LISTING test_ipcksm.s 			page 6


 205 0278 09C2     		orl	%eax, %edx
 206 027a C1C105   		roll	$5, %ecx
 207 027d 01D1     		addl	%edx, %ecx
 208 027f 038DA0FE 		addl	-352(%ebp), %ecx
 208      FFFF
 209 0285 8B859CFE 		movl	-356(%ebp), %eax
 209      FFFF
 210 028b 8B9598FE 		movl	-360(%ebp), %edx
 210      FFFF
 211 0291 030C82   		addl	(%edx,%eax,4), %ecx
 212 0294 40       		incl	%eax
 213 0295 89BDA0FE 		movl	%edi, -352(%ebp)
 213      FFFF
 214 029b 89F7     		movl	%esi, %edi
 215 029d 89DE     		movl	%ebx, %esi
 216 029f 81C19979 		addl	$1518500249, %ecx
 216      825A
 217 02a5 C1C61E   		roll	$30, %esi
 218 02a8 83F813   		cmpl	$19, %eax
 219 02ab 8B9DA4FE 		movl	-348(%ebp), %ebx
 219      FFFF
 220 02b1 89859CFE 		movl	%eax, -356(%ebp)
 220      FFFF
 221 02b7 898DA4FE 		movl	%ecx, -348(%ebp)
 221      FFFF
 222 02bd 7EA9     		jle	.L221
 223 02bf 83F827   		cmpl	$39, %eax
 224 02c2 7F53     		jg	.L241
 225 02c4 8D8DA8FE 		leal	-344(%ebp), %ecx
 225      FFFF
 226 02ca 89F6     		.p2align 2
 227              	.L226:
 228 02cc 89D8     		movl	%ebx, %eax
 229 02ce 31F0     		xorl	%esi, %eax
 230 02d0 8B95A4FE 		movl	-348(%ebp), %edx
 230      FFFF
 231 02d6 31F8     		xorl	%edi, %eax
 232 02d8 C1C205   		roll	$5, %edx
 233 02db 01C2     		addl	%eax, %edx
 234 02dd 0395A0FE 		addl	-352(%ebp), %edx
 234      FFFF
 235 02e3 8B859CFE 		movl	-356(%ebp), %eax
 235      FFFF
 236 02e9 031481   		addl	(%ecx,%eax,4), %edx
 237 02ec 40       		incl	%eax
 238 02ed 89BDA0FE 		movl	%edi, -352(%ebp)
 238      FFFF
 239 02f3 89F7     		movl	%esi, %edi
 240 02f5 89DE     		movl	%ebx, %esi
 241 02f7 81C2A1EB 		addl	$1859775393, %edx
 241      D96E
 242 02fd C1C61E   		roll	$30, %esi
 243 0300 83F827   		cmpl	$39, %eax
 244 0303 8B9DA4FE 		movl	-348(%ebp), %ebx
 244      FFFF
 245 0309 89859CFE 		movl	%eax, -356(%ebp)
 245      FFFF
GAS LISTING test_ipcksm.s 			page 7


 246 030f 8995A4FE 		movl	%edx, -348(%ebp)
 246      FFFF
 247 0315 7EB5     		jle	.L226
 248              	.L241:
 249 0317 83BD9CFE 		cmpl	$59, -356(%ebp)
 249      FFFF3B
 250 031e 7F63     		jg	.L242
 251 0320 8D95A8FE 		leal	-344(%ebp), %edx
 251      FFFF
 252 0326 899594FE 		movl	%edx, -364(%ebp)
 252      FFFF
 253              		.p2align 2
 254              	.L231:
 255 032c 89F2     		movl	%esi, %edx
 256 032e 09FA     		orl	%edi, %edx
 257 0330 89F0     		movl	%esi, %eax
 258 0332 21F8     		andl	%edi, %eax
 259 0334 21DA     		andl	%ebx, %edx
 260 0336 8B8DA4FE 		movl	-348(%ebp), %ecx
 260      FFFF
 261 033c 09C2     		orl	%eax, %edx
 262 033e C1C105   		roll	$5, %ecx
 263 0341 01D1     		addl	%edx, %ecx
 264 0343 038DA0FE 		addl	-352(%ebp), %ecx
 264      FFFF
 265 0349 8B859CFE 		movl	-356(%ebp), %eax
 265      FFFF
 266 034f 8B9594FE 		movl	-364(%ebp), %edx
 266      FFFF
 267 0355 030C82   		addl	(%edx,%eax,4), %ecx
 268 0358 40       		incl	%eax
 269 0359 89BDA0FE 		movl	%edi, -352(%ebp)
 269      FFFF
 270 035f 89F7     		movl	%esi, %edi
 271 0361 89DE     		movl	%ebx, %esi
 272 0363 81E92443 		subl	$1894007588, %ecx
 272      E470
 273 0369 C1C61E   		roll	$30, %esi
 274 036c 83F83B   		cmpl	$59, %eax
 275 036f 8B9DA4FE 		movl	-348(%ebp), %ebx
 275      FFFF
 276 0375 89859CFE 		movl	%eax, -356(%ebp)
 276      FFFF
 277 037b 898DA4FE 		movl	%ecx, -348(%ebp)
 277      FFFF
 278 0381 7EA9     		jle	.L231
 279              	.L242:
 280 0383 83BD9CFE 		cmpl	$79, -356(%ebp)
 280      FFFF4F
 281 038a 7F53     		jg	.L243
 282 038c 8D8DA8FE 		leal	-344(%ebp), %ecx
 282      FFFF
 283 0392 89F6     		.p2align 2
 284              	.L236:
 285 0394 89D8     		movl	%ebx, %eax
 286 0396 31F0     		xorl	%esi, %eax
 287 0398 8B95A4FE 		movl	-348(%ebp), %edx
GAS LISTING test_ipcksm.s 			page 8


 287      FFFF
 288 039e 31F8     		xorl	%edi, %eax
 289 03a0 C1C205   		roll	$5, %edx
 290 03a3 01C2     		addl	%eax, %edx
 291 03a5 0395A0FE 		addl	-352(%ebp), %edx
 291      FFFF
 292 03ab 8B859CFE 		movl	-356(%ebp), %eax
 292      FFFF
 293 03b1 031481   		addl	(%ecx,%eax,4), %edx
 294 03b4 40       		incl	%eax
 295 03b5 89BDA0FE 		movl	%edi, -352(%ebp)
 295      FFFF
 296 03bb 89F7     		movl	%esi, %edi
 297 03bd 89DE     		movl	%ebx, %esi
 298 03bf 81EA2A3E 		subl	$899497514, %edx
 298      9D35
 299 03c5 C1C61E   		roll	$30, %esi
 300 03c8 83F84F   		cmpl	$79, %eax
 301 03cb 8B9DA4FE 		movl	-348(%ebp), %ebx
 301      FFFF
 302 03d1 89859CFE 		movl	%eax, -356(%ebp)
 302      FFFF
 303 03d7 8995A4FE 		movl	%edx, -348(%ebp)
 303      FFFF
 304 03dd 7EB5     		jle	.L236
 305              	.L243:
 306 03df 8B8590FE 		movl	-368(%ebp), %eax
 306      FFFF
 307 03e5 0385A4FE 		addl	-348(%ebp), %eax
 307      FFFF
 308 03eb 8B550C   		movl	12(%ebp), %edx
 309 03ee 8902     		movl	%eax, (%edx)
 310 03f0 015A04   		addl	%ebx, 4(%edx)
 311 03f3 017208   		addl	%esi, 8(%edx)
 312 03f6 017A0C   		addl	%edi, 12(%edx)
 313 03f9 8B85A0FE 		movl	-352(%ebp), %eax
 313      FFFF
 314 03ff 014210   		addl	%eax, 16(%edx)
 315 0402 81C46C01 		addl	$364, %esp
 315      0000
 316 0408 5B       		popl	%ebx
 317 0409 5E       		popl	%esi
 318 040a 5F       		popl	%edi
 319 040b 5D       		popl	%ebp
 320 040c C3       		ret
 321              	.Lfe2:
 322              		.size	 transform,.Lfe2-transform
 323 040d 8D7600   		.align 4
 324              	.globl sc_owh_init
 325              		.type	 sc_owh_init,@function
 326              	sc_owh_init:
 327 0410 55       		pushl	%ebp
 328 0411 89E5     		movl	%esp, %ebp
 329 0413 83EC14   		subl	$20, %esp
 330 0416 6A5C     		pushl	$92
 331 0418 E8FCFFFF 		call	malloc
 331      FF
GAS LISTING test_ipcksm.s 			page 9


 332 041d C7000123 		movl	$1732584193, (%eax)
 332      4567
 333 0423 C7400489 		movl	$-271733879, 4(%eax)
 333      ABCDEF
 334 042a C74008FE 		movl	$-1732584194, 8(%eax)
 334      DCBA98
 335 0431 C7400C76 		movl	$271733878, 12(%eax)
 335      543210
 336 0438 C74010F0 		movl	$-1009589776, 16(%eax)
 336      E1D2C3
 337 043f C7401800 		movl	$0, 24(%eax)
 337      000000
 338 0446 C7401400 		movl	$0, 20(%eax)
 338      000000
 339 044d C9       		leave
 340 044e C3       		ret
 341              	.Lfe3:
 342              		.size	 sc_owh_init,.Lfe3-sc_owh_init
 343 044f 90       		.align 4
 344              	.globl sc_owh_data
 345              		.type	 sc_owh_data,@function
 346              	sc_owh_data:
 347 0450 55       		pushl	%ebp
 348 0451 89E5     		movl	%esp, %ebp
 349 0453 57       		pushl	%edi
 350 0454 56       		pushl	%esi
 351 0455 53       		pushl	%ebx
 352 0456 83EC0C   		subl	$12, %esp
 353 0459 8B4508   		movl	8(%ebp), %eax
 354 045c 8945F0   		movl	%eax, -16(%ebp)
 355 045f 89C3     		movl	%eax, %ebx
 356 0461 8B750C   		movl	12(%ebp), %esi
 357 0464 8B5318   		movl	24(%ebx), %edx
 358 0467 8D0416   		leal	(%esi,%edx), %eax
 359 046a 017314   		addl	%esi, 20(%ebx)
 360 046d 83F83F   		cmpl	$63, %eax
 361 0470 8B7D10   		movl	16(%ebp), %edi
 362 0473 771B     		ja	.L246
 363 0475 51       		pushl	%ecx
 364 0476 56       		pushl	%esi
 365 0477 57       		pushl	%edi
 366 0478 8D441A1C 		leal	28(%edx,%ebx), %eax
 367 047c 50       		pushl	%eax
 368 047d E8FCFFFF 		call	memcpy
 368      FF
 369 0482 017318   		addl	%esi, 24(%ebx)
 370 0485 8D65F4   		leal	-12(%ebp), %esp
 371 0488 5B       		popl	%ebx
 372 0489 5E       		popl	%esi
 373 048a 5F       		popl	%edi
 374 048b 5D       		popl	%ebp
 375 048c C3       		ret
 376 048d 8D7600   		.p2align 2
 377              	.L246:
 378 0490 85D2     		testl	%edx, %edx
 379 0492 7445     		je	.L253
 380 0494 C745EC40 		movl	$64, -20(%ebp)
GAS LISTING test_ipcksm.s 			page 10


 380      000000
 381 049b 2955EC   		subl	%edx, -20(%ebp)
 382 049e 8D441A1C 		leal	28(%edx,%ebx), %eax
 383 04a2 52       		pushl	%edx
 384 04a3 FF75EC   		pushl	-20(%ebp)
 385 04a6 57       		pushl	%edi
 386 04a7 50       		pushl	%eax
 387 04a8 E8FCFFFF 		call	memcpy
 387      FF
 388 04ad 5A       		popl	%edx
 389 04ae 59       		popl	%ecx
 390 04af 53       		pushl	%ebx
 391 04b0 8D431C   		leal	28(%ebx), %eax
 392 04b3 50       		pushl	%eax
 393 04b4 E8DFFCFF 		call	transform
 393      FF
 394 04b9 83C410   		addl	$16, %esp
 395 04bc 2B75EC   		subl	-20(%ebp), %esi
 396 04bf 037DEC   		addl	-20(%ebp), %edi
 397 04c2 EB15     		jmp	.L253
 398              		.p2align 2
 399              	.L250:
 400 04c4 83EC08   		subl	$8, %esp
 401 04c7 FF75F0   		pushl	-16(%ebp)
 402 04ca 57       		pushl	%edi
 403 04cb E8C8FCFF 		call	transform
 403      FF
 404 04d0 83EE40   		subl	$64, %esi
 405 04d3 83C740   		addl	$64, %edi
 406 04d6 83C410   		addl	$16, %esp
 407              	.L253:
 408 04d9 83FE3F   		cmpl	$63, %esi
 409 04dc 77E6     		ja	.L250
 410 04de 8B45F0   		movl	-16(%ebp), %eax
 411 04e1 897018   		movl	%esi, 24(%eax)
 412 04e4 8D431C   		leal	28(%ebx), %eax
 413 04e7 897510   		movl	%esi, 16(%ebp)
 414 04ea 897D0C   		movl	%edi, 12(%ebp)
 415 04ed 894508   		movl	%eax, 8(%ebp)
 416 04f0 8D65F4   		leal	-12(%ebp), %esp
 417 04f3 5B       		popl	%ebx
 418 04f4 5E       		popl	%esi
 419 04f5 5F       		popl	%edi
 420 04f6 5D       		popl	%ebp
 421 04f7 E9FCFFFF 		jmp	memcpy
 421      FF
 422              	.Lfe4:
 423              		.size	 sc_owh_data,.Lfe4-sc_owh_data
 424              		.align 4
 425              	.globl sc_owh_term
 426              		.type	 sc_owh_term,@function
 427              	sc_owh_term:
 428 04fc 55       		pushl	%ebp
 429 04fd 89E5     		movl	%esp, %ebp
 430 04ff 56       		pushl	%esi
 431 0500 53       		pushl	%ebx
 432 0501 8B5D08   		movl	8(%ebp), %ebx
GAS LISTING test_ipcksm.s 			page 11


 433 0504 8B4B18   		movl	24(%ebx), %ecx
 434 0507 8B750C   		movl	12(%ebp), %esi
 435 050a C644191C 		movb	$-128, 28(%ecx,%ebx)
 435      80
 436 050f 41       		incl	%ecx
 437 0510 83F93C   		cmpl	$60, %ecx
 438 0513 7626     		jbe	.L255
 439 0515 BA400000 		movl	$64, %edx
 439      00
 440 051a 29CA     		subl	%ecx, %edx
 441 051c 8D44191C 		leal	28(%ecx,%ebx), %eax
 442 0520 51       		pushl	%ecx
 443 0521 52       		pushl	%edx
 444 0522 6A00     		pushl	$0
 445 0524 50       		pushl	%eax
 446 0525 E8FCFFFF 		call	memset
 446      FF
 447 052a 58       		popl	%eax
 448 052b 5A       		popl	%edx
 449 052c 53       		pushl	%ebx
 450 052d 8D431C   		leal	28(%ebx), %eax
 451 0530 50       		pushl	%eax
 452 0531 E862FCFF 		call	transform
 452      FF
 453 0536 83C410   		addl	$16, %esp
 454 0539 31C9     		xorl	%ecx, %ecx
 455              	.L255:
 456 053b B83C0000 		movl	$60, %eax
 456      00
 457 0540 8D54191C 		leal	28(%ecx,%ebx), %edx
 458 0544 29C8     		subl	%ecx, %eax
 459 0546 51       		pushl	%ecx
 460 0547 50       		pushl	%eax
 461 0548 6A00     		pushl	$0
 462 054a 52       		pushl	%edx
 463 054b E8FCFFFF 		call	memset
 463      FF
 464 0550 58       		popl	%eax
 465 0551 5A       		popl	%edx
 466 0552 8B5314   		movl	20(%ebx), %edx
 467 0555 89D0     		movl	%edx, %eax
 468 0557 C1E818   		shrl	$24, %eax
 469 055a 884358   		movb	%al, 88(%ebx)
 470 055d 89D0     		movl	%edx, %eax
 471 055f C1E810   		shrl	$16, %eax
 472 0562 884359   		movb	%al, 89(%ebx)
 473 0565 C1EA08   		shrl	$8, %edx
 474 0568 8A4314   		movb	20(%ebx), %al
 475 056b 88535A   		movb	%dl, 90(%ebx)
 476 056e 88435B   		movb	%al, 91(%ebx)
 477 0571 53       		pushl	%ebx
 478 0572 8D431C   		leal	28(%ebx), %eax
 479 0575 50       		pushl	%eax
 480 0576 E81DFCFF 		call	transform
 480      FF
 481 057b 8B4B10   		movl	16(%ebx), %ecx
 482 057e 89C8     		movl	%ecx, %eax
GAS LISTING test_ipcksm.s 			page 12


 483 0580 25997982 		andl	$1518500249, %eax
 483      5A
 484 0585 0103     		addl	%eax, (%ebx)
 485 0587 89C8     		movl	%ecx, %eax
 486 0589 25A1EBD9 		andl	$1859775393, %eax
 486      6E
 487 058e 014304   		addl	%eax, 4(%ebx)
 488 0591 89C8     		movl	%ecx, %eax
 489 0593 25DCBC1B 		andl	$-1894007588, %eax
 489      8F
 490 0598 81E1D6C1 		andl	$-899497514, %ecx
 490      62CA
 491 059e 014B0C   		addl	%ecx, 12(%ebx)
 492 05a1 89F2     		movl	%esi, %edx
 493 05a3 014308   		addl	%eax, 8(%ebx)
 494 05a6 31C9     		xorl	%ecx, %ecx
 495 05a8 83C410   		addl	$16, %esp
 496 05ab 31F6     		xorl	%esi, %esi
 497 05ad 8D7600   		.p2align 2
 498              	.L311:
 499 05b0 0FB64433 		movzbl	3(%ebx,%esi), %eax
 499      03
 500 05b5 8802     		movb	%al, (%edx)
 501 05b7 0FB74433 		movzwl	2(%ebx,%esi), %eax
 501      02
 502 05bc 42       		incl	%edx
 503 05bd 8802     		movb	%al, (%edx)
 504 05bf 8B0433   		movl	(%ebx,%esi), %eax
 505 05c2 42       		incl	%edx
 506 05c3 C1E808   		shrl	$8, %eax
 507 05c6 8802     		movb	%al, (%edx)
 508 05c8 8A0433   		movb	(%ebx,%esi), %al
 509 05cb 42       		incl	%edx
 510 05cc 41       		incl	%ecx
 511 05cd 8802     		movb	%al, (%edx)
 512 05cf 83C604   		addl	$4, %esi
 513 05d2 42       		incl	%edx
 514 05d3 83F903   		cmpl	$3, %ecx
 515 05d6 76D8     		jbe	.L311
 516 05d8 895D08   		movl	%ebx, 8(%ebp)
 517 05db 8D65F8   		leal	-8(%ebp), %esp
 518 05de 5B       		popl	%ebx
 519 05df 5E       		popl	%esi
 520 05e0 5D       		popl	%ebp
 521 05e1 E9FCFFFF 		jmp	free
 521      FF
 522              	.Lfe5:
 523              		.size	 sc_owh_term,.Lfe5-sc_owh_term
 524 05e6 89F6     		.align 4
 525              	.globl sc_onewayhash
 526              		.type	 sc_onewayhash,@function
 527              	sc_onewayhash:
 528 05e8 55       		pushl	%ebp
 529 05e9 89E5     		movl	%esp, %ebp
 530 05eb 57       		pushl	%edi
 531 05ec 56       		pushl	%esi
 532 05ed 53       		pushl	%ebx
GAS LISTING test_ipcksm.s 			page 13


 533 05ee 83EC0C   		subl	$12, %esp
 534 05f1 8B4510   		movl	16(%ebp), %eax
 535 05f4 8945F0   		movl	%eax, -16(%ebp)
 536 05f7 8B5D08   		movl	8(%ebp), %ebx
 537 05fa 8B750C   		movl	12(%ebp), %esi
 538 05fd E8FCFFFF 		call	sc_owh_init
 538      FF
 539 0602 89C7     		movl	%eax, %edi
 540 0604 50       		pushl	%eax
 541 0605 56       		pushl	%esi
 542 0606 53       		pushl	%ebx
 543 0607 57       		pushl	%edi
 544 0608 E8FCFFFF 		call	sc_owh_data
 544      FF
 545 060d 8B45F0   		movl	-16(%ebp), %eax
 546 0610 89450C   		movl	%eax, 12(%ebp)
 547 0613 897D08   		movl	%edi, 8(%ebp)
 548 0616 83C410   		addl	$16, %esp
 549 0619 8D65F4   		leal	-12(%ebp), %esp
 550 061c 5B       		popl	%ebx
 551 061d 5E       		popl	%esi
 552 061e 5F       		popl	%edi
 553 061f 5D       		popl	%ebp
 554 0620 E9FCFFFF 		jmp	sc_owh_term
 554      FF
 555              	.Lfe6:
 556              		.size	 sc_onewayhash,.Lfe6-sc_onewayhash
 557 0625 8D7600   		.align 4
 558              	.globl gencksm
 559              		.type	 gencksm,@function
 560              	gencksm:
 561 0628 55       		pushl	%ebp
 562 0629 89E5     		movl	%esp, %ebp
 563 062b 0FB74510 		movzwl	16(%ebp), %eax
 564 062f 53       		pushl	%ebx
 565 0630 F7D0     		notl	%eax
 566 0632 8B5D08   		movl	8(%ebp), %ebx
 567 0635 0FB7D0   		movzwl	%ax,%edx
 568 0638 85DB     		testl	%ebx, %ebx
 569 063a 8B4D0C   		movl	12(%ebp), %ecx
 570 063d 741B     		je	.L328
 571 063f 90       		.p2align 2
 572              	.L319:
 573 0640 0FB601   		movzbl	(%ecx), %eax
 574 0643 C1E008   		sall	$8, %eax
 575 0646 41       		incl	%ecx
 576 0647 01C2     		addl	%eax, %edx
 577 0649 0FB601   		movzbl	(%ecx), %eax
 578 064c 01C2     		addl	%eax, %edx
 579 064e 41       		incl	%ecx
 580 064f 4B       		decl	%ebx
 581 0650 75EE     		jne	.L319
 582 0652 EB06     		jmp	.L328
 583              		.p2align 2
 584              	.L323:
 585 0654 0FB7C2   		movzwl	%dx,%eax
 586 0657 8D1403   		leal	(%ebx,%eax), %edx
GAS LISTING test_ipcksm.s 			page 14


 587              	.L328:
 588 065a 89D3     		movl	%edx, %ebx
 589 065c C1EB10   		shrl	$16, %ebx
 590 065f 75F3     		jne	.L323
 591 0661 89D0     		movl	%edx, %eax
 592 0663 F7D0     		notl	%eax
 593 0665 89C2     		movl	%eax, %edx
 594 0667 81E2FFFF 		andl	$65535, %edx
 594      0000
 595 066d 7505     		jne	.L325
 596 066f BAFFFF00 		movl	$65535, %edx
 596      00
 597              	.L325:
 598 0674 0FB7C2   		movzwl	%dx, %eax
 599 0677 8B1C24   		movl	(%esp), %ebx
 600 067a C9       		leave
 601 067b C3       		ret
 602              	.Lfe7:
 603              		.size	 gencksm,.Lfe7-gencksm
 604              		.ident	"GCC: (GNU) 2.96 20000731 (Red Hat Linux 7.0)"
GAS LISTING test_ipcksm.s 			page 15


DEFINED SYMBOLS
                            *ABS*:00000000 test_ipcksm.c
       test_ipcksm.s:3      .text:00000000 gcc2_compiled.
       test_ipcksm.s:17     .text:00000000 main
       test_ipcksm.s:527    .text:000005e8 sc_onewayhash
       test_ipcksm.s:560    .text:00000628 gencksm
       test_ipcksm.s:141    .text:00000198 transform
       test_ipcksm.s:326    .text:00000410 sc_owh_init
       test_ipcksm.s:346    .text:00000450 sc_owh_data
       test_ipcksm.s:427    .text:000004fc sc_owh_term

UNDEFINED SYMBOLS
memset
rdtsc
oz_dev_ip_gencksm
printf
stdout
fflush
malloc
memcpy
free
