//+++2003-03-01
//    Copyright (C) 2001,2002,2003  Mike Rieker, Beverly, MA USA
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; version 2 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//---2003-03-01

/* $Id: rtl8139.c,v 1.1 2002/08/17 17:08:32 pavlovskii Exp $ */

#include "ozone.h"
#include "oz_dev_pci.h"
#include "oz_knl_hw.h"
#include "oz_knl_image.h"
#include "oz_knl_kmalloc.h"
#include "oz_knl_phymem.h"
#include "oz_knl_sdata.h"
#include "oz_knl_status.h"
#include "oz_sys_dateconv.h"

/*--beg of OZONE added stuff--*/

#define in oz_hw_inb
#define in16 oz_hw_inw
#define in32 oz_hw_inl
#define out(port,data) oz_hw_outb (data, port)
#define out16(port,data) oz_hw_outw (data, port)
#define out32(port,data) oz_hw_outl (data, port)
#define wprintf oz_knl_printk
#define SysUpTime() ((oz_hw_tod_getnow () - oz_s_boottime) / (OZ_TIMER_RESOLUTION / 1000))
#define _countof(array) ((sizeof array) / (sizeof *(array)))

#define addr_t OZ_Phyaddr
#define bool int
#define spinlock_t OZ_Smplock
#define size_t uLong
#define uint16_t uWord
#define uint32_t uLong
#define uint8_t uByte

#define ETH_ZLEN 64
#define ETH_FRAME_LEN 1518

	/* Alloc physically contig memory, return physical address */

addr_t MemAlloc (size_t sizeinbytes)

{
  OZ_Mempage numpages, phypages;

  numpages = (sizeinbytes + (1 << OZ_HW_L2PAGESIZE) - 1) >> OZ_HW_L2PAGESIZE;
  phypages = oz_knl_phymem_allocontig (numpages, OZ_PHYMEM_PAGESTATE_ALLOCSECT, 0);
  if (phypages == OZ_PHYPAGE_NULL) oz_crash ("tim_rtl8139: failed to alloc %u phy pages", numpages);
  return (phypages << OZ_HW_L2PAGESIZE);
}

	/* Alloc system pagetable entries, return virtual address that they map */

uint8_t *sbrk_virtual (size_t sizeinbytes)

{
  OZ_Mempage numpages;
  uLong sts;
  void *sysvaddr;

  numpages = (sizeinbytes + (1 << OZ_HW_L2PAGESIZE) - 1) >> OZ_HW_L2PAGESIZE;
  sts = oz_knl_spte_alloc (numpages, &sysvaddr, NULL, NULL);
  if (sts != OZ_SUCCESS) oz_crash ("tim_rtl8139: error %u allocating %u sptes", sts, numpages);
  return (sysvaddr);
}

	/* Map physical pages to system pagetable entries */

#define PRIV_KERN 1
#define PRIV_PRES 2
#define PRIV_RD   4
#define PRIV_WR   8

void MemMap (addr_t virtaddr, addr_t physaddr, addr_t endvaexc, uLong privs)

{
  OZ_Mempage ipage, npage, ppage, vpage;

  if (virtaddr & ((1 << OZ_HW_L2PAGESIZE) - 1)) oz_crash ("tim_rtl8139: bad virtaddr %X", virtaddr);
  if (physaddr & ((1 << OZ_HW_L2PAGESIZE) - 1)) oz_crash ("tim_rtl8139: bad physaddr %X", physaddr);
  if (endvaexc <= virtaddr) oz_crash ("tim_rtl8139: bad endvaexc %X", endvaexc);

  ppage = physaddr >> OZ_HW_L2PAGESIZE;
  vpage = virtaddr >> OZ_HW_L2PAGESIZE;
  npage = ((endvaexc - 1) >> OZ_HW_L2PAGESIZE) - vpage + 1;
  for (ipage = 0; ipage < npage; ipage ++) {
    oz_hw_pte_writeall (vpage + ipage, OZ_SECTION_PAGESTATE_VALID_W, ppage + ipage, OZ_HW_PAGEPROT_KW, OZ_HW_PAGEPROT_NA);
  }
}

/*--end of OZONE added stuff--*/

typedef struct rxpacket_t rxpacket_t;
struct rxpacket_t
{
    rxpacket_t *prev, *next;
    size_t length;
    uint16_t type;
    uint8_t data[1];
};

typedef struct txpacket_t txpacket_t;
struct txpacket_t
{
    int buffer;
    unsigned timeout;
};

typedef struct rtl8139_t rtl8139_t;
struct rtl8139_t
{

    OZ_Smplock *smplock;

    uint16_t iobase;
    uint8_t station_address[6];
    bool speed10;
    bool fullduplex;
    unsigned cur_tx;
    unsigned cur_rx;

    addr_t rx_phys, tx_phys;
    uint8_t *rx_ring, *tx_ring;

    rxpacket_t *packet_first, *packet_last;

    OZ_Hw486_irq_many irq_many;
};

/* PCI Tuning Parameters
   Threshold is bytes transferred to chip before transmission starts. */
#define TX_FIFO_THRESH      256             /* In bytes, rounded down to 32 byte units. */
#define RX_FIFO_THRESH      4               /* Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST        4               /* Maximum PCI burst, '4' is 256 bytes */
#define TX_DMA_BURST        4               /* Calculate as 16<<val. */
#define NUM_TX_DESC         4               /* Number of Tx descriptor registers. */
#define TX_BUF_SIZE         ETH_FRAME_LEN   /* FCS is added by the chip */
#define RX_BUF_LEN_IDX      0               /* 0, 1, 2 is allowed - 8,16,32K rx buffer */
#define RX_BUF_LEN          (8192 << RX_BUF_LEN_IDX)

/* Symbolic offsets to registers. */
enum RTL8139_registers
{
    MAC0=0,             /* Ethernet hardware address. */
    MAR0=8,             /* Multicast filter. */
    TxStatus0=0x10,     /* Transmit status (four 32bit registers). */
    TxAddr0=0x20,       /* Tx descriptors (also four 32bit). */
    RxBuf=0x30, RxEarlyCnt=0x34, RxEarlyStatus=0x36,
    ChipCmd=0x37, RxBufPtr=0x38, RxBufAddr=0x3A,
    IntrMask=0x3C, IntrStatus=0x3E,
    TxConfig=0x40, RxConfig=0x44,
    Timer=0x48,         /* general-purpose counter. */
    RxMissed=0x4C,      /* 24 bits valid, write clears. */
    Cfg9346=0x50, Config0=0x51, Config1=0x52,
    TimerIntrReg=0x54,  /* intr if gp counter reaches this value */
    MediaStatus=0x58,
    Config3=0x59,
    MultiIntr=0x5C,
    RevisionID=0x5E,    /* revision of the RTL8139 chip */
    TxSummary=0x60,
    MII_BMCR=0x62, MII_BMSR=0x64, NWayAdvert=0x66, NWayLPAR=0x68,
    NWayExpansion=0x6A,
    DisconnectCnt=0x6C, FalseCarrierCnt=0x6E,
    NWayTestReg=0x70,
    RxCnt=0x72,         /* packet received counter */
    CSCR=0x74,          /* chip status and configuration register */
    PhyParm1=0x78,TwisterParm=0x7c,PhyParm2=0x80,   /* undocumented */
    /* from 0x84 onwards are a number of power management/wakeup frame
     * definitions we will probably never need to know about.  */
};

enum ChipCmdBits
{
    CmdReset=0x10, CmdRxEnb=0x08, CmdTxEnb=0x04, RxBufEmpty=0x01,
};

/* Interrupt register bits, using my own meaningful names. */
enum IntrStatusBits
{
    PCIErr=0x8000, PCSTimeout=0x4000, CableLenChange= 0x2000,
    RxFIFOOver=0x40, RxUnderrun=0x20, RxOverflow=0x10,
    TxErr=0x08, TxOK=0x04, RxErr=0x02, RxOK=0x01,
    IntrDefault = RxOK | TxOK,
};

enum TxStatusBits
{
    TxHostOwns=0x2000, TxUnderrun=0x4000, TxStatOK=0x8000,
    TxOutOfWindow=0x20000000, TxAborted=0x40000000,
    TxCarrierLost=0x80000000,
};

enum RxStatusBits
{
    RxMulticast=0x8000, RxPhysical=0x4000, RxBroadcast=0x2000,
    RxBadSymbol=0x0020, RxRunt=0x0010, RxTooLong=0x0008, RxCRCErr=0x0004,
    RxBadAlign=0x0002, RxStatusOK=0x0001,
};

enum MediaStatusBits
{
    MSRTxFlowEnable=0x80, MSRRxFlowEnable=0x40, MSRSpeed10=0x08,
    MSRLinkFail=0x04, MSRRxPauseFlag=0x02, MSRTxPauseFlag=0x01,
};

enum MIIBMCRBits
{
    BMCRReset=0x8000, BMCRSpeed100=0x2000, BMCRNWayEnable=0x1000,
    BMCRRestartNWay=0x0200, BMCRDuplex=0x0100,
};

enum CSCRBits
{
    CSCR_LinkOKBit=0x0400, CSCR_LinkChangeBit=0x0800,
    CSCR_LinkStatusBits=0x0f000, CSCR_LinkDownOffCmd=0x003c0,
    CSCR_LinkDownCmd=0x0f3c0,
};

/* Bits in RxConfig. */
enum rx_mode_bits
{
    RxCfgWrap=0x80,
    AcceptErr=0x20, AcceptRunt=0x10, AcceptBroadcast=0x08,
    AcceptMulticast=0x04, AcceptMyPhys=0x02, AcceptAllPhys=0x01,
};

/* Serial EEPROM section. */

/*  EEPROM_Ctrl bits. */
#define EE_SHIFT_CLK    0x04    /* EEPROM shift clock. */
#define EE_CS           0x08    /* EEPROM chip select. */
#define EE_DATA_WRITE   0x02    /* EEPROM chip data in. */
#define EE_WRITE_0      0x00
#define EE_WRITE_1      0x02
#define EE_DATA_READ    0x01    /* EEPROM chip data out. */
#define EE_ENB          (0x80 | EE_CS)

/*
    Delay between EEPROM clock transitions.
    No extra delay is needed with 33Mhz PCI, but 66Mhz may change this.
*/

#define eeprom_delay()  in32(ee_addr)

/* The EEPROM commands include the alway-set leading bit. */
#define EE_WRITE_CMD    (5 << 6)
#define EE_READ_CMD     (6 << 6)
#define EE_ERASE_CMD    (7 << 6)

static unsigned RtlReadEeprom(rtl8139_t *rtl, unsigned location)
{
    int i;
    unsigned int retval = 0;
    long ee_addr = rtl->iobase + Cfg9346;
    int read_cmd = location | EE_READ_CMD;

    out(ee_addr, EE_ENB & ~EE_CS);
    out(ee_addr, EE_ENB);

    /* Shift the read command bits out. */
    for (i = 10; i >= 0; i--)
    {
        int dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
        out(ee_addr, EE_ENB | dataval);
        eeprom_delay();
        out(ee_addr, EE_ENB | dataval | EE_SHIFT_CLK);
        eeprom_delay();
    }

    out(ee_addr, EE_ENB);
    eeprom_delay();

    for (i = 16; i > 0; i--)
    {
        out(ee_addr, EE_ENB | EE_SHIFT_CLK);
        eeprom_delay();
        retval = (retval << 1) | ((in(ee_addr) & EE_DATA_READ) ? 1 : 0);
        out(ee_addr, EE_ENB);
        eeprom_delay();
    }

    /* Terminate the EEPROM access. */
    out(ee_addr, ~EE_CS);
    return retval;
}

void RtlReset(rtl8139_t *rtl)
{
    unsigned i;

    out(rtl->iobase + ChipCmd, CmdReset);

    rtl->cur_rx = 0;
    rtl->cur_tx = 0;

    /* Give the chip 10ms to finish the reset. */
    i = SysUpTime() + 10;
    while ((in(rtl->iobase + ChipCmd) & CmdReset) != 0 && 
        SysUpTime() < i)
        /* wait */;

    for (i = 0; i < _countof(rtl->station_address); i++)
        out(rtl->iobase + MAC0 + i, rtl->station_address[i]);

    /* Must enable Tx/Rx before setting transfer thresholds! */
    out(rtl->iobase + ChipCmd, CmdRxEnb | CmdTxEnb);
    out32(rtl->iobase + RxConfig, 
        (RX_FIFO_THRESH<<13) | (RX_BUF_LEN_IDX<<11) | (RX_DMA_BURST<<8)); /* accept no frames yet!  */
    out32(rtl->iobase + TxConfig, (TX_DMA_BURST<<8)|0x03000000);

    /* The Linux driver changes Config1 here to use a different LED pattern
     * for half duplex or full/autodetect duplex (for full/autodetect, the
     * outputs are TX/RX, Link10/100, FULL, while for half duplex it uses
     * TX/RX, Link100, Link10).  This is messy, because it doesn't match
     * the inscription on the mounting bracket.  It should not be changed
     * from the configuration EEPROM default, because the card manufacturer
     * should have set that to match the card.  */

    out32(rtl->iobase + RxBuf, rtl->rx_phys);

    /* Start the chip's Tx and Rx process. */
    out32(rtl->iobase + RxMissed, 0);
    /* set_rx_mode */
    out(rtl->iobase + RxConfig, AcceptBroadcast|AcceptMyPhys|AcceptAllPhys);
    /* If we add multicast support, the MAR0 register would have to be
     * initialized to 0xffffffffffffffff (two 32 bit accesses).  Etherboot
     * only needs broadcast (for ARP/RARP/BOOTP/DHCP) and unicast.  */
    out(rtl->iobase + ChipCmd, CmdRxEnb | CmdTxEnb);

    /* Disable all known interrupts by setting the interrupt mask. */
    //out16(rtl->iobase + IntrMask, 0);
    out16(rtl->iobase + IntrMask, IntrDefault);
}

void RtlInit(rtl8139_t *rtl)
{
    unsigned i, sl;

    wprintf("rtl8139: resetting... ");

    /* Bring the chip out of low-power mode. */
    out(rtl->iobase + Config1, 0x00);

    if (RtlReadEeprom(rtl, 0) != 0xffff)
    {
        unsigned short *ap = (unsigned short*)rtl->station_address;
        for (i = 0; i < 3; i++)
            *ap++ = RtlReadEeprom(rtl, i + 7);
    }
    else
    {
        unsigned char *ap = (unsigned char*)rtl->station_address;
        for (i = 0; i < 6; i++)
            *ap++ = in(rtl->iobase + MAC0 + i);
    }

    rtl->speed10 = (in(rtl->iobase + MediaStatus) & MSRSpeed10) != 0;
    rtl->fullduplex = (in16(rtl->iobase + MII_BMCR) & BMCRDuplex) != 0;
    wprintf("rtl8139: %sMbps %s-duplex\n", 
        rtl->speed10 ? "10" : "100",
        rtl->fullduplex ? "full" : "half");

    rtl->rx_phys = MemAlloc(RX_BUF_LEN);
    rtl->tx_phys = MemAlloc(TX_BUF_SIZE);

    rtl->rx_ring = sbrk_virtual(RX_BUF_LEN);
    rtl->tx_ring = sbrk_virtual(TX_BUF_SIZE);

    wprintf("rtl8139: rx_ring = %p, tx_ring = %p\n",
        rtl->rx_ring, rtl->tx_ring);

    MemMap((addr_t) rtl->rx_ring, 
        rtl->rx_phys, 
        (addr_t) rtl->rx_ring + RX_BUF_LEN,
        PRIV_RD | PRIV_PRES | PRIV_KERN);
    MemMap((addr_t) rtl->tx_ring, 
        rtl->tx_phys, 
        (addr_t) rtl->tx_ring + TX_BUF_SIZE,
        PRIV_WR | PRIV_PRES | PRIV_KERN);

    sl = oz_hw_smplock_wait (rtl -> smplock);
    RtlReset(rtl);
    oz_hw_smplock_clr (rtl -> smplock, sl);
}

void RtlHandleRx(rtl8139_t *rtl)
{
    uint32_t ring_offs, rx_size, rx_status;

    ring_offs = rtl->cur_rx % RX_BUF_LEN;
    rx_status = *(uint32_t*) (rtl->rx_ring + ring_offs);
    rx_size = rx_status >> 16;
    rx_status &= 0xffff;

    oz_knl_printk ("tim_rtl8139*: ring_offs %X, rx_status %X, rx_size %X\n", ring_offs, rx_status, rx_size);

    if ((rx_status & (RxBadSymbol | RxRunt | RxTooLong | RxCRCErr | RxBadAlign)) ||
        (rx_size < ETH_ZLEN) || 
        (rx_size > ETH_FRAME_LEN + 4))
    {
        wprintf("rx error 0x%x\n", rx_status);
        RtlReset(rtl);  /* this clears all interrupts still pending */
        return;
    }

    /* Received a good packet */
    if (ring_offs + 4 + rx_size - 4 > RX_BUF_LEN)
    {
        int semi_count = RX_BUF_LEN - ring_offs - 4;

        oz_knl_dumpmem (semi_count, rtl->rx_ring + ring_offs + 4);
        oz_knl_dumpmem (rx_size - 4 - semi_count, rtl->rx_ring);
    }
    else
    {
        oz_knl_dumpmem (rx_size - 4, rtl->rx_ring + ring_offs + 4);
    }

    rtl->cur_rx = (rtl->cur_rx + rx_size + 4 + 3) & ~3;
    out16(rtl->iobase + RxBufPtr, rtl->cur_rx - 16);

}

void RtlHandleTx(rtl8139_t *rtl)
{
    oz_knl_printk ("tim_rtl8139: transmit interrupt\n");
}

int RtlIsr (void *rtlv, OZ_Mchargs *mchargs)

{
    rtl8139_t *rtl;
    uint16_t status;

    rtl = rtlv;

    out16(rtl->iobase + IntrMask, 0);

    status = in16(rtl->iobase + IntrStatus);
    out16(rtl->iobase + IntrStatus, status & ~(RxFIFOOver | RxOverflow | RxOK));

    oz_knl_printk ("rtl8139: int status %X\n", status);

    if (status & (RxOK | RxErr))
        RtlHandleRx(rtl);
    else if (status & TxOK)
        RtlHandleTx(rtl);
    else
        wprintf("rtl8139: unknown interrupt: isr = %04x\n", status);

    out16(rtl->iobase + IntrStatus, status & (RxFIFOOver | RxOverflow | RxOK));
    out16(rtl->iobase + IntrMask, IntrDefault);

    return (0);
}

/************************************************************************/
/*									*/
/*  Entrypoint suitable for loading via 'kimage load' command		*/
/*									*/
/************************************************************************/

uLong _start (int argc, char **argv, OZ_Image *image)

{
    OZ_Dev_pci_conf_p pciconfp;
    rtl8139_t *rtl;
    unsigned i;
    uint8_t irq;

    /* We just use the first 8139 we find */

    if (!oz_dev_pci_conf_scan_didvid (&pciconfp, 1, 0x813910EC)) {
        oz_knl_printk ("tim_rtl8139: no 8139 found\n");
        return (OZ_NOSUCHFILE);
    }
    oz_knl_printk ("tim_rtl8139: found 8139: bus/dev/func %u/%u/%u\n", pciconfp.pcibus, pciconfp.pcidev, pciconfp.pcifunc);

    /* Allocate struct for it */

    rtl = OZ_KNL_NPPMALLOC (sizeof(rtl8139_t));
    memset(rtl, 0, sizeof(rtl8139_t));

    /* Get the I/O register base address and interrupt vector number */

    rtl -> iobase = oz_dev_pci_conf_inl (&pciconfp, OZ_DEV_PCI_CONF_L_BASADR0);
    if (!(rtl -> iobase & 1)) {
        oz_knl_printk ("tim_rtl8139: iobase %X is memory based\n", rtl -> iobase);
        return (OZ_BADBLOCKSIZE);
    }
    rtl -> iobase --;

    irq = oz_dev_pci_conf_inb (&pciconfp, OZ_DEV_PCI_CONF_B_INTLINE);

    /* Lock this image in memory so 'kimage load' utility won't unload it */

    oz_knl_image_lockinmem (image, OZ_PROCMODE_KNL);
    oz_knl_image_increfc (image, 1);

    /* Register the interrupt routine */

    rtl -> irq_many.entry = RtlIsr;
    rtl -> irq_many.param = rtl;
    rtl -> irq_many.descr = "rtl 8139";

    rtl -> smplock = oz_hw486_irq_many_add (irq, &(rtl -> irq_many));

    /* Start the card - it will dump out all packets it sees */

    wprintf("rtl8139: starting card: io=0x%x, irq=%u\n", rtl->iobase, irq);

    RtlInit(rtl);

    wprintf("rtl8139: station address is %02x-%02x-%02x-%02x-%02x-%02x\n",
        rtl->station_address[0], rtl->station_address[1], rtl->station_address[2], 
        rtl->station_address[3], rtl->station_address[4], rtl->station_address[5]);

    return (OZ_SUCCESS);
}
