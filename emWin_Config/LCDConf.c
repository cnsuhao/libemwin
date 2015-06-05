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
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUIDRV_Lin.h"
#include <stdio.h>
#include "unistd.h"
#include "sys/utsname.h"
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>

/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/
//
// Color conversion
//
#define COLOR_CONVERSION GUICC_8888

//
// Buffers
//
#define NUM_BUFFERS  1 // Number of multiple buffers to be used

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*   
*/
void LCD_X_Config(void) {
  LW_GM_VARINFO VarInfo;
  LW_GM_SCRINFO ScrInfo;
  uint8_t * pucFramebuffer;
  int iFd;
  int iRet;
  const GUI_DEVICE_API * pDeviceAPI;
  const LCD_API_COLOR_CONV * pColorConvAPI;

  //
  // At first initialize use of multiple buffers on demand
  //
  #if (NUM_BUFFERS > 1)
    GUI_MULTIBUF_Config(NUM_BUFFERS);
  #endif

  iFd = open("/dev/fb0", O_RDWR);
  if (iFd < 0) {
      fprintf(stderr, "failed to open /dev/fb0\n");
      exit(0);
      return;
  }

  iRet = ioctl(iFd, LW_GM_GET_SCRINFO, &ScrInfo);
  if (iRet < 0) {
      fprintf(stderr, "failed to get screen info\n");
      close(iFd);
      exit(0);
      return;
  }

  pucFramebuffer = (uint8_t *)mmap(0, ScrInfo.GMSI_stMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);
  if ((long)pucFramebuffer == -1) {
      fprintf(stderr, "failed to mmap\n");
      close(iFd);
      exit(0);
      return;
  }

  iRet = ioctl(iFd, LW_GM_GET_VARINFO, &VarInfo);
  if (iRet < 0) {
      fprintf(stderr, "failed to get var info\n");
      close(iFd);
      exit(0);
      return;
  }

  switch (VarInfo.GMVI_ulBitsPerPixel) {
  case 1:
      pDeviceAPI = GUIDRV_LIN_1;
      pColorConvAPI = GUICC_1;
      break;

  case 2:
      pDeviceAPI = GUIDRV_LIN_2;
      pColorConvAPI = GUICC_2;
      break;

  case 4:
      pDeviceAPI = GUIDRV_LIN_4;
      pColorConvAPI = GUICC_4;
      break;

  case 8:
      pDeviceAPI = GUIDRV_LIN_8;
      pColorConvAPI = GUICC_8;
      break;

  case 16:
      pDeviceAPI = GUIDRV_LIN_16;
      pColorConvAPI = GUICC_M565;
      break;

  case 24:
      pDeviceAPI = GUIDRV_LIN_24;
      pColorConvAPI = GUICC_M888;
      break;

  case 32:
      pDeviceAPI = GUIDRV_LIN_32;
      pColorConvAPI = GUICC_M8888;
      break;

  default:
      fprintf(stderr, "failed to mmap\n");
      close(iFd);
      exit(0);
      return;
  }

  //
  // Set display driver and color conversion for 1st layer
  //
  GUI_DEVICE_CreateAndLink(pDeviceAPI, pColorConvAPI, 0, 0);
  //
  // Display driver configuration, required for Lin-driver
  //
  if (LCD_GetSwapXY()) {
    LCD_SetSizeEx (0, VarInfo.GMVI_ulYRes, VarInfo.GMVI_ulXRes);
    LCD_SetVSizeEx(0, VarInfo.GMVI_ulYResVirtual, VarInfo.GMVI_ulXResVirtual);
  } else {
    LCD_SetSizeEx (0, VarInfo.GMVI_ulXRes, VarInfo.GMVI_ulYRes);
    LCD_SetVSizeEx(0, VarInfo.GMVI_ulXResVirtual, VarInfo.GMVI_ulYResVirtual);
  }
  LCD_SetVRAMAddrEx(0, (void *)pucFramebuffer);
  //
  // Set user palette data (only required if no fixed palette is used)
  //
  #if defined(PALETTE)
    LCD_SetLUTEx(0, PALETTE);
  #endif
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if 
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;

  switch (Cmd) {
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
    //
    // ...
    return 0;
  }
  case LCD_X_SETVRAMADDR: {
    //
    // Required for setting the address of the video RAM for drivers
    // with memory mapped video RAM which is passed in the 'pVRAM' element of p
    //
    LCD_X_SETVRAMADDR_INFO * p;
    p = (LCD_X_SETVRAMADDR_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SETORG: {
    //
    // Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
    //
    LCD_X_SETORG_INFO * p;
    p = (LCD_X_SETORG_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SHOWBUFFER: {
    //
    // Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
    //
    LCD_X_SHOWBUFFER_INFO * p;
    p = (LCD_X_SHOWBUFFER_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SETLUTENTRY: {
    //
    // Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
    //
    LCD_X_SETLUTENTRY_INFO * p;
    p = (LCD_X_SETLUTENTRY_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_ON: {
    //
    // Required if the display controller should support switching on and off
    //
    return 0;
  }
  case LCD_X_OFF: {
    //
    // Required if the display controller should support switching on and off
    //
    // ...
    return 0;
  }
  case LCD_X_SETSIZE: {
    LCD_X_SETSIZE_INFO * p;
    p = (LCD_X_SETSIZE_INFO *)pData;
    return 0;
  }

  default:
    r = -1;
  }
  return r;
}

/*************************** End of file ****************************/
