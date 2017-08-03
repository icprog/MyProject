
#include "stdafx.h"
#include "ifcapi_6850.h"
#include <pdh.h>
#include <string>
using namespace std;

LARGE_INTEGER pdhFreq;

inline double GetPdhTime( LARGE_INTEGER start, LARGE_INTEGER end)
{
  __int64 tdiff = end.QuadPart - start.QuadPart;
  double tm = ((double)tdiff)/pdhFreq.QuadPart;
  return tm;
}

inline void DecodePdh( double inMS, WORD &hour, WORD &min, WORD &sec, int &uSec)
{
  hour = inMS/3600.;
  min = (int)(inMS/60.) - hour*60;
  sec = (int)(inMS) - hour*3600 - min*60;
  uSec = (int)(((inMS - hour*3600 - min*60 - sec)*1000000)/100)*100;
}



int	ifc_adc_data = 0;
bool ifcDlyFlagErr=false;



//----------------------------------------------------------------------------------
int PCIINF_Init(char *filePath,char *msg)
{
	int returnVal=0;
   //if(!ATE_IfcLoadFPGA())
	if(!ATE_IsLoadIfcFPGA())
	{
		returnVal=ATE_IfcLoadFPGA(filePath,msg);
		ATE_ResetIfcBoard();
		ATE_MasterCLK(0,true);
		ATE_MasterCLK(1,true);
	}
	   //return false;
   //QueryPerformanceFrequency(&pdhFreq);
   ifcDlyFlagErr=false;
   ATE_DelayTimeMS(10);
   return returnVal;
}
void IFC_SystemReset(void)
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  PCI_Wr( IFC_CTRL_RGE, IFC_GLOBAL_RESET_WE|IFC_GLOBAL_RESET_ON);
  PCI_Wr( IFC_CTRL_RGE, IFC_GLOBAL_RESET_WE|IFC_GLOBAL_RESET_OFF);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}

int ATE_CheckPowerVal(char *msg,bool bPowerON)
{
////well_debug
//	return 2;
////end    
	
	 int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	 PCI_Wr(IFC_MID_REG,0x0);
  	 int iPowerON;
	 int bAllPowerOn=0;  // 0 is power on, 1 is power error, 2 is power off
	 double powVal;
	 string strChkMsg;
	 char message[512];
     int dlyTime;
	 strChkMsg.insert(strChkMsg.length(),"    standard      actual\n");

	 if(!ATE_IsIfcBoardID())
		bAllPowerOn=2;
	 else
	 {
		 if(!bPowerON)
			 dlyTime=500;//ATE_DelayTimeMS(2000);
		 else
             dlyTime=10;

		 for(int i=0;i<5;i++)
		 {

			iPowerON = ATE_CheckPower3P3V(powVal);
			if(!iPowerON )
			{
			   if(i==4)
			   {
					sprintf(message,"       3.3V           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);     
			   }
					bAllPowerOn |=1;
			   
			}

			iPowerON = ATE_CheckPowerP5VR(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"       5VR           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);        
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerP5V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"       5V           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);         
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerP18V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"       18V           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerN18V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"      -18V         %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}

// 97/09/04 Tom MB
//#ifdef _TR6850
// 99/06/29 arlone MB
#if defined(_TR6850) || defined(_TR6850FT)
		
			iPowerON = ATE_CheckPowerN18VA(powVal);
		
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"      -18VA         %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}
			iPowerON = ATE_CheckPowerP18VA(powVal);

			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"       18VA           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}
// 99/06/29 arlone ME
			iPowerON = ATE_CheckPowerN26V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"      -26V         %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerP26V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"       26V           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerN54V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"      -54V         %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);         
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerP54V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"       54V           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);         
				}
				bAllPowerOn |=1;
			}
#endif
// 10/02/02 Poly 6836S MB
#if defined(_TR6836) || defined(_TR6836S)
// 10/02/02 Poly 6836S ME
			iPowerON = ATE_CheckPowerP12VR(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"      +12VR         %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerN5VR(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"       -5VR           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerN24V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"      -24V         %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}

			iPowerON = ATE_CheckPowerP24V(powVal);
			if(!iPowerON)
			{
				if(i==4)
			    {
					sprintf(message,"       24V           %0.2f\n",powVal);
					strChkMsg.insert(strChkMsg.length(),message);          
				}
				bAllPowerOn |=1;
			}
#endif
// 97/09/04 Tom ME
           if(bAllPowerOn==0)
			   break;
		   else if(i<4)
		      bAllPowerOn=0;   
		   ATE_PciDelayTimeMS(dlyTime);
		}
		if(bAllPowerOn)
		{
			dlyTime=250;
			for(int i=0;i<5;i++)
			{
			   if(!ATE_IsIfcBoardID())
			   {
			    	bAllPowerOn=2;
					break;
			   }
			   ATE_PciDelayTimeMS(dlyTime);
			}
		}

	}
		//msg=strChkMsg.c_str();
	sprintf(msg,"%s",strChkMsg.c_str());
	PCI_Wr(IFC_MID_REG,nEnSlot);
    return bAllPowerOn;


}
//----------------------------------------------------------------------------------
// slot 1~6 digital boards
// slot 7~22 analog boards
//
int  ATE_RdAc64ModuleID(int slot)
{
	 if( (slot<1) || (slot>22))
        return 0;
	 PCI_Wr( IFC_MID_REG, 1<<(slot-1));
     return PCI_Rd(0x640);
}

// 2014/09/24 Tom MB : new PPMU chip
int  ATE_RdAc64ModulePPMUID(int slot)
{
	 if( (slot<1) || (slot>22))
        return 0;

	 int nValue = 0;
	 PCI_Wr( IFC_MID_REG, 1<<(slot-1));
#if defined(_TR6850) || defined(_TR6850FT)
	#define REGISTER_PPMU_BOARD_ID   0x63F

    nValue = PCI_Rd(REGISTER_PPMU_BOARD_ID);
#else
	#define REGISTER_PPMU_BOARD_ID   0x70A
	#define REGISTER_PPMU2_BOARD_ID  0x70B

	 nValue = ((PCI_Rd(REGISTER_PPMU_BOARD_ID) & 0xffff) << 16) | (PCI_Rd(REGISTER_PPMU2_BOARD_ID) & 0xffff);
#endif

	 return nValue;
}
// 2014/09/24 Tom ME

int ATE_RdModuleID(int slot)
{
	
    if( (slot<1) || (slot>22))
        return 0;
	

    PCI_Wr( IFC_MID_REG, 1<<(slot-1));
    return PCI_Rd(0x0);
}
//----------------------------------------------------------------------------------
int ATE_IfcLoadFPGA(char *filePath,char *msg)
{
   //Download FPGA
  //Set prog_b=0
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
	
  for(int ii=0;ii<4;ii++)
    PCI_Wr(0xe30,0x8);

  //Set prog_b=1
  PCI_Wr(0xe30,0xc);

  //check if DONE PIN is Low
  if( (PCI_Rd(0xe30) & 0x8)!=0x0 ){
  
    
    //MessageBox(NULL, "pciinf FPGA DOWN signal error. DOWN is not low!!\n System will exit", "PCIINF ERROR", MB_OK | MB_TOPMOST);
	sprintf(msg,"pciinf FPGA DOWN signal error. DOWN is not low!!\n Retry or Cancel to exit system");
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
    return 1;
  }

  //check if /INIT PIN is High
 /* for(int ii=0;ii<1000000;ii++){}
  for(int ii=0;ii<1000000;ii++){}
  for(int ii=0;ii<1000000;ii++){}
  for(int ii=0;ii<1000000;ii++){}*/
   ATE_PciDelayTimeMS(1000);
  if( (PCI_Rd(0xe30) & 0x10)!=0x10 ){
    
    //MessageBox(NULL,"pciinf FPGA /INIT fail. /INIT not goes high!!\n System will exit", "PCIINF ERROR", MB_OK | MB_TOPMOST);
	sprintf(msg,"pciinf FPGA /INIT fail. /INIT not goes high!!\n Retry or Cancel to exit system");
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
    return 1;
  }

  //Check Configuration DATA
  int brdId=ATE_RdIfcBoardID();
  FILE *fpt;
  char fileName[512];
  
#ifdef _TR6850
  if( (brdId & 0x000f)<=0x3 )
          sprintf(fileName,"%s\\pci_bin_03.rbt",filePath);
  else if(  (brdId & 0x000f)<=0x5 )
          sprintf(fileName,"%s\\pci_bin_04.rbt",filePath);
 else
#endif
          sprintf(fileName,"%s\\pci_bin_04.rbt",filePath);

  if( (fpt=fopen(fileName,"r"))==NULL){
	 //MessageBox(NULL,"Can't open pciinf configuration data file!!\n System will exit", "PCIINF ERROR", MB_OK | MB_TOPMOST);
     sprintf(msg,"Can't open pciinf configuration data file!!\n System will exit");
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
     return 2;
  }
  

  //Loading device configuration data---
  char A[60];
  while( fgets(A,160,fpt) ){
    for(int j=0;j<32;j++ ){
      int Dt=(int)(A[j])-(int)('0');
      PCI_Wr(0xe30, Dt|0xc);        // protect=1, prog_b=1, CCLK=0, DIN=Dt
      PCI_Wr(0xe30, Dt|0xc);        // protect=1, prog_b=1, CCLK=0, DIN=Dt
      PCI_Wr(0xe30, Dt|0xe);        // protect=1, prog_b=1, CCLK=1, DIN=Dt
    }
  }
  fclose( fpt );

  if( (PCI_Rd(0xe30) & 0x8)!=0x8){     //Check if DONE!=1
    //MessageBox(NULL,"pciinf FPGA Download fail!!\n System will exit", "PCIINF ERROR", MB_OK | MB_TOPMOST);
	sprintf(msg,"pciinf FPGA Download fail!!\n Retry or Cancel to exit system");
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
    return 1;
  }

  //RESET FPGA
  // SET RESET_P_ to High
  PCI_Wr (0xe30, 0x30);

  // DELAY 1ms
  ATE_PciDelayTimeMS(1);

  // SET RESET_P_ to Low
  PCI_Wr (0xe30, 0x10);

  // DELAY 1ms
  ATE_PciDelayTimeMS(1);

  // SET RESET_P_ to High
  PCI_Wr (0xe30, 0x30);

// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
  return 0;
}
bool ATE_IsLoadIfcFPGA()
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
	bool bResult = false;
// 2012/06/11 Tom ME

   if( (PCI_Rd(0xe30) & 0x8)==0x8)
	   bResult = true;
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
	return bResult;
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
void ATE_ResetIfcBoard()
{
// 97/08/01 eric MB
  //IFC_SystemReset();
// 97/08/01 eric ME
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME

  // Set H/W bin to 0, set bin period to 1mS
  int data = IFC_BIN_DATA_WE|IFC_BIN_PERIOD_WE|(1<<IFC_BIN_PERIOD_SHIFT);
  for(int site=0; site<IFC_TTL_SITE_CNT; site++)
    PCI_Wr( IFC_SITE1_BIN_REG+site, data);

  // Set EOT data to 0, set EOT period to 1mS
  data = IFC_EOT_DATA_WE | IFC_EOT_PERIOD_WE | (1<<IFC_EOT_PERIOD_SHIFT);
  for(int site=0; site<IFC_TTL_SITE_CNT; site++)
    PCI_Wr( IFC_SITE1_EOT_REG+site, data);

  // Set REJ data to 0, set REJ period to 1mS
  // 2011/04/06 chiu MB, add new Rej_Mode "Fixed Mode" for 8 site TTL
  //data = IFC_REJ_DATA_WE | IFC_REJ_PERIOD_WE | (1<<IFC_REJ_PERIOD_SHIFT);
  data = IFC_REJ_DATA_WE | IFC_REJ_PERIOD_WE | (1<<IFC_REJ_PERIOD_SHIFT) | IFC_REJ_FORCE_WE;
  // 2011/04/06 chiu ME, add new Rej_Mode "Fixed Mode" for 8 site TTL
  for(int site=0; site<IFC_TTL_SITE_CNT; site++)
    PCI_Wr( IFC_SITE1_REJ_REG+site, data);

// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
  // initial CLK_CTRL, RESET
// 97/08/01 eric MB
  //data = IFC_CTRL_CLK_WE | IFC_GLOBAL_RESET_WE | IFC_CTRL_PWR_DOWN_WE;
  //PCI_Wr( IFC_CTRL_RGE, data);
// 97/08/01 eric ME
  IFC_SystemReset();

  // 7/29/03 Eddy, for Greatex case. continue power on, off, on, abnormal status
  ATE_ClearLatchStart();
  ATE_ClearLatchEow();
}
//----------------------------------------------------------------------------------
// return a boolean to indicate match the default ID or not
bool ATE_IsIfcBoardID()
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int brdid = PCI_Rd(IFC_BRDID_REG) & IFC_BRDID_MASK;
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
  return (IFC_BRDID == brdid);
}
//----------------------------------------------------------------------------------
int ATE_RdIfcBoardID()
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME

    int nValue = PCI_Rd(IFC_BRDID_REG);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME

	return nValue;
}

//----------------------------------------------------------------------------------
// return a boolean to indicate match the default ID or not
// use the reserve port address to test the R/W function, to make sure the R/W
// is as stable as the IFC ID code
bool ATE_IFC_RWOK()
{
  int wrData[] = { 0x55555555, 0xAAAAAAAA};

// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
	bool bResult = true;
// 2012/06/11 Tom ME

  for (int ii=0; ii<2; ii++){
    PCI_Wr(IFC_RESERVED2_RGE, wrData[ii]);
    ATE_DelayTimeMS(1);
    if ( wrData[ii]!=PCI_Rd(IFC_RESERVED2_RGE) )
	{
      bResult= false;
	  break;
	}
  }
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
    return bResult;
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// trigEdge=0-Normal(High),1-Reverse(Low),2-Rising edge,3-Falling edge
void ATE_WrStartTriggerEdge(int trigEdge)
{
  int buf = IFC_START_MODE_WE | trigEdge&IFC_START_MODE_MASK;
  int data=0;
  int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
  PCI_Wr(IFC_MID_REG,0x0);

  for (int site=0; site<IFC_TTL_SITE_CNT; site++)
    data |= (buf<<IFC_START_MODE_SHIFT*site);

  PCI_Wr( IFC_START_REG, data);
  ATE_DelayTimeMS(0.05);
  PCI_Wr(IFC_MID_REG,nEnSlot);	
}
//----------------------------------------------------------------------------------
// trigEdge=0-Normal,1-Reverse,2-Rising edge,3-Falling edge
void ATE_WrEowTriggerEdge(int trigEdge)
{
  int buf = IFC_EOW_MODE_WE | trigEdge&IFC_EOW_MODE_MASK;
  int data=0;
  int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
  PCI_Wr(IFC_MID_REG,0x0);
  for (int site=0; site<IFC_TTL_SITE_CNT; site++)
    data |= (buf<<IFC_EOW_MODE_SHIFT*site);

  PCI_Wr( IFC_EOW_REG, data);
  ATE_DelayTimeMS(0.05);
  PCI_Wr(IFC_MID_REG,nEnSlot);	
}
//----------------------------------------------------------------------------------
// eotInverse : 0 or 1
void ATE_WrEotActiveState(int eotInverse)
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int data = IFC_EOT_INVERSE_WE | eotInverse << IFC_EOT_INVERSE_SHIFT;
  for(int site=0; site<IFC_TTL_SITE_CNT; site++)
    PCI_Wr( IFC_SITE1_EOT_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
// rejInverse : 0 or 1
void ATE_WrRejActiveState(int rejInverse)
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int data = IFC_REJ_INVERSE_WE | rejInverse << IFC_REJ_INVERSE_SHIFT;
  for(int site=0; site<IFC_TTL_SITE_CNT; site++)
    PCI_Wr( IFC_SITE1_REJ_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
// binInverse : 0 or 1
void ATE_WrBinActiveState(int binInverse)
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int data = IFC_BIN_INVERSE_WE | binInverse << IFC_BIN_INVERSE_SHIFT;
  for(int site=0; site<IFC_TTL_SITE_CNT; site++)
    PCI_Wr( IFC_SITE1_BIN_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
// mode: 0:line mode, 1:binary mode
void ATE_WrParallelBinOut(int bin[], bool mode)
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  for(int site=1; site<=IFC_TTL_SITE_CNT; site++)
// 2012/03/09 eric MB
//    ATE_WrSerialBinOut( site, *bin++, mode);
    ATE_WrSerialBinOut( site, bin[site-1], mode);
// 2012/03/09 eric ME
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
// site: 1~4
// mode: 0:line mode, 1:binary mode
/*
void ATE_WrSerialBinOut(int site, int bin, bool mode)
{
  if( bin <= 0 )
    return;

  if( (site<1)||(site>IFC_TTL_SITE_CNT)){
    Application->MessageBox("Invalid site number!!!", "ATE_WrSerialBinOut()", MB_OK);
    return;
  }

  // Reset bin data
  int data = IFC_BIN_DATA_WE;
  PCI_Wr( IFC_SITE1_BIN_REG+site-1, data);

  if (mode==0){// line mode
    if (bin >=1 && bin <=8)
       data |= 1 << (bin-1);
  }
  else
     data |= bin;

  // Set bin data
  PCI_Wr( IFC_SITE1_BIN_REG+site-1, data);
}
*/
//----------------------------------------------------------------------------------
// site: 1~4
// mode: 0:line mode, 1:binary mode
void ATE_WrSerialBinOut(int site, int bin, bool mode)
{
  if( bin <= 0 )
    return;

  if( (site<1)||(site>IFC_TTL_SITE_CNT)){
   MessageBox(NULL,"Invalid site number!!!", "ATE_WrSerialBinOut()", MB_OK);
    return;
  }

  // Reset bin data
  int data = IFC_BIN_DATA_WE;
  if (mode==0){// line mode
    if (bin >=1 && bin <=8)
       data |= 1 << (bin-1);
  }
  else
     data |= bin;

// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  site--;
  PCI_Wr( IFC_SITE1_BIN_REG+site, data);

  PCI_Wr( IFC_SITE1_BIN_REG+site, IFC_BIN_DATA_WE);

  // Set bin data
  PCI_Wr( IFC_SITE1_BIN_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
int ATE_RdStartLatchSignal(int start[], int eow[], int onWf[])
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int buf;
  do{
    buf = PCI_Rd(IFC_BIN_CON_REG);
  }while( buf!=PCI_Rd(IFC_BIN_CON_REG) );
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME

  for(int site=0; site<IFC_TTL_SITE_CNT; site++){
    start[site] = (buf & (0x1<<site))? 1:0;
    eow[site] = (buf & (0x10<<site))? 1:0;
    onWf[site] = 0;
  }

  // 2012/06/08 chiu MB, TTL receive start trigger when hardware power off.
  if (ATE_IsIfcBoardID() == false)
  {
	  memset(start, 0, IFC_TTL_SITE_CNT * sizeof(int));
	  memset(eow, 0, IFC_TTL_SITE_CNT * sizeof(int));
	  memset(onWf, 0, IFC_TTL_SITE_CNT * sizeof(int));
	  return 0;
  }
  // 2012/06/08 chiu ME

  return buf&0xff;
}
//----------------------------------------------------------------------------------
// Reset Start Signal
void ATE_ClearLatchStart()
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int buf = IFC_START_RESET_WE | IFC_START_RESET_RESET;
  int data=0;
  for (int site=0; site<IFC_TTL_SITE_CNT; site++)
    data |= (buf<<IFC_START_MODE_SHIFT*site);

  PCI_Wr( IFC_START_REG, data);//Reset

  buf = IFC_START_RESET_WE;
  data=0;
  for (int site=0; site<IFC_TTL_SITE_CNT; site++)
    data |= (buf<<IFC_START_MODE_SHIFT*site);

  PCI_Wr( IFC_START_REG, data);//Normal
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}

//----------------------------------------------------------------------------------
// Reset EOW Signal
void ATE_ClearLatchEow()
{
  int buf = IFC_EOW_RESET_WE | IFC_EOW_RESET_RESET;
  int data=0;
  int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
  PCI_Wr(IFC_MID_REG,0x0);
  for (int site=0; site<IFC_TTL_SITE_CNT; site++)
    data |= (buf<<IFC_EOW_MODE_SHIFT*site);

  PCI_Wr( IFC_EOW_REG, data);//Reset

  buf = IFC_EOW_RESET_WE;
  data=0;
  for (int site=0; site<IFC_TTL_SITE_CNT; site++)
    data |= (buf<<IFC_EOW_MODE_SHIFT*site);

  PCI_Wr( IFC_EOW_REG, data);//Normal

  PCI_Wr(IFC_MID_REG,nEnSlot);
}

//----------------------------------------------------------------------------------
//---- Eot are sendout site by site. -------------------------------
void ATE_WrSerialEotxM(int site)
{
  if( (site<1)||(site>IFC_TTL_SITE_CNT)){
   MessageBox(NULL,"Invalid site number!!!", "ATE_WrSerialEot()", MB_OK);
    return;
  }

// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int data = IFC_EOT_DATA_WE | 1;
  site--;
  PCI_Wr(IFC_SITE1_EOT_REG+site , data);
  PCI_Wr(IFC_SITE1_EOT_REG+site, IFC_EOT_DATA_WE);
  PCI_Wr(IFC_SITE1_EOT_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
//---- All Eot1/Eot2/Eot3/Eot4 are sendout simultaneously. -------------------------
void ATE_WrParallelEot(int onSite[])
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  for(int site=0; site<IFC_TTL_SITE_CNT; site++){
    if (onSite[site]) /* if this site is active  */
      ATE_WrSerialEotxM(site+1);
  }
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//---- Use only Eot1, Eot1 is sendout site by site. --------------------------------
void ATE_WrSerialNullEot()
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  for(int site=0; site<IFC_TTL_SITE_CNT; site++){
    int data = IFC_EOT_DATA_WE;
    PCI_Wr(IFC_SITE1_EOT_REG+site, data);

    ATE_WrSerialEotxM(site+1);
  }
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
//---- Eot1/Eot2 are sendout site by site. -------------------------------
void ATE_WrSerialRejxM(int site, int fail)
{
  if (fail==0)
    return;

  if( (site<1)||(site>IFC_TTL_SITE_CNT)){
    MessageBox(NULL,"Invalid site number!!!", "ATE_WrSerialRej()", MB_OK);
    return;
  }

// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  // 2011/04/06 chiu MB, add new Rej_Mode "Fixed Mode" for 8 site TTL
  //int data = IFC_REJ_DATA_WE|1;
  int data = IFC_REJ_DATA_WE|1 | IFC_REJ_FORCE_WE;
  // 2011/04/06 chiu ME, add new Rej_Mode "Fixed Mode" for 8 site TTL

  site--;
  PCI_Wr(IFC_SITE1_REJ_REG+site, data);
  PCI_Wr(IFC_SITE1_REJ_REG+site, IFC_REJ_DATA_WE);
  PCI_Wr(IFC_SITE1_REJ_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//---- All Rej1/Rej2/Rej3/Rej4 are sendout simultaneously. -------------------------
// fail[] = { site1Fail, site2Fail, site3Fail, site4Fail }
void ATE_WrParallelRej(int onSite[], int fail[])
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  for(int site=0; site<IFC_TTL_SITE_CNT; site++){
    if (onSite[site]) /* if this site is active  */
      ATE_WrSerialRejxM(site+1, fail[site]);
  }
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
// site: 1~4
//---- Resolution = 100us, EOT: 0 ~ 9:  1000 mS Max. ----------------
void ATE_WrEotWidth(int site, double width)
{
  if( (site<1)||(site>IFC_TTL_SITE_CNT)){
    MessageBox(NULL,"Invalid site number!!!", "ATE_WrEotWidth()", MB_OK);
    return;
  }

// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int cnt = width*10;
  int data = IFC_EOT_PERIOD_WE | (cnt<<IFC_EOT_PERIOD_SHIFT);
  site--;
  PCI_Wr(IFC_SITE1_EOT_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}

void ATE_WrEotWidth(double width)  // unit is "mS". resolution is 100uS
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  for(int site=1; site<=IFC_TTL_SITE_CNT; site++)
    ATE_WrEotWidth(site, width);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
// site: 1~4
//---- Resolution = 100us, EOT: 0 ~ 9:  1000 mS Max. ----------------
void ATE_WrRejWidth(int site, double width)
{
  if( (site<1)||(site>IFC_TTL_SITE_CNT)){
    MessageBox(NULL,"Invalid site number!!!", "ATE_WrRejWidth()", MB_OK);
    return;
  }

// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int cnt = width*10;
  int data = IFC_REJ_PERIOD_WE | (cnt<<IFC_REJ_PERIOD_SHIFT);
  site--;
  PCI_Wr(IFC_SITE1_REJ_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}

void ATE_WrRejWidth(double width)  // unit is "mS". resolution is 100uS
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  for(int site=1; site<=IFC_TTL_SITE_CNT; site++)
    ATE_WrRejWidth(site, width);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
// 2011/04/06 chiu MB, add new Rej_Mode "Fixed Mode" for 8 site TTL
void ATE_WrSerialRejF(int site, bool high)
{
	if( (site<1)||(site>IFC_TTL_SITE_CNT)){
		//MessageBox(NULL,"Invalid site number!!!", "ATE_WrSerialRej()", MB_OK|MB_TOPMOST);
		return;
	}

// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
	//int data = IFC_REJ_FORCE_WE|(1<<IFC_REJ_FORCE_SHIFT)|IFC_REJ_DATA_WE|(high?0:1);
	int forceMode = IFC_REJ_FORCE_WE|(1<<IFC_REJ_FORCE_SHIFT);
	int data = IFC_REJ_DATA_WE|(high?0:1);
	site--;
	//PCI_Wr(IFC_SITE1_REJ_REG+site, data);
	//PCI_Wr(IFC_SITE1_REJ_REG+site, IFC_REJ_DATA_WE);
	//PCI_Wr(IFC_SITE1_REJ_REG+site, data);
	PCI_Wr(IFC_SITE1_REJ_REG+site, forceMode);
	PCI_Wr(IFC_SITE1_REJ_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME

}

void ATE_WrParallelRejF(int onSite[], bool high[])
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
	for(int site=0; site<IFC_TTL_SITE_CNT; site++){
		if (onSite[site]) /* if this site is active  */
			 ATE_WrSerialRejF(site+1, high[site]);
	}
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
// 2011/04/06 chiu ME, add new Rej_Mode "Fixed Mode" for 8 site TTL

//----------------------------------------------------------------------------------
// site: 1~4
void ATE_WrBinWidth(int site, double width)  // unit is "mS". resolution is 100uS
{
  if( (site<1)||(site>IFC_TTL_SITE_CNT)){
    MessageBox(NULL,"Invalid site number!!!", "ATE_WrBinWidth()", MB_OK);
    return;
  }

// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  int cnt = width*10;
  int data = IFC_BIN_PERIOD_WE | (cnt<<IFC_BIN_PERIOD_SHIFT);
  site--;
  PCI_Wr(IFC_SITE1_BIN_REG+site, data);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}

void ATE_WrBinWidth(double width)  // unit is "mS". resolution is 100uS
{
// 2012/06/11 Tom MB
	int nEnSlot=(~(PCI_Rd(IFC_MID_REG) & 0x0fffffff)) | 0x10000000;
	PCI_Wr(IFC_MID_REG,0x0);
// 2012/06/11 Tom ME
  for(int site=1; site<=IFC_TTL_SITE_CNT; site++)
    ATE_WrBinWidth(site, width);
// 2012/06/11 Tom MB
	PCI_Wr(IFC_MID_REG,nEnSlot);
// 2012/06/11 Tom ME
}
//----------------------------------------------------------------------------------
#ifdef _TR6850
const double IFC_PW_MAX[] = { 10.0, 30.0, 10.0, 60.0, 60.0, 10.0, 10.0,
                              57.0, 57.0, 85.0, 85.0, 150.0, 150.0, 85.0, 10.0};
#endif
// 10/02/02 Poly 6836S MB
#if defined(_TR6836) || defined(_TR6836S)
// 10/02/02 Poly 6836S ME
const double IFC_PW_MAX[] = { 10.0, 30.0, 10.0, 72.0, 72.0, 10.0, 10.0,
                              57.0, 57.0, 85.0, 85.0, 150.0, 150.0, 40.0, 10.0};
#endif
#ifdef _TR6850FT
const double IFC_PW_MAX[] = { 10.0, 30.0, 10.0, 60.0, 60.0, 10.0, 10.0,
                              57.0, 57.0, 85.0, 85.0, 150.0, 150.0, 85.0, 10.0};
#endif
double IFC_adc2v(int data, int idx)
{
  double tmp = data;
  return (tmp - 32768)/32768*IFC_PW_MAX[idx];
}
const int IFC_PW[] = { IFC_ADC_SOURCE_3P3V, IFC_ADC_SOURCE_P5VR, IFC_ADC_SOURCE_P5V,
                       IFC_ADC_SOURCE_P15V, IFC_ADC_SOURCE_N15V, IFC_ADC_SOURCE_AGND,
                       IFC_ADC_SOURCE_REF10V, IFC_ADC_SOURCE_N15VA, IFC_ADC_SOURCE_P15VA,
                       IFC_ADC_SOURCE_N24V, IFC_ADC_SOURCE_P24V, IFC_ADC_SOURCE_N60V,
                       IFC_ADC_SOURCE_P60V, IFC_ADC_SOURCE_P12VR, IFC_ADC_SOURCE_N5VR};
// pwrIdx 0~12
bool ATE_CheckPower(int pwrIdx, double &value)
{
  if( (pwrIdx<0) || (pwrIdx>=IFC_ADC_SOURCE_CNT) )
    return false;

  //1.select adc source
  PCI_Wr(IFC_ADC_RGE, IFC_ADC_SOURCE_WE|IFC_ADC_DATA_RD|IFC_PW[pwrIdx]);
  ATE_PciDelayTimeMS(1);

  //2. converta
   PCI_Wr(IFC_ADC_RGE, IFC_ADC_SOURCE_WE|IFC_PW[pwrIdx]);
  PCI_Wr(IFC_ADC_CONVERT_RGE, 0);
  ATE_PciDelayTimeMS(1);


  //3. read back data	
  PCI_Wr(IFC_ADC_RGE, IFC_ADC_SOURCE_WE|IFC_ADC_DATA_RD|IFC_PW[pwrIdx]);
  PCI_Wr(IFC_ADC_CONVERT_RGE, 0); 
  int data = PCI_Rd(IFC_ADC_CONVERT_RGE) & 0xffff;
  ifc_adc_data = data;
  data^= 0x8000;
  value = IFC_adc2v(data, pwrIdx);
    ATE_PciDelayTimeMS(1);
  return true;
}

bool ATE_CheckPower3P3V(double &powVal)
{
	double value;
	ATE_CheckPower(0,value);
	powVal=value;
	value=(value-3.3)/3.3;
	if( value<=0.05 && value>=-0.05)
		return true;
	else 
		return false;
}
bool ATE_CheckPowerP5VR(double &powVal)
{
   double value;
  ATE_CheckPower(1,value);
  powVal=value;
  value=(value-5)/5;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerP5V(double &powVal)
{
  double value;
  ATE_CheckPower(2,value);
  powVal=value;
  value=(value-5)/5;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerP18V(double &powVal)
{
  double value;
  ATE_CheckPower(3,value);
  powVal=value;
  value=(value-18)/18;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerN18V(double &powVal)
{
  double value;
  ATE_CheckPower(4,value);
  powVal=value;
  value=(value-(-18))/18;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;

}
bool ATE_CheckPowerN18VA(double &powVal)
{
  double value;
  ATE_CheckPower(7,value);
  powVal=value;
  value=(value-(-18))/18;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerP18VA(double &powVal)
{
   double value;
  ATE_CheckPower(8,value);
  powVal=value;
  value=(value-18)/18;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;

}
bool ATE_CheckPowerN26V(double &powVal)
{
   double value;
  ATE_CheckPower(9,value);
  powVal=value;
  value=(value-(-26))/26;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerP26V(double &powVal)
{
    double value;
  ATE_CheckPower(10,value);
  powVal=value;
  value=(value-26)/26;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerN54V(double &powVal)
{
   double value;
  ATE_CheckPower(11,value);
  powVal=value;
  value=(value-(-54))/54;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerP54V(double &powVal)
{
   double value;
  ATE_CheckPower(12,value);
  powVal=value;
  value=(value-54)/54;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
// 97/09/04 Tom MB
bool ATE_CheckPowerP12VR(double &powVal)
{
  double value;
  ATE_CheckPower(13,value);
  powVal=value;
  value=(value-12)/12;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerN5VR(double &powVal)
{
   double value;
  ATE_CheckPower(14,value);
  powVal=value;
  value=(value-(-5))/5;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;

}
bool ATE_CheckPowerN24V(double &powVal)
{
   double value;
  ATE_CheckPower(9,value);
  powVal=value;
  value=(value-(-24))/24;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
bool ATE_CheckPowerP24V(double &powVal)
{
    double value;
  ATE_CheckPower(10,value);
  powVal=value;
  value=(value-24)/24;
  if( value<=0.05 && value>=-0.05)
	  return true;
  else 
	  return false;
}
// 97/09/04 Tom ME
//----------------------------------------------------------------------------------
const int MCLK[] = { IFC_MCLK_ANA1_SHIFT, IFC_MCLK_ANA2_SHIFT, IFC_MCLK_DIG1_SHIFT,
                     IFC_MCLK_DIG2_SHIFT, IFC_MCLK_DIG3_SHIFT, IFC_MCLK_DIG4_SHIFT};

void ATE_MasterCLK(int clkIdx, bool onOff)
{
  PCI_Wr(IFC_CTRL_RGE, (onOff?0x3:0x2)<<MCLK[clkIdx]);
}
//----------------------------------------------------------------------------------

LARGE_INTEGER start,end;
double m_time;   
void ATE_DelayTimeMS(double tm)
{
	
	 int dlyTime=tm*1000;
	 int ii=0;
	 
   if(ATE_IsLoadIfcFPGA() && !ifcDlyFlagErr)
   {
	    int   hd_cnt_timeout;
        int   hd_cnt_value;
      //  int   hd_cnt_rst;
        int   hd_cnt_out;
        int   hd_cnt_en;

	 if(dlyTime<=3)
		 dlyTime=5;
	 //load counter timeout value
	hd_cnt_timeout = dlyTime-3;
	//QueryPerformanceCounter(&start);
	// QueryPerformanceCounter(&start);
	PCI_Wr((IFC_CNT_HIGH_ADDR | IFC_CNT_LOAD_ADDR),(IFC_CNT_TIMEOUT_EN | (hd_cnt_timeout << IFC_CNT_TIMEOUT_OFFSET)));

	//reset counter
	//hd_cnt_rst = 1;
	PCI_Wr(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR, IFC_CNT_RST_EN | ( 1 << IFC_CNT_RST_OFFSET));
	//hd_cnt_rst = 0;
	PCI_Wr(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR, IFC_CNT_RST_EN | ( 0 << IFC_CNT_RST_OFFSET));
	//hd_cnt_rst = 1;
	PCI_Wr(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR, IFC_CNT_RST_EN | ( 1 << IFC_CNT_RST_OFFSET));

	//start counter enable
	//hd_cnt_en = 1;
	PCI_Wr(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR, IFC_CNT_EN_EN | (1 << IFC_CNT_EN_OFFSET));
   // PCI_Wr(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR, IFC_CNT_EN_EN | (1 << IFC_CNT_EN_OFFSET));

     /*   QueryPerformanceCounter(&end);
		  m_time=GetPdhTime(start,end);*/
    /*   int z=PCI_Rd(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR);
	       z=PCI_Rd(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR);
		   z=PCI_Rd(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR);
		   z=PCI_Rd(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR);
		   z=PCI_Rd(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR);
		   z=PCI_Rd(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR);*/
	//QueryPerformanceCounter(&end);
	///	 m_time=GetPdhTime(start,end);
	
	
		int cnt=dlyTime<10?dlyTime+10:dlyTime*2;
	   while( (PCI_Rd(IFC_CNT_HIGH_ADDR | IFC_CNT_CTRL_ADDR)& 0x04)!=0x4)
	   {
		  ii++;
	      if(ii>cnt)
		  {
		     
		      ifcDlyFlagErr=true;
			     break;
			 
		  }
	     /*if(m_time>dlyTime+10)
	     {
		    ifcDlyFlagErr=true;
	         break;
	     }*/
	
	   }
	    
   }
   else
		ATE_PciDelayTimeMS(tm);
  
}
