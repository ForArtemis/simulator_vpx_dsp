/*
 * SRIO_switch.h
 *
 *  Created on: 2012-11-18
 *      Author: Administrator
 */

#ifndef SRIO_SWITCH_H_
#define SRIO_SWITCH_H_


#include <stdio.h>
#include <ti\csl\csl_srioAux.h>
#include "Keystone_common.h"

#define RIO_DIDCAR_OFFSET 0x0
#define RIO_HOSTLOCK_OFFSET 0x68
#define RIO_BDIDCSR_OFFSET 0x60
#define RIO_LCSBACSR_OFFSET 0x5c
#define RIO_ROUTE_CFG_DESTID 0x70
#define RIO_ROUTE_CFG_PORT 0x74

#define FPGA_DIDCAR_VALUE 0x0370000E
#define CPS1432_DIDCAR_VALUE 0x03750038
#define TMS6678_DIDCAR_VALUE 0x009d0030
#define HBDIDLCSR_NOT_LOCKED 0x0000FFFF
#define CPS1432_SW_PORT 0x14
#define BOTTOM_BYTE_ONLY_MASK 0x000000FF
#define P0_ERR_STATUS_OFFSET 0x158
#define P0_CTL_OFFSET 0x15c
#define CPS1432_PORTOK_BIT 0x02
#define CPS1432_INIT_PWIDTH_MASK 0x38000000
#define CPS1432_INIT_PWIDTH_1L0 0x00000000
#define CPS1432_INIT_PWIDTH_1L2 0x08000000
#define CPS1432_INIT_PWIDTH_4 0x10000000
#define SHIFT_BDIDCSR_TO_DEVICEID 16
#define CPS1432_PORT_STRIDE 0x20
#define DEVICE_CONFIG_HOST 0xe0000000

#define UNKNOWN_DEVICE 102
#define RIO_NOT_TRAINED 101
#define UNKNOWN_DEVICE 102
#define ROUTING_NOT_UPDATED 103
#define NOT_ON_BOOT_PROCESSOR 104
#define NOT_HOST_PROCESSOR 105
#define TEST_FAILED
#define PWIDTH_MASK 0x38000000
#define PWIDTH1L0 0x00000000
#define PWIDTH1L2 0x08000000
#define PWIDTH4 0x10000000

#define SUCCESS 0
#define FABRIC_SYSTEM 0
#define P2P_SYSTEM 1
#define MAX_DEVICES 8

extern int rio_discovery_fabric(Uint32 uiLocalPort);
extern int rio_local_config (void);
extern int rio_discovery(Uint32 uiLocalPort);
#endif /* SRIO_SWITCH_H_ */
