/*!
 ******************************************************************************
 * @file    api_peb64.cpp
 * @author  Aaron of TRI
 * @version V0.0.0
 * @date    Fri Apr 14 18:10:37 2017
 * @brief   This file ...
 ******************************************************************************
 */
#define __API_PEB64_C
/* Private includes ----------------------------------------------------------*/
#include "stdafx.h"
#include "api_peb64.h"
#include <Strsafe.h>
#include "utils.h"
#include <conio.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/*! @brief   Function for retrieving the name of main board of a specific slot
 *  @details 
 *  @param   slotNum 
 *  @return  TCHAR const * const
 */
TCHAR const * const apiDscvMainBrdName( int slotNum )
{
    int id;
    
    id = drvRdMainBrdId( slotNum );
    for ( int i=0; i<sizeofarray(psMainIdNameTable); i++ )
        if (( psMainIdNameTable[i].bitmask & id ) == psMainIdNameTable[i].id )
            return psMainIdNameTable[i].pChName;
    
    return _T("Unknow");
}

unsigned char apiIsPEB64( int slotNum )
{
    return ( drvRdMainBrdId( slotNum ) == PEB64_ID );
}

/*! @brief   Function for checking all channel boards statuses in a slot
 *  @details 
 *  @param   slotNum 
 *  @param   psChbStatus 
 *  @return  None
 */
void apiDscvChnBrdPresence ( int slotNum, sChnBrdStatus * psChbStatus )
{
    if ( drvRdMainBrdId( slotNum ) != PEB64_ID )
    {
        for ( int i=0; i<MAX_CHANNEL_BOARD_SUPPORT; i++ )
            psChbStatus->pBrdStt[i] = CHB_NG;
        return;
    }
    
    /*--- Read channel status ---*/
    int chnOkBits;
    chnOkBits = drvRdChnStatusReg( slotNum );
    
    /*--- Transfer status form ---*/
    for ( int i=0, chnBitmask=1; i<MAX_CHANNEL_BOARD_SUPPORT; i++, chnBitmask<<=1 )
    {
        psChbStatus->pBrdStt[i] = ( chnOkBits & chnBitmask ) ? CHB_OK : CHB_NG;
    }
}

/*! @brief   Function for controlling one channel relay
 *  @details 
 *  @param   chn 
 *  @param   onNotOff 
 *  @return  int 1 if succedded else 0
 */
int apiChnRlySwitch( int slotNum, unsigned char chnNum, unsigned char onNotOff )
{
    /*--- Control ---*/
    if ( drvWrChnRly( slotNum, chnNum, onNotOff ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: API:Write relay failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    /*--- Read back check ---*/
    unsigned char rdOnNotOff;
    if ( drvRdChnRly( slotNum, chnNum, &rdOnNotOff ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: API:Read back relay failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    if (( rdOnNotOff != 0 ) == ( onNotOff != 0 ))
        return 1;
    
#if _DEBUG
    _cprintf("Line %d of file %s: API:Relay Read back %d not equal to write %d\n", __LINE__, __FILE__,  rdOnNotOff,  onNotOff );
#endif
    return 0;
}


