/*
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/*  
 * This file contains the definitions that are used to configure the
 * Hyperlink peripheral, such as selecting the serial speed and 
 * loopbacks.
 */
/*****************************************************************************
 * Select what if any register contents to print
 *****************************************************************************/
/* Allowed values for hyplnk_EXAMPLE_PRINT_*_REG */
#define hyplnk_EXAMPLE_PRINT_NONE    0
#define hyplnk_EXAMPLE_PRINT_RAW     1
#define hyplnk_EXAMPLE_PRINT_DETAILS 2

/* Change definitions as desired */
#define hyplnk_EXAMPLE_PRINT_REV_REG         hyplnk_EXAMPLE_PRINT_RAW
#define hyplnk_EXAMPLE_PRINT_STATUS_REG      hyplnk_EXAMPLE_PRINT_RAW
#define hyplnk_EXAMPLE_PRINT_LINK_STATUS_REG hyplnk_EXAMPLE_PRINT_RAW
#define hyplnk_EXAMPLE_PRINT_CONTROL_REG     hyplnk_EXAMPLE_PRINT_RAW
#define hyplnk_EXAMPLE_PRINT_ECC_ERRORS_REG  hyplnk_EXAMPLE_PRINT_RAW
#define hyplnk_EXAMPLE_PRINT_SERDES_STS_REGS hyplnk_EXAMPLE_PRINT_RAW

/*****************************************************************************
 * Match the reference clock on your board
 *
 * The value hyplnk_EXAMPLE_REFCLK_USE_PLATCFG uses the reference clock 
 * defined through hyplnk_EXAMPLE_HYPLNK_PLATCFG_REF_CLK_MHZ 
 * in hyplnkPlatCfg.h.
 * 
 * hyplnk_EXAMPLE_REFCLK_USE_PLATCFG can be commented out and the specific 
 * value specified below.
 *****************************************************************************/
#define hyplnk_EXAMPLE_REFCLK_USE_PLATCFG
//#define hyplnk_EXAMPLE_REFCLK_156p25
//#define hyplnk_EXAMPLE_REFCLK_250p00
//#define hyplnk_EXAMPLE_REFCLK_312p50

/*****************************************************************************
 * Select internal loopback or use the SERDES connection
 *****************************************************************************/
//#define hyplnk_EXAMPLE_LOOPBACK

/*****************************************************************************
 * Select number of lanes allowed
 *****************************************************************************/
//#define hyplnk_EXAMPLE_ALLOW_0_LANES
//#define hyplnk_EXAMPLE_ALLOW_1_LANE
#define hyplnk_EXAMPLE_ALLOW_4_LANES

/*****************************************************************************
 * Select a serial rate
 *****************************************************************************/
//#define hyplnk_EXAMPLE_SERRATE_01p250
//#define hyplnk_EXAMPLE_SERRATE_03p125
//#define hyplnk_EXAMPLE_SERRATE_06p250
#define hyplnk_EXAMPLE_SERRATE_07p500
//#define hyplnk_EXAMPLE_SERRATE_10p000
//#define hyplnk_EXAMPLE_SERRATE_12p500

/*****************************************************************************
 * Set if each endpoint has its own hyperlink clock
 *****************************************************************************/
#define hyplnk_EXAMPLE_ASYNC_CLOCKS

/*****************************************************************************
 * Set to perform equalization analysis
 *****************************************************************************/
#define hyplnk_EXAMPLE_EQ_ANALYSIS

/*****************************************************************************
 * Set to enable an error interrupt on uncorrectable serial errors
 *****************************************************************************/
#define hyplnk_EXAMPLE_ERROR_INTERRUPT


/* Nothing past this point */

