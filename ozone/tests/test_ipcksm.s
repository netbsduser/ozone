	.file	"test_ipcksm.c"
	.version	"01.01"
gcc2_compiled.:
		.section	.rodata
	.align 32
.LC0:
	.string	"start: %4.4x  nwords: %3u  old: %4u new: %4u    %3d%%\n"
	.align 32
.LC1:
	.string	"\nerror: seq %d: nwords %u, start %4.4x, cs1 %4.4x, cs2 %4.4x\n"
.LC2:
	.string	"."
.text
	.align 4
.globl main
	.type	 main,@function
main:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$576, %esp
	pushl	$512
	pushl	$0
	leal	-536(%ebp), %eax
	pushl	%eax
	movl	$0, -540(%ebp)
	call	memset
.L206:
	addl	$16, %esp
.L196:
	leal	-536(%ebp), %esi
	xorl	%ebx, %ebx
	movl	%esi, %edi
	.p2align 2
.L200:
	pushl	%eax
	leal	248(%ebx), %eax
	testl	%eax, %eax
	movl	%eax, %edx
	jns	.L201
	leal	255(%eax), %edx
.L201:
	xorb	%dl, %dl
	subl	%edx, %eax
	leal	(%esi,%eax,2), %eax
	pushl	%eax
	pushl	%edi
	pushl	$16
	addl	$8, %ebx
	call	sc_onewayhash
	addl	$16, %esp
	addl	$16, %edi
	cmpl	$255, %ebx
	jle	.L200
	xorl	%ecx, %ecx
	movw	-26(%ebp), %si
	movl	%ecx, %edx
	movl	$255, %edi
	movl	%esi, %eax
	divw	%di
	movl	-28(%ebp), %ebx
	movl	%edx, %esi
	call	rdtsc
	pushl	%edi
	movzwl	%bx, %ebx
	pushl	%ebx
	movl	%ebx, -560(%ebp)
	leal	-536(%ebp), %ebx
	pushl	%ebx
	movzwl	%si, %esi
	pushl	%esi
	movl	%edx, -548(%ebp)
	movl	%esi, -564(%ebp)
	movl	%eax, -552(%ebp)
	call	gencksm
	movw	%ax, -554(%ebp)
	call	rdtsc
	addl	$12, %esp
	pushl	-560(%ebp)
	pushl	%ebx
	movl	%eax, %esi
	pushl	-564(%ebp)
	call	oz_dev_ip_gencksm
	movw	%ax, -556(%ebp)
	call	rdtsc
	movl	%eax, %ecx
	subl	%esi, %ecx
	leal	(%ecx,%ecx,4), %eax
	movl	%esi, %ebx
	leal	(%eax,%eax,4), %eax
	subl	-552(%ebp), %ebx
	popl	%edx
	sall	$2, %eax
	xorl	%edx, %edx
	popl	%esi
	divl	%ebx
	pushl	%eax
	pushl	%ecx
	pushl	%ebx
	pushl	-564(%ebp)
	pushl	-560(%ebp)
	pushl	$.LC0
	call	printf
	movw	-554(%ebp), %cx
	addl	$32, %esp
	cmpw	%cx, -556(%ebp)
	je	.L203
	subl	$8, %esp
	movzwl	-556(%ebp), %eax
	pushl	%eax
	movzwl	%cx, %eax
	pushl	%eax
	pushl	-560(%ebp)
	pushl	-564(%ebp)
	pushl	-540(%ebp)
	pushl	$.LC1
	call	printf
	addl	$32, %esp
.L203:
	incl	-540(%ebp)
	movl	$100, %edx
	movl	-540(%ebp), %eax
	movl	%edx, %ecx
	cltd
	idivl	%ecx
	testl	%edx, %edx
	jne	.L196
	subl	$12, %esp
	pushl	$.LC2
	call	printf
	popl	%eax
	pushl	stdout
	call	fflush
	jmp	.L206
.Lfe1:
	.size	 main,.Lfe1-main
	.align 4
	.type	 transform,@function
transform:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$364, %esp
	xorl	%ebx, %ebx
	movl	8(%ebp), %ecx
	movl	$15, -356(%ebp)
	.p2align 2
.L211:
	movzbl	(%ecx), %eax
	incl	%ecx
	movzbl	(%ecx), %edx
	sall	$8, %eax
	addl	%edx, %eax
	incl	%ecx
	movzbl	(%ecx), %edx
	sall	$8, %eax
	addl	%edx, %eax
	incl	%ecx
	movzbl	(%ecx), %edx
	sall	$8, %eax
	addl	%edx, %eax
	movl	%eax, -344(%ebp,%ebx)
	incl	%ecx
	addl	$4, %ebx
	decl	-356(%ebp)
	jns	.L211
	movl	$16, -356(%ebp)
	movl	$64, %edx
	.p2align 2
.L216:
	movl	-376(%ebp,%edx), %eax
	xorl	-356(%ebp,%edx), %eax
	xorl	-400(%ebp,%edx), %eax
	xorl	-408(%ebp,%edx), %eax
	movl	%eax, -344(%ebp,%edx)
	incl	-356(%ebp)
	addl	$4, %edx
	cmpl	$79, -356(%ebp)
	jle	.L216
	movl	12(%ebp), %eax
	movl	(%eax), %eax
	movl	12(%ebp), %edx
	movl	%eax, -348(%ebp)
	movl	%eax, -368(%ebp)
	movl	4(%edx), %ebx
	movl	8(%edx), %esi
	movl	12(%edx), %edi
	movl	16(%edx), %eax
	leal	-344(%ebp), %edx
	movl	%eax, -352(%ebp)
	movl	$0, -356(%ebp)
	movl	%edx, -360(%ebp)
	.p2align 2
.L221:
	movl	%ebx, %eax
	movl	%ebx, %edx
	notl	%eax
	andl	%edi, %eax
	andl	%esi, %edx
	movl	-348(%ebp), %ecx
	orl	%eax, %edx
	roll	$5, %ecx
	addl	%edx, %ecx
	addl	-352(%ebp), %ecx
	movl	-356(%ebp), %eax
	movl	-360(%ebp), %edx
	addl	(%edx,%eax,4), %ecx
	incl	%eax
	movl	%edi, -352(%ebp)
	movl	%esi, %edi
	movl	%ebx, %esi
	addl	$1518500249, %ecx
	roll	$30, %esi
	cmpl	$19, %eax
	movl	-348(%ebp), %ebx
	movl	%eax, -356(%ebp)
	movl	%ecx, -348(%ebp)
	jle	.L221
	cmpl	$39, %eax
	jg	.L241
	leal	-344(%ebp), %ecx
	.p2align 2
.L226:
	movl	%ebx, %eax
	xorl	%esi, %eax
	movl	-348(%ebp), %edx
	xorl	%edi, %eax
	roll	$5, %edx
	addl	%eax, %edx
	addl	-352(%ebp), %edx
	movl	-356(%ebp), %eax
	addl	(%ecx,%eax,4), %edx
	incl	%eax
	movl	%edi, -352(%ebp)
	movl	%esi, %edi
	movl	%ebx, %esi
	addl	$1859775393, %edx
	roll	$30, %esi
	cmpl	$39, %eax
	movl	-348(%ebp), %ebx
	movl	%eax, -356(%ebp)
	movl	%edx, -348(%ebp)
	jle	.L226
.L241:
	cmpl	$59, -356(%ebp)
	jg	.L242
	leal	-344(%ebp), %edx
	movl	%edx, -364(%ebp)
	.p2align 2
.L231:
	movl	%esi, %edx
	orl	%edi, %edx
	movl	%esi, %eax
	andl	%edi, %eax
	andl	%ebx, %edx
	movl	-348(%ebp), %ecx
	orl	%eax, %edx
	roll	$5, %ecx
	addl	%edx, %ecx
	addl	-352(%ebp), %ecx
	movl	-356(%ebp), %eax
	movl	-364(%ebp), %edx
	addl	(%edx,%eax,4), %ecx
	incl	%eax
	movl	%edi, -352(%ebp)
	movl	%esi, %edi
	movl	%ebx, %esi
	subl	$1894007588, %ecx
	roll	$30, %esi
	cmpl	$59, %eax
	movl	-348(%ebp), %ebx
	movl	%eax, -356(%ebp)
	movl	%ecx, -348(%ebp)
	jle	.L231
.L242:
	cmpl	$79, -356(%ebp)
	jg	.L243
	leal	-344(%ebp), %ecx
	.p2align 2
.L236:
	movl	%ebx, %eax
	xorl	%esi, %eax
	movl	-348(%ebp), %edx
	xorl	%edi, %eax
	roll	$5, %edx
	addl	%eax, %edx
	addl	-352(%ebp), %edx
	movl	-356(%ebp), %eax
	addl	(%ecx,%eax,4), %edx
	incl	%eax
	movl	%edi, -352(%ebp)
	movl	%esi, %edi
	movl	%ebx, %esi
	subl	$899497514, %edx
	roll	$30, %esi
	cmpl	$79, %eax
	movl	-348(%ebp), %ebx
	movl	%eax, -356(%ebp)
	movl	%edx, -348(%ebp)
	jle	.L236
.L243:
	movl	-368(%ebp), %eax
	addl	-348(%ebp), %eax
	movl	12(%ebp), %edx
	movl	%eax, (%edx)
	addl	%ebx, 4(%edx)
	addl	%esi, 8(%edx)
	addl	%edi, 12(%edx)
	movl	-352(%ebp), %eax
	addl	%eax, 16(%edx)
	addl	$364, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.Lfe2:
	.size	 transform,.Lfe2-transform
	.align 4
.globl sc_owh_init
	.type	 sc_owh_init,@function
sc_owh_init:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$20, %esp
	pushl	$92
	call	malloc
	movl	$1732584193, (%eax)
	movl	$-271733879, 4(%eax)
	movl	$-1732584194, 8(%eax)
	movl	$271733878, 12(%eax)
	movl	$-1009589776, 16(%eax)
	movl	$0, 24(%eax)
	movl	$0, 20(%eax)
	leave
	ret
.Lfe3:
	.size	 sc_owh_init,.Lfe3-sc_owh_init
	.align 4
.globl sc_owh_data
	.type	 sc_owh_data,@function
sc_owh_data:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %eax
	movl	%eax, -16(%ebp)
	movl	%eax, %ebx
	movl	12(%ebp), %esi
	movl	24(%ebx), %edx
	leal	(%esi,%edx), %eax
	addl	%esi, 20(%ebx)
	cmpl	$63, %eax
	movl	16(%ebp), %edi
	ja	.L246
	pushl	%ecx
	pushl	%esi
	pushl	%edi
	leal	28(%edx,%ebx), %eax
	pushl	%eax
	call	memcpy
	addl	%esi, 24(%ebx)
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 2
.L246:
	testl	%edx, %edx
	je	.L253
	movl	$64, -20(%ebp)
	subl	%edx, -20(%ebp)
	leal	28(%edx,%ebx), %eax
	pushl	%edx
	pushl	-20(%ebp)
	pushl	%edi
	pushl	%eax
	call	memcpy
	popl	%edx
	popl	%ecx
	pushl	%ebx
	leal	28(%ebx), %eax
	pushl	%eax
	call	transform
	addl	$16, %esp
	subl	-20(%ebp), %esi
	addl	-20(%ebp), %edi
	jmp	.L253
	.p2align 2
.L250:
	subl	$8, %esp
	pushl	-16(%ebp)
	pushl	%edi
	call	transform
	subl	$64, %esi
	addl	$64, %edi
	addl	$16, %esp
.L253:
	cmpl	$63, %esi
	ja	.L250
	movl	-16(%ebp), %eax
	movl	%esi, 24(%eax)
	leal	28(%ebx), %eax
	movl	%esi, 16(%ebp)
	movl	%edi, 12(%ebp)
	movl	%eax, 8(%ebp)
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	jmp	memcpy
.Lfe4:
	.size	 sc_owh_data,.Lfe4-sc_owh_data
	.align 4
.globl sc_owh_term
	.type	 sc_owh_term,@function
sc_owh_term:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	movl	24(%ebx), %ecx
	movl	12(%ebp), %esi
	movb	$-128, 28(%ecx,%ebx)
	incl	%ecx
	cmpl	$60, %ecx
	jbe	.L255
	movl	$64, %edx
	subl	%ecx, %edx
	leal	28(%ecx,%ebx), %eax
	pushl	%ecx
	pushl	%edx
	pushl	$0
	pushl	%eax
	call	memset
	popl	%eax
	popl	%edx
	pushl	%ebx
	leal	28(%ebx), %eax
	pushl	%eax
	call	transform
	addl	$16, %esp
	xorl	%ecx, %ecx
.L255:
	movl	$60, %eax
	leal	28(%ecx,%ebx), %edx
	subl	%ecx, %eax
	pushl	%ecx
	pushl	%eax
	pushl	$0
	pushl	%edx
	call	memset
	popl	%eax
	popl	%edx
	movl	20(%ebx), %edx
	movl	%edx, %eax
	shrl	$24, %eax
	movb	%al, 88(%ebx)
	movl	%edx, %eax
	shrl	$16, %eax
	movb	%al, 89(%ebx)
	shrl	$8, %edx
	movb	20(%ebx), %al
	movb	%dl, 90(%ebx)
	movb	%al, 91(%ebx)
	pushl	%ebx
	leal	28(%ebx), %eax
	pushl	%eax
	call	transform
	movl	16(%ebx), %ecx
	movl	%ecx, %eax
	andl	$1518500249, %eax
	addl	%eax, (%ebx)
	movl	%ecx, %eax
	andl	$1859775393, %eax
	addl	%eax, 4(%ebx)
	movl	%ecx, %eax
	andl	$-1894007588, %eax
	andl	$-899497514, %ecx
	addl	%ecx, 12(%ebx)
	movl	%esi, %edx
	addl	%eax, 8(%ebx)
	xorl	%ecx, %ecx
	addl	$16, %esp
	xorl	%esi, %esi
	.p2align 2
.L311:
	movzbl	3(%ebx,%esi), %eax
	movb	%al, (%edx)
	movzwl	2(%ebx,%esi), %eax
	incl	%edx
	movb	%al, (%edx)
	movl	(%ebx,%esi), %eax
	incl	%edx
	shrl	$8, %eax
	movb	%al, (%edx)
	movb	(%ebx,%esi), %al
	incl	%edx
	incl	%ecx
	movb	%al, (%edx)
	addl	$4, %esi
	incl	%edx
	cmpl	$3, %ecx
	jbe	.L311
	movl	%ebx, 8(%ebp)
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	jmp	free
.Lfe5:
	.size	 sc_owh_term,.Lfe5-sc_owh_term
	.align 4
.globl sc_onewayhash
	.type	 sc_onewayhash,@function
sc_onewayhash:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	16(%ebp), %eax
	movl	%eax, -16(%ebp)
	movl	8(%ebp), %ebx
	movl	12(%ebp), %esi
	call	sc_owh_init
	movl	%eax, %edi
	pushl	%eax
	pushl	%esi
	pushl	%ebx
	pushl	%edi
	call	sc_owh_data
	movl	-16(%ebp), %eax
	movl	%eax, 12(%ebp)
	movl	%edi, 8(%ebp)
	addl	$16, %esp
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	jmp	sc_owh_term
.Lfe6:
	.size	 sc_onewayhash,.Lfe6-sc_onewayhash
	.align 4
.globl gencksm
	.type	 gencksm,@function
gencksm:
	pushl	%ebp
	movl	%esp, %ebp
	movzwl	16(%ebp), %eax
	pushl	%ebx
	notl	%eax
	movl	8(%ebp), %ebx
	movzwl	%ax,%edx
	testl	%ebx, %ebx
	movl	12(%ebp), %ecx
	je	.L328
	.p2align 2
.L319:
	movzbl	(%ecx), %eax
	sall	$8, %eax
	incl	%ecx
	addl	%eax, %edx
	movzbl	(%ecx), %eax
	addl	%eax, %edx
	incl	%ecx
	decl	%ebx
	jne	.L319
	jmp	.L328
	.p2align 2
.L323:
	movzwl	%dx,%eax
	leal	(%ebx,%eax), %edx
.L328:
	movl	%edx, %ebx
	shrl	$16, %ebx
	jne	.L323
	movl	%edx, %eax
	notl	%eax
	movl	%eax, %edx
	andl	$65535, %edx
	jne	.L325
	movl	$65535, %edx
.L325:
	movzwl	%dx, %eax
	movl	(%esp), %ebx
	leave
	ret
.Lfe7:
	.size	 gencksm,.Lfe7-gencksm
	.ident	"GCC: (GNU) 2.96 20000731 (Red Hat Linux 7.0)"
