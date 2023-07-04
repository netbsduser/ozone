//+++2003-12-12
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
//---2003-12-12

#ifndef _OZ_DEV_PCI_H
#define _OZ_DEV_PCI_H

/************************************************************************/
/*									*/
/*  Accessing PCI devices						*/
/*									*/
/************************************************************************/

#define OZ_DEV_PCI_CONF_L_DIDVID  (0x00)	/* device id (in <16:31>) and vendor id (in <00:15>) registers */
#define OZ_DEV_PCI_CONF_W_PCICMD  (0x04)	/* pci command register */
#define OZ_DEV_PCI_CONF_PCICMD_ENAB_IO  0x0001	/* - enable I/O space accesses */
#define OZ_DEV_PCI_CONF_PCICMD_ENAB_MEM 0x0002	/* - enable memory space accesses */
#define OZ_DEV_PCI_CONF_PCICMD_ENAB_MAS 0x0004	/* - enable bus master access */
#define OZ_DEV_PCI_CONF_W_PCISTS  (0x06)	/* pci device status register */
#define OZ_DEV_PCI_CONF_B_PI      (0x09)	/* programming interface register */
#define OZ_DEV_PCI_CONF_B_SCC     (0x0A)	/* sub-class code */
#define OZ_DEV_PCI_CONF_B_BASEC   (0x0B)	/* base class code */
#define OZ_DEV_PCI_CONF_B_CSHLNSZ (0x0C)	/* cache line size */
#define OZ_DEV_PCI_CONF_B_LATIMER (0x0D)	/* pci latency timer */
#define OZ_DEV_PCI_CONF_B_HDRTYPE (0x0E)	/* header type */
#define OZ_DEV_PCI_CONF_B_BIST    (0x0F)	/* */
#define OZ_DEV_PCI_CONF_L_BASADR0 (0x10)	/* base address 0 */
#define OZ_DEV_PCI_CONF_L_BASADR1 (0x14)	/* base address 1 */
#define OZ_DEV_PCI_CONF_L_BASADR2 (0x18)	/* base address 2 */
#define OZ_DEV_PCI_CONF_L_BASADR3 (0x1C)	/* base address 3 */
#define OZ_DEV_PCI_CONF_L_BASADR4 (0x20)	/* base address 4 */
#define OZ_DEV_PCI_CONF_L_BASADR5 (0x24)	/* base address 5 */
#define OZ_DEV_PCI_CONF_B_INTLINE (0x3C)	/* interrupt line (irq number) */
#define OZ_DEV_PCI_CONF_B_INTPIN  (0x3D)	/* interrupt pin (1=INTA, 2=INTB, 3=INTC, 4=INTD) */

#define OZ_DEV_PCI_CLASSCODE_PRE_PCI_20   0x00
#define OZ_DEV_PCI_CLASSCODE_MASS_STORAGE 0x01
#define OZ_DEV_PCI_CLASSCODE_NETWORK      0x02
#define OZ_DEV_PCI_CLASSCODE_DISPLAY      0x03
#define OZ_DEV_PCI_CLASSCODE_MULTIMEDIA   0x04
#define OZ_DEV_PCI_CLASSCODE_MEMORY       0x05
#define OZ_DEV_PCI_CLASSCODE_BRIDGE       0x06
#define OZ_DEV_PCI_CLASSCODE_SIMPLE_COMM  0x07
#define OZ_DEV_PCI_CLASSCODE_BASE_SYSTEM  0x08
#define OZ_DEV_PCI_CLASSCODE_INPUT        0x09
#define OZ_DEV_PCI_CLASSCODE_DOCKING      0x0A
#define OZ_DEV_PCI_CLASSCODE_PROCESSOR    0x0B
#define OZ_DEV_PCI_CLASSCODE_SERIAL_BUS   0x0C
#define OZ_DEV_PCI_CLASSCODE_MISC         0xFF

#define OZ_DEV_PCI_FINDFLAG_HASBASADR0 0x01
#define OZ_DEV_PCI_FINDFLAG_HASBASADR1 0x02
#define OZ_DEV_PCI_FINDFLAG_HASBASADR2 0x04
#define OZ_DEV_PCI_FINDFLAG_HASBASADR3 0x08
#define OZ_DEV_PCI_FINDFLAG_HASBASADR4 0x10
#define OZ_DEV_PCI_FINDFLAG_HASBASADR5 0x20
#define OZ_DEV_PCI_FINDFLAG_HASNOIRQ   0x40

/* Flags for allocating PCI DMA mapping */

#define OZ_DEV_PCI_DMAFLAG_16M (1)	// all pci addresses must be < 16M
#define OZ_DEV_PCI_DMAFLAG_64K (2)	// neither the table nor any mapping may cross 64K boundary

/* Internal struct definitions */

#ifdef _OZ_DEV_PCI_C
typedef struct OZ_Dev_Pci_Conf OZ_Dev_Pci_Conf;
typedef struct OZ_Dev_Pci_Dma32map OZ_Dev_Pci_Dma32map;
typedef struct OZ_Dev_Pci_Dma64map OZ_Dev_Pci_Dma64map;
typedef struct OZ_Dev_Pci_Irq OZ_Dev_Pci_Irq;
#else
typedef void OZ_Dev_Pci_Conf;
typedef void OZ_Dev_Pci_Dma32map;
typedef void OZ_Dev_Pci_Dma64map;
typedef void OZ_Dev_Pci_Irq;
#endif

/* Needed by entrypoint definitions */

#include "oz_knl_section.h"

/* Entrypoints */

int oz_dev_pci_present (void);
void oz_dev_pci_find_didvid (uLong didvid, 
                             int func, 
                             uLong flags, 
                             int (*entry) (void *param, 
                                           uLong didvid, 
                                           int func, 
                                           OZ_Dev_Pci_Conf *pciconf, 
                                           char const *addrsuffix, 
                                           char const *addrdescrip), 
                             void *param);

OZ_Dev_Pci_Dma32map *oz_dev_pci_dma32map_alloc (OZ_Dev_Pci_Conf *pciconf, uLong npages, uLong flags);
int oz_dev_pci_dma32map_start (OZ_Dev_Pci_Dma32map *dma32map, int memtodev, uLong size, const OZ_Mempage *phypages, uLong offset, OZ_Ieeedma32 **mapvirtadr, uLong *mappciaddr);
void oz_dev_pci_dma32map_stop (OZ_Dev_Pci_Dma32map *dma32map);
void oz_dev_pci_dma32map_free (OZ_Dev_Pci_Dma32map *dma32map);

OZ_Dev_Pci_Dma64map *oz_dev_pci_dma64map_alloc (OZ_Dev_Pci_Conf *pciconf, uLong npages, uLong flags);
int oz_dev_pci_dma64map_start (OZ_Dev_Pci_Dma64map *dma64map, int memtodev, uLong size, const OZ_Mempage *phypages, uLong offset, OZ_Ieeedma64 **mapvirtadr, uQuad *mappciaddr);
void oz_dev_pci_dma64map_stop (OZ_Dev_Pci_Dma64map *dma64map);
void oz_dev_pci_dma64map_free (OZ_Dev_Pci_Dma64map *dma64map);

uByte oz_dev_pci_conf_inb (OZ_Dev_Pci_Conf *pciconf, uByte confadd);
uWord oz_dev_pci_conf_inw (OZ_Dev_Pci_Conf *pciconf, uByte confadd);
uLong oz_dev_pci_conf_inl (OZ_Dev_Pci_Conf *pciconf, uByte confadd);
void oz_dev_pci_conf_outb (uByte value, OZ_Dev_Pci_Conf *pciconf, uByte confadd);
void oz_dev_pci_conf_outw (uWord value, OZ_Dev_Pci_Conf *pciconf, uByte confadd);
void oz_dev_pci_conf_outl (uLong value, OZ_Dev_Pci_Conf *pciconf, uByte confadd);

OZ_Dev_Pci_Irq *oz_dev_pci_irq_alloc (OZ_Dev_Pci_Conf *pciconf, uByte intpin, void (*entry) (void *param, OZ_Mchargs *mchargs), void *param);
OZ_Smplock *oz_dev_pci_irq_smplock (OZ_Dev_Pci_Irq *pciirq);
void oz_dev_pci_irq_reset (OZ_Dev_Pci_Irq *pciirq, void (*entry) (void *param, OZ_Mchargs *mchargs), void *param);
void oz_dev_pci_irq_free (OZ_Dev_Pci_Irq *pciirq);

#endif
