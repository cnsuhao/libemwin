/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2014  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.24 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  NXP Semiconductors USA, Inc.  whose
registered  office  is  situated  at 411 E. Plumeria Drive, San  Jose,
CA 95134, USA  solely for  the  purposes  of  creating  libraries  for
NXPs M0, M3/M4 and  ARM7/9 processor-based  devices,  sublicensed  and
distributed under the terms and conditions of the NXP End User License
Agreement.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIMTDRV_TangoC32.h
Purpose     : Interface definition for GUIMTDRV_TangoC32 driver
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUIMTDRV_TANGOC32_H
#define GUIMTDRV_TANGOC32_H

#include "GUI_Type.h"

#if defined(__cplusplus)
//extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int LayerIndex;
  //
  // Initialization
  //
  void (* pf_I2C_Init)(unsigned char SlaveAddr);
  //
  // Read- and write access
  //
  int (* pf_I2C_Read  )(unsigned char * pData, int Start, int Stop);
  int (* pf_I2C_ReadM )(unsigned char * pData, int NumItems, int Start, int Stop);
  int (* pf_I2C_Write )(unsigned char Data, int Start, int Stop);
  int (* pf_I2C_WriteM)(unsigned char * pData, int NumItems, int Start, int Stop);
  //
  // 7-bit address to be used to address the I2C slave
  //
  U8 SlaveAddr;
} GUIMTDRV_TANGOC32_CONFIG;

/*********************************************************************
*
*       Interface
*
**********************************************************************
*/
int GUIMTDRV_TangoC32_Init(GUIMTDRV_TANGOC32_CONFIG * pConfig);
int GUIMTDRV_TangoC32_Exec(void);

#endif /* GUIMTDRV_TANGOC32_H */

/*************************** End of file ****************************/