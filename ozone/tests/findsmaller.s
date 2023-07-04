# find the smaller of two numbers
	.globl	findsmaller
findsmaller:
	movl	4(%esp),%eax	# get one in %eax
	movl	8(%esp),%ecx	# get other in %ecx
	cmpl	%ecx,%eax	# set carry if %eax is smaller than %ecx
	cmovnc	%ecx,%eax	# put %ecx into %eax if no carry
	ret			# done

