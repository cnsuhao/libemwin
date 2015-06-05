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
** 文   件   名: GUI_XINPUT.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2014 年 09 月 23 日
**
** 描        述: emWin 与 xinput 模块对接层(使用与 Qt 相同的环境变量及校正文件，支持热插拨鼠标)
*********************************************************************************************************/
#include "GUI.h"
#include <SylixOS.h>
#include <mouse.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "GUI_XINPUT.h"
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#ifndef min
#define min(a, b)   ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)   ((a) > (b) ? (a) : (b))
#endif
/*********************************************************************************************************
  全局结构变量定义
*********************************************************************************************************/
typedef struct {
    int                 iMouseFd;
    int                 iFbdFd;
    int                 bCalibrated;
    GUI_XPOINT          mousePos;
    LW_OBJECT_HANDLE    hThread;
    int                 a, b, c;
    int                 d, e, f;
    int                 s;
    int                 iNumSamples;
    int                 iSamplesCount;
    WM_HWIN             hWin;
    GUI_XPOINT         *pSamples;
} GUI_XINPUT;

static GUI_XINPUT       _XINPUT;
/*********************************************************************************************************
** 函数名称: GUI_XPOINT_Create
** 功能描述: 生成 GUI_XPOINT
** 输　入  : x             x 坐标
**           y             y 坐标
** 输　出  : GUI_XPOINT
** 全局变量:
** 调用模块:
*********************************************************************************************************/
GUI_XPOINT  GUI_XPOINT_Create (long long  x, long long  y)
{
    GUI_XPOINT pos;

    pos.x = x;
    pos.y = y;
    return pos;
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_ClearCalibrateInfo
** 功能描述: 清除校正信息
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  GUI_XINPUT_ClearCalibrateInfo (void)
{
    _XINPUT.a = 1;
    _XINPUT.b = 0;
    _XINPUT.c = 0;
    _XINPUT.d = 0;
    _XINPUT.e = 1;
    _XINPUT.f = 0;
    _XINPUT.s = 1;
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_ReadCalibrateInfo
** 功能描述: 从校正文件读校正信息
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  GUI_XINPUT_ReadCalibrateInfo (void)
{
    FILE  *pFile;
    CHAR  *pcCalFileName;
    int    iRet;

    pcCalFileName = getenv("POINTERCAL_FILE");
    if (!pcCalFileName)
        pcCalFileName = "/etc/pointercal";

    pFile = fopen(pcCalFileName, "r");
    if (pFile) {
        iRet = fscanf(pFile, "%d %d %d %d %d %d %d", &_XINPUT.a, &_XINPUT.b,
               &_XINPUT.c, &_XINPUT.d, &_XINPUT.e, &_XINPUT.f, &_XINPUT.s);
        fclose(pFile);
        if (_XINPUT.s == 0 || iRet != 7) {
            fprintf(stderr, "Corrupt calibration data: %s\n", pcCalFileName);
            GUI_XINPUT_ClearCalibrateInfo();
        }
        _XINPUT.bCalibrated = LW_TRUE;
    } else {
        fprintf(stderr, "Could not read calibration: %s\n", pcCalFileName);
    }
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_WriteCalibrateInfo
** 功能描述: 写校正信息到校正文件
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  GUI_XINPUT_WriteCalibrateInfo (void)
{
    FILE  *pFile;
    CHAR  *pcCalFileName;

    pcCalFileName = getenv("POINTERCAL_FILE");
    if (!pcCalFileName)
        pcCalFileName = "/etc/pointercal";

    pFile = fopen(pcCalFileName, "w+");
    if (pFile) {
        fprintf(pFile, "%d %d %d %d %d %d %d", _XINPUT.a, _XINPUT.b,
               _XINPUT.c, _XINPUT.d, _XINPUT.e, _XINPUT.f, _XINPUT.s);
        fclose(pFile);

        _XINPUT.bCalibrated = LW_TRUE;
    } else {
        fprintf(stderr, "Could not save calibration into: %s\n", pcCalFileName);
    }
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_IsCalibrated
** 功能描述: 判断是否屏幕被校正过
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
int  GUI_XINPUT_IsCalibrated (void)
{
    return  (_XINPUT.bCalibrated);
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_DelCalibrateFile
** 功能描述: 删除校正文件并清除校正信息
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
void  GUI_XINPUT_DelCalibrateFile (void)
{
    CHAR  *pcCalFileName;

    pcCalFileName = getenv("POINTERCAL_FILE");
    if (!pcCalFileName)
        pcCalFileName = "/etc/pointercal";

    remove(pcCalFileName);
}
/*********************************************************************************************************
** 函数名称: ilog2
** 功能描述: log2
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static int  ilog2 (uint32_t  uiValue)
{
    int  iResult = 0;

    if (uiValue & 0xffff0000) {
        uiValue >>= 16;
        iResult += 16;
    }
    if (uiValue & 0xff00) {
        uiValue >>= 8;
        iResult += 8;}
    if (uiValue & 0xf0) {
        uiValue >>= 4;
        iResult += 4;
    }
    if (uiValue & 0xc) {
        uiValue >>= 2;
        iResult += 2;
    }
    if (uiValue & 0x2)
        iResult += 1;

    return  (iResult);
}
/*********************************************************************************************************
** 函数名称: sort_by_x
** 功能描述: sort_by_x
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static int  sort_by_x(const void *a, const void *b)
{
    return (((GUI_XPOINT *)a)->x - ((GUI_XPOINT *)b)->x);
}
/*********************************************************************************************************
** 函数名称: sort_by_y
** 功能描述: sort_by_y
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static int  sort_by_y(const void *a, const void *b)
{
    return (((GUI_XPOINT *)a)->y - ((GUI_XPOINT *)b)->y);
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_GetCalibrateData
** 功能描述: 获得校正数据
** 输　入  : pData         校正数据
** 输　出  : NONE
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
void  GUI_XINPUT_GetCalibrateData (GUI_XINPUT_PointerCalibrationData  *pData)
{
    const int64_t  lScale = (int64_t)_XINPUT.a * (int64_t)_XINPUT.e - (int64_t)_XINPUT.b * (int64_t)_XINPUT.d;
    const int64_t  lXOff  = (int64_t)_XINPUT.b * (int64_t)_XINPUT.f - (int64_t)_XINPUT.c * (int64_t)_XINPUT.e;
    const int64_t  lYOff  = (int64_t)_XINPUT.c * (int64_t)_XINPUT.d - (int64_t)_XINPUT.a * (int64_t)_XINPUT.f;
    int            i;

    for (i = 0; i <= GUI_XINPUT_LastLocation; ++i) {
        const int64_t  sX = pData->screenPoints[i].x;
        const int64_t  sY = pData->screenPoints[i].y;
        const int64_t  dX = (_XINPUT.s*(_XINPUT.e*sX - _XINPUT.b*sY) + lXOff) / lScale;
        const int64_t  dY = (_XINPUT.s*(_XINPUT.a*sY - _XINPUT.d*sX) + lYOff) / lScale;
        pData->devPoints[i].x = dX;
        pData->devPoints[i].y = dY;
    }
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_SetCalibrateData
** 功能描述: 设置校正数据
** 输　入  : pData         校正数据
** 输　出  : NONE
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
void  GUI_XINPUT_SetCalibrateData (const GUI_XINPUT_PointerCalibrationData  *pData)
{
    // Algorithm derived from
    // "How To Calibrate Touch Screens" by Carlos E. Vidales,
    // printed in Embedded Systems Programming, Vol. 15 no 6, June 2002
    // URL: http://www.embedded.com/showArticle.jhtml?articleID=9900629

    const GUI_XPOINT  pd0 = pData->devPoints[GUI_XINPUT_TopLeft];
    const GUI_XPOINT  pd1 = pData->devPoints[GUI_XINPUT_TopRight];
    const GUI_XPOINT  pd2 = pData->devPoints[GUI_XINPUT_BottomRight];
    const GUI_XPOINT  p0 = pData->screenPoints[GUI_XINPUT_TopLeft];
    const GUI_XPOINT  p1 = pData->screenPoints[GUI_XINPUT_TopRight];
    const GUI_XPOINT  p2 = pData->screenPoints[GUI_XINPUT_BottomRight];

    const int64_t  xd0 = pd0.x;
    const int64_t  xd1 = pd1.x;
    const int64_t  xd2 = pd2.x;
    const int64_t  yd0 = pd0.y;
    const int64_t  yd1 = pd1.y;
    const int64_t  yd2 = pd2.y;
    const int64_t  x0 = p0.x;
    const int64_t  x1 = p1.x;
    const int64_t  x2 = p2.x;
    const int64_t  y0 = p0.y;
    const int64_t  y1 = p1.y;
    const int64_t  y2 = p2.y;

    int      iShift = 0;
    int64_t  lScale = ((xd0 - xd2)*(yd1 - yd2) - (xd1 - xd2)*(yd0 - yd2));
    int64_t  lAbsScale = llabs(lScale);
    // use maximum 16 bit precision to reduce risk of integer overflow
    if (lAbsScale > (1 << 16)) {
        iShift = ilog2(lAbsScale >> 16) + 1;
        lScale >>= iShift;
    }

    _XINPUT.s = lScale;
    _XINPUT.a = ((x0 - x2)*(yd1 - yd2) - (x1 - x2)*(yd0 - yd2)) >> iShift;
    _XINPUT.b = ((xd0 - xd2)*(x1 - x2) - (x0 - x2)*(xd1 - xd2)) >> iShift;
    _XINPUT.c = (yd0*(xd2*x1 - xd1*x2) + yd1*(xd0*x2 - xd2*x0) + yd2*(xd1*x0 - xd0*x1)) >> iShift;
    _XINPUT.d = ((y0 - y2)*(yd1 - yd2) - (y1 - y2)*(yd0 - yd2)) >> iShift;
    _XINPUT.e = ((xd0 - xd2)*(y1 - y2) - (y0 - y2)*(xd1 - xd2)) >> iShift;
    _XINPUT.f = (yd0*(xd2*y1 - xd1*y2) + yd1*(xd0*y2 - xd2*y0) + yd2*(xd1*y0 - xd0*y1)) >> iShift;

    GUI_XINPUT_WriteCalibrateInfo();
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_Transform
** 功能描述: 采样转换成坐标点
** 输　入  : pPos          采样
** 输　出  : 坐标点
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static GUI_XPOINT  GUI_XINPUT_Transform (const GUI_XPOINT  *pPos)
{
    GUI_XPOINT  pos;

    pos.x = (_XINPUT.a * pPos->x + _XINPUT.b * pPos->y + _XINPUT.c) / _XINPUT.s;
    pos.y = (_XINPUT.d * pPos->x + _XINPUT.e * pPos->y + _XINPUT.f) / _XINPUT.s;

    return  (pos);
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_LimitToScreen
** 功能描述: 限制坐标点不超出屏幕
** 输　入  : pPos          坐标点
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  GUI_XINPUT_LimitToScreen (GUI_XPOINT  *pPos)
{
    pPos->x = min(LCD_GetXSize(), max(pPos->x, 0));
    pPos->y = min(LCD_GetYSize(), max(pPos->y, 0));
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_MouseChanged
** 功能描述: 通知 emWin 鼠标发生变化
** 输　入  : pPos          坐标点
**           iButton       键值
**           iScroll       滚轮数据
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  GUI_XINPUT_MouseChanged (const GUI_XPOINT  *pPos, int  iButton, int  iScroll)
{
    GUI_PID_STATE  pidState;
    WM_MESSAGE     msg;

    pidState.x = pPos->x;
    pidState.y = pPos->y;
    pidState.Pressed = iButton;
    pidState.Layer = 0;

    if (_XINPUT.bCalibrated) {
        GUI_PID_StoreState(&pidState);
    } else if (_XINPUT.hWin) {
        msg.hWin    = _XINPUT.hWin;
        msg.hWinSrc = 0;
        msg.MsgId   = WM_USER + 100;
        msg.Data.p  = &pidState;
        WM_SendMessage(msg.hWin, &msg);
    }
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_SetCalibrateWin
** 功能描述: 设置 Calibrate 窗口
** 输　入  : hWin              Calibrate 窗口
** 输　出  : NONE
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
void  GUI_XINPUT_SetCalibrateWin (WM_HWIN  hWin)
{
    _XINPUT.hWin = hWin;
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_SetFilterSize
** 功能描述: 设置 Filter 大小
** 输　入  : iSize         Filter 大小
** 输　出  : NONE
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
void  GUI_XINPUT_SetFilterSize (int  iSize)
{
    _XINPUT.iSamplesCount = max(1, iSize);
    _XINPUT.pSamples = realloc(_XINPUT.pSamples, _XINPUT.iSamplesCount * sizeof(GUI_XPOINT));

    _XINPUT.iNumSamples = 0;
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_SendFilter
** 功能描述: 发送采样到 Filter
** 输　入  : pPos          采样
**           iButton       键值
** 输　出  : 是否发送到 Filter
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static int  GUI_XINPUT_SendFilter (const GUI_XPOINT  *pPos, int  iButton)
{
    int         bSent;
    GUI_XPOINT  pos;
    int         iMiddle;
    GUI_XPOINT  newPos;

    if (!iButton) {
        return LW_FALSE;
    }

    bSent = LW_FALSE;
    _XINPUT.pSamples[_XINPUT.iNumSamples] = *pPos;
    _XINPUT.iNumSamples++;

    if (_XINPUT.iNumSamples >= _XINPUT.iSamplesCount) {

        iMiddle = _XINPUT.iSamplesCount / 2;

        qsort(_XINPUT.pSamples,  _XINPUT.iSamplesCount, sizeof(GUI_XPOINT), sort_by_x);

        if (_XINPUT.iSamplesCount & 1) {
            pos.x = _XINPUT.pSamples[iMiddle].x;
        } else {
            pos.x = (_XINPUT.pSamples[iMiddle - 1].x + _XINPUT.pSamples[iMiddle].x) / 2;
        }

        qsort(_XINPUT.pSamples,  _XINPUT.iSamplesCount, sizeof(GUI_XPOINT), sort_by_y);

        if (_XINPUT.iSamplesCount & 1) {
            pos.y = _XINPUT.pSamples[iMiddle].y;
        } else {
            pos.y = (_XINPUT.pSamples[iMiddle - 1].y + _XINPUT.pSamples[iMiddle].y) / 2;
        }

        newPos = GUI_XINPUT_Transform(&pos);

        GUI_XINPUT_MouseChanged(&newPos, iButton, 0);

        bSent = LW_TRUE;

        _XINPUT.iNumSamples = 0;
    }

    return  (bSent);
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_MouseChanged
** 功能描述: 处理鼠标可读
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  GUI_XINPUT_MouseReadable (void)
{
    GUI_XPOINT          queuedPos = _XINPUT.mousePos;
    mouse_event_notify  mouseEvents[2];
    int                 i;
    GUI_XPOINT          pos;

    int  n = read(_XINPUT.iMouseFd, (char *)mouseEvents, sizeof(mouseEvents));
    if (n <= 0) {
        perror("GUI_XINPUT_MouseReadable: Could not read from input device\n");
        return;
    } else if (n % sizeof(mouseEvents[0]) != 0) {
        fprintf(stderr, "GUI_XINPUT_MouseReadable: Internal error\n");
        return;
    }
    n /= sizeof(mouseEvents[0]);

    for (i = 0; i < n; i++) {
        const mouse_event_notify  *pMouseEvent = &mouseEvents[i];
        int  iButtons = pMouseEvent->kstat & MOUSE_LEFT ? LW_TRUE : LW_FALSE;

        if (pMouseEvent->ctype == MOUSE_CTYPE_ABS) {
            pos.x = pMouseEvent->xanalog;
            pos.y = pMouseEvent->yanalog;

            if (_XINPUT.bCalibrated) {
                pos = GUI_XINPUT_Transform(&pos);
                GUI_XINPUT_LimitToScreen(&pos);
                GUI_XINPUT_MouseChanged(&pos, iButtons, 0);
            } else {
                fprintf(stdout, "GUI_XINPUT_MouseReadable: send (%d, %d) to filter\n",
                        pMouseEvent->xanalog, pMouseEvent->yanalog);
                GUI_XINPUT_SendFilter(&pos, iButtons);
            }
        } else if (pMouseEvent->ctype == MOUSE_CTYPE_REL) {
            queuedPos.x += pMouseEvent->xanalog;
            queuedPos.y += pMouseEvent->yanalog;
            GUI_XINPUT_LimitToScreen(&queuedPos);
            GUI_XINPUT_MouseChanged(&queuedPos, iButtons, pMouseEvent->wscroll[0] << 7 /* same as *128 */ );
        } else {
            fprintf(stderr, "GUI_XINPUT_MouseReadable: unknown mouse event type=%x\n", pMouseEvent->ctype);
        }
    }

    _XINPUT.mousePos = queuedPos;
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_KeyboardReadable
** 功能描述: 处理键盘可读
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  GUI_XINPUT_KeyboardReadable (void)
{
    /*
     * TODO
     */
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_HandleInit
** 功能描述: 初始化 GUI XINPUT Handle
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void  GUI_XINPUT_HandleInit (void)
{
    _XINPUT.iMouseFd = open("/dev/input/xmse", O_RDONLY | O_NONBLOCK);
    if (_XINPUT.iMouseFd == -1) {
        perror("GUI_XINPUT_Thread: Unable to open mouse device\n");
        return;
    }
    ioctl(_XINPUT.iMouseFd, FIOFLUSH, 0); // flush buffered data

    _XINPUT.iFbdFd = open("/dev/input/xkbd", O_RDONLY | O_NONBLOCK);
    if (_XINPUT.iFbdFd == -1) {
        perror("GUI_XINPUT_Thread: Unable to open keyboard device\n");
        return;
    }
    ioctl(_XINPUT.iFbdFd, FIOFLUSH, 0); // flush buffered data
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_HandleEvent
** 功能描述: GUI XINPUT 处理事件
** 输　入  : uiMs          延时多少 ms
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
void  GUI_XINPUT_HandleEvent (U32  uiMs)
{
    fd_set  readFds;
    int     iMaxFd;
    int     iRet;
    struct timeval tv;

    iMaxFd = max(_XINPUT.iMouseFd, _XINPUT.iFbdFd);

    FD_ZERO(&readFds);
    FD_SET(_XINPUT.iMouseFd, &readFds);
    FD_SET(_XINPUT.iFbdFd,   &readFds);

    if (uiMs) {
        tv.tv_sec  = uiMs / 1000;
        tv.tv_usec = (uiMs % 1000) * 1000;

        iRet = select(iMaxFd + 1, &readFds, NULL, NULL, &tv);
    } else {
        iRet = select(iMaxFd + 1, &readFds, NULL, NULL, NULL);
    }
    if (iRet < 0) {
        perror("GUI_XINPUT_Thread: Unable to select device\n");
        return;
    } else if (iRet == 0) {
        return;
    }

    if (FD_ISSET(_XINPUT.iMouseFd, &readFds)) {
        GUI_XINPUT_MouseReadable();

    } else if (FD_ISSET(_XINPUT.iFbdFd, &readFds)) {
        GUI_XINPUT_KeyboardReadable();
    }
}
/*********************************************************************************************************
** 函数名称: GUI_XINPUT_Init
** 功能描述: GUI 初始化 XINPUT
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
void  GUI_XINPUT_Init (void)
{
    bzero(&_XINPUT, sizeof(_XINPUT));

    GUI_XINPUT_ClearCalibrateInfo();

    GUI_XINPUT_ReadCalibrateInfo();

    GUI_XINPUT_SetFilterSize(64);

    GUI_XINPUT_HandleInit();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
