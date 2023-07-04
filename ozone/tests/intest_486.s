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
##  This is an interrupt test program					##
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
##########################################################################

	.include "oz_params_486.s"

	MAST_8259_APORT = 0x20
	MAST_8259_DPORT = 0x21
	SLAV_8259_APORT = 0xA0
	SLAV_8259_DPORT = 0xA1

	KB_DP = 0x60			# keyboard data port
	KB_CP = 0x64			# keyboard command port

	VIDEOBASE = 0xB8000		# address of video memory
	VIDEOLINESIZE = 160		# number of bytes in a line of video (80 chars)
	VIDEOSCREENSIZE = 4000		# number of bytes in the whole screen (25 lines)

	CONFADDR = 0x0CF8		# PCI configuration space address I/O port
					#     <31> = 1 to enable
					#  <16:23> = bus number
					#  <11:15> = device number
					#   <8:10> = function number
					#    <7:2> = register number
					#    <0:1> = zeroes
	CONFDATA = 0x0CFC		# PCI configuration space data I/O port

	.text
	.globl	_start
_start:

## Set up the stack and data segment registers

	movw	$KNLDATASEG,%cx		# get data segment selector (16) in cx
	movw	%cx,%ds			# set all the data related segment registers
	movw	%cx,%es			# - now we can forget all about segment registers
	movw	%cx,%fs			# - it's beginning to look like a real 32-bit processor
	movw	%cx,%gs
	movw	%cx,%ss
	movl	$GDTBASE,%esp		# set up the stack pointer to put stack just below GDT

## Now our 32-bit environment is complete

## Output red '*' to screen with minimal resources just to see if we get here

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
	movl	$VIDEOBASE,%edi		## point to base of video memory
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

	pushl	$0x2E			## output a '+' using video driver routine
	call	oz_dev_video_putchar
	addl	$4,%esp

## Make a null stack frame so frame tracer will know when to stop

	pushl	$0			# null return address
	pushl	$0			# null previous stack frame
	pushl	$0			# null prodecure descriptor pointer
	leal	4(%esp),%ebp		# set up current frame pointer

## Make sure floppy motor is turned off

	xorl	%eax,%eax
	outb	$0x03F2

## Fill in the IDT with default handler

	movl	$IDTBASE,%edi		# point to where we will put IDT
	call	oz_hw_idt_init		# initialize the IDT

	lidt	idtr_init		# enable it

##  Fill in IDT with IRQ interrupt vectors for IRQ0 (timer) and IRQ1 (keyboard)
##  The rest get the dummy interrupt handler

	movl	$IDTBASE,%edi		# point to base of IDT
	movl	$16,%ebx		# number of entries
	movl	$idt_irq_fix,%esi	# point to fixup table
	addl	$8*INT_IRQBASE,%edi	# point to irq vectors in table
idt_irq_fix_loop:
	movw	0(%esi),%ax		# copy in low-order entrypoint
	movw	2(%esi),%dx		# copy in high-order entrypoint
	movw	%ax,0(%edi)
	movw	$KNLCODESEG,2(%edi)	# code segment
	movw	%dx,6(%edi)
	addl	$4,%esi			# increment to next fixup table entry
	addl	$8,%edi			# increment to next irq table entry
	decl	%ebx
	jne	idt_irq_fix_loop

##  Reprogram the 8259's to not overlay the cpu vector table
##  Use vectors 0x30-0x3F for IRQ0-IRQ15

	movb	$0x11,%al		# ICW1: initialization sequence
	outb	$MAST_8259_APORT	# send it to master
	call	iodelay
	outb	$SLAV_8259_APORT	# send it to slave
	call	iodelay
	movb	$INT_IRQBASE+0,%al	# ICW2 master: start of hardware int's (0x30)
	outb	$MAST_8259_DPORT
	call	iodelay
	movb	$INT_IRQBASE+8,%al	# ICW2 slave: start of hardware int's (0x38)
	outb	$SLAV_8259_DPORT
	call	iodelay
	movb	$0x04,%al		# ICW3 master: slave is connected to IR2
	outb	$MAST_8259_DPORT
	call	iodelay
	movb	$0x02,%al		# ICW3 slave: slave is on master's IR2
	outb	$SLAV_8259_DPORT
	call	iodelay
	movb	$0x01,%al		# ICW4: 8086 mode for both
	outb	$MAST_8259_DPORT
	call	iodelay
	outb	$SLAV_8259_DPORT
	call	iodelay
	movb	$0,%al			# OCW1 master: enable all interrupts
	outb	$MAST_8259_DPORT
	call	iodelay
	movb	$0,%al			# OCW1 slave: enable all interrupts
	outb	$SLAV_8259_DPORT
	call	iodelay

## Scan bus for PIIX4's

	movl	$0x80000000,%ebx	# start with enabled, bus 0, dev 0, func 0, register 0
piix4scan_loop:
	movl	%ebx,%eax		# output the pci address
	movw	$CONFADDR,%dx
	outl	%dx
	call	iodelay
	movw	$CONFDATA,%dx		# read the corresponding data
	inl	%dx
	call	iodelay			# - device-id in <16:31>, vendor-id in <00:15>
	cmpl	$0x71108086,%eax	# check for PIIX4
	je	piix4scan_done		# stop scanning if found
	addl	$0x800,%ebx		# increment pci device number, overflow to bus number
	bt	$24,%ebx		# stop when bus number overflows
	jnc	piix4scan_loop		# repeat if more to scan
piix4scan_done:
	movl	%ebx,piix4address

## Clear screen and output initial status display

	movl	$VIDEOBASE,%edi
	xorl	%eax,%eax
	movl	$VIDEOSCREENSIZE,%ecx
	cld
	rep
	stosb
	call	updatestatus

## Just loop forever, process everything via interrupts

	sti
loop_forever:
	hlt
	incl	hlt_counter
	call	updatestatus
	jmp	loop_forever

##
## Data used by the above
##

	.align	4

		.word	0		# align so .long IDTBASE will be aligned (ugly cpu's)
idtr_init:	.word	8*256		# size (in bytes) of idt
		.long	IDTBASE		# address of idt

idt_irq_fix:	.long	irq_0_entry
		.long	irq_1_entry
		.long	irq_x_entry,irq_x_entry,irq_x_entry,irq_x_entry,irq_x_entry,irq_x_entry
		.long	irq_y_entry,irq_y_entry,irq_y_entry,irq_y_entry,irq_y_entry,irq_y_entry,irq_y_entry,irq_y_entry

start_msg:	.ascii	"\noz_intest_486: now in 32-bit mode\n"
	start_msglen = . - start_msg

		.align	4

keyboard_index:	.long	0
hlt_counter:	.long	0
timer_counter:	.long	0
kb_int_counter:	.long	0
piix4address:	.long	0
kbenable:	.long	0

#
# Timer interrupts come here
#
	.align	4
irq_0_entry:
	pushal
	movl	kbenable,%ecx
	sti
	incl	timer_counter
	jecxz	irq_0_nokbenab
	decl	kbenable
	jne	irq_0_nokbenab
	movb	$0xAE,%al
	outb	$KB_CP
irq_0_nokbenab:
	cli
	movb	$0,%bl
	call	ack_8259
	call	updatestatus
	popal
	iret

#
# Keyboard interrupts come here
#
	.align	4
irq_1_entry:
	pushal
	sti
	incl	kb_int_counter

check_keyboard:
	call	keyboard_getc		# see if any keyboard character to process
	testb	%al,%al
	je	keyboard_irq_ret	# exit ISR if not
	cmpb	$32,%al
	jl	keyboard_noint
	movb	$0x07,%ah		# display it on screen
	movl	keyboard_index,%ecx
	addl	%ecx,%ecx
	addl	$VIDEOBASE+(VIDEOLINESIZE*10),%ecx
	movw	%ax,(%ecx)
	incl	keyboard_index
	jmp	check_keyboard		# go back to get another

keyboard_irq_ret:
	cli
	movb	$1,%bl
	call	ack_8259
	call	updatestatus
	popal
	iret

keyboard_noint:
	xorb	%dl,%dl			# clear IRQ1 enable
	call	setirq1enable
	movl	$0,keyboard_index	# reset interrupt stuff to first column
	xorl	%ebx,%ebx		# start at first column for us
	movl	$VIDEOBASE+(VIDEOLINESIZE*11),%ebx
keyboard_noint_check:
	call	keyboard_getc		# see if any keyboard character to process
	testb	%al,%al
	je	keyboard_noint_check	# if not, repeat to check again
	cmpb	$32,%al
	jl	keyboard_noint_done
	movb	$0x07,%ah		# display it on screen
	movw	%ax,(%ebx)
	addl	$2,%ebx
	jmp	keyboard_noint_check	# go back to get another
keyboard_noint_done:
	movb	$1,%dl			# set the IRQ1 enable
	call	setirq1enable
	jmp	keyboard_irq_ret

setirq1enable:
	inb	$KB_CP			# see if there is already something in the keyboard data buffer
	call	iodelay
	testb	$1,%al
	je	keyboard_noint_done0
	inb	$KB_DP			# if so, read and discard it
	call	iodelay
	jmp	keyboard_noint
keyboard_noint_done0:
	movb	$0x20,%al		# tell it to fetch the 'command byte'
	outb	$KB_CP
	call	iodelay
keyboard_noint_loop1:
	inb	$KB_CP			# wait for it to fetch the 'command byte'
	call	iodelay
	testb	$1,%al
	je	keyboard_noint_loop1
	inb	$KB_DP			# read the 'command byte'
	call	iodelay
	movb	%al,%ah			# clear bit 0 of 'command byte'
	andb	$0xFE,%ah
	orb	%dl,%ah			# now maybe set it
	movb	$0x60,%al		# tell keyboard we want to write 'command byte'
	outb	$KB_CP
	call	iodelay
	movb	%ah,%al			# write the 'command byte'
	outb	$KB_DP
	ret
#
#  All other interrupts come here
#
	.align	4
irq_x_entry:			# irq2..7
	pushal
	movb	$2,%bl
	call	ack_8259
	call	updatestatus
	popal
	iret

	.align	4
irq_y_entry:			# irq8..15
	pushal
	movb	$8,%bl
	call	ack_8259
	call	updatestatus
	popal
	iret

#
#  Ack the interrupt out of the 8259(s)
#
#    Input:
#
#	bl = irq level
#
	.align	4
ack_8259:
	movb	$0x20,%al		# set up the interrupt acknowledge command code
	testb	$8,%bl			# see if master or slave irq
	je	ack_8259_master
	outb	$SLAV_8259_APORT	# slave, acknowledge this irq in both slave and master
	call	iodelay
ack_8259_master:
	outb	$MAST_8259_APORT	# acknowledge this irq in master
	call	iodelay
	ret

#
#  Update status display on screen
#
updatestatus:
	pushfl

	pushl	piix4address
	pushl	$line0_msg
	pushl	$0
	call	updatestatusline
	addl	$12,%esp

	pushl	$line1_msg
	pushl	$1
	call	updatestatusline
	addl	$12,%esp

	pushl	timer_counter
	pushl	$line2_msg
	pushl	$2
	call	updatestatusline
	addl	$12,%esp

	pushl	kb_int_counter
	pushl	$line3_msg
	pushl	$3
	call	updatestatusline
	addl	$12,%esp

	pushl	hlt_counter
	pushl	$line4_msg
	pushl	$4
	call	updatestatusline
	addl	$12,%esp

	xorl	%eax,%eax
	inb	$KB_CP
	call	iodelay
	pushl	%eax
	pushl	$line5_msg
	pushl	$5
	call	updatestatusline
	addl	$12,%esp

	movb	$0x0A,%al
	outb	$SLAV_8259_APORT
	call	iodelay
	inb	$SLAV_8259_APORT
	call	iodelay
	movb	%al,%ah
	movb	$0x0A,%al
	outb	$MAST_8259_APORT
	call	iodelay
	inb	$MAST_8259_APORT
	call	iodelay
	pushl	%eax
	pushl	$line6_msg
	pushl	$6
	call	updatestatusline
	addl	$12,%esp

	movb	$0x0B,%al
	outb	$SLAV_8259_APORT
	call	iodelay
	inb	$SLAV_8259_APORT
	call	iodelay
	movb	%al,%ah
	movb	$0x0B,%al
	outb	$MAST_8259_APORT
	call	iodelay
	inb	$MAST_8259_APORT
	call	iodelay
	pushl	%eax
	pushl	$line7_msg
	pushl	$7
	call	updatestatusline
	addl	$12,%esp

	inb	$SLAV_8259_DPORT
	call	iodelay
	movb	%al,%ah
	inb	$MAST_8259_APORT
	call	iodelay
	pushl	%eax
	pushl	$line8_msg
	pushl	$8
	call	updatestatusline
	addl	$12,%esp

	ret

# Output status line

#   4(%esp) = line number
#   8(%esp) = pointer to message string (use X's at end for hex number)
#  12(%esp) = binary value to be displayed in hex

	.align	4
updatestatusline:
	movl	4(%esp),%eax			# get line number
	movl	$VIDEOLINESIZE,%ecx		# multiply by line size
	mull	%ecx
	leal	VIDEOBASE(%eax),%edi		# point to memory address for that line
	movl	8(%esp),%esi			# point to message string
	movb	$0x07,%ah			# top of word is 07 for white chars on black background
updatestatusline_msgloop:
	movb	(%esi),%al			# get a message character
	testb	%al,%al				# stop outputting if hit terminating null
	je	updatestatusline_msgdone
	movw	%ax,(%edi)			# ok, store in video memory with attribute byte
	incl	%esi				# increment message pointer
	addl	$2,%edi				# increment video pointer
	jmp	updatestatusline_msgloop	# repeat until we hit the null
updatestatusline_msgdone:
	movl	12(%esp),%edx			# get the binary value to output
updatestatusline_hexloop:
	decl	%esi				# decrement message pointer
	cmpb	$'X',(%esi)			# see if we have an 'X'
	jne	updatestatusline_hexdone	# stop if not
	movb	%dl,%al				# ok, get the low hex digit
	andb	$0x0F,%al
	addb	$0x90,%al			# convert hex digit to ascii
	daa
	adcb	$0x40,%al
	daa
	subl	$2,%edi				# store in video buffer with attribute byte
	movw	%ax,(%edi)
	shrl	$4,%edx				# remove the digit from value
	jmp	updatestatusline_hexloop	# repeat until we run out of 'X's
updatestatusline_hexdone:
	ret					# return

line0_msg:	.string	"  piix4 addr: XXXXXXXX"
line1_msg:	.string	"      eflags: XXXXXXXX"
line2_msg:	.string	"timercounter: XXXXXXXX"
line3_msg:	.string	"kbintcounter: XXXXXXXX"
line4_msg:	.string	"  hltcounter: XXXXXXXX"
line5_msg:	.string	"  kb sts reg: XX"
line6_msg:	.string	"     intreqs: XXXX"
line7_msg:	.string	"     intserv: XXXX"
line8_msg:	.string	"     intmask: XXXX"


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
## C-callable routine to read a byte
##

	.align	4
	.globl	oz_hw_inb
oz_hw_inb:
	movl	4(%esp),%edx
	xorl	%eax,%eax
	inb	%dx
	call	iodelay
	ret

	.align	4
	.globl	oz_hw_outb
oz_hw_outb:
	movl	4(%esp),%eax
	movl	8(%esp),%edx
	outb	%dx
	call	iodelay
	ret

##
## Delay a very short time after doing an I/O
##
iodelay:
	pushl	%eax
	movl	$200,%ax
iodelay_loop:
	decl	%eax
	jne	iodelay_loop
	popl	%eax
	ret

##########################################################################
##									##
##  Default exception / interrupt handler				##
##									##
##  It just prints out the registers then hangs				##
##									##
##  These are called with an extra byte on the stack containing the 	##
##  vector number							##
##									##
##########################################################################

	.data

handler_entry_count:	.byte	1		# allow it to execute just once

	.text

handler_except_msg:
	.byte	13,10
	.ascii	"oz_common_486: exception "
	.byte	-1
	.byte	0

handler_regs_msg:
	.byte	13,10
	.ascii	"  eax="
	.byte	-4
	.ascii	"  ebx="
	.byte	-4
	.ascii	"  ecx="
	.byte	-4
	.ascii	"  edx="
	.byte	-4
	.byte	13,10
	.ascii	"  esi="
	.byte	-4
	.ascii	"  edi="
	.byte	-4
	.ascii	"  ebp="
	.byte	-4
	.ascii	"  esp="
	.byte	-4
	.byte	13,10
	.ascii	"  ds="
	.byte	-2
	.ascii	"  es="
	.byte	-2
	.ascii	"  fs="
	.byte	-2
	.ascii	"  gs="
	.byte	-2
	.byte	0

handler_PF_msg:
	.byte	13,10
	.ascii	"  cr2="
	.byte	-4
	.ascii	"  ec="
	.byte	-4
	.byte	0

handler_ec_msg:
	.byte	13,10
	.ascii	"  ec="
	.byte	-4
	.byte	0

handler_from_msg_knl:
	.byte	13,10
	.ascii	"  eip="
	.byte	-4
	.ascii	"  cs="
	.byte	-4
	.ascii	"  ef="
	.byte	-4
	.byte	0

handler_from_msg_outer:
	.byte	13,10
	.ascii	"  eip="
	.byte	-4
	.ascii	"  cs="
	.byte	-4
	.ascii	"  ef="
	.byte	-4
	.ascii	"  xsp="
	.byte	-4
	.ascii	"  xss="
	.byte	-4
	.byte	0

handler_tracemsg:
	.byte	13,10
	.ascii	"  "
	.byte	-4
	.ascii	": "
	.byte	-4
	.ascii	"  "
	.byte	-4
	.byte	0

handler_dumpmem_msg:
	.byte	13,10
	.ascii	"  "
	.byte	-4
	.ascii	" "
	.byte	-4
	.ascii	" "
	.byte	-4
	.ascii	" "
	.byte	-4
	.ascii	" : "
	.byte	-4
	.ascii	" > "
	.byte	0

##########################################################################
##									##
##  Fill in the IDT with default handlers				##
##									##
##  These handlers execute in kernel mode with interrupts disabled	##
##  They dump out the registers and a traceback then hang in a loop 	##
##  forever								##
##									##
##    Input:								##
##									##
##	edi = points to 256*16 byte area for idt and pushb/jmp's	##
##									##
##    Scratch:								##
##									##
##	eax,ebx,ecx,esi,edi						##
##									##
##########################################################################

	.align	4
	.globl	oz_hw_idt_init
oz_hw_idt_init:
	movl	$KNLCODESEG*65536,%eax	# put KNLCODESEG in top of eax
	movl	$256,%ecx		# get number of entries to fill in
	leal	256*8(%edi),%esi	# point to where the pushb/jmp's will go
idt_init_loop1:
	movw	%si,%ax			# %eax<00:15> = low 16 bits of pushb/jmp vector entry
	movl	%esi,%ebx		# %ebx<16:31> = high 16 bits of pushb/jmp vector entry
	movw	$0x8e00,%bx		# %ebx<00:15> = vector type : kernel mode, 32-bit, interrupts disabled
	movl	%eax,0(%edi)		# store in table
	movl	%ebx,4(%edi)
	addl	$8,%edi			# increment pointers
	addl	$8,%esi
	loop	idt_init_loop1		# repeat if more to do

	leal	handler,%esi		# point to the handler
	xorb	%al,%al			# start with vector number zero
	movl	$-7,%ebx		# make displacement for first jmp
	subl	%edi,%ebx
	addl	%esi,%ebx
	movl	$256,%ecx		# get number of IDT vectors = number of pushb/jmp's to fill in
	movl	%edi,%esi		# save beginning of where the pushb/jmp's are
idt_init_loop2:
	movb	$0x6A,0(%edi)		# store pushb imm8 opcode
	movb	%al,1(%edi)		# store imm8 value = vector number
	movb	$0xE9,2(%edi)		# store jmp rel32 opcode
	movl	%ebx,3(%edi)		# store displacement from '7(%edi)' to 'handler'
	movb	$0,7(%edi)		# clear out the unused byte
	incb	%al			# increment vector number
	subl	$8,%ebx			# decrement offset from pushb/jmp to 'handler' entrypoint
	addl	$8,%edi			# increment pointer for next pushb/jmp
	loop	idt_init_loop2		# repeat for all 256 pushb/jmp's to fill in

	movl	$handler_ec-handler,%ecx # get displacement from one entrypoint to the other
	addl	%ecx, 8*8+3(%esi)	# modify vector  8 for error-code style handler
	addl	%ecx,10*8+3(%esi)	# modify vector 10 for error-code style handler
	addl	%ecx,11*8+3(%esi)	# modify vector 11 for error-code style handler
	addl	%ecx,12*8+3(%esi)	# modify vector 12 for error-code style handler
	addl	%ecx,13*8+3(%esi)	# modify vector 13 for error-code style handler
	addl	$handler_PF-handler,14*8+3(%esi) # modify vector 14 for pagefault style handler
	addl	%ecx,17*8+3(%esi)	# modify vector 17 for error-code style handler
	addl	%ecx,18*8+3(%esi)	# modify vector 18 for error-code style handler
	ret

##########################################################################
##									##
##  pagefault exception							##
##  different from the others only to print out cr2			##
##									##
##########################################################################

handler_PF:
	decb	handler_entry_count		# hang if error printing error message
handler_PF_hang:
	js	handler_PF_hang
	call	handler_printregs		# print out the registers
	movl	%cr2,%eax			# get virtual address
	movl	%eax,(%esp)			# store over vector number
	leal	handler_PF_msg,%esi		# print out the virtual address and error code
	movl	%esp,%edi
	call	handler_print
	pop	%edx				# wipe error code from stack
	jmp	handler_printfrom		# print where the error came from

##########################################################################
##									##
##  exception with error code						##
##									##
##########################################################################

handler_ec:					# exception with error code
	decb	handler_entry_count		# hang if error printing error message
handler_ec_hang:
	js	handler_ec_hang
	call	handler_printregs		# print out the registers
	leal	handler_ec_msg,%esi		# print out the error code
	leal	4(%esp),%edi
	call	handler_print
	pop	%edx				# wipe error code from stack
	jmp	handler_printfrom		# print where the error came from

##########################################################################
##									##
##  exception without error code					##
##									##
##########################################################################

handler:
	decb	handler_entry_count		# hang if error printing error message
handler_hang:
	js	handler_hang
	call	handler_printregs		# print out the registers

#
#  Common handler routine
#
handler_printfrom:
	addl	$4,%esp				# pop exception number from stack to be nice
	leal	handler_from_msg_knl,%esi	# point to message for EIP, etc
	cmpw	$KNLCODESEG,4(%esp)		# see if faulting from kernel mode
	je	handler_printfromprint
	leal	handler_from_msg_outer,%esi	# point to message for EIP, etc, include outer esp, ss
handler_printfromprint:
	movl	%esp,%edi			# point to the EIP, etc
	call	handler_print			# print it
	pushl	%ebp				# save ebp
	movl	$12,%ecx			# print at most this many frames
handler_printtrace:
	movl	%ebp,%eax			# see if end of trace dump
	testl	%eax,%eax
	je	handler_printtrace_done
	leal	handler_tracemsg,%esi		# if not, trace a frame
	pushl	%ecx
	pushl	4(%ebp)
	pushl	0(%ebp)
	pushl	%ebp
	movl	%esp,%edi
	call	handler_print
	addl	$12,%esp
	popl	%ecx
	movl	(%ebp),%ebp			# point to next frame
	loop	handler_printtrace		# maybe go dump it out
handler_printtrace_done:
	popl	%ebp				# restore ebp
handler_dumpmem:
	pushl	%ebp				# dump 16 bytes @%ebp
	pushl	  (%ebp)
	pushl	 4(%ebp)
	pushl	 8(%ebp)
	pushl	12(%ebp)
	leal	handler_dumpmem_msg,%esi
	movl	%esp,%edi
	call	handler_print
	addl	$20,%esp
	addl	$16,%ebp			# increment %ebp to next 16
handler_stop:
	inb	$0x64				# see if keyboard char present
	call	iodelay
	testb	$1,%al
	je	handler_stop
	inb	$0x60				# if so, read it
	call	iodelay
	jmp	handler_dumpmem			# then dump another line
##
##  Print out all the registers - be sure to leave %ebp intact
##
handler_printregs:
	pushw	%gs				# push all registers on stack so we can print them
	pushw	%fs
	pushw	%es
	pushw	%ds
	pushl	%esp
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%eax
	movw	$KNLDATASEG,%ax			# in case data segment register got wacked
	movw	%ax,%ds
	leal	handler_except_msg,%esi		# print exception message
	leal	44(%esp),%edi			# - including exception number
	call	handler_print
	leal	handler_regs_msg,%esi		# point to beginning of text string
	movl	%esp,%edi			# point to where the registers are stored
	call	handler_print			# print out registers
	movl	24(%esp),%ebp			# done, restore ebp
	addl	$40,%esp			# wipe values from stack
	ret
##
##  Print out a list of registers with format string
##
##    Input:
##
##	esi = format string pointer
##	edi = register list pointer
##
handler_print:
	movb	(%esi),%al			# get a byte from the text string
	incl	%esi				# increment text string pointer
	testb	%al,%al				# test the text string byte
	js	handler_print_value		# negative means output value from stack
	je	handler_print_done		# zero means we're done
	pushl	%eax				# positive, print out the character
	pushl	$0
	call	oz_dev_video_putchar
	addl	$8,%esp
	jmp	handler_print			# repeat until end of text string
handler_print_value:
	orl	$0xffffff00,%eax		# extend al to eax
	subl	%eax,%edi			# this really increments edi to next value on stack
	movl	%eax,%ecx			# move it here out of the way
	pushl	%edi				# save address of next value
handler_print_value_loop:
	decl	%edi				# get (next) most significant byte from value on stack
	movb	(%edi),%al			# print out the byte
	call	oz_hw_print_hexb
	incb	%cl				# see if there are more bytes in the value
	jne	handler_print_value_loop	# repeat if more to do
	popl	%edi				# pop pointer to next value on stack
	jmp	handler_print			# resume processing text string
handler_print_done:
	ret

##########################################################################
##									##
##  Print hex values in eax - save and restore all scratch registers	##
##									##
##########################################################################

##
## Print a long value in hex
##
##   Input:
##
##	eax = long to print in hex
##
	.globl	oz_hw_print_hexl

oz_hw_print_hexl:
	roll	$16,%eax
	call	oz_hw_print_hexw
	roll	$16,%eax
##
## Print a word value in hex
##
##   Input:
##
##	ax = word to print in hex
##
	.globl	oz_hw_print_hexw

oz_hw_print_hexw:
	rolw	$8,%ax
	call	oz_hw_print_hexb
	rolw	$8,%ax
##
## Print a byte value in hex
##
##   Input:
##
##	al = byte to print in hex
##
	.globl	oz_hw_print_hexb

oz_hw_print_hexb:
	pushl	%edi		# save everything that gets zapped
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	movl	$2,%ecx		# print 2 digits
print_hexb_loop:
	rolb	$4,%al		# get digit to print in ax<0:3>
	pushl	%ecx		# save counter and value on stack
	pushl	%eax
	andb	$0x0f,%al	# and out the flags
	addb	$0x90,%al	# convert to ascii
	daa
	adcb	$0x40,%al
	daa
	pushl	%eax		# output to screen
	pushl	$0
	call	oz_dev_video_putchar
	addl	$8,%esp
	popl	%eax		# restore value and counter from stack
	popl	%ecx
	loop	print_hexb_loop	# repeat if more digits to print
	popl	%ebx		# restore the stuff we saved
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	ret
