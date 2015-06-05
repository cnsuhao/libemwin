/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2013  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.22 - Graphical user interface for embedded applications **
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
File        : GUIDEMO_Start.c
Purpose     : GUIDEMO initialization
----------------------------------------------------------------------
*/

#include "GUIDEMO.h"
#include "GUI_XINPUT_Calibrate.h"

/*********************************************************************
*
*       MainTask
*/
void MainTask(void);
void MainTask(void) {
  WM_SetCreateFlags(WM_CF_MEMDEV);
  GUI_Init();
  WM_MULTIBUF_Enable(1);

  if (!GUI_XINPUT_IsCalibrated()) {
      WM_HWIN hWin = GUI_XINPUT_CreateCalibrateFramewin();
      while (!GUI_XINPUT_IsCalibrated()) {
          GUI_Delay(100);
          GUI_Exec();
      }
      GUI_EndDialog(hWin, 0);
  }

  GUIDEMO_Main();
}

/*************************** End of file ****************************/

