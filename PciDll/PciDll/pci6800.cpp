/*================================================================================*
 * File Name:: pci9030.cpp
 * Revision History::
 *    1. New Issue:: Sep/01/2001, Cliff Wang
 *================================================================================*/
#include "stdafx.h"
 //#include <vcl.h>
 //#include <iostream.h>
 #include "pci6800.h"
 //#include "pcidll.h"


 //---------------------------------------------------------------------------------

 // PCI9030_ADDR    sys_addrPCI9030 = 3;
 // PCI9030_HANDLE  sys_hPCI9030 = NULL;

 //---------------------------------------------------------------------------------

 bool PCICheckOK = false;
const int slot[]={0x20,0x10,0x8,0x4,0x2,0x1,0x200000,0x100000,0x80000,0x40000,0x20000,0x10000,0x8000,0x4000,0x2000,0x1000,0x800,0x400,0x200,0x100,0x80,0x40};
 void PCI_Init()
 {
	
     if (PCICheckOK)
        PCIClose();
     if (PCIOpen()){
           PCICheckOK = true;
//           cout << "PCI Check OK\n";
     }
//         sys_DatalogManager->ShowMessage("Open PCI : Valid PCI9030 PCI card found!");
     else {
           PCICheckOK = false;
//           cout << "Without PCI\n";
//         sys_Datalog->ShowMessage("Error opening PCI : Invalid PCI9030 PCI card!");
//         throw TR_Exception(PCI_EXCEPTION, PCI9030_ErrorString);
     }

 }

 //---------------------------------------------------------------------------------

 void PCI_Close()
 {
      if (PCICheckOK)
         PCIClose();
      PCICheckOK = false;
 }

 //---------------------------------------------------------------------------------

 void PCI_Wr(int addr , int data)
 {
#ifdef _TR6850
	 int tmpData=0;
	 if(( data & 0x10000000) && addr==0xe0e)
			;
	 else if( addr==0x0e0e && data>0x20)
	 {
	     //data=0x8000000/data;
		for(int i=6;i<22;i++)
		{
			int tmpVal=data & (1<<i);
			if(tmpVal)
				tmpData=tmpData | (0x8000000/tmpVal);
		}
		data=tmpData;
	  }
#endif
      if (PCICheckOK)
         PCIWriteDword( addr, data);
 }

 //---------------------------------------------------------------------------------

 int PCI_Rd(int addr)
 {
     if (PCICheckOK)
        return PCIReadDword(addr);
     else
        return 0;
 }
 //---------------------------------------------------------------------------------
 void PCI_WrTimeAddr(int addr)
 {
     if (PCICheckOK)
        PCIWriteTimeAddr(addr);
 }

 //---------------------------------------------------------------------------------
 void PCI_WrTimeData(int data)
 {
     if (PCICheckOK)
        PCIWriteTimeData(data);
 }

 //---------------------------------------------------------------------------------
 int  PCI_RdTimeData(void)
 {
     if (PCICheckOK)
       return PCIReadTimeData();
     else
       return 0;
 }

 //---------------------------------------------------------------------------------
 void PCI_IOW(int addr , int data)
 {
    if (PCICheckOK)
        PCIWriteIO( addr, data);
 }

 //---------------------------------------------------------------------------------

 int PCI_IOR(int addr)
 {
     if (PCICheckOK)
        return PCIReadIO(addr);
     else
        return 0;
 }

 //---------------------------------------------------------------------------------

 void ErrorMessage(char* msg)
 {
    MessageBox(NULL,msg,"Error !!",MB_OK);
 }
 //---------------------------------------------------------------------------------

