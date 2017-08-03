/*!
 ******************************************************************************
 * @file    drv_peb64.cpp
 * @author  Aaron of TRI
 * @version V0.0.0
 * @date    Wed Apr 12 19:03:39 2017
 * @brief   This file implements PEB64's drivers and makes them as independent
 *          as possible, driver functions are expected to be singly copied to 
 *          other projects
 ******************************************************************************
 */
#define __DRV_PEB64_C
/* Private includes ----------------------------------------------------------*/
#include "stdafx.h"
#include "../PciDll/PciDll/pci6800.h"
#include "../PciinfDll/PciinfDll/ifcapi_6850.h"
// #include "d:/PEB64_api_dev/trunk/SRC/PCI_WRDLL/PciinfDll/PciinfDll/ifcapi_6850.h"
#include "drv_ate.h"
#include "drv_peb64.h"
#include <Strsafe.h>
#include "utils.h"
#include <Windows.h>
#if _DEBUG
#include <conio.h>
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// Increase major version number when you think the code is very robust
#define MAJOR_VERSION		0
// Incerase minor version number after you release code to other engineers
#define MINOR_VERSION		0
// Manage to increase build version after you commit to SVN
#define BUILD_VERSION		15

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const struct
{
	unsigned int const major;
	unsigned int const release;
	unsigned int const build;
} Version = { MAJOR_VERSION, MINOR_VERSION, BUILD_VERSION };

sAteCurrRangeInfo psAteCurrRangeInfos[] = {
    { 0, 250e3, 4e-6,   _T("2uA") }
  , { 4, 25e3,  40e-6,  _T("20uA") }
  , { 5, 2.5e3, 400e-6, _T("200uA") }
  , { 6, 250,   4e-3,   _T("2mA") }
  , { 7, 15.5,  50e-3,  _T("32mA") }
};
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*! @brief   Function for 
 *  @details 
 *  @param   addr 
 *  @return  unsigned int: I changed the return value to unsigned, because signed
             data might cause padding issue when shifting
 */
unsigned int PCI_Rd16( unsigned int addr )
{
    return PCI_Rd( addr ) & READ_DATA_MASK;
}

/*! @brief   Function for waiting relay finish data transmitting
 *  @details 
 *  @param   None 
 *  @return  0 if "relay stuck in busy mode" else 1
 */
int drvWaitChnRlyIdle( void )
{
	for ( int ms=0; ms<500; ms++ )
	{
		if (( PCI_Rd( ADDR_CHN_RLY_STATUS ) & BITMASK_CHN_WORKING ) == 0)
		{// Not busy
			return 1;
		}
		delayms(1.0F);
	}
	// Buzy too long
    return 0;
}

/*! @brief   Function for enable a specific slot and FPGA
 *  @details 
 *  @param   slotNum 
 *  @param   fpgaNum 
 *  @return  void
 */
void drvOpen( int slotNum, int fpgaNum )
{
    /*--- Enable a specific slot ---*/
	unsigned int slotBitMask = slotNumToBitmask(slotNum);
    PCI_Wr( ADDR_ENABLE_SLOTS, slotBitMask );
    
    /*--- Enable a specific FPGA ---*/
	unsigned int fpgaBitMask = fpgaNumToBitmask( fpgaNum );
    PCI_Wr( ADDR_ONOFF_FPGAS, fpgaBitMask );
}

unsigned int drvRdMemAddr( void )
{
    unsigned int addr, addrH, addrL;
    addrH = PCI_Rd16( ADDR_CHN_MEM_ADDR );
	addrL = PCI_Rd16( ADDR_CHN_MEM_ADDR );
    addr = (addrH<<16) + addrL;
    return addr;
    
}

int drvWrMemAddr( unsigned int startAddr )
{
    /*--- Test indirect accessing address register ---*/
    PCI_Wr( ADDR_CHN_MEM_ADDR, 0x000A );
    PCI_Wr( ADDR_CHN_MEM_ADDR, 0xAAAA );
    if ( PCI_Rd16( ADDR_CHN_MEM_ADDR ) != 0x000A )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Address register cannot be accessed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    if ( PCI_Rd16( ADDR_CHN_MEM_ADDR ) != 0xAAAA )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Address register cannot be accessed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    /*--- Write start address into indirect accessing address ---*/
    PCI_Wr( ADDR_CHN_MEM_ADDR, startAddr>>16 );
    PCI_Wr( ADDR_CHN_MEM_ADDR, startAddr&0xFFFF );
    
    /*--- Read address back and check ---*/
    if ( PCI_Rd16( ADDR_CHN_MEM_ADDR ) != startAddr>>16 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Address %d read back failed\n", __LINE__, __FILE__,  startAddr );
    #endif
        return 0;
    }
    if ( PCI_Rd16( ADDR_CHN_MEM_ADDR) != (startAddr&0xFFFF) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Address %d read back failed\n", __LINE__, __FILE__,  startAddr );
    #endif
        return 0;
    }
	return 1;
}

unsigned int drvRdMemInstData( void )
{
    unsigned int inst, instH, instL;
    instH = PCI_Rd16( ADDR_CHN_MEM_INST );
	instL = PCI_Rd16( ADDR_CHN_MEM_INST );
    inst = (instH<<16) + instL;
    return inst;
}

void drvWrMemInstData( unsigned int data )
{
    PCI_Wr( ADDR_CHN_MEM_INST, data>>16);
    PCI_Wr( ADDR_CHN_MEM_INST, data&0xFFFF);
}

unsigned long long drvRdMemPattData( void )
{
    unsigned long long inst, instU, instH, instL;
    instU = PCI_Rd16( ADDR_CHN_MEM_PATT );
    instH = PCI_Rd16( ADDR_CHN_MEM_PATT );
	instL = PCI_Rd16( ADDR_CHN_MEM_PATT );
    inst = (instU<<32) + (instH<<16) + instL;
    return inst;
}

void drvWrMemPattData( unsigned long long data48 )
{
    PCI_Wr( ADDR_CHN_MEM_PATT, (unsigned int)((data48>>32)&0xFFFF));
    PCI_Wr( ADDR_CHN_MEM_PATT, (unsigned int)((data48>>16)&0xFFFF));
    PCI_Wr( ADDR_CHN_MEM_PATT, (unsigned int)(data48&0xFFFF));
}

/* Public functions ----------------------------------------------------------*/

int getApiVersionString( TCHAR * pCh, int maxLength )
{
	return StringCbPrintf( pCh, maxLength, _T("Version %d.%d.%d"), Version.major, Version.release, Version.build );
}

int drvRdMainBrdId( int slotNum )
{
    /*--- Enable a specific slot ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, slotNumToBitmask(slotNum));
    
    /*--- Disable all FPGA on that PEB slot ---*/
    PCI_Wr( ADDR_ONOFF_FPGAS, FPGAS_ALL_OFF );
    
    /*--- Read board ID ---*/
    int id;
    id = PCI_Rd16( ADDR_READ_PEB_ID ) & BOARD_ID_BIT_MASK;
    
    /*--- Disable all slots ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, SLOTS_ALL_OFF );
    
    return id;
}

/*! @brief   Function for checking FPGA's presence by writting data in its test register
 *  @details This function makes address of register transparent to the caller
 *  @param   slotNum: start from 1
 *  @param   fpgaNum: start from 1
 *  @param   inquiryData 
 *  @return  Read back data
 */
int drvInquiryFpgaTestReg( int slotNum, int fpgaNum, int inquiryData )
{
    int fpgaInd = fpgaNum-1;
    
    /*--- Enable a specific slot ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, slotNumToBitmask(slotNum));
    
    /*--- Enable a specific FPGA ---*/
    PCI_Wr( ADDR_ONOFF_FPGAS, fpgaIndToBitmask( fpgaInd ));
    
    /*--- Choosing correct address of test regitster ---*/
    int addrTestReg;
    addrTestReg = ( fpgaNum == MAIN_FPGA_NUMBER) ? ADDR_FPGA_TEST_REG : ADDR_CH_FPGA_TEST_REG;
    
    /*--- Write specific value to FPGA's test register ---*/
    PCI_Wr( addrTestReg, inquiryData );
    
    /*--- Read and check ---*/
    int rdata;
    rdata = PCI_Rd16( addrTestReg );
    
    /*--- Disable all slots ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, SLOTS_ALL_OFF );
    
    return rdata;
}

int drvRdChnStatusReg( int slotNum )
{
    /*--- Enable a specific slot ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, slotNumToBitmask(slotNum));
    
    /*--- Enable main FPGA ---*/
    PCI_Wr( ADDR_ONOFF_FPGAS, fpgaIndToBitmask( MAIN_FPGA_IND ));
    
    /*--- Read channel status ---*/
    int channelOkBits;
    channelOkBits = PCI_Rd16( ADDR_FPGA_CH_STATUS );
    
    return channelOkBits;
}

/*! @brief   Function for controlling a relay channel
 *  @details 
 *  @param   chn : channel number, from 1 to 64, 65 to 96(user)
 *  @param   onNotOff : 1 if on else 0
 *  @return  int: 0 if "relay work too long" else 1
 */
int drvWrChnRly( int slotNum, unsigned char chnNum, unsigned char onNotOff )
{
    unsigned char chn = chnNum-1;
    
    /*--- Enable a specific slot ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, slotNumToBitmask(slotNum));
    
    /*--- Enable main FPGA ---*/
    PCI_Wr( ADDR_ONOFF_FPGAS, fpgaIndToBitmask( MAIN_FPGA_IND ));
    
    /*--- Check if relay is busy ---*/
    if ( !drvWaitChnRlyIdle() ) return 0;
    
    int addr, data;
    addr = chnRlyChnToAddr( chn );
    data = onNotOff ? getDataChnRlyOn( chn ) : getDataChnRlyOff( chn );
    
    PCI_Wr( addr, data );
    
    return 1;
}

/*! @brief   Function for reading a relay channel
 *  @details 
 *  @param   chnNum : channel number, from 1 to 64, 65 to 96(user)
 *  @param   pOnNotOff : a storage to put the relay status
 *  @return  0 if "relay work too long" else 1
 */
int drvRdChnRly( int slotNum, unsigned char chnNum, unsigned char * pOnNotOff )
{
    unsigned char chn = chnNum-1;
    
    /*--- Enable a specific slot ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, slotNumToBitmask(slotNum));
    
    /*--- Enable main FPGA ---*/
    PCI_Wr( ADDR_ONOFF_FPGAS, fpgaIndToBitmask( MAIN_FPGA_IND ));
    
    int addr, data;
    addr = chnRlyChnToAddr( chn );
    /*--- First write intent to send the reading address to relay manager ---*/
    if ( !drvWaitChnRlyIdle() ) return 0;
    PCI_Wr( addr, 0 );
    
    /*--- Second write intent to read the data into common read register ---*/
    if ( !drvWaitChnRlyIdle() ) return 0;
    PCI_Wr( addr, 0 );
    if ( !drvWaitChnRlyIdle() ) return 0;
    
    data = PCI_Rd16( ADDR_CHN_RLY_READ );
    *pOnNotOff = ((data & chnRlyChnToOnOffBitmask(chn)) != 0);
    return 1;
}

/*! @brief   Function for reading multiple channels relay fast
 *  @details 
 *  @param   slotNum 
 *  @param   psChnRlyStatus : a buffer to put results
             The contents:
                Each unsigned char unit represent one channel
                1 if relay is on else 0
 *  @param   firstChn : first channel index to be read
 *  @param   lastChn : last channel index to be read
 *  @return  int 1 if successed else 0
 */
int drvRdChnRlyAll( int slotNum, sChnRlyStatus * psChnRlyStatus, unsigned char firstChn, unsigned char lastChn )
{
    ASSERT( chnRlyChnToAddr(firstChn) >= ADDR_CHN_RLY_FIRST );
    ASSERT( chnRlyChnToAddr(lastChn) <= ADDR_CHN_RLY_LAST );
    
    /*--- Enable a specific slot ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, slotNumToBitmask(slotNum));
    
    /*--- Enable main FPGA ---*/
    PCI_Wr( ADDR_ONOFF_FPGAS, fpgaIndToBitmask( MAIN_FPGA_IND ));
    
    
    /*--- Preload the first address to let the following write retrieve data of the first address ---*/
    if ( !drvWaitChnRlyIdle() ) return 0;
    PCI_Wr( ADDR_CHN_RLY_FIRST, 0 );
    
    unsigned int pChnStatusBits[ 1 + ADDR_CHN_RLY_LAST - ADDR_CHN_RLY_FIRST ];
    for ( int addr=ADDR_CHN_RLY_FIRST, ibuf=0; addr<=ADDR_CHN_RLY_LAST; addr++, ibuf++ )
    {
        if ( !drvWaitChnRlyIdle() ) return 0;
        PCI_Wr( addr, 0 );  // Wr ADDR_CHN_RLY_FIRST also retrieve data of first ( because we preload the fitst address in advance )
        if ( !drvWaitChnRlyIdle() ) return 0;
        pChnStatusBits[ibuf] = PCI_Rd16( ADDR_CHN_RLY_READ );
    }
    
    /*--- Map channel status bits to array ---*/
    for ( unsigned char chnInd=firstChn; chnInd<=lastChn; chnInd++ )
    {
        int addr = chnRlyChnToAddr( chnInd );
        int data = pChnStatusBits[ addr - ADDR_CHN_RLY_FIRST ];
        psChnRlyStatus->pOnOff[ chnInd ] = ((data & chnRlyChnToOnOffBitmask( chnInd )) != 0);
    }
    return 1;
}

/*! @brief   Function for closing all relay
 *  @details 
 *  @param   None 
 *  @return  int: 0 if "relay work too long" else 1
 */
int drvResetAllChnRly( int slotNum )
{
    /*--- Enable a specific slot ---*/
    PCI_Wr( ADDR_ENABLE_SLOTS, slotNumToBitmask(slotNum));
    
    /*--- Enable main FPGA ---*/
    PCI_Wr( ADDR_ONOFF_FPGAS, fpgaIndToBitmask( MAIN_FPGA_IND ));
    
    if ( !drvWaitChnRlyIdle() ) return 0;
    PCI_Wr( ADDR_CHN_RLY_STATUS, 0 );    // Writing this register initiates a reset
    return 1;
}

/*! @brief   Function for writting data into instruction memory of channel board
 *  @details 
 *  @param   slotNum : Slot number
 *  @param   fpgaNum : FPGA number
 *  @param   startAddr : Address to write the first data
 *  @param   pInstBuf : Data array to be wirtten
 *  @param   len : Amount of instructions data in pInstBuf
 *  @return  int: 1 if success else 0
 */
int drvWrChnMemInst( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int * pInstBuf, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d instruction memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iInst=0; iInst<len; iInst++ )
    {
        drvWrMemInstData( pInstBuf[ iInst ] );
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr  != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: instruction memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvRdChnMemInst( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int * pInstBuf, unsigned int len )
{
	ASSERT( startAddr + len >= startAddr );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d instruction memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iInst=0; iInst<len; iInst++ )
    {
        pInstBuf[ iInst ] = drvRdMemInstData();
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr  != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: instruction memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvRdChnMemPatt( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long * pPattBuf, unsigned int len )
{
	ASSERT( startAddr + len >= startAddr );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d pattern memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iPatt=0; iPatt<len; iPatt++ )
    {
        pPattBuf[iPatt] = drvRdMemPattData();
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr  != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: pattern memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrChnMemPatt( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long * pPattBuf, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d pattern memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iPatt=0; iPatt<len; iPatt++ )
    {
        drvWrMemPattData( pPattBuf[iPatt] );
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr  != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: pattern memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

/*! @brief   Function for channel's instruction memory test
 *  @details 
 *  @param   slotNum 
 *  @param   fpgaNum 
 *  @param   startAddr 
 *  @param   fixData 
 *  @param   len 
 *  @return  int: 1 if success else 0
 */
int drvWrChnMemInstFixData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int fixData, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d instruction memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iInst=0; iInst<len; iInst++ )
    {
        drvWrMemInstData( fixData );
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr  != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: instruction memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvRdChnMemInstFixData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int fixData, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d instruction memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iInst=0; iInst<len; iInst++ )
    {
        unsigned int rd = drvRdMemInstData();
        if ( rd != fixData )
        {
        #if _DEBUG
            _cprintf("Line %d of file %s: Slot number %d FPGA number %d instruction memory falied at address %08X, read data %08X != assigned data %08X \n", __LINE__, __FILE__,  slotNum,  fpgaNum,  iInst+startAddr,  rd,  fixData );
        #endif
            return 0;
        }
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: instruction memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrChnMemInstIncData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int startData, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d instruction memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iInst=0, serial=startData; iInst<len; iInst++, serial++ )
    {
        drvWrMemInstData( serial );
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: instruction memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvRdChnMemInstIncData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int startData, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d instruction memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iInst=0, serial=startData; iInst<len; iInst++, serial++ )
    {
        unsigned int rd = drvRdMemInstData();
        if ( rd != serial )
        {
        #if _DEBUG
            _cprintf("Line %d of file %s: Slot number %d FPGA number %d instruction memory falied at address %08X, read data %08X != assigned data %08X \n", __LINE__, __FILE__,  slotNum,  fpgaNum,  iInst+startAddr,  rd,  serial );
        #endif
            return 0;
        }
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr  != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}


/*! @brief   Function for channel's instruction memory test
 *  @details 
 *  @param   slotNum 
 *  @param   fpgaNum 
 *  @param   startAddr 
 *  @param   fixData48 
 *  @param   len 
 *  @return  int: 1 if success else 0
 */
int drvWrChnMemPattFixData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long fixData48, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    ASSERT( ( fixData48 & 0xFFFF000000000000 ) == 0 );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d pattern memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iInst=0; iInst<len; iInst++ )
    {
        drvWrMemPattData( fixData48 );
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr  != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: pattern memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvRdChnMemPattFixData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long fixData48, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    ASSERT( ( fixData48 & 0xFFFF000000000000 ) == 0 );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d pattern memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    for ( unsigned int iInst=0; iInst<len; iInst++ )
    {
        unsigned long long rd = drvRdMemPattData();
        if ( rd != fixData48 )
        {
        #if _DEBUG
            _cprintf("Line %d of file %s: Slot number %d FPGA number %d pattern memory falied at address %08X, read data %08X != assigned data %08X \n", __LINE__, __FILE__,  slotNum,  fpgaNum,  iInst+startAddr,  rd,  fixData48 );
        #endif
            return 0;
        }
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrChnMemPattIncData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long startData48, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    ASSERT( ( startData48 & 0xFFFF000000000000 ) == 0 );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d pattern memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    unsigned long long serial = startData48;
    for ( unsigned int iInst=0; iInst<len; iInst++, serial++ )
    {
        drvWrMemPattData( serial & 0xFFFFFFFFFFFF );
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: pattern memory final address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvRdChnMemPattIncData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long startData48, unsigned int len )
{
    ASSERT( startAddr + len >= startAddr );
    ASSERT( ( startData48 & 0xFFFF000000000000 ) == 0 );
    
    drvOpen( slotNum, fpgaNum );
    
    if ( drvWrMemAddr( startAddr ) == 0 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot number %d FPGA number %d pattern memory falied\n", __LINE__, __FILE__,  slotNum,  fpgaNum );
    #endif
        return 0;
    }
    
    /*--- Start writing instructions ---*/
    unsigned long long serial = startData48;
    for ( unsigned int iInst=0; iInst<len; iInst++, serial++ )
    {
        unsigned long long rd = drvRdMemPattData();
        if ( rd != serial )
        {
        #if _DEBUG
            _cprintf("Line %d of file %s: Slot number %d FPGA number %d pattern memory falied at address %08X, read data %08X != assigned data %08X \n", __LINE__, __FILE__,  slotNum,  fpgaNum,  iInst+startAddr,  rd,  serial );
        #endif
            return 0;
        }
    }
    
    /*--- Address increment check ---*/
	unsigned int finalAddr;
	finalAddr = drvRdMemAddr();
    if ( finalAddr  != len + startAddr )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Wr %d,%d,addr: fpattern memory inal address read back failed(%X should be %X)\n", __LINE__, __FILE__,  slotNum,  fpgaNum,  finalAddr,  len + startAddr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvOpenAte( int slotNum, unsigned char chnNum )
{
    /*--- Calculate FPGA number from PEB channel number ---*/
    int fpgaNum = chnNumToFpgaNum( chnNum );
    
    ASSERT( fpgaNum <= drvGetLastChnFpgaNumOnPeb64() );
    
    drvOpen( slotNum, fpgaNum );
    
    /*--- Wait idle ---*/
    if ( ateWaitIdle() != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: Slot %d FGGA %d ATE stuck in busy mode!\n", __LINE__, __FILE__, slotNum, fpgaNum );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAteVt( int slotNum, unsigned char chnNum, double volt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = vtVoltToDac(volt);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VT_DAC_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    
    return 1;
}

int drvRdAteVt( int slotNum, unsigned char chnNum, double * pvolt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int ctrl = adjAteCtrl( chnNum, ATE_READ, ATEADDR_VT_DAC_LEVEL );
    unsigned int dacValue;
    if ( !ateRdReg( ctrl, &dacValue ) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Read ATE VT value failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    *pvolt = vtDacToVolt(dacValue);
    
    return 1;
}

int drvWrAteVh( int slotNum, unsigned char chnNum, double volt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = vhVoltToDac(volt);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VH_DAC_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    
    return 1;
}

int drvRdAteVh( int slotNum, unsigned char chnNum, double * pvolt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int ctrl = adjAteCtrl( chnNum, ATE_READ, ATEADDR_VH_DAC_LEVEL );
    unsigned int dacValue;
    if ( !ateRdReg( ctrl, &dacValue ) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Read ATE VH value failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    *pvolt = vhDacToVolt(dacValue);
    
    return 1;
}

int drvWrAteVl( int slotNum, unsigned char chnNum, double volt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = vlVoltToDac(volt);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VL_DAC_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    
    return 1;
}

int drvRdAteVl( int slotNum, unsigned char chnNum, double * pvolt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int ctrl = adjAteCtrl( chnNum, ATE_READ, ATEADDR_VL_DAC_LEVEL );
    unsigned int dacValue;
    if ( !ateRdReg( ctrl, &dacValue ) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Read ATE VH value failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    *pvolt = vlDacToVolt(dacValue);
    
    return 1;
}

int drvWrAteVoh( int slotNum, unsigned char chnNum, double volt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = vohVoltToDac(volt);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VOH_DAC_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    
    return 1;
}

int drvWrAteVol( int slotNum, unsigned char chnNum, double volt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = volVoltToDac(volt);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VOL_DAC_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    
    return 1;
}

int drvWrAteIoh( int slotNum, unsigned char chnNum, double curr )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = IohCurrToDac(curr);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VIOH_DAC_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    
    return 1;
}

int drvWrAteIol( int slotNum, unsigned char chnNum, double curr )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = IolCurrToDac(curr);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VIOL_DAC_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    
    return 1;
}

int drvWrAteActiveLoadOnOff( int slotNum, unsigned char chnNum, int enableNotDisable )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    // Alter LOAD setting bit by read then weite
    unsigned int rdCtrlVal = adjAteCtrl( chnNum, ATE_READ, ATEADDR_CHANNEL_STATE );
    unsigned int rdSttValue;
    if ( !ateRdReg( rdCtrlVal, &rdSttValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE read register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    unsigned int newSttValue = ( rdSttValue & (0xFFFFFFFF^(1<<BIT_CHN_LOAD_EN))) | (enableNotDisable<<BIT_CHN_LOAD_EN);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_CHANNEL_STATE );
    
    if ( !ateWrReg( ctrlVal, newSttValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAteOvdh( int slotNum, unsigned char chnNum, double volt )
{
	unsigned char ovdChNum = chnNum & 0xFFFFFFFE;
    
    if ( drvOpenAte( slotNum, ovdChNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = OvdhVoltToDac(volt);
    unsigned int ctrlVal = adjAteCtrl( ovdChNum, ATE_WRITE, ATEADDR_OVD_HI_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAteOvdl( int slotNum, unsigned char chnNum, double volt )
{
    unsigned char ovdChNum = chnNum | 1;
    
    if ( drvOpenAte( slotNum, ovdChNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    unsigned int dacValue = OvdlVoltToDac(volt);
    unsigned int ctrlVal = adjAteCtrl( ovdChNum, ATE_WRITE, ATEADDR_OVD_HI_LEVEL );
    
    if ( !ateWrReg( ctrlVal, dacValue ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAteClampH( int slotNum, unsigned char chnNum, double volt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: %s\n", __LINE__, __FILE__, "ATE stuck in buzy mode!\n" );
    #endif
        return 0;
    }
    
    unsigned int ctrl = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VCH_DAC_LEVEL );
    unsigned int dacValue = vcVoltToDac( volt );
    
    if ( !ateWrReg( ctrl, dacValue ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: %s\n", __LINE__, __FILE__, "ATE write clamp high voltage failed\n!" );
    #endif
    }
    return 1;
}

int drvWrAteClampL( int slotNum, unsigned char chnNum, double volt )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: %s\n", __LINE__, __FILE__, "ATE stuck in buzy mode!\n" );
    #endif
        return 0;
    }
    
    unsigned int ctrl = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_VCL_DAC_LEVEL );
    unsigned int dacValue = vcVoltToDac( volt );
    
    if ( !ateWrReg( ctrl, dacValue ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: %s\n", __LINE__, __FILE__, "ATE write clamp low voltage failed\n!" );
    #endif
    }
    return 1;
}

int drvWrAtePmuVolt( int slotNum, unsigned char chnNum, double volt )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    unsigned int dacValue = pmuVoltToDac( volt );
	// Over / underflow check
	if ( dacValue & 0x80000000 ) dacValue = 0;
	else if ( dacValue & 0xFFFF0000 ) dacValue = 0xFFFF;

    unsigned int ctrl = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMUDAC_LEVEL );
    
    if ( !ateWrReg( ctrl, dacValue ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Writing PMUDAC value failed!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAtePmuCurr( int slotNum, unsigned char chnNum, double amp, sAteCurrIndex gsAteCurrIndex )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    unsigned int dacValue = pmuCurrToDac( amp, psAteCurrRangeInfos[ gsAteCurrIndex ].forceIPeakAmp );
	// Over / underflow check
	if ( dacValue & 0x80000000 ) dacValue = 0;
	else if ( dacValue & 0xFFFF0000 ) dacValue = 0xFFFF;

    unsigned int ctrl = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMUDAC_LEVEL );
    
    if ( !ateWrReg( ctrl, dacValue ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Writing PMUDAC value failed!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    return 1;
}

int drvRdAtePmuVolt( int slotNum, unsigned char chnNum, double * pvolt )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    unsigned int dacValue;
    unsigned int ctrl = adjAteCtrl( chnNum, ATE_READ, ATEADDR_PMUDAC_LEVEL );
    
    if ( !ateRdReg( ctrl, &dacValue ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Writing PMUDAC value failed!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    *pvolt = pmuDacToVolt( dacValue );
    
    return 1;
}

int drvWrAteVtMode( int slotNum, unsigned char chnNum )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    /*--- Tell FPGA Stop Driving ---*/
	unsigned char fpgaPmuChnmask = PCI_Rd16( ADDR_PMU_HZ_VT_ENABLE );
    PCI_Wr( ADDR_PMU_HZ_VT_ENABLE, fpgaPmuChnmask | chnNumToPmuChnBitMask( chnNum ));

    
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
	/*--- Congfigure ADATE305 PMU State register ---*/
    unsigned int sttRegVal = (1<<BIT_PMU_CLAMP_EN)|(1<<BIT_PMU_RANGE_B2)|(1<<BIT_PMU_RANGE_B1)|(1<<BIT_PMU_RANGE_B0);
    unsigned int sttCtrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_STATE );
    
    if ( !ateWrReg( sttCtrlVal, sttRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }

    /*--- Configure ADATE305 as Vt Mode ---*/
    unsigned int pmuRegVal = (1<<BIT_PMU_FORCE_VT);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PE_PMU_ENABLE );
    
    if ( !ateWrReg( ctrlVal, pmuRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write %X, %X register failed\n", __LINE__, __FILE__, ctrlVal, pmuRegVal );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAtePatMode( int slotNum, unsigned char chnNum )
{
	if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
	/*--- Config ADATE305 Disable PMU ---*/
    unsigned int pmuRegVal = (0<<BIT_PMU_EN)|(1<<BIT_PMU_CLAMP_EN)|(0<<BIT_PE_DISABLE);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PE_PMU_ENABLE );
    
    if ( !ateWrReg( ctrlVal, pmuRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
	
    /*--- Tell FPGA Disconnect PMU Driving ---*/
	unsigned char fpgaPmuChnmask = PCI_Rd16( ADDR_PMU_HZ_VT_ENABLE );
    PCI_Wr( ADDR_PMU_HZ_VT_ENABLE, fpgaPmuChnmask | chnNumToPmuChnBitMask( chnNum ));
	/*--- Tell FPGA Start Driving ---*/
	unsigned char fpgaPmuChnmask2 = PCI_Rd16( ADDR_PMU_MODE_ENABLE );
    PCI_Wr( ADDR_PMU_MODE_ENABLE, fpgaPmuChnmask2 & ( 0xFFFF ^ chnNumToPmuChnBitMask( chnNum )));
    
    return 1;
}

int drvWrAteHzMode( int slotNum, unsigned char chnNum )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    /*--- Tell FPGA Stop Driving ---*/
	unsigned char fpgaPmuChnmask = PCI_Rd16( ADDR_PMU_HZ_VT_ENABLE );
    PCI_Wr( ADDR_PMU_HZ_VT_ENABLE, fpgaPmuChnmask | chnNumToPmuChnBitMask( chnNum ));
    
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
	/*--- Congfigure ADATE305 PMU State register ---*/
    unsigned int sttRegVal = (1<<BIT_PMU_CLAMP_EN);
    unsigned int sttCtrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_STATE );
    
    if ( !ateWrReg( sttCtrlVal, sttRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }

    /*--- Configure ADATE305 as High Impedance Mode ---*/
    unsigned int pmuRegVal = 0;
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PE_PMU_ENABLE );
    
    if ( !ateWrReg( ctrlVal, pmuRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write %X, %X register failed\n", __LINE__, __FILE__, ctrlVal, pmuRegVal );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAteOffMode( int slotNum, unsigned char chnNum )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    /*--- Tell FPGA Stop Driving ---*/
	unsigned char fpgaPmuChnmask = PCI_Rd16( ADDR_PMU_HZ_VT_ENABLE );
    PCI_Wr( ADDR_PMU_HZ_VT_ENABLE, fpgaPmuChnmask | chnNumToPmuChnBitMask( chnNum ));
    
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    /*--- Configure ADATE305 as High Impedance Mode ---*/
    unsigned int pmuRegVal = (1<<BIT_PE_DISABLE);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PE_PMU_ENABLE );
    
    if ( !ateWrReg( ctrlVal, pmuRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write %X, %X register failed\n", __LINE__, __FILE__, ctrlVal, pmuRegVal );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAtePmuMode( int slotNum, unsigned char chnNum )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    /*--- Tell FPGA Stop Driving ---*/
    unsigned int fpgaPmuChnmask;
	fpgaPmuChnmask = PCI_Rd16( ADDR_PMU_HZ_VT_ENABLE );
    PCI_Wr( ADDR_PMU_HZ_VT_ENABLE, fpgaPmuChnmask | chnNumToPmuChnBitMask( chnNum ));
    
	/*--- Tell FPGA Enable PMU Mode ---*/
	fpgaPmuChnmask = PCI_Rd16( ADDR_PMU_MODE_ENABLE );
    PCI_Wr( ADDR_PMU_MODE_ENABLE, fpgaPmuChnmask | chnNumToPmuChnBitMask( chnNum ));

    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    // Doing RECOMMANDED PMU MODE SWITCHING SEQUENCES - PMU Disable to PMU Enable - Step 2: MUST BE FORCE VOLTAGE MODE
    if ( !drvWrAtePmuState( slotNum, chnNum, ATE_CURR_02uA, 0, 1 ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE PMU State failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    /*--- Configure ADATE305 as PMU with clamp mode ---*/
    unsigned int pmuRegVal = (1<<BIT_PMU_EN)|(1<<BIT_PMU_CLAMP_EN)|(1<<BIT_PE_DISABLE);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PE_PMU_ENABLE );
    
    if ( !ateWrReg( ctrlVal, pmuRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }

    return 1;
}

int drvWrAtePmuStateToGnd( int slotNum, unsigned char chnNum )
{
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    /*--- Configure ADATE305's ATE State register ---*/
    unsigned int pmuSttVal = ((0<<BIT_PMU_IN_SEL_B1)|   // Set 2.5V +DUTGND input selection
                              (1<<BIT_PMU_IN_SEL_B0)|   // 
                              (1<<BIT_PMU_CLAMP_EN)|
                              (1<<BIT_PMU_FORCE_CURR_NOT_VOLT)| //  Set force current mode
                              (0<<BIT_PMU_MEAS_CURR_NOT_VOLT)|
                              (000<<BIT_PMU_RANGE_B0)   // 2uA
                             );
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_STATE );
    
    if ( !ateWrReg( ctrlVal, pmuSttVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write %X, %X register failed\n", __LINE__, __FILE__, ctrlVal, pmuSttVal );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAtePmuState( int slotNum, unsigned char chnNum, sAteCurrIndex gsAteCurrIndex, unsigned char forceCurrNotVolt, unsigned char measCurrNotVolt )
{
    ASSERT( gsAteCurrIndex < sizeofarray(psAteCurrRangeInfos) );
    ASSERT( ( psAteCurrRangeInfos[gsAteCurrIndex].regFieldVal & (0xFF^0x07) ) == 0 );
    
    if ( drvOpenAte( slotNum, chnNum ) != 1 )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode!\n", __LINE__, __FILE__);
    #endif
        return 0;
    }
    
    /*--- Configure ADATE305's ATE State register ---*/
    unsigned int pmuSttVal = ((1<<BIT_PMU_IN_SEL_B1)|
                              (1<<BIT_PMU_IN_SEL_B0)|
                              (1<<BIT_PMU_CLAMP_EN)|
                              ((forceCurrNotVolt!=0)<<BIT_PMU_FORCE_CURR_NOT_VOLT)|
                              ((measCurrNotVolt!=0)<<BIT_PMU_MEAS_CURR_NOT_VOLT)|
                              (psAteCurrRangeInfos[gsAteCurrIndex].regFieldVal <<BIT_PMU_RANGE_B0)
                             );
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_STATE );
    
    if ( !ateWrReg( ctrlVal, pmuSttVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write %X, %X register failed\n", __LINE__, __FILE__, ctrlVal, pmuSttVal );
    #endif
        return 0;
    }
    
    return 1;
}

int drvGetAtePmuCurrRangeText( sAteCurrIndex gsAteCurrIndex, TCHAR const ** ppChText )
{
    if ( gsAteCurrIndex >= sizeofarray( psAteCurrRangeInfos ) )
        return 0;
    *ppChText = psAteCurrRangeInfos[ gsAteCurrIndex ].ptext;
    return 1;
}

int drvWrAteForceVolt( int slotNum, unsigned char chnNum, double volt, sAteCurrIndex gsAteCurrIndex , double clampH, double clampL, float delayms=100.0F )
{
    if ( !drvWrAteClampH( slotNum, chnNum, clampH ) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE Clamp High failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    if ( !drvWrAteClampL( slotNum, chnNum, clampL ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE Clamp Low failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    if ( !drvWrAtePmuMode( slotNum, chnNum ) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE PMU Mode failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    if ( !drvWrAtePmuState( slotNum, chnNum, gsAteCurrIndex, 0, 1 ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE PMU State failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    if ( !drvWrAtePmuVolt( slotNum, chnNum, volt ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE PMU DAC value failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAteForceCurr( int slotNum, unsigned char chnNum, double amp, sAteCurrIndex gsAteCurrIndex , double clampH, double clampL, float delayms=100.0F )
{
    if ( !drvWrAteClampH( slotNum, chnNum, clampH ) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE Clamp High failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    if ( !drvWrAteClampL( slotNum, chnNum, clampL ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE Clamp Low failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    if ( !drvWrAtePmuMode( slotNum, chnNum ) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE PMU Mode failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    // Doing RECOMMANDED PMU MODE SWITCHING SEQUENCES - PMU Force Voltage Mode to PMU Force Current Mode - Step 2
    if ( !drvWrAtePmuStateToGnd( slotNum, chnNum ) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE PMU Mode failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    // Doing RECOMMANDED PMU MODE SWITCHING SEQUENCES - PMU Force Voltage Mode to PMU Force Current Mode - Step 3
    if ( !drvWrAtePmuCurr( slotNum, chnNum, amp, gsAteCurrIndex ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE PMU DAC value failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    // Doing RECOMMANDED PMU MODE SWITCHING SEQUENCES - PMU Force Voltage Mode to PMU Force Current Mode - Step 4
    if ( !drvWrAtePmuState( slotNum, chnNum, gsAteCurrIndex, 1, 0 ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ATE PMU State failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    return 1;
}

int drvWrAteReg( int slotNum, unsigned char chnNum, int addr, int data )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    unsigned int ctrl = adjAteCtrl( chnNum, ATE_WRITE, addr );
    if ( !ateWrReg( ctrl, data ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Write ADATE305 register %02Xh failed\n", __LINE__, __FILE__, addr );
    #endif
        return 0;
    }
    
    return 1;
}

int drvRdAteReg( int slotNum, unsigned char chnNum, int addr, int * rdata )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    unsigned int ctrl = adjAteCtrl( chnNum, ATE_READ, addr );
    unsigned int regVal;
    if ( !ateRdReg( ctrl, &regVal ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Read ADATE305 register %02Xh failed\n", __LINE__, __FILE__, addr );
    #endif
        return 0;
    }
    
    *rdata = regVal;
    return 1;
}

int drvRdAteAD( int slotNum, unsigned char chnNum, unsigned int count, long long * pAdcVal32 )
{
	if ( count > 65536 || count == 0 )
	{
	#if _DEBUG
		_cprintf( "Line %d file %s: Slot %d channel %d ADC count should be 1~65536!\n", __LINE__, __FILE__, slotNum, chnNum );
    #endif
		return 0;
	}
    
    if ( !drvOpenAte( slotNum, chnNum ))
	{
	#if _DEBUG
		_cprintf( "Line %d file %s: Slot %d channel %d ADC cannot be read because it stuck in busy mode!\n", __LINE__, __FILE__, slotNum, chnNum );
    #endif
		return 0;
	}
    
    PCI_Wr( ADDR_ADC_WR_CNT_RD_BUSY, count-1 );
    
    #define ADC_TIME_CONSUME_PER_SAMPLE     (0.001)
	long long startTickstamp = getTickstamp();
	long long cpuTickPerSecond = getTicksPerSecond();
    long long expireTickstamp = startTickstamp + (long long)(count * ADC_TIME_CONSUME_PER_SAMPLE * cpuTickPerSecond );
    
    while( 1 )
    {
        if (( PCI_Rd( ADDR_ADC_WR_CNT_RD_BUSY ) & (1<<BIT_ADC_BUSY)) == 0 )
            break;
		long long checkTickstamp = getTickstamp();
        if ( expireTickstamp <= checkTickstamp  )
        {// ADC stuck
        #if _DEBUG
            _cprintf( "Line %d file %s: Slot %d channel %d ADC cannot be read because it stuck in busy mode!\n", __LINE__, __FILE__, slotNum, chnNum );
        #endif
            return 0;
        }
    }
    
    unsigned long long  adcSum;
	unsigned int lowB;
	unsigned int highB;
    
    highB = PCI_Rd16( ADDR_ADC_DATA_H16BIT );
    lowB = PCI_Rd16( ADDR_ADC_DATA_L16BIT );
    adcSum = ((unsigned long long)highB) & 0xFFFFFFFFL;
	adcSum <<= 16;
	adcSum += ((unsigned long long)lowB) & 0xFFFFFFFFL;
    *pAdcVal32 = adcSum / count;
    
    return 1;
}

int drvWrAteMeasCurr( int slotNum, unsigned char chnNum, unsigned int count, double * pAmps )
{
    if ( !drvOpenAte( slotNum, chnNum ) )
	{
	#if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode\n", __LINE__, __FILE__  );
    #endif
		return 0;
	}
    
    /*--- Read back ADATE305's PMU state register ---*/
    unsigned int rdCtrlVal = adjAteCtrl( chnNum, ATE_READ, ATEADDR_PMU_STATE );
    unsigned int sttRegVal;
    if ( !ateRdReg( rdCtrlVal, &sttRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE read %X, %X register failed\n", __LINE__, __FILE__, rdCtrlVal, sttRegVal );
    #endif
        return 0;
    }
    
    /*--- Obtain Current Range from ADAT305's register ---*/
    unsigned int readbackCurrentRangeFieldValue = (( sttRegVal & PMU_RANGE_BITMASK )>>BIT_PMU_RANGE_B0);
    
    /*--- Check and find out the Resistance of that current range ---*/
    double measOhm = psAteCurrRangeInfos[ATE_CURR_02uA].ohm;    // The field is full mapping, b'0xx are all mapped to 2uA
    for ( int i=0; i<sizeofarray(psAteCurrRangeInfos); i++ )
    {
        if ( psAteCurrRangeInfos[i].regFieldVal == readbackCurrentRangeFieldValue )
        {
            measOhm = psAteCurrRangeInfos[i].ohm;
            break;
        }
    }
    
    /*--- Change Measure Purpose ---*/
    unsigned int miSttRegVal = sttRegVal | (1<<BIT_PMU_MEAS_CURR_NOT_VOLT);
    
    /*--- Write back ATE's PMU State Register with measure purpose became current ---*/
    unsigned int wrCtrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_STATE );
    if ( !ateWrReg( wrCtrlVal, miSttRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write back %X, %X register failed\n", __LINE__, __FILE__, wrCtrlVal, miSttRegVal );
    #endif
        return 0;
    }

	/*--- Select Measurement Channel Mux and Enable Measurement ---*/
	unsigned int meaChCtrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_MEAS_EN );
	unsigned int measVal = (1<<BIT_PMU_MEAS_OUT_EN);
	if ( chnNumToAteChnInd( chnNum ) == 1 )
	{
		measVal |= (1<<BIT_PMU_MEAS_CH_SEL_B0);
	}

	if ( !ateWrReg( meaChCtrlVal, measVal))
	{
	#if _DEBUG
        _cprintf("Line %d of file %s: ATE select ADC measurement channel failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
	}
    
    /*--- Start measure ---*/
    long long adcVal32;
    if ( !drvRdAteAD( slotNum, chnNum, count, &adcVal32 ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Measure Current failed becaure ADC cannot be read\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    *pAmps = measAdcToAmp( adcVal32, measOhm );
    return 1;
}

int drvWrAteMeasVolt( int slotNum, unsigned char chnNum, unsigned int count, double * pVolts )
{
    if ( !drvOpenAte( slotNum, chnNum ) )
	{
	#if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode\n", __LINE__, __FILE__  );
    #endif
		return 0;
	}
    
    /*--- Read back ADATE305's PMU state register ---*/
    unsigned int rdCtrlVal = adjAteCtrl( chnNum, ATE_READ, ATEADDR_PMU_STATE );
    unsigned int sttRegVal;
    if ( !ateRdReg( rdCtrlVal, &sttRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE read %X, %X register failed\n", __LINE__, __FILE__, rdCtrlVal, sttRegVal );
    #endif
        return 0;
    }
    
    /*--- Obtain Current Range from ADAT305's register ---*/
    unsigned int readbackCurrentRangeFieldValue = (( sttRegVal & PMU_RANGE_BITMASK )>>BIT_PMU_RANGE_B0);
    
    /*--- Change Measure Purpose to Voltage ---*/
    unsigned int mvSttRegVal = sttRegVal & ((1<<BIT_PMU_MEAS_CURR_NOT_VOLT) ^ 0xFFFFFFFF);
    
    /*--- Write back ATE's PMU State Register with measure purpose became voltage ---*/
    unsigned int wrCtrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_STATE );
    if ( !ateWrReg( wrCtrlVal, mvSttRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write back %X, %X register failed\n", __LINE__, __FILE__, wrCtrlVal, mvSttRegVal );
    #endif
        return 0;
    }

	/*--- Select Measurement Channel Mux and Enable Measurement ---*/
	unsigned int meaChCtrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_MEAS_EN );
	unsigned int measVal = (1<<BIT_PMU_MEAS_OUT_EN);
	if ( chnNumToAteChnInd( chnNum ) == 1 )
	{
		measVal |= (1<<BIT_PMU_MEAS_CH_SEL_B0);
	}

	if ( !ateWrReg( meaChCtrlVal, measVal))
	{
	#if _DEBUG
        _cprintf("Line %d of file %s: ATE select ADC measurement channel failed\n", __LINE__, __FILE__ );
    #endif
        return 0;
	}
    
    /*--- Start measure ---*/
    long long adcVal32;
    if ( !drvRdAteAD( slotNum, chnNum, count, &adcVal32 ))
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: Measure Voltage failed becaure ADC cannot be read\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
    *pVolts = measAdcToVolt( adcVal32 );
    return 1;
}

int drvWrAteMeasOff( int slotNum, int chnNum )
{
	if ( !drvOpenAte( slotNum, chnNum ) )
	{
	#if _DEBUG
        _cprintf("Line %d of file %s: ATE stuck in busy mode\n", __LINE__, __FILE__  );
    #endif
		return 0;
	}
    
    /*--- Read back ADATE305's PMU state register ---*/
    unsigned int wrCtrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PMU_MEAS_EN );
    if ( ! ateWrReg( wrCtrlVal, 0 ))
	{
	#if _DEBUG
        _cprintf("Line %d of file %s: ATE turn off ADC failed\n", __LINE__, __FILE__ );
    #endif
		return 0;
	}
    return 1;
}

int drvWrPatCht( int slotNum
               , int chnNum
               , unsigned int ts
               , unsigned int cht
               , unsigned int d0Loc
               , unsigned int d0Dat
               , unsigned int d1Loc
               , unsigned int d1Dat
               , unsigned int cLoc
               , unsigned int cLoc2
               , unsigned int cDat
               , unsigned int cMod
               , unsigned int io0Loc
               , unsigned int io0Dat
               , unsigned int io1Loc
               , unsigned int io1Dat )
{	
    int fpgaNum = chnNumToFpgaNum( chnNum );
    int chnInd = chnNum - 1;
    
    ASSERT( fpgaNum <= drvGetLastChnFpgaNumOnPeb64() );
    
    drvOpen( slotNum, fpgaNum );
	
    /*--- Prevent the field from overflow ---*/
    
    // Note: Those are protocol limits, not product limit, for example
    ASSERT( ts < 64 );
    ASSERT( cht < 16 );
    ASSERT( d0Dat < 4 );
    ASSERT( d1Dat < 4 );
    ASSERT( cDat < 4 );
    ASSERT( cMod < 4 ); ///todo: expend mode?
    ASSERT( io0Dat < 4 );
    ASSERT( io1Dat < 4 );
    
    /*--- Select drive marker and channel in slot ---*/
	unsigned int selChDrv = adjTimslotSel( DRIVE_MARKER, chnInd );
	PCI_Wr( ADDR_TIMESLOT_CONTENT_SEL, selChDrv );
    
    /*--- Assign address ---*/
	unsigned int addr = timeslotChtToMemAddr( ts, cht );
	PCI_Wr( ADDR_TIMESLOT_ADDR, addr );
    
    /*--- Write data marker ---*/
    PCI_Wr( ADDR_TIMESLOT_DATA_H, adjTimeslotMarkerHi( d0Loc, d0Dat ) );
    PCI_Wr( ADDR_TIMESLOT_DATA_L, adjTimeslotMarkerLo( d0Loc        ) );
    
    PCI_Wr( ADDR_TIMESLOT_DATA_H, adjTimeslotMarkerHi( d1Loc, d1Dat ) );
    PCI_Wr( ADDR_TIMESLOT_DATA_L, adjTimeslotMarkerLo( d1Loc        ) );
    
    
    
    
    /*--- Select compare marker and channel in slot ---*/
	unsigned int selChCmp = adjTimslotSel( COMPARE_MARKER, chnInd );
	PCI_Wr( ADDR_TIMESLOT_CONTENT_SEL, selChCmp );
    
    /*--- Assign address ---*/
	PCI_Wr( ADDR_TIMESLOT_ADDR, addr );
    
    /*--- Write io marker ---*/
    PCI_Wr( ADDR_TIMESLOT_DATA_H, adjTimeslotMarkerHi( cLoc, cDat  ) ); ///todo: adapt to new spec
    PCI_Wr( ADDR_TIMESLOT_DATA_L, adjTimeslotMarkerLo( cLoc        ) );
    
    PCI_Wr( ADDR_TIMESLOT_DATA_H, adjTimeslotMarkerHi( cLoc2, cMod ) );
    PCI_Wr( ADDR_TIMESLOT_DATA_L, adjTimeslotMarkerLo( cLoc2       ) );
    
    
    
    /*--- Select io marker and channel in slot ---*/
	unsigned int selChIo = adjTimslotSel( IO_MARKER, chnInd );
	PCI_Wr( ADDR_TIMESLOT_CONTENT_SEL, selChIo );
    
    /*--- Assign address ---*/
	PCI_Wr( ADDR_TIMESLOT_ADDR, addr );
    
    /*--- Write io marker ---*/
    PCI_Wr( ADDR_TIMESLOT_DATA_H, adjTimeslotMarkerHi( io0Loc, io0Dat ) );
    PCI_Wr( ADDR_TIMESLOT_DATA_L, adjTimeslotMarkerLo( io0Loc         ) );
    
    PCI_Wr( ADDR_TIMESLOT_DATA_H, adjTimeslotMarkerHi( io1Loc, io1Dat ) );
    PCI_Wr( ADDR_TIMESLOT_DATA_L, adjTimeslotMarkerLo( io1Loc         ) );
    
    return 1;
}

int drvRdPatCht( sTimeslotChannelInfo * psTimeslotChannelInfo
               , int slotNum
               , int chnNum
               , unsigned int ts
               , unsigned int cht)
{
    int fpgaNum = chnNumToFpgaNum( chnNum );
    int chnInd = chnNum - 1;
    
    ASSERT( fpgaNum <= drvGetLastChnFpgaNumOnPeb64() );
    
    drvOpen( slotNum, fpgaNum );
	
    /*--- Prevent the field from overflow ---*/
    
    // Note: Those are protocol limits, not product limit, for example
    ASSERT( ts < 64 );
    ASSERT( cht < 16 );
    
    /*--- Select drive marker and channel in slot ---*/
	PCI_Wr( ADDR_TIMESLOT_CONTENT_SEL, adjTimslotSel( DRIVE_MARKER, chnInd ));
    
    /*--- Assign address ---*/
	PCI_Wr( ADDR_TIMESLOT_ADDR, timeslotChtToMemAddr( ts, cht ));
    
    /*--- Read data marker ---*/
    unsigned int d0h = PCI_Rd16( ADDR_TIMESLOT_DATA_H );
    unsigned int d0l = PCI_Rd16( ADDR_TIMESLOT_DATA_L );
    
    unsigned int d1h = PCI_Rd16( ADDR_TIMESLOT_DATA_H );
    unsigned int d1l = PCI_Rd16( ADDR_TIMESLOT_DATA_L );

#define timeslotMemHighByteToDat(_HB)      (((_HB)>>14) & 3)
#define timeslotMemToLoc(_HB, _LB)         ((( (_HB) &0x3FFF)<<16) | ((_LB)&0xFFFF));
    psTimeslotChannelInfo->d0Dat = timeslotMemHighByteToDat( d0h );
    psTimeslotChannelInfo->d0Loc = timeslotMemToLoc        ( d0h, d0l );
    psTimeslotChannelInfo->d1Dat = timeslotMemHighByteToDat( d1h );
    psTimeslotChannelInfo->d1Loc = timeslotMemToLoc        ( d1h, d1l );
    
    
    /*--- Select drive marker and channel in slot ---*/
	PCI_Wr( ADDR_TIMESLOT_CONTENT_SEL, adjTimslotSel( COMPARE_MARKER, chnInd ));
    
    /*--- Assign address ---*/
	PCI_Wr( ADDR_TIMESLOT_ADDR, timeslotChtToMemAddr( ts, cht ));
    
    /*--- Read compare marker ---*/
    unsigned int c0h = PCI_Rd16( ADDR_TIMESLOT_DATA_H );
    unsigned int c0l = PCI_Rd16( ADDR_TIMESLOT_DATA_L );
    
    unsigned int c1h = PCI_Rd16( ADDR_TIMESLOT_DATA_H );
    unsigned int c1l = PCI_Rd16( ADDR_TIMESLOT_DATA_L );
    
    psTimeslotChannelInfo->cDat  = timeslotMemHighByteToDat( c0h );
    psTimeslotChannelInfo->cLoc  = timeslotMemToLoc        ( c0h, c0l );
#define timeslotMemHighByteToMod(_HB)      (((_HB)>>14) & 3)
    psTimeslotChannelInfo->cMod  = timeslotMemHighByteToMod( c1h );
    psTimeslotChannelInfo->cLoc2 = timeslotMemToLoc        ( c1h, c1l );
    
    
    
    
    /*--- Select drive marker and channel in slot ---*/
	PCI_Wr( ADDR_TIMESLOT_CONTENT_SEL, adjTimslotSel( IO_MARKER, chnInd ));
    
    /*--- Assign address ---*/
	PCI_Wr( ADDR_TIMESLOT_ADDR, timeslotChtToMemAddr( ts, cht ));
    
    /*--- Read io marker ---*/
    unsigned int io0h = PCI_Rd16( ADDR_TIMESLOT_DATA_H );
    unsigned int io0l = PCI_Rd16( ADDR_TIMESLOT_DATA_L );
    
    unsigned int io1h = PCI_Rd16( ADDR_TIMESLOT_DATA_H );
    unsigned int io1l = PCI_Rd16( ADDR_TIMESLOT_DATA_L );
    
    psTimeslotChannelInfo->io0Dat = timeslotMemHighByteToDat( io0h );
    psTimeslotChannelInfo->io0Loc = timeslotMemToLoc        ( io0h, io0l );
    psTimeslotChannelInfo->io1Dat = timeslotMemHighByteToDat( io1h );
    psTimeslotChannelInfo->io1Loc = timeslotMemToLoc        ( io1h, io1l );
    
    
    return 1;
}

int drvRdPatTs( int slotNum, int fpgaNum, int ts, unsigned int * pdata )
{
	drvOpen( slotNum, fpgaNum );
	
    /*--- Prevent the field from overflow ---*/
    
    // Note: Those are protocol limits, not product limit, for example
    ASSERT( ts < 64 );
    
    /*--- Address to specific time slot ---*/
	PCI_Wr( ADDR_TIMESLOT_PERIOD_ADDR, ts );
    
	/*--- Read ---*/
	unsigned int rdhi, rdlo;
	PCI_Wr( ADDR_TIMESLOT_PERIOD_ADDR, ts );
	rdhi = PCI_Rd16( ADDR_TIMESLOT_PERIOD_H );
	rdlo = PCI_Rd16( ADDR_TIMESLOT_PERIOD_L );

	*pdata = (rdhi<<16) + rdlo;
	return 1;
}

int drvWrPatTs( int slotNum
               , int fpgaNum
               , unsigned int ts
               , unsigned int tm )
{
	drvOpen( slotNum, fpgaNum );
	
    /*--- Prevent the field from overflow ---*/
    
    // Note: Those are protocol limits, not product limit, for example
    ASSERT( ts < 64 );
    
    /*--- Address to specific time slot ---*/
	PCI_Wr( ADDR_TIMESLOT_PERIOD_ADDR, ts );
    
    /*--- Write period ---*/
	PCI_Wr( ADDR_TIMESLOT_PERIOD_H, (tm>>16)&0xFFFF );
	PCI_Wr( ADDR_TIMESLOT_PERIOD_L, tm&0xFFFF );
	
	/*--- Read back check ---*/
	unsigned int rdhi, rdlo;
	PCI_Wr( ADDR_TIMESLOT_PERIOD_ADDR, ts );
	rdhi = PCI_Rd16( ADDR_TIMESLOT_PERIOD_H );
	rdlo = PCI_Rd16( ADDR_TIMESLOT_PERIOD_L );
	if ( tm == (rdhi<<16) + rdlo )
		return 1;
	else
		return 0;
}


int drvPatBurstPrepareIgnoreFailed( int slotNum )
{
    // Channel FPGAs
    for ( int fpgaNum=drvGetFirstChnFpgaNumOnPeb64()
        ; fpgaNum<=drvGetLastChnFpgaNumOnPeb64()
        ; fpgaNum++ )
    {
        drvOpen( slotNum, fpgaNum );
        
        PCI_Wr( ADDR_PAT_FAILED_REACTION, FAIL_IGNORE );
        PCI_Wr( ADDR_PAT_BURST_PREPARE, BURST_PREPARE );
    }
        
	return 1;
}

int drvPatBurstPrepareStopWhenAllSiteFailed( int slotNum )
{
    // Channel FPGAs
    for ( int fpgaNum=drvGetFirstChnFpgaNumOnPeb64()
        ; fpgaNum<=drvGetLastChnFpgaNumOnPeb64()
        ; fpgaNum++ )
    {
        drvOpen( slotNum, fpgaNum );
        
        PCI_Wr( ADDR_PAT_FAILED_REACTION, FAIL_STOP );
        PCI_Wr( ADDR_PAT_BURST_PREPARE, BURST_PREPARE );
    }
        
	return 1;
}

int drvPatBurstPrepareStopFailedReachLimit( int slotNum, unsigned int limit16bit )
{
    // Channel FPGAs
    ASSERT( limit16bit<=drvGetPatMaxFailLimit() );
    
    for ( int fpgaNum=drvGetFirstChnFpgaNumOnPeb64()
        ; fpgaNum<=drvGetLastChnFpgaNumOnPeb64()
        ; fpgaNum++ )
    {
        drvOpen( slotNum, fpgaNum );
        
        PCI_Wr( ADDR_PAT_FAILED_REACTION, FAIL_LIMIT );
        PCI_Wr( ADDR_PAT_BURST_PREPARE, BURST_PREPARE );
        PCI_Wr( ADDR_PAT_FAILED_LIMIT, limit16bit );
    }
    
	return 1;
}

int drvPatMasterStartBurst( void )
{
    /*--- Search Master ---*/
    for ( int sn=1; sn< 1+getSupportSlotAmount(); sn++ )
    {
        if ( drvRdMainBrdId( sn ) != PEB64_ID )
            continue;
        
        // Main FPGA
        drvOpen( sn, MAIN_FPGA_NUMBER );
        unsigned int rdata = PCI_Rd16( ADDR_PAT_SET_MASTER_BOARD );
        
        // Found Master 
        if ( ( rdata & 1 ) == THIS_IS_PAT_MASTER )
        {// Start Burst Mode
            PCI_Wr( ADDR_PAT_BURST_START, START_BURST_MODE );
            return 1;
        }
    }
    /*--- No Master Found ---*/
    return 0;
}

int drvPatSetMaster( int slotNum )
{
    /*--- Set every main FPGAs as slave ---*/
    for ( int sn=1; sn< 1+getSupportSlotAmount(); sn++ )
    {
        if ( drvRdMainBrdId( sn ) != PEB64_ID )
            continue;
        
        // Main FPGA
        drvOpen( sn, MAIN_FPGA_NUMBER );
        
        PCI_Wr( ADDR_PAT_SET_MASTER_BOARD, THIS_IS_NOT_PAT_MASTER );
    }
	
    /*--- Set the assigned FPGA as main ---*/
    drvOpen( slotNum, MAIN_FPGA_NUMBER );
    
	PCI_Wr( ADDR_PAT_SET_MASTER_BOARD, THIS_IS_PAT_MASTER );
    
	return 1;
}

int drvPatResetChannels( int slotNum )
{
    drvOpen( slotNum, MAIN_FPGA_NUMBER );
    
    PCI_Wr( ADDR_PAT_RESET_CHANNELS, START_RESET_CHANNEL );

	return 1;
}

/*! @brief   Function for hooking channels to fail link of specific channel FPGA
 *  @details 
 *  @param   slotNum 
 *  @param   chnFpgaNum : Channel FPGA number, 1~4
 *  @param   linkNum : Link number of a channel FPGA, 1~2
 *  @param   fpgaChnNum : FPGA's channel number, 1~16
 *  @return  int 1 if success else 0
 */
int drvFailLinkHookChn( int slotNum, int chnFpgaNum, int linkNum, int fpgaChnNum )
{
    ASSERT( linkNum >= 1 );
    ASSERT( linkNum <= (ADDR_LAST_LINK-ADDR_FISRT_LINK+1) );
    ASSERT( fpgaChnNum >= 1 );
    ASSERT( fpgaChnNum <= drvGetAteChnAmountPerFpga() );
    
    drvOpen( slotNum, chnFpgaNumToRealFpgaNum( chnFpgaNum ) );
    
    unsigned int addr = linkNumToLinkEnableAddress( linkNum );
    unsigned int data = adjLinkEnableMask( fpgaChnNum );
    
    unsigned int val = PCI_Rd16( addr );
    unsigned int newdata = val | data;
    
    PCI_Wr( addr, newdata );
    
    return 1;
}

int drvWrFailLinkHookReg( int slotNum, int chnFpgaNum, int linkNum, unsigned int data )
{
    ASSERT( linkNum >= 1 );
    ASSERT( linkNum <= (ADDR_LAST_LINK-ADDR_FISRT_LINK+1) );
    
	int fpgaNum = chnFpgaNumToRealFpgaNum( chnFpgaNum );
    drvOpen( slotNum, fpgaNum );
    
    unsigned int addr = linkNumToLinkEnableAddress( linkNum );
    
    PCI_Wr( addr, data );
    
    return 1;
}

int drvFailLinkClearHooks( int slotNum, int chnFpgaNum, int linkNum )
{
    return drvWrFailLinkHookReg( slotNum, chnFpgaNum, linkNum, 0 );
}

/*! @brief   Function for hooking fail link to fail site
 *  @details 
 *  @param   slotNum : Slot number, 1~16
 *  @param   siteNum : Fail bus site number, 1~16
 *  @param   chnFpgaNum : Channel FPGA number, 1~4
 *  @param   linkNum : Link number of a channel FPGA, 1~2
 *  @return  int 1 if success else 0
 */
int drvFailSiteHookLink( int slotNum, int siteNum, int chnFpgaNum, int linkNum )
{
    ASSERT( siteNum >= 1 );
    ASSERT( siteNum <= (ADDR_LAST_SITE-ADDR_FISRT_SITE+1) );
    
    drvOpen( slotNum, MAIN_FPGA_NUMBER );
    
    unsigned int addr = siteNumToSiteEnableAddress( siteNum );
    unsigned int data = adjSiteEnableMask( chnFpgaNum, linkNum );
    
    unsigned int val = PCI_Rd16( addr );
    unsigned int newdata = val | data;
    
    PCI_Wr( addr, newdata );
    
    return 1;
}

int drvWrFailSiteHookReg( int slotNum, int siteNum, unsigned int data )
{
    ASSERT( siteNum >= 1 );
    ASSERT( siteNum <= (ADDR_LAST_SITE-ADDR_FISRT_SITE+1) );
    
    drvOpen( slotNum, MAIN_FPGA_NUMBER );
    
    unsigned int addr = siteNumToSiteEnableAddress( siteNum );
    
    PCI_Wr( addr, data );
    
    return 1;
}

int drvFailSiteClearHooks( int slotNum, int siteNum )
{
    return drvWrFailSiteHookReg( slotNum, siteNum, 0 );
}

/*! @brief   Function for electing fail bus master for each site
 *  @details Assuring that site_enable registers are all set correctly before
             electing master
 *  @param   None 
 *  @return  int
 */
int drvFailSiteElectMaster( void )
{
    unsigned int masterAvalible = 0xFFFFFFFF;
    for ( int slotNum=1; slotNum<=getSupportSlotAmount(); slotNum++ )
    {
        if ( drvRdMainBrdId( slotNum ) != PEB64_ID ) continue;
        
        drvOpen( slotNum, MAIN_FPGA_NUMBER );
        
        unsigned int slaveBits = 0;
        for ( unsigned int addr=ADDR_FISRT_SITE, siteNum=1; addr<=ADDR_LAST_SITE; siteNum++, addr++ )
            if ( PCI_Rd16(addr) != 0 ) slaveBits |= adjFailMasterMask( siteNum );
        
        unsigned int newMasterBits = slaveBits & masterAvalible;
        
        PCI_Wr( ADDR_FAIL_MASTER_ENABLE, newMasterBits );
        
        masterAvalible &= ( newMasterBits^0xFFFFFFFF );
    }
    
    return 1;
}


// Match Site
int drvMatchSiteClearFpgaEnable( int slotNum )
{
    drvOpen( slotNum, MAIN_FPGA_NUMBER );
    
    for ( unsigned int addr=ADDR_FISRT_MATCH_SITE; addr<=ADDR_LAST_MATCH_SITE; addr++ )
    {
        PCI_Wr( addr, MATCH_SITE_ALL_CH_DISABLE );
    }
    return 1;
}

int drvMatchSiteSetFpgaEnable( int slotNum, int matchSiteNum, unsigned int fgpaEnBits )
{
    ASSERT( matchSiteNum >= 1 );
    ASSERT( matchSiteNum <= lastMatchSiteNum() );
    
    drvOpen( slotNum, MAIN_FPGA_NUMBER );
    
    unsigned int addr = matchSiteNumToMatchSiteAddress( matchSiteNum );
    
    PCI_Wr( addr, fgpaEnBits );
    
    return 1;
}


int drvMatchSiteClearLinkChannel( int slotNum, int chnFpgaNum )
{
    int fpgaNum = chnFpgaNumToRealFpgaNum( chnFpgaNum );
    
    drvOpen( slotNum, fpgaNum );
    
    PCI_Wr( ADDR_MATCH_TO_FAIL_LINK_SEL, MATCH_TO_FAIL_LINK_1 );
    PCI_Wr( ADDR_MATCH_CH_ENABLE, MATCH_CH_ALL_CH_DISABLE );
    
    return 1;
}

int drvMatchSiteSetLinkChannel( int slotNum, int chnFpgaNum, unsigned int fpgaChnEnBits )
{
    int fpgaNum = chnFpgaNumToRealFpgaNum( chnFpgaNum );
    
    drvOpen( slotNum, fpgaNum );
    
    PCI_Wr( ADDR_MATCH_CH_ENABLE, fpgaChnEnBits );
    
    return 1;
}


int drvMatchSiteElectMaster( void )
{
    unsigned int masterAvalible = 0xFFFFFFFF;
    for ( int slotNum=1; slotNum<=getSupportSlotAmount(); slotNum++ )
    {
        if ( drvRdMainBrdId( slotNum ) != PEB64_ID ) continue;
        
        drvOpen( slotNum, MAIN_FPGA_NUMBER );
        
        unsigned int slaveBits = 0;
        for ( unsigned int addr=ADDR_FISRT_MATCH_SITE, matchSiteNum=1; addr<=ADDR_LAST_MATCH_SITE; matchSiteNum++, addr++ )
            if ( PCI_Rd16(addr) != 0 ) slaveBits |= adjMatchMasterMask( matchSiteNum );
        
        unsigned int newMasterBits = slaveBits & masterAvalible;
        
        PCI_Wr( ADDR_MATCH_MASTER_ENABLE, newMasterBits );
        
        masterAvalible &= ( newMasterBits^0xFFFFFFFF );
    }
    
    return 1;
}











// Log Trigger Setup

int drvLogSetVectorAddr( int slotNum, int fpgaNum, unsigned int addr )
{
    drvOpen( slotNum, fpgaNum );
    
    PCI_Wr( ADDR_LOG_START_CYCLE_COUNT_H, (addr>>16)&0xFFFF );
    PCI_Wr( ADDR_LOG_START_CYCLE_COUNT_L, addr&0xFFFF );
    
    return 1;
}

int drvLogSetCycleCount( int slotNum, int fpgaNum, unsigned int cycleCount )
{
    drvOpen( slotNum, fpgaNum );
    
    PCI_Wr( ADDR_LOG_START_CYCLE_COUNT_H, (cycleCount>>16)&0xFFFF );
    PCI_Wr( ADDR_LOG_START_CYCLE_COUNT_L, cycleCount&0xFFFF );
    
    return 1;
}

int drvLogSetControl( int slotNum, int fpgaNum, sLogMode gsLogMode, unsigned int recoverEnable )
{
    drvOpen( slotNum, fpgaNum );
    
    unsigned int data = adjLogModeContorl( gsLogMode, recoverEnable );
    
    PCI_Wr( ADDR_LOG_MODE_CONTROL, data );
    
    return 1;
}

int drvGetLastLogAddress( int slotNum, int fpgaNum, unsigned int * pRecovered, unsigned int * pAddress )
{
    drvOpen( slotNum, fpgaNum );
    
    unsigned int status = PCI_Rd16( ADDR_LOG_MEMORY_STATUS );
    
    *pAddress = getDataLastLogAddr( status );
    *pRecovered = getDataLogRecover( status );
    
    return 1;
}

int xdrvReadLogMem( int slotNum, int fpgaNum, unsigned int startAddress, unsigned int lastAddress, sLogData * psLogData )
{
    drvOpen( slotNum, fpgaNum );
    
    PCI_Wr( ADDR_SET_LOG_MEM_ADDR_FOR_READ, startAddress );
    
    for ( unsigned int ind=0; ind<=(lastAddress-startAddress); ind++ )
    {
        unsigned int rdata[8];
        for ( int rdCnt=0; rdCnt<8; rdCnt++ )
        {
            rdata[ rdCnt ] = PCI_Rd16( ADDR_READ_LOG_MEM_DATA );
        }
        
        psLogData[ ind ].cycleCount  = (rdata[0]<<23)     | rdata[1]      | ((rdata[2]>>9)&0xEF);
        psLogData[ ind ].vectorAddr  = (rdata[2]&0x1FF)   | (rdata[3]>>2);
        psLogData[ ind ].LoopCount   = ((rdata[3]&3)<<14) | (rdata[4]>>2);
        psLogData[ ind ].siteFail    = rdata[4]&3;
        psLogData[ ind ].channelFail = rdata[5];
        psLogData[ ind ].channelsCompareFail = (rdata[6]<<16)  | rdata[7];
    }
    
    return 1;
}

int drvReadLogMem( int slotNum, int fpgaNum, unsigned int startAddress, unsigned int lastAddress, sLogData * psLogData )
{
    drvOpen( slotNum, fpgaNum );
    
    PCI_Wr( ADDR_SET_LOG_MEM_ADDR_FOR_READ, startAddress );
    
    for ( unsigned int ind=0; ind<=(lastAddress-startAddress); ind++ )
    {
        unsigned int rdata[8];
        for ( int rdCnt=0; rdCnt<8; rdCnt++ )
        {
            rdata[ rdCnt ] = PCI_Rd16( ADDR_READ_LOG_MEM_DATA );
        }
        
        psLogData[ ind ].cycleCount  = ALIGN3( rdata[0], 8, 0, rdata[1], 15, 0, rdata[2], 15, 9 );
        psLogData[ ind ].vectorAddr  = ALIGN2( rdata[2], 8, 0, rdata[3], 15, 2 );
        psLogData[ ind ].LoopCount   = ALIGN2( rdata[3], 1, 0, rdata[4], 15, 2 );
        psLogData[ ind ].siteFail    = FIELD( rdata[4], 1, 0 );
        psLogData[ ind ].channelFail = rdata[5];
        psLogData[ ind ].channelsCompareFail = (rdata[6]<<16)  | rdata[7];
    }
    
    return 1;
}

/*! @brief   Function for enabling TMU frequency unit
 *  @details 
 *  @param   slotNum 
 *  @param   chnNum : 1~64
 *  @param   tmuCmp : TMU_CMP_H, TMU_CMP_L or TMU_CMP_HL
 *  @param   periodInNanoSec : 1 ~ 0xFFFFFFFF ( nano second )
 *  @return  
 */
int drvTmuStartMeasFreq( int slotNum, int chnNum, unsigned int tmuCmp, unsigned int periodInNanoSec )
{
    drvWrAteTmuMode( slotNum, chnNum );
    
    int fpgaNum = chnNumToFpgaNum( chnNum );
    
    drvOpen( slotNum, fpgaNum );
    
    int tmuInd = chnNumToTmuInd( chnNum );
    
    /*--- Set Period ---*/
    unsigned int addrPrdH = tmuInd==0 ? ADDR_TMU1_FREQ_W_PRD_H_R_CNT_H : ADDR_TMU2_FREQ_W_PRD_H_R_CNT_H;
    unsigned int addrPrdL = tmuInd==0 ? ADDR_TMU1_FREQ_W_PRD_L_R_CNT_L : ADDR_TMU2_FREQ_W_PRD_L_R_CNT_L;
    
	unsigned int dataH = periodInNanoSec>>16;
	unsigned int dataL = periodInNanoSec&0xFFFF;
    PCI_Wr ( addrPrdH, dataH );
    PCI_Wr ( addrPrdL, dataL );
    
    /*--- Start ---*/
    unsigned int freqModeAddr = tmuInd==0 ? ADDR_TMU1_FREQ_MODE : ADDR_TMU2_FREQ_MODE;
    
    unsigned int freqModeData = adjTmuFreqStart( tmuCmp, chnNumToTmuChnSel( chnNum ) );
    
    PCI_Wr( freqModeAddr, freqModeData );
    
    return 1;
}

/*! @brief   Function for 
 *  @details 
 *  @param   slotNum 
 *  @param   chnNum 
 *  @param   pBusyNotDone : Output, 1 if busy else 0.  If busy, data in "pOverflow" and "pCounter" are invalid
 *  @param   pOverflow : Output
 *  @param   pCounter  : Output
 *  @return  int
 */
int drvTmuGetFreqMeasResult( int slotNum, int chnNum, unsigned int * pBusyNotDone, unsigned int * pOverflow, unsigned int * pCounter )
{
    int fpgaNum = chnNumToFpgaNum( chnNum );
    
    drvOpen( slotNum, fpgaNum );
    
    int tmuInd = chnNumToTmuInd( chnNum );
    
    /*--- Start ---*/
    unsigned int freqModeAddr = tmuInd==0 ? ADDR_TMU1_FREQ_MODE : ADDR_TMU2_FREQ_MODE;
    
    unsigned int freqStateData = PCI_Rd16( freqModeAddr );
    
    unsigned int busy = workingStateFromTmuFregStt( freqStateData );
    
    if ( !busy )
    {
        unsigned int addrCntH = tmuInd==0 ? ADDR_TMU1_FREQ_W_PRD_H_R_CNT_H : ADDR_TMU2_FREQ_W_PRD_H_R_CNT_H;
        unsigned int addrCntL = tmuInd==0 ? ADDR_TMU1_FREQ_W_PRD_L_R_CNT_L : ADDR_TMU2_FREQ_W_PRD_L_R_CNT_L;
        
        unsigned int cntH = PCI_Rd16( addrCntH );
        unsigned int cntL = PCI_Rd16( addrCntL );
        
        * pOverflow = overflowFromTmuCntH( cntH );
        * pCounter = counterFromTmuCntHL( cntH, cntL );
    }
    
    *pBusyNotDone = busy;
    
    return 1;
    
}

/*! @brief   Function for enabling TMU period unit
 *  @details 
 *  @param   slotNum 
 *  @param   chnNum  : 1~64
 *  @param   tmuCmp :  TMU_CMP_H 
                       TMU_CMP_L 
                       TMU_CMP_HL
 *  @param   prdMode : TMU_PRD_MODE_R2R
                       TMU_PRD_MODE_F2F
                       TMU_PRD_MODE_R2F
                       TMU_PRD_MODE_F2R
 *  @return  int
 */
int drvTmuStartMeasPeriod( int slotNum, int chnNum, unsigned int tmuCmp, unsigned int prdMode )
{
    drvWrAteTmuMode( slotNum, chnNum );
    
    int fpgaNum = chnNumToFpgaNum( chnNum );
    
    drvOpen( slotNum, fpgaNum );
    
    int tmuInd = chnNumToTmuInd( chnNum );
    
    /*--- Start ---*/
    unsigned int prdModeAddr = tmuInd==0 ? ADDR_TMU1_PRD_MODE : ADDR_TMU2_PRD_MODE;
    
    unsigned int prdModeData = adjTmuPrdStart( prdMode, tmuCmp, chnNumToTmuChnSel( chnNum ) );
    
    PCI_Wr( prdModeAddr, prdModeData );
    
    return 1;
}

int drvTmuStopMeasPeriod( int slotNum, int chnNum )
{
    int fpgaNum = chnNumToFpgaNum( chnNum );
    
    drvOpen( slotNum, fpgaNum );
    
    int tmuInd = chnNumToTmuInd( chnNum );
    
    /*--- Start ---*/
    unsigned int prdModeAddr = tmuInd==0 ? ADDR_TMU1_PRD_MODE : ADDR_TMU2_PRD_MODE;
    
    unsigned int prdModeData = adjTmuPrdStop( );
    
    PCI_Wr( prdModeAddr, prdModeData );
    
    return 1;
}

int drvTmuGetPeriodMeasResult( int slotNum, int chnNum, unsigned int * pBusyNotDone, unsigned int * pTriggered, unsigned int * pOverflow, unsigned int * pPeriodInNanoSec )
{
    int fpgaNum = chnNumToFpgaNum( chnNum );
    
    drvOpen( slotNum, fpgaNum );
    
    int tmuInd = chnNumToTmuInd( chnNum );
    
    /*--- Start ---*/
    unsigned int prdModeAddr = tmuInd==0 ? ADDR_TMU1_PRD_MODE : ADDR_TMU2_PRD_MODE;
    
    unsigned int prdStateData = PCI_Rd16( prdModeAddr );
    
    unsigned int busy = workingStateFromTmuPrdStt( prdStateData );
    unsigned int trig = triggerStateFromTmuPrdStt( prdStateData );
    
    if ( !busy )
    {
        unsigned int addrMeasH = tmuInd==0 ? ADDR_TMU1_PRD_MEAS_H : ADDR_TMU2_PRD_MEAS_H;
        unsigned int addrMeasL = tmuInd==0 ? ADDR_TMU1_PRD_MEAS_L : ADDR_TMU2_PRD_MEAS_L;
        
        unsigned int measH = PCI_Rd16( addrMeasH );
        unsigned int measL = PCI_Rd16( addrMeasL );
        
        * pOverflow = overflowFromTmuPrdMeasH( measH );
        * pPeriodInNanoSec = measurementFromTmuPrdMeasHL( measH, measL );
    }
    
    *pBusyNotDone = busy;
    *pTriggered = trig;
    
    return 1;
}

/*! @brief   Function for enabling TMU edge unit
 *  @details 
 *  @param   slotNum 
 *  @param   chnNum1 
 *  @param   tmuCmp1 
 *  @param   tmuEdge1 : TMU_EDGE_CMP_RISING
                        TMU_EDGE_CMP_FALLING
 *  @param   chnNum2 
 *  @param   tmuCmp2 
 *  @param   tmuEdge2 : TMU_EDGE_CMP_RISING
                        TMU_EDGE_CMP_FALLING
 *  @return  int
 */
int drvTmuStartMeasEdge( int slotNum
                         , unsigned int chnNum1
                         , unsigned int tmuCmp1
                         , unsigned int tmuEdge1
                         , unsigned int chnNum2
                         , unsigned int tmuCmp2
                         , unsigned int tmuEdge2 )
{
    drvWrAteTmuMode( slotNum, chnNum1 );
	drvWrAteTmuMode( slotNum, chnNum2 );
    
    int fpgaNum = chnNumToFpgaNum( chnNum1 );
    
    drvOpen( slotNum, fpgaNum );
    
    int tmuInd = chnNumToTmuInd( chnNum1 );
    
    /*--- Start ---*/
    unsigned int edgeModeAddr = tmuInd==0 ? ADDR_TMU1_EDGE_W_MODE_R_STT : ADDR_TMU2_EDGE_W_MODE_R_STT;
    
    unsigned int edgeModeData = adjTmuEdgeStart( chnNumToTmuChnSel( chnNum1 )
                                               , tmuCmp1
                                               , tmuEdge1
                                               , chnNumToTmuChnSel( chnNum2 )
                                               , tmuCmp2
                                               , tmuEdge2 );
    
    PCI_Wr( edgeModeAddr, edgeModeData );
    
    return 1;
}

int drvTmuStopMeasEdge( int slotNum, int chnNum )
{
    int fpgaNum = chnNumToFpgaNum( chnNum );
    
    drvOpen( slotNum, fpgaNum );
    
    int tmuInd = chnNumToTmuInd( chnNum );
    
    /*--- Start ---*/
    unsigned int edgeModeAddr = tmuInd==0 ? ADDR_TMU1_EDGE_W_MODE_R_STT : ADDR_TMU2_EDGE_W_MODE_R_STT;
    
    unsigned int edgeModeData = adjTmuEdgeStop( );
    
    PCI_Wr( edgeModeAddr, edgeModeData );
    
    return 1;
}


int drvTmuGetEdgeMeasResult( int slotNum, int chnNum, unsigned int * pBusyNotDone, unsigned int * pTriggered, unsigned int * pOverflow, unsigned int * pPeriodInNanoSec )
{
    int fpgaNum = chnNumToFpgaNum( chnNum );
    
    drvOpen( slotNum, fpgaNum );
    
    int tmuInd = chnNumToTmuInd( chnNum );
    
    /*--- Start ---*/
    unsigned int edgeModeAddr = tmuInd==0 ? ADDR_TMU1_EDGE_W_MODE_R_STT : ADDR_TMU2_EDGE_W_MODE_R_STT;
    
    unsigned int edgeStateData = PCI_Rd16( edgeModeAddr );
    
    unsigned int busy = workingStateFromTmuEdgeStt( edgeStateData );
    unsigned int trig = triggerStateFromTmuEdgeStt( edgeStateData );
    
    if ( !busy )
    {
        unsigned int addrMeasH = tmuInd==0 ? ADDR_TMU1_EDGE_MEAS_H : ADDR_TMU2_EDGE_MEAS_H;
        unsigned int addrMeasL = tmuInd==0 ? ADDR_TMU1_EDGE_MEAS_L : ADDR_TMU2_EDGE_MEAS_L;
        
        unsigned int measH = PCI_Rd16( addrMeasH );
        unsigned int measL = PCI_Rd16( addrMeasL );
        
        * pOverflow = overflowFromTmuEdgeMeasH( measH );
        * pPeriodInNanoSec = measurementFromTmuEdgeMeasHL( measH, measL );
    }
    
    *pBusyNotDone = busy;
    *pTriggered = trig;
    
    return 1;
}

int drvWrAteTmuMode( int slotNum, unsigned char chnNum )
{
    if( !drvOpenAte( slotNum, chnNum) )
    {
    #if _DEBUG
        _cprintf( "Line %d file %s: ATE stuck in buzy mode!\n", __LINE__, __FILE__ );
    #endif
        return 0;
    }
    
	/*--- Config ADATE305 Disable PMU ---*/
    unsigned int pmuRegVal = (0<<BIT_PMU_EN)|(1<<BIT_PMU_CLAMP_EN)|(0<<BIT_PE_DISABLE);
    unsigned int ctrlVal = adjAteCtrl( chnNum, ATE_WRITE, ATEADDR_PE_PMU_ENABLE );
    
    if ( !ateWrReg( ctrlVal, pmuRegVal ) )
    {
    #if _DEBUG
        _cprintf("Line %d of file %s: ATE write register failed\n", __LINE__, __FILE__);
    #endif
        return 0;
    }

    /*--- Tell FPGA Stop Driving ---*/
    unsigned int fpgaPmuChnmask;
	fpgaPmuChnmask = PCI_Rd16( ADDR_PMU_HZ_VT_ENABLE );
    PCI_Wr( ADDR_PMU_HZ_VT_ENABLE, fpgaPmuChnmask | chnNumToPmuChnBitMask( chnNum ));
    
    /*--- Tell FPGA Disable PMU Mode ---*/
	/*
    unsigned char fpgaPmuChnmask2 = PCI_Rd16( ADDR_PMU_MODE_ENABLE );
    PCI_Wr( ADDR_PMU_MODE_ENABLE, fpgaPmuChnmask2 & ( 0xFFFF ^ chnNumToPmuChnBitMask( chnNum )));
	*/
    /*--- Tell FPGA Enable PMU Mode ---*/
    unsigned char fpgaPmuChnmask2 = PCI_Rd16( ADDR_PMU_MODE_ENABLE );
    PCI_Wr( ADDR_PMU_MODE_ENABLE, fpgaPmuChnmask2 | chnNumToPmuChnBitMask( chnNum ));

	
}

