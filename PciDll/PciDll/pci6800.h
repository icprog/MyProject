/*================================================================================*
 * File Name:: pci6800.h
 * Revision History::
 *    1. New Issue:: Jul/05/2002, Cliff Wang
 *================================================================================*/

 

 //---------------------------------------------------------------------------------

 // extern PCI9030_ADDR sys_addrPCI9030;
 // extern PCI9030_HANDLE sys_hPCI9030;
// 95/07/05 Tom MB
#ifdef BUILDING_PCIDLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
// 95/07/05 Tom ME
extern "C"
{
 DLLEXPORT extern bool PCICheckOK;

 DLLEXPORT void PCI_Wr(int addr,int data);
 DLLEXPORT int PCI_Rd(int addr);

 DLLEXPORT void PCI_IOW(int addr , int data);
 DLLEXPORT int PCI_IOR(int addr);

 DLLEXPORT void PCI_Init();
 DLLEXPORT void PCI_Close();

 DLLEXPORT void PCI_WrTimeAddr(int addr);
 DLLEXPORT void PCI_WrTimeData(int data);
 DLLEXPORT int  PCI_RdTimeData(void);
}
 void ErrorMessage(char* msg);
 //---------------------------------------------------------------------------------



 

#define DLLAPIENTRY __declspec(dllimport)
extern "C" 
{

DLLAPIENTRY BOOL PCIOpen();
DLLAPIENTRY void PCIClose();
DLLAPIENTRY DWORD PCIReadDword( DWORD dwOffset);
DLLAPIENTRY void PCIWriteDword( DWORD dwOffset, DWORD data);

DLLAPIENTRY DWORD PCIReadIO( DWORD dwOffset);
DLLAPIENTRY void PCIWriteIO( DWORD dwOffset, DWORD data);

DLLAPIENTRY void PCIGetBaseAddr( DWORD *pPCIIOBase, DWORD *pMemBase0, DWORD *pMemBase1);
DLLAPIENTRY void PCIUseBaseAddr( int baseIndex);

// 8/12/02 eddy, add for timer
void PCIWriteTimeAddr(DWORD data);
void PCIWriteTimeData(DWORD data);
DWORD PCIReadTimeData(void);
}
