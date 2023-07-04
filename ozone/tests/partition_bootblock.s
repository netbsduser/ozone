moveto = 0x0600
bootbase = 0x7c00

	.text

x:
	cli			!000	.byte	0xFA 	! inhibit interrupt delivery
	xor	ax,ax		!001	.byte	0x33	! set stack segment to zero
				!002	.byte	0xC0
	mov	ss,ax		!003	.byte	0x8E
				!004	.byte	0xD0
	mov	sp,#bootbase	!005	.byte	0xBC	! set stack pointer just below this block
				!006	.byte	0x00
				!007	.byte	0x7C
	mov	si,sp		!008	.byte	0x8B	! set source pointer to this block
				!009	.byte	0xF4
	push	ax		!00a	.byte	0x50	! clear es and ds registers
	pop	es		!00b	.byte	0x07
	push	ax		!00c	.byte	0x50
	pop	ds		!00d	.byte	0x1F
	sti			!00e	.byte	0xFB	! enable interrupt delivery
	cld			!00f	.byte	0xFC	! copy forward
	mov	di,#moveto	!010	.byte	0xBF	! copy boot block to memory address 0600
				!011	.byte	0x00
				!012	.byte	0x06
	mov	cx,#0x0100	!013	.byte	0xB9	! copy 256 words = 512 bytes
				!014	.byte	0x00
				!015	.byte	0x01
	repnz			!016	.byte	0xF2
	movs			!017	.byte	0xA5
	jmpi	0,moved-x+moveto !018	.byte	0xEA	! jump to new address
				!019	.byte	0x1D
				!01a	.byte	0x06
				!01b	.byte	0x00
				!01c	.byte	0x00
moved:
	mov	si,#partitions-x+moveto !01d .byte 0xBE	! point to partition table
				!01e	.byte	0xBE
				!01f	.byte	0x07
	mov	bl,#0x04	!020	.byte	0xB3	! up to four partitions in table
				!021	.byte	0x04
checkentry:
	cmpb	(si),#0x80	!022	.byte	0x80	! first byte 0x80 indicates bootable
				!023	.byte	0x3C
				!024	.byte	0x80
	je	bootable	!025	.byte	0x74
				!026	.byte	0x0E
	cmpb	(si),#0x00	!027	.byte	0x80	! first byte 0x00 indicates non-bootable
				!028	.byte	0x3C
				!029	.byte	0x00
	jne	invpart		!02a	.byte	0x75	! all else is illegal
				!02b	.byte	0x1C
	add	si,#0x10	!02c	.byte	0x83	! increment to next entry in partition table
				!02d	.byte	0xC6
				!02e	.byte	0x10
	dec	bl		!02f	.byte	0xFE
				!030	.byte	0xCB
	jne	checkentry	!031	.byte	0x75
				!032	.byte	0xEF
	int	#0x18		!033	.byte	0xCD	! no bootable partition - ??
				!034	.byte	0x18
bootable:
	mov	dx,(si)		!035	.byte	0x8B	! set up disk read parameters
				!036	.byte	0x14
	mov	cx,0x02(si)	!037	.byte	0x8B
				!038	.byte	0x4C
				!039	.byte	0x02
	mov	bp,si		!03a	.byte	0x8B	! save table entry pointer
				!03b	.byte	0xEE
checkmult:
	add	si,#0x10	!03c	.byte	0x83	! point to next table entry
				!03d	.byte	0xC6
				!03e	.byte	0x10
	dec	bl		!03f	.byte	0xFE	! see if this is last entry
				!040	.byte	0xCB
	je	readit		!041	.byte	0x74	! if so, table is ok, go read in boot block
				!042	.byte	0x1A
	cmpb	(si),#0x00	!043	.byte	0x80	! see if next one is bootable
				!044	.byte	0x3C
				!045	.byte	0x00
	je	checkmult	!046	.byte	0x74	! if not, repeat to check them all
				!047	.byte	0xF4
invpart:
	mov	si,#invparmsg-x+moveto  !048 .byte 0xBE	! print "Invalid partition table"
				!049	.byte	0x8B
				!04a	.byte	0x06
printmsg:
	lodsb			!04b	.byte	0xAC	! get character of message and increment pointer
	cmp	al,#0x00	!04c	.byte	0x3C	! see if it is a zero byte
				!04d	.byte	0x00
	je	hanghere	!04e	.byte	0x74	! if so, we're done outputting message
				!04f	.byte	0x0B
	push	si		!050	.byte	0x56	! save message pointer
	mov	bx,#0x0007	!051	.byte	0xBB	! output character
				!052	.byte	0x07
				!053	.byte	0x00
	mov	ah,#0x0e	!054	.byte	0xB4
				!055	.byte	0x0E
	int	#0x10		!056	.byte	0xCD
				!057	.byte	0x10
	pop	si		!058	.byte	0x5E	! restore message pointer
	jmp	printmsg	!059	.byte	0xEB	! repeat for next character
				!05a	.byte	0xF0
hanghere:
	jmp	hanghere	!05b	.byte	0xEB
				!05c	.byte	0xFE
readit:
	mov	di,#0x0005	!05d	.byte	0xBF	! set up retry count
				!05e	.byte	0x05
				!05f	.byte	0x00
readretry:
	mov	bx,#0xbootbase	!060	.byte	0xBB	! where to read into
				!061	.byte	0x00
				!062	.byte	0x7C
	mov	ax,#0x0201	!063	.byte	0xB8	! ah=2 means read, al=1 means one block
				!064	.byte	0x01
				!065	.byte	0x02
	push	di		!066	.byte	0x57	! save retry count
	int	#0x13		!067	.byte	0xCD	! read the sector
				!068	.byte	0x13
	pop	di		!069	.byte	0x5F	! restore retry count
	jnc	readok		!06a	.byte	0x73	! jump if read was successful
				!06b	.byte	0x0C
	xor	ax,ax		!06c	.byte	0x33	! reset drive ??
				!06d	.byte	0xC0
	int	#0x13		!06e	.byte	0xCD
				!06f	.byte	0x13
	dec	di		!070	.byte	0x4F	! decrement retry counter
	jne	readretry	!071	.byte	0x75	! repeat if more to go
				!072	.byte	0xED
	mov	si,#readerr-x+moveto !073 .byte	0xBE	! print read error message
				!074	.byte	0xA3
				!075	.byte	0x06
	jmp	printmsg	!076	.byte	0xEB
				!077	.byte	0xD3
readok:
	mov	si,#misopsys-x+moveto !078 .byte 0xBE	! block read in, set up err msg pointer
				!079	.byte	0xC2
				!07a	.byte	0x06
	mov	di,#flagword-x+bootbase !07b .byte 0xBF	! point to where flag word should be
				!07c	.byte	0xFE
				!07d	.byte	0x7D
	cmp	(di),#0xAA55	!07e	.byte	0x81	! see if flag word is ok
				!07f	.byte	0x3D
				!080	.byte	0x55
				!081	.byte	0xAA
	jne	printmsg	!082	.byte	0x75	! if not, print error message (Missing operating system)
				!083	.byte	0xC7
	mov	si,bp		!084	.byte	0x8B	! restore partition table entry pointer
				!085	.byte	0xF5
	jmpi	0,bootbase	!086	.byte	0xEA	! jump to partition's boot block
				!087	.byte	0x00
				!088	.byte	0x7C
				!089	.byte	0x00
				!08a	.byte	0x00

invparmsg:	.ascii	"Invalid partition table"
		.byte	0
readerr:	.ascii	"Error loading operating system"
		.byte	0
misopsys:	.ascii	"Missing operating system"
		.byte	0

	.org	x+512-64-2

partitions:
partition1:	.byte	0x80		! 80 = bootable
		.byte	0x01		! beginning track
		.byte	0x01		! beginning sector
		.byte	0x00		! beginning cylinder
		.byte	0x83		! filesystem type id
		.byte	0xFE		! ending track
		.byte	0x3F		! ending sector
		.byte	0x3F		! ending cylinder
		.long	0x0000003F	! beginning block number
		.long	0x000FB001	! number of blocks
partition2:	.byte	0x00		! 00 = not bootable
		.byte	0x00		! beginning track
		.byte	0x01		! beginning sector
		.byte	0x40		! beginning cylinder
		.byte	0x83		! filesystem type id
		.byte	0xFE		! ending track
		.byte	0x7F		! ending sector
		.byte	0x06		! ending cylinder
		.long	0x000FB040	! beginning block number
		.long	0x0030C807	! number of blocks
partition3:	.long	0,0,0,0
partition4:	.long	0,0,0,0

flagword:	.word	0xAA55
