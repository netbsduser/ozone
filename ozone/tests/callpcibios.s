
#  8(%ebp) = cprocmode
# 12(%ebp) = param block
#          +  0 : pci bios' entrypoint
#             4 : input eax
#             8 : input ebx
#            12 : input ecx
#            16 : input edx
#            20 : output eflags
#            24 : output eax
#            28 : output ebx
#            32 : output ecx
#            36 : output edx
#  returns 1 in %eax (OZ_SUCCESS)

	.text
	.align	4
	.globl	callpcibios
callpcibios:
	pushl	%ebp		# make a call frame
	pushl	$0
	leal	4(%esp),%ebp
	pushl	%edi		# save scratch regs
	pushl	%esi
	pushl	%ebx

	movl	12(%ebp),%edi	# param block pointer
	movl	  (%edi),%esi	# point to pci bios entrypoint
	movl	 4(%edi),%eax	# input eax
	movl	 8(%edi),%ebx	# input ebx
	movl	12(%edi),%ecx	# input ecx
	movl	16(%edi),%edx	# input edx

	pushl	$0		# make like a 'far' call
	movw	%cs,(%esp)
	call	%esi		# call the routine

	pushfl			# save return flags

	movl	%eax,24(%edi)	# output eax
	movl	%ebx,28(%edi)	# output eax
	movl	%ecx,32(%edi)	# output eax
	movl	%edx,36(%edi)	# output eax

	popl	20(%edi)	# output flags

	popl	%ebx		# restore scratch regs
	popl	%esi
	popl	%edi
	movl	$1,%eax		# set up success status
	leave
	ret
