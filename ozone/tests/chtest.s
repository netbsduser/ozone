
	.text

msg1:	.string	"chtest: started\n"

	.align	4

	.globl	_start
main_pd:
	.byte	5,1
	.word	0x13
	.long	-16
	.long	_start
	.long	0,0
_start:
	pushl	%ebp
	pushl	$main_pd
	leal	4(%esp),%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	pushl	$msg1
	pushl	$0
	call	oz_sys_printkp

	# set up known register values

	movl	$0x11223344,%ebx
	movl	$0x23344556,%esi
	movl	$0x35577991,%edi

	# do a divide-by-zero error

	call	divbyzero

	# hopefully the registers are the same and status is OZ_DIVIDEBYZERO

	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%eax
	pushl	$divbyzero_msg
	pushl	$0
	call	oz_sys_printkp

	# do an access violation error

	call	accvio

	# hopefully the registers are the same and status is OZ_ACCVIO

	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%eax
	pushl	$accvio_msg
	pushl	$0
	call	oz_sys_printkp

	# do an aritmetic overflow (execution should resume where it left off)

	call	arithover

	# hopefully the registers are the same and status is OZ_SUCCESS

	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%eax
	pushl	$arithover_msg
	pushl	$0
	call	oz_sys_printkp

	# all done

	lea	-16(%ebp),%esp
	popl	%edi
	popl	%esi
	popl	%ebx
	leave
	ret

# The handler for dividebyzero executes in the callers mode

divbyzero_prm:	.string	"divbyzero"
divbyzero_msg:	.string	"chtest: divbyzero sts %u, ebx %x, esi %x, edi %x\n"

	.align	4
divbyzero_pd:
	.byte	5,1
	.word	0x13
	.long	-16
	.long	divbyzero
	.long	condhand
	.long	divbyzero_prm
divbyzero:
	pushl	%ebp
	pushl	$divbyzero_pd
	leal	4(%esp),%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	movl	$0x47036925,%ebx
	movl	$0x59371593,%esi
	movl	$0x61616161,%edi

	movl	$1,%eax
	xorl	%ecx,%ecx
	divb	%cl

	movl	oz_SUCCESS,%eax

	lea	-16(%ebp),%esp
	popl	%edi
	popl	%esi
	popl	%ebx
	leave
	ret

# The handler for accvio executes in kernel mode initially to see if it is a valid pagefault and if 
# not, copies the mchargs and sigargs to the user stack where normal exception processing happens

accvio_prm:	.string	"accvio"
accvio_msg:	.string	"chtest: accvio sts %u, ebx %x, esi %x, edi %x\n"

	.align	4
accvio_pd:
	.byte	5,1
	.word	0x13
	.long	-16
	.long	accvio
	.long	condhand
	.long	accvio_prm
accvio:
	pushl	%ebp
	pushl	$accvio_pd
	leal	4(%esp),%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	movl	$0x72389450,%ebx
	movl	$0x81470369,%esi
	movl	$0x90561278,%edi

	movl	0x65437890,%ecx

	movl	oz_SUCCESS,%eax

	lea	-16(%ebp),%esp
	popl	%edi
	popl	%esi
	popl	%ebx
	leave
	ret

# The handler for arithover executes in the callers mode but we can resume execution

arithover_prm:	.string	"arithover"
arithover_msg:	.string	"chtest: arithover sts %u, ebx %x, esi %x, edi %x\n"

	.align	4
arithover_pd:
	.byte	5,1
	.word	0x13
	.long	-16
	.long	arithover
	.long	condhand
	.long	arithover_prm
arithover:
	pushl	%ebp
	pushl	$arithover_pd
	leal	4(%esp),%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	movl	$0x97867564,%ebx
	movl	$0x86756453,%esi
	movl	$0x74635241,%edi

	movb	$0x80,%al
	addb	$0x80,%al
	into

	movl	oz_SUCCESS,%eax

	lea	-16(%ebp),%esp
	popl	%edi
	popl	%esi
	popl	%ebx
	leave
	ret

#
#  Condition handler
#
#    Input:
#
#	 4(%esp) = chparam
#	 8(%esp) = unwinding flag
#	12(%esp) = sigargs pointer
#	16(%esp) = mchargs pointer
#

chmsg1:	.string	"condhand (%p, %u, %p, %p)\n"
chmsg2:	.string	"condhand: %s sigargs %u %u %8.8x %8.8x %8.8x\n"
chmsg3:	.string	"condhand: %s ec2 %8.8x, ec1 %8.8x, ebx %8.8x, eip %8.8x, eflags %8.8x\n"

	.align	4
condhand_pd:
	.byte	5,1
	.word	0x13
	.long	-16
	.long	condhand
	.long	0,0
condhand:
	pushl	%ebp
	pushl	$condhand_pd
	leal	4(%esp),%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	movl	$0x87654321,%ebx
	movl	$0x98765432,%esi
	movl	$0x19876543,%edi

	# print out our call args as they are

	pushl	20(%ebp)
	pushl	16(%ebp)
	pushl	12(%ebp)
	pushl	 8(%ebp)
	pushl	$chmsg1
	pushl	$0
	call	oz_sys_printkp

	# print out the first few longs of the sigargs

	movl	16(%ebp),%edx
	pushl	16(%edx)	# sigargs[4]
	pushl	12(%edx)	# sigargs[3]
	pushl	 8(%edx)	# sigargs[2]
	pushl	 4(%edx)	# sigargs[1]
	pushl	  (%edx)	# sigargs[0]
	pushl	 8(%ebp)
	pushl	$chmsg2
	pushl	$0
	call	oz_sys_printkp

	# print out some stuff from the mchargs

	movl	20(%ebp),%edx
	pushl	60(%edx)	# eflags
	pushl	52(%edx)	# eip
	pushl	28(%edx)	# ebx
	pushl	20(%edx)	# ec1
	pushl	  (%edx)	# ec2
	pushl	 8(%ebp)
	pushl	$chmsg3
	pushl	$0
	call	oz_sys_printkp

	# unwind the stack to return the signal code as the return value for the function

	movl	oz_UNWIND,%eax

	# but if arithmetic overflow, resume execution where it left off

	movl	16(%ebp),%edx
	movl	 4(%edx),%edx
	cmpl	%edx,oz_ARITHOVER
	jne	condhand_rtn
	movl	oz_RESUME,%eax

condhand_rtn:
	lea	-16(%ebp),%esp
	popl	%edi
	popl	%esi
	popl	%ebx
	leave
	ret
