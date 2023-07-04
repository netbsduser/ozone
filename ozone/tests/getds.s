	.globl	getds
getds:
	movw	%ds,%ax
	ret

	.globl	getss
getss:
	movw	%ss,%ax
	ret

	.globl	timesetfs
timesetfs:
	movl	4(%esp),%ecx
	rdtsc
	pushl	%eax
	movw	%cx,%fs
	rdtsc
	popl	%edx
	subl	%edx,%eax
	ret

	.globl	timesetfs2
timesetfs2:
	pushl	%ebx
	movl	 8(%esp),%ecx
	movl	12(%esp),%ebx
	rdtsc
	pushl	%eax
	movw	%cx,%fs
	movw	%bx,%fs
	rdtsc
	popl	%edx
	subl	%edx,%eax
	popl	%ebx
	ret
