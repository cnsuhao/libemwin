/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: GUI_XINPUT_Calibrate.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2014 年 09 月 23 日
**
** 描        述: emWin 触摸屏校正程序
*********************************************************************************************************/
#include "GUI_XINPUT.h"
/*********************************************************************************************************
  全局变量定义
*********************************************************************************************************/
static GUI_XINPUT_PointerCalibrationData    _G_CalData;
static int                                  _G_iPressCount;
static WM_HWIN                              _G_hWin;
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define ID_WINDOW_0            (GUI_ID_USER + 0x03)
/*********************************************************************************************************
  全局结构变量定义
*********************************************************************************************************/
static GUI_WIDGET_CREATE_INFO  _G_aDialogCreate[] =
{
        { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 320, 240, 0, 0x0, 0 },
};
/*********************************************************************************************************
** 函数名称: _cbDialog
** 功能描述: 窗口回调函数
** 输　入  : pMsg          窗口消息
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  _cbDialog (WM_MESSAGE  *pMsg)
{
    switch (pMsg->MsgId) {
    case WM_INIT_DIALOG:
    case WM_PAINT: {
        GUI_XPOINT point = _G_CalData.screenPoints[_G_iPressCount];
        GUI_SetColor(GUI_BLACK);
        GUI_FillRect(point.x - 6, point.y - 1, point.x + 6, point.y + 1);
        GUI_FillRect(point.x - 1, point.y - 6, point.x + 1, point.y + 6);
    }
    break;

    case WM_USER + 100: {
        const GUI_PID_STATE *pPidState = pMsg->Data.p;
        _G_CalData.devPoints[_G_iPressCount] = GUI_XPOINT_Create(pPidState->x, pPidState->y);
        if (++_G_iPressCount < 5) {
            WM_Paint(_G_hWin);
        } else {
            GUI_XINPUT_SetCalibrateData(&_G_CalData);
        }
    }
    break;

    default:
        WM_DefaultProc(pMsg);
        break;
    }
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_CreateCalibrateFramewin
** 功能描述: 创建屏幕校正窗口
** 输　入  : NONE
** 输　出  : 窗口句柄
** 全局变量:
** 调用模块:
*********************************************************************************************************/
WM_HWIN  GUI_XINPUT_CreateCalibrateFramewin (void)
{
    GUI_XPOINT  *pPoints;
    int  iWidth  = LCD_GetXSize();
    int  iHeight = LCD_GetYSize();

    int  iDx = iWidth / 10;
    int  iDy = iHeight / 10;

    _G_aDialogCreate[0].xSize = iWidth;
    _G_aDialogCreate[0].ySize = iHeight;

    GUI_XINPUT_DelCalibrateFile();

    GUI_XINPUT_SetFilterSize(64);

    pPoints = _G_CalData.screenPoints;

    pPoints[GUI_XINPUT_TopLeft] = GUI_XPOINT_Create(iDx, iDy);
    pPoints[GUI_XINPUT_BottomLeft] = GUI_XPOINT_Create(iDx, iHeight - iDy);
    pPoints[GUI_XINPUT_BottomRight] = GUI_XPOINT_Create(iWidth - iDx, iHeight - iDy);
    pPoints[GUI_XINPUT_TopRight] = GUI_XPOINT_Create(iWidth - iDx, iDy);
    pPoints[GUI_XINPUT_Center] = GUI_XPOINT_Create(iWidth / 2, iHeight / 2);

    _G_iPressCount = 0;

    _G_hWin = GUI_CreateDialogBox(_G_aDialogCreate,
                                  GUI_COUNTOF( _G_aDialogCreate),
                                  _cbDialog,
                                  WM_HBKWIN, 0, 0);

    GUI_XINPUT_SetCalibrateWin(_G_hWin);

    return _G_hWin;
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
