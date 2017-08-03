/*!
 ******************************************************************************
 * @file    drv_ate.cpp
 * @author  Aaron of TRI
 * @version V0.0.0
 * @date    Fri Apr 28 10:43:33 2017
 * @brief   This file provide api for controling ADATE305 IC through PEB64
 *          Channel FPGA.  Before calling those APIs, the user must ensure 
 *          the slot and fpga selections on PEB64 are correct
 ******************************************************************************
 */
#define __DRV_ATE_C
/* Private includes ----------------------------------------------------------*/
#include "stdafx.h"
#include "../PciDll/PciDll/pci6800.h"
#include "../PciinfDll/PciinfDll/ifcapi_6850.h"
// #include "d:/PEB64_api_dev/trunk/SRC/PCI_WRDLL/PciinfDll/PciinfDll/ifcapi_6850.h"
#include <Strsafe.h>
#include "utils.h"
#include <Windows.h>
#include "drv_ate.h"
#if _DEBUG
#include <conio.h>
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/*! @brief   Function for 
 *  @details 
 *  @param   None 
 *  @return  int: 0 if ATE stuck in busy mode else 1
 */
int ateWaitIdle( void )
{
	for ( int ms=0; ms<500; ms++ )
	{
		if (( PCI_Rd( ADDR_ATE_WR_CTRL_RD_BUSY ) & BITMASK_ATE_WORKING ) == 0)
		{// Not busy
			return 1;
		}
		delayms(1.0F);
	}
	// Buzy too long
    return 0;
}

int ateWrReg( unsigned int ctrl, unsigned int regData )
{
    /*--- Wait idle ---*/
    if ( ateWaitIdle() != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    /*--- Write Data ---*/
    PCI_Wr( ADDR_ATE_DATA, regData );
    
    /*--- Write Ctrl ---*/
    PCI_Wr( ADDR_ATE_WR_CTRL_RD_BUSY, ctrl );
    
    /*--- Wait idle ---*/
    if ( ateWaitIdle() != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    return 1;
}

int ateRdReg( unsigned int ctrl, unsigned int * pregData )
{
    /*--- Wait idle ---*/
    if ( ateWaitIdle() != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    /*--- Write Ctrl ---*/
    PCI_Wr( ADDR_ATE_WR_CTRL_RD_BUSY, ctrl );
    
    /*--- Wait idle ---*/
    if ( ateWaitIdle() != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    /*--- Write Data ---*/
    *pregData = PCI_Rd( ADDR_ATE_DATA ) & 0xFFFF;
    
    return 1;
}

