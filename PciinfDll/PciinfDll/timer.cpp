#include "stdafx.h"
#include "timer.h"


//#include <Windows.hpp>
//#define TIMER_FREQ      1.193180        /* timer freq = 1.19 MHz */
#define TIMER_FACTOR    1                 /* timer factor = 1 */

#define TIME_CTRL       0x0b
#define TIME_CNT_0      0x08
#define TIME_CNT_1      0x09
#define TIME_CNT_2      0x0a
#define TIME_FLAG       0x0c

/*#################################################################*/
/*# Function Name : UnitTimePCI                                   #*/
/*# Description   :                                               #*/
/*#                                                               #*/
/*# ID Number :                                                   #*/
/*# Synopsis  : void  UnitTime(unsigned short int TimeCnt)        #*/
/*#                                                               #*/
/*# Process   :                                                   #*/
/*#                                                               #*/
/*# Return    : void                                              #*/
/*#################################################################*/
char LowByte(unsigned  WordData)
{
    return ( (char)WordData );
}  /* End of LowByte() */

char HighByte(unsigned  WordData)
{
    return ( (char)(WordData >> 8) );
}   /* End of HighByte()*/

void UnitTime(unsigned TimeCnt)
{
    unsigned InTime=0;

// 11/05/02 Eddy
    if (TimeCnt==0)
           return;

    PCI_WrTimeAddr(TIME_CTRL);
    PCI_WrTimeData(0x030);                       /* select count 0, mode 0
						     write control word      */
    PCI_WrTimeAddr(TIME_CNT_0);
    PCI_WrTimeData(LowByte(TimeCnt));
    PCI_WrTimeData(HighByte(TimeCnt));

    PCI_WrTimeAddr(TIME_FLAG);

    while(((InTime=PCI_RdTimeData()) & 0x0001) == 0)
	 ;
}

/*#################################################################*/
/*# Function Name : dlyms                                         #*/
/*# Description   : delay ms                                      #*/
/*#                                                               #*/
/*# ID Number :                                                   #*/
/*# Synopsis  : dlyms(float tm)                                   #*/
/*#                                                               #*/
/*# Process   :                                                   #*/
/*#                                                               #*/
/*# Return    : void                                              #*/
/*#################################################################*/
void ATE_PciDelayTimeMS(double tm)
{
	
  unsigned int  i,loop, remainder;
  DWORD count;

  if ( tm<=0.) return;//well6800

//11/05/02 Eddy
//    if (tm==0  || !PCICheckOK) return; /* no delay */
//    count = (unsigned long)(TIMER_FREQ * tm * 1000);
    if (!PCICheckOK)
    {
      Sleep(tm);
      return;
    }
    count = (unsigned long)(TIMER_FACTOR * tm * 1000);

    loop = count/50000;
    remainder = count%50000;

    for ( i = 0; i < loop; i++ )
	UnitTime(50000);

    UnitTime(remainder);
} /* end dlyms() */

// 12/19/03
void dlyms(float tm)
{
  ATE_PciDelayTimeMS(tm);
}
