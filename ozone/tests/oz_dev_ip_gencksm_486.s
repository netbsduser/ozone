
# Return cpu cycle count in %edx:%eax

	.text
	.align	4
	.globl	rdtsc
rdtsc:
	rdtsc
	ret

/************************************************************************/
/*									*/
/*  Generate ip-style checksum on a buffer				*/
/*									*/
/*    Input:								*/
/*									*/
/*	 4(%esp) = number of bytes to process				*/
/*	 8(%esp) = source buffer address				*/
/*	12(%esp) = starting checksum value				*/
/*									*/
/*    Output:								*/
/*									*/
/*	%ax = resulting checksum					*/
/*									*/
/*    Note:								*/
/*									*/
/*	If odd bytecount, resulting checksum will be byteswapped.  If 	*/
/*	you will be appending onto the same buffer, just continue 	*/
/*	using this checksum.  Then, when ready to send, and the total 	*/
/*	is odd, swap the checksum bytes.  If the total is even, just 	*/
/*	use it as is.							*/
/*									*/
/************************************************************************/

	.text
	.align	4
	.globl	oz_dev_ip_gencksmb
oz_dev_ip_gencksmb:
	pushl	%ebp			# make call frame on stack in case of error
	pushl	$0
	leal	 4(%esp),%ebp
	pushl	%esi			# save scratch registers
	xorl	%eax,%eax		# clear top half of accumulator
	xorl	%edx,%edx		# clear all of %edx
	movl	 8(%ebp),%ecx		# get number of bytes to be checksummed
	movl	12(%ebp),%esi		# point to array of bytes to checksum
	movw	16(%ebp),%dx		# get start value
	jecxz	cksmb_null		# check for zero length
	notw	%dx
	shrl	$1,%ecx
	cld
	jnc	cksmb_loop
	bswap	%edx
	movb	(%esi),%dl
	incl	%esi
cksmb_loop:
	movzbl	(%esi),%eax
	incl	%esi
	shll	$8,%eax
	addl	%eax,%edx
	movzbl	(%esi),%eax
	incl	%esi
	addl	%eax,%edx
	decl	%ecx			# repeat for more bytes
	jne	cksmb_loop		# (decl/jne faster than loop!)
	movl	%edx,%eax		# get final end-arounds
	shrl	$16,%eax
	addw	%dx,%ax
	adcw	$0,%ax
	xorw	$0xFFFF,%ax
	je	cksmb_zero
	popl	%esi
	leave
	ret
cksmb_null:
	movw	%dx,%ax			# nothing to process, just return start value as is
	popl	%esi
	leave
	ret
cksmb_zero:
	notw	%ax			# return FFFF en lieu de 0000
	popl	%esi
	leave
	ret

/************************************************************************/
/*									*/
/*  Generate ip-style checksum on a buffer and copy it			*/
/*									*/
/*    Input:								*/
/*									*/
/*	 4(%esp) = number of bytes to process				*/
/*	 8(%esp) = source buffer address				*/
/*	12(%esp) = destination buffer address				*/
/*	16(%esp) = starting checksum value				*/
/*									*/
/*    Output:								*/
/*									*/
/*	bytes copied from source to destination				*/
/*	%ax = resulting checksum					*/
/*									*/
/*    Note:								*/
/*									*/
/*	If odd bytecount, resulting checksum will be byteswapped.  If 	*/
/*	you will be appending onto the same buffer, just continue 	*/
/*	using this checksum.  Then, when ready to send, and the total 	*/
/*	is odd, swap the checksum bytes.  If the total is even, just 	*/
/*	use it as is.							*/
/*									*/
/************************************************************************/

	.text
	.align	4
	.globl	oz_dev_ip_gencksmbc
oz_dev_ip_gencksmbc:
	pushl	%ebp			# make call frame on stack in case of error
	pushl	$0
	leal	 4(%esp),%ebp
	pushl	%edi			# save scratch registers
	pushl	%esi
	xorl	%eax,%eax		# clear top half of accumulator
	xorl	%edx,%edx		# clear all of %edx
	movl	 8(%ebp),%ecx		# get number of bytes to be checksummed
	movl	12(%ebp),%esi		# point to array of bytes to checksum
	movl	16(%ebp),%edi		# point to where to copy them to
	movw	20(%ebp),%dx		# get start value
	jecxz	cksmbc_null		# check for zero length
	cld				# make sure string instructions go forward
	.align	4
cksmbc_loop:
	lodsb				# get byte from (%esi)+ into eax<00:07>
	bswap	%edx			# swap accumulator bytes
	stosb				# store byte to (%edi)+
	subl	%eax,%edx		# subtract byte from accumulator
	loop	cksmbc_loop		# repeat for more bytes
	movl	%edx,%eax		# get final end-arounds
	shrl	$16,%eax
	addw	%dx,%ax
	adcw	$0,%ax
	je	cksmbc_zero
	popl	%esi
	popl	%edi
	leave
	ret
cksmbc_null:
	movw	%dx,%ax			# nothing to process, just return start value as is
	popl	%esi
	popl	%edi
	leave
	ret
cksmbc_zero:
	notw	%ax			# return FFFF en lieu de 0000
	popl	%esi
	popl	%edi
	leave
	ret
