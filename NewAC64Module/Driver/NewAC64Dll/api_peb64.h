/*!
 ******************************************************************************
 * @file    api_peb64.h
 * @author  Aaron of TRI
 * @version V0.0.0
 * @date    Fri Apr 14 18:11:21 2017
 * @brief   This file ...
 ******************************************************************************
 */
#ifndef __API_PEB64_H
#define __API_PEB64_H
/* Includes ------------------------------------------------------------------*/
#include "drv_peb64.h"
/* Typedef -------------------------------------------------------------------*/
typedef struct
{
    int const id;
    int const bitmask;
    TCHAR const * const pChName;
} sIdNames;
//
sIdNames const psMainIdNameTable[] = {
    { PEB64_ID,    BOARD_ID_BIT_MASK, _T("PEB64") }
};
//
#define CHB_OK  1
#define CHB_NG  0
#define MAX_CHANNEL_BOARD_SUPPORT   4
typedef struct
{
    unsigned char pBrdStt[MAX_CHANNEL_BOARD_SUPPORT];
} sChnBrdStatus;
//
/* Define --------------------------------------------------------------------*/
/* Macro ---------------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
TCHAR const * const apiDscvMainBrdName( int slotNum );
void apiDscvChnBrdPresence( int slotNum, sChnBrdStatus * psChnBrdStatus );
#define api_getMaxChannelBoardSupported()       (MAX_CHANNEL_BOARD_SUPPORT)
int apiChnRlySwitch( int slotNum, unsigned char chnNum, unsigned char onNotOff );
unsigned char apiIsPEB64( int slotNum );

#endif
