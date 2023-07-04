##+++2001-10-06
##    Copyright (C) 2001, Mike Rieker, Beverly, MA USA
##
##    This program is free software; you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation; version 2 of the License.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program; if not, write to the Free Software
##    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##---2001-10-06

##########################################################################
##									##
##  This is the main program for the loader				##
##									##
##  It is loaded into memory at 0x0000A000 by the bootblock		##
##  The processor is in 32-bit mode as set up by the preloader		##
##									##
##  The registers are set up:						##
##									##
##	cs  = 8 (GDT[1] is the code segment)				##
##	eip = 0xA000							##
##									##
##  There is memory for the IDT at 7E00.  The IDT proper ends at 	##
##  8600 and is followed by 256 pushb/jmp instruction pairs for 	##
##  the default exception handler.  The 24-byte GDT is in 7DE8.		##
##									##
##  The load parameter block it at address 9000-9FFF.			##
##									##
##  This routine may overwrite the bootblock and preloader if it 	##
##  wishes, ie, it can assume all the memory between 1000 and 7DE8 is 	##
##  available for its use (it uses it for its own stack).		##
##									##
##  This is basically what it does:					##
##									##
##   1) Call oz_ldr_start to read the kernel into memory at 00100000	##
##   2) Enable paging							##
##   3) Jump to the kernel						##
##									##
##  It uses the pager to set up a 1:1 mapping of the memory from 0 to 	##
##  through where the kernel ends, except that page 0 is disabled from 	##
##  any access.  Pages from 1 up to the base of the kernel are set to 	##
##  kernel read/write.  Pages in the kernel itself are either kernel 	##
##  read/write or user read-only.  The remaining entries in the spt 	##
##  are set to no-access.  Any memory following the spt can be used by 	##
##  the kernel for non-paged pool (ie, memory occupied by this loader 	##
##  at addresses from after the spt through 0x9FFFF).			##
##									##
##########################################################################

	.include "oz_params_486.s"

	.text
	.globl	_start
_start:

## Address of the memory hole is A0000

	movl	$0x000A0000,holevaddr

## Set up the stack and data segment registers

	movw	$KNLDATASEG,%cx		# get data segment selector (16) in cx
	movw	%cx,%ds			# set all the data related segment registers
	movw	%cx,%es			# - now we can forget all about segment registers
	movw	%cx,%fs			# - it's beginning to look like a real 32-bit processor
	movw	%cx,%gs
	movw	%cx,%ss
	movl	$GDTBASE,%esp		# set up the stack pointer to put stack just below GDT

## Now our 32-bit environment is complete

	movw	$0x03cc,%dx		## read the misc output register
	inb	%dx
	call	iodelay
	movw	$0x03b4,%dx		## assume use 03b4/03b5
	and	$1,%al
	je	video_init_useit
	movw	$0x03d4,%dx		## if misc<0> set, use 03d4/03d5
video_init_useit:
	xorl	%ecx,%ecx		## read cursor into %ecx...
	movb	$0x0e,%al		## ... output 0e -> 03?4
	outb	%dx
	call	iodelay
	incw	%dx			## ... input 03?5 -> al
	inb	%dx
	call	iodelay
	movb	%al,%ch			## ... that's high order cursor addr
	decw	%dx			## ... output 0f -> 03?4
	movb	$0x0f,%al
	outb	%dx
	call	iodelay
	incw	%dx			## ... input 03?5 -> al
	inb	%dx
	call	iodelay
	movb	%al,%cl			## ... that's low order cursor addr
	movl	$0xb8000,%edi		## point to base of video memory
	addl	%ecx,%edi		## add twice the cursor offset
	addl	%ecx,%edi
	movb	$0x2a,%al		## store an '*' there in red
	movb	$4,%ah
	movw	%ax,(%edi)
	incw	%cx			## increment cursor position
	decw	%dx			## output 0e -> 03?4
	movb	$0x0e,%al
	outb	%dx
	call	iodelay
	incw	%dx			## output %ch -> 03?5
	movb	%ch,%al
	outb	%dx
	call	iodelay
	decw	%dx			## output 0f -> 03?4
	movb	$0x0f,%al
	outb	%dx
	call	iodelay
	incw	%dx			## output %cl -> 03?5
	movb	%cl,%al
	outb	%dx
	movb	$0x2b,%al		## try a plus sign via routine
	call	oz_ldr_debchar

## Set the 'in loader' flag

	movl	$1,oz_s_inloader

## Initialize video

	call	oz_dev_video_init

## Output a message

	pushl	$start_msg
	pushl	$start_msglen
	call	oz_hw_putcon
	addl	$8,%esp

## Make a null stack frame so frame tracer will know when to stop

	xorl	%ebx,%ebx		# get zeroes
	pushl	%ebx			# null return address
	pushl	%ebx			# null previous stack frame
	pushl	$0			# null prodecure descriptor pointer
	leal	4(%esp),%ebp		# set up current frame pointer

## Fill in the IDT with default handler

	movl	$IDTBASE,%edi		# point to where we will put IDT
	call	oz_hw_idt_init		# initialize the IDT

	lidt	idtr_init		# enable it

## Count how much memory we have

	pushl	$mem_msg		# display a message
	pushl	$mem_msglen
	call	oz_hw_putcon
	addl	$8,%esp

	xorl	%eax,%eax		# point to memory location zero
	movl	%eax,%esi
	movl	%eax,(%esi)		# clear memory location zero
	movl	$0x400000,%edx		# start at 4MB boundary
memory_loop:
	movl	%edx,%edi		# point to location to test
	movl	%edx,%eax		# store the memory address back in itself
	movl	%eax,(%edi)
	cmpl	(%edi),%eax		# see if it reads back ok
	jne	memory_done
	notl	%eax			# the complement should also work
	movl	%eax,(%edi)
	cmpl	(%edi),%eax
	jne	memory_done
	cmpl	$0,(%esi)		# make sure location zero is still zero
	jne	memory_done		# if not, we wrapped around
	addl	$0x400000,%edx		# increment by a super page (4MB)
	jne	memory_loop
	pushl	$mem_errmsg		# wrapped all the way around, display an message
	pushl	$mem_errmsglen
	call	oz_hw_putcon
memory_stop:
	jmp	memory_stop
memory_done:
	movl	%edx,topvaddr		# save my top address
	movl	%edx,%eax		# print out memory size
	shrl	$12,%edx		# (save physical pages)
	movl	%edx,phypages
	movl	%edx,oz_s_phymem_totalpages
	call	oz_hw_print_hexl
	pushl	$mem_donemsg		# finish printing message
	pushl	$mem_donemsglen
	call	oz_hw_putcon
	addl	$8,%esp

## Initialize the mpd to identity map everything using 4MB pages

	movl	$MPDBASE,%edi		# point to base of mpd
	movl	$0x0000009B,%eax	# this is basic mpd entry
					#  <00> = 1 : page present in phys memory
					#  <01> = 1 : page is writable
					#  <02> = 0 : page not accessible to user mode
					#  <03> = 1 : write-through cache mode
					#  <04> = 1 : caching disabled
					#  <05> = 0 : page has been accessed
					#  <06> = 0 : page is dirty
					#  <07> = 1 : 4MB page
mpd_init_loop:
	movl	%eax,(%edi)		# store the entry
	addl	$4,%edi			# increment to next mpd entry
	addl	$0x00400000,%eax	# increment to next 4MB page
	jnc	mpd_init_loop		# repeat until mpd is filled
	movb	$0x83,$MPDBASE		# enable caching on virt addr 0 super-page (my code and data)

## Enable paging - since we are identity mapped, we should not notice anything special here

	pushl	$enpag_msg
	pushl	$enpag_msglen
	call	oz_hw_putcon
	addl	$8,%esp

	??? set PSE bit in cr4 for 4MB pages ???
	movl	$MPDBASE,%ebx		# point to the MPD
	movl	%ebx,%cr3		# store in CR3
	movl	%cr0,%eax		# get what's currently in CR0
	orl	$0x80000000,%eax	# set the paging enable bit
	movl	%eax,%cr0		# now paging is enabled

	pushl	$pagen_msg
	pushl	$pagen_msglen
	call	oz_hw_putcon
	addl	$8,%esp

## Do a memory pass

test_loop:
	pushl	$MPDBASE
	pushl	topvaddr
	pushl	$0x00400000
	call	testmemory		# test memory 4MB..topvaddr
	addl	$12,%esp

## Move this stuff to a different physical address but keep its virtual address the same

	movl	$640*1024/4,%ecx	# copy this 640KB program to next 4M physical page block
	movl	$0,%esi
	movl	$0x00400000,%edi
	cld
	movsl

	movl	topvaddr,%ecx		# rotate MPD entries down one so we execute out of new phys pages
	shrl	$22,%ecx
	decl	%ecx
	movl	$MPDBASE,%esi
	movl	(%esi),%edx
mpd_rotate_loop:
	movl	4(%esi),%eax
	movb	$0x9B,%al		# (make sure caching is disabled)
	movl	%eax,(%esi)
	addl	$4,%esi
	loop	mpd_rotate_loop
	movl	%edx,(%esi)
	movb	$0x83,$MPDBASE		# enable caching on virt addr 0 super-page (my code and data)

	movl	$MPDBASE,%eax		# invalidate the whole TLB cache so cpu will read the new MPD entries
	movl	%eax,%cr3

## The hole is at a different virtual address now

	subl	$0x00400000,holevaddr	# shift virt addr down one super-page
	jnc	holevaddr_ok
	movl	topvaddr,%eax		# maybe have to wrap it
	addl	%eax,holevaddr
holevaddr_ok:

	jmp	test_loop

##
## Data used by the above
##

	.align	4

phypages:	.long	0		# number of physical pages this computer has

		.word	0		# align so .long IDTBASE will be aligned (ugly cpu's)
idtr_init:	.word	8*256		# size (in bytes) of idt
		.long	IDTBASE		# address of idt

start_msg:	.ascii	"\noz_loader_486: now in 32-bit mode\n"
	start_msglen = . - start_msg

mem_msg:	.ascii	"oz_loader_486: counting memory - "
	mem_msglen = . - mem_msg

mem_errmsg:	.ascii	"wrapped all the way around, cannot continue"
	mem_errmsglen = . - mem_errmsg

mem_donemsg:	.ascii	" (hex) bytes of memory\n"
	mem_donemsglen = . - mem_donemsg

enpag_msg:	.ascii	"oz_loader_486: enabling paging\n"
	enpag_msglen = . - enpag_msg

pagen_msg:	.ascii	"oz_loader_486: paging enabled\n"
	pagen_msglen = . - pagen_msg


##########################################################################
##									##
##  These exception handlers call the kernel debugger			##
##									##
##########################################################################

	.align	4
exception_DE:				# divide by zero
	pushl	%ebp
	movl	oz_DIVBYZERO,%ebp
	jmp	exception_xx

	.align	4
exception_DB:				# debug (single step, etc)
	pushl	%ebp
	movl	oz_SINGLESTEP,%ebp
	jmp	exception_xx

	.align	4
exception_BP:				# breakpoint (int3)
	pushl	%ebp
	movl	oz_BREAKPOINT,%ebp
	jmp	exception_xx

	.align	4
exception_OF:				# arithmetic overflow
	pushl	%ebp
	movl	oz_ARITHOVER,%ebp
	jmp	exception_xx

	.align	4
exception_BR:				# subscript range
	pushl	%ebp
	movl	oz_SUBSCRIPT,%ebp
	jmp	exception_xx

	.align	4
exception_UD:				# undefined opcode
	pushl	%ebp
	movl	oz_UNDEFOPCODE,%ebp
	jmp	exception_xx

	.align	4
exception_xx:
	pushl	$0			# build rest of mchargs
	pushal	
	pushw	%gs
	pushw	%fs
	pushw	%es
	pushw	%ds
	pushl	$0
	movw	$KNLDATASEG,%dx		# make sure data segment registers are ok
	movw	%dx,%ds
	movw	%dx,%es
	movw	%dx,%fs
	movw	%dx,%gs
	movl	MCH_L_EC1(%esp),%eax	# get OZ_ error status code
	movl	$0,MCH_L_EC1(%esp)	# clear out the ec1 in the mchargs

	jmp	exception_crash

	.align	4
exception_GP:				# general protection
	xchgl	%ebp,(%esp)		# swap with error code
	pushl	$0			# build rest of mchargs
	pushal	
	pushw	%gs
	pushw	%fs
	pushw	%es
	pushw	%ds
	pushl	$0
	movw	$KNLDATASEG,%dx		# make sure data segment registers are ok
	movw	%dx,%ds
	movw	%dx,%es
	movw	%dx,%fs
	movw	%dx,%gs
	movl	oz_GENERALPROT,%eax
	jmp	exception_crash

	.align	4
exception_PF:				# page fault
	xchgl	%ebp,(%esp)		# swap with error code
	pushl	$0			# build rest of mchargs
	pushal	
	pushw	%gs
	pushw	%fs
	pushw	%es
	pushw	%ds
	movl	%cr2,%eax		# (get address that caused the fault)
	pushl	%eax
	movw	$KNLDATASEG,%dx		# make sure data segment registers are ok
	movw	%dx,%ds
	movw	%dx,%es
	movw	%dx,%fs
	movw	%dx,%gs
	movl	oz_ACCVIO,%eax
	jmp	exception_crash

	.align	4

exception_crash:
	leal	MCH_L_EBP(%esp),%ebp	# set up frame pointer
	leal	MCH_L_XSP(%esp),%ebx	# calc esp at time of exception
	movl	%ebx,MCH_L_ESP(%esp)	# save esp at time of exception in the mchargs

	movl	%esp,%ebx		# save mchargs pointer

	pushl	$0			# push sigargs on stack
	pushl	%eax
	pushl	$2

	movl	%esp,%eax		# save sigargs pointer

	pushl	%ebx			# call kernel debugger
	pushl	%eax
	call	oz_knl_debug_exception

	movl	%ebx,%esp		# wipe stack to machine arguments

	addl	$4,%esp			# pop ec2 from stack
	popw	%ds			# restore data segment registers
	popw	%es
	popw	%fs
	popw	%gs
	popal				# restore all the registers (except ebp gets ec1's value)
	addl	$4,%esp			# wipe procedure descriptor pointer
	popl	%ebp			# pop ebp from stack
	iret				# retry the faulting instruction

##########################################################################
##									##
##  Debug exception handler						##
##									##
##  Prints out message when debug exception occurs			##
##									##
##########################################################################

	.text

debug_msg1:	.ascii	"oz_loader_486 debug_except: flags %x, eip %x, addr %x, new %x"
		.byte	10,0

	.align	4
debug_except:
	pushal				# save general registers
	movl	%dr0,%eax		# get debug registers
	movl	%dr6,%ebx
	movl	32(%esp),%edx		# get saved eip
	pushl	(%eax)			# print message
	pushl	%eax
	pushl	%edx
	pushl	%ebx
	pushl	$debug_msg1
	call	oz_knl_printk
	addl	$20,%esp		# pop call args
	popal				# restore general registers
	iret				# return

##
## Output a long beep followed by the given number of short beeps
##
##   Input:
##
##	dx = number of short beeps
##
##   Scratch:
##
##	eax,ebx,ecx,edx
##
beep:
	movl	$100000000,%ecx		# output a long beep and a short pause
	call	beeptimed
beep_count_loop:
	movl	$30000000,%ecx		# output a short beep and a short pause
	call	beeptimed
	decw	%dx			# decrement beep counter
	jne	beep_count_loop		# loop back if more beeps to go
	call	beepshortdelay		# an extra short pause when done
	ret
##
beeptimed:
	inb	$0x61		# turn the beep on
	call	iodelay
	orb	$3,%al
	outb	$0x61
	call	iodelay
	movb	$0xb6,%al
	outb	$0x43
	call	iodelay
	movb	$0x36,%al
	outb	$0x42
	call	iodelay
	movb	$0x03,%al
	outb	$0x42
	call	iodelay
##
	call	beepdelay	# leave it on for time in ecx
##
	inb	$0x61		# turn the beep off
	call	iodelay
	andb	$0xfc,%al
	outb	$0x61
	call	iodelay
##
beepshortdelay:
	movl	$20000000,%ecx	# follow the turn-off by a short pause
beepdelay:
	decl	%ecx
	jne	beepdelay
	ret
##
## Delay a very short time after doing an I/O
##
iodelay:
	movw	$200,%bx
iodelay_loop:
	decw	%bx
	jne	iodelay_loop
	ret
