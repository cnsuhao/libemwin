/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: GUI_XINPUT_Calibrate.c
**
** ��   ��   ��: Jiao.JinXing (������)
**
** �ļ���������: 2014 �� 09 �� 23 ��
**
** ��        ��: emWin ������У������
*********************************************************************************************************/
#include "GUI_XINPUT.h"
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
static GUI_XINPUT_PointerCalibrationData    _G_CalData;
static int                                  _G_iPressCount;
static WM_HWIN                              _G_hWin;
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define ID_WINDOW_0            (GUI_ID_USER + 0x03)
/*********************************************************************************************************
  ȫ�ֽṹ��������
*********************************************************************************************************/
static GUI_WIDGET_CREATE_INFO  _G_aDialogCreate[] =
{
        { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 320, 240, 0, 0x0, 0 },
};
/*********************************************************************************************************
** ��������: _cbDialog
** ��������: ���ڻص�����
** �䡡��  : pMsg          ������Ϣ
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
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
** ��������: GUI_XINPUT_CreateCalibrateFramewin
** ��������: ������ĻУ������
** �䡡��  : NONE
** �䡡��  : ���ھ��
** ȫ�ֱ���:
** ����ģ��:
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
