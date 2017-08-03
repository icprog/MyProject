//#include "pciimport.h"
#include "timer.h"
#include "pciimport.h"




#ifndef IFCAPI_H
#define IFCAPI_H

#define IFC_TTL_SITE_CNT        4

// 97/12/05 chiu MB
#define IFC_BRDID_REG           0x0e00
//#define IFC_BRDID_MASK          0xffff0000
//#ifdef _TR6850
//#define IFC_BRDID               0x68500000
//#else
#define IFC_BRDID_MASK          0xffffff00


#ifdef _TR6850
#define IFC_BRDID               0x68500300
#endif

#ifdef _TR6850FT
#define IFC_BRDID               0x68508300
#endif

// 10/02/02 Poly 6836S MB
#ifdef _TR6836
#define IFC_BRDID               0x68360200
#endif

#ifdef _TR6836S
#define IFC_BRDID               0x68368200
#endif
// 10/02/02 Poly 6836S ME
// 97/12/05 chiu ME
#define IFC_MID_REG             0x0e0e

#define IFC_SITE1_BIN_REG       0x0e10
#define IFC_SITE2_BIN_REG       0x0e11
#define IFC_SITE3_BIN_REG       0x0e12
#define IFC_SITE4_BIN_REG       0x0e13
#define IFC_BIN_DATA_WE         0x0100
#define IFC_BIN_DATA_MASK       0x00ff
#define IFC_BIN_PERIOD_WE       0x08000000
#define IFC_BIN_PERIOD_MASK     0x007ffe00
#define IFC_BIN_PERIOD_SHIFT    9
#define IFC_BIN_INVERSE_WE      0x20000000
#define IFC_BIN_INVERSE_SHIFT   28
#define IFC_BIN_FORCE_WE        0x80000000
#define IFC_BIN_FORCE_SHIFT     30


#define IFC_SITE1_EOT_REG       0x0e14
#define IFC_SITE2_EOT_REG       0x0e15
#define IFC_SITE3_EOT_REG       0x0e16
#define IFC_SITE4_EOT_REG       0x0e17
#define IFC_EOT_DATA_WE         0x0002
#define IFC_EOT_PERIOD_WE       0x08000000
#define IFC_EOT_PERIOD_SHIFT    2
#define IFC_EOT_INVERSE_WE      0x20000000
#define IFC_EOT_INVERSE_SHIFT   28
#define IFC_EOT_FORCE_WE        0x80000000
#define IFC_EOT_FORCE_SHIFT     30

#define IFC_SITE1_REJ_REG       0x0e18
#define IFC_SITE2_REJ_REG       0x0e19
#define IFC_SITE3_REJ_REG       0x0e1a
#define IFC_SITE4_REJ_REG       0x0e1b
#define IFC_REJ_DATA_WE         0x0002
#define IFC_REJ_PERIOD_WE       0x08000000
#define IFC_REJ_PERIOD_SHIFT    2
#define IFC_REJ_INVERSE_WE      0x20000000
#define IFC_REJ_INVERSE_SHIFT   28
#define IFC_REJ_FORCE_WE        0x80000000
#define IFC_REJ_FORCE_SHIFT     30

#define IFC_START_REG           0x0e1c
#define IFC_START_MODE_WE       0x0004
#define IFC_START_MODE_MASK     0x0003
#define IFC_START_MODE_SHIFT    5
#define IFC_START_RESET_WE      0x0010
#define IFC_START_RESET_NORMAL  0x0000
#define IFC_START_RESET_RESET   0x0008


#define IFC_EOW_REG             0x0e1d
#define IFC_EOW_MODE_WE         0x0004
#define IFC_EOW_MODE_MASK       0x0003
#define IFC_EOW_MODE_SHIFT      5
#define IFC_EOW_RESET_WE        0x0010
#define IFC_EOW_RESET_NORMAL    0x0000
#define IFC_EOW_RESET_RESET     0x0008

#define IFC_BIN_CON_REG         0x0e1e

#define IFC_RESERVED1_RGE       0x0e20
#define IFC_RESERVED2_RGE       0x0e21
#define IFC_RESERVED3_RGE       0x0e22

#define IFC_FPGA_RGE            0x0e30

#define IFC_CTRL_RGE            0x0e33
#define IFC_CTRL_CLK_WE         0x0002
#define IFC_CTRL_PWR_DOWN_WE    0x0020
#define IFC_MCLK_ANA1_SHIFT     10
#define IFC_MCLK_ANA2_SHIFT     12
#define IFC_MCLK_DIG1_SHIFT     14
#define IFC_MCLK_DIG2_SHIFT     16
#define IFC_MCLK_DIG3_SHIFT     18
#define IFC_MCLK_DIG4_SHIFT     20


#define IFC_GLOBAL_RESET_WE     0x0008
#define IFC_GLOBAL_RESET_OFF    0x0004
#define IFC_GLOBAL_RESET_ON     0x0000

#define IFC_ADC_RGE             0x0e34
#define IFC_ADC_CONVERT_RGE     0x0e35

// 97/09/04 Tom MB
//#define IFC_ADC_SOURCE_CNT      13
#define IFC_ADC_SOURCE_CNT      15
// 97/09/04 Tom ME
#define IFC_ADC_SOURCE_WE       0x0040
#define IFC_ADC_DATA_RD         0x0020
#define IFC_ADC_SOURCE_3P3V     0x0008
#define IFC_ADC_SOURCE_P5VR     0x0009
#define IFC_ADC_SOURCE_P5V      0x000A
#define IFC_ADC_SOURCE_P15V     0x000B
#define IFC_ADC_SOURCE_N15V     0x000C
#define IFC_ADC_SOURCE_AGND     0x000D
#define IFC_ADC_SOURCE_REF10V   0x000E
#define IFC_ADC_SOURCE_N15VA    0x0010
#define IFC_ADC_SOURCE_P15VA    0x0011
#define IFC_ADC_SOURCE_N24V     0x0012
#define IFC_ADC_SOURCE_P24V     0x0013
#define IFC_ADC_SOURCE_N60V     0x0014
#define IFC_ADC_SOURCE_P60V     0x0015
// 97/09/04 Tom MB
#define IFC_ADC_SOURCE_N5VR     0x0010 // for TR-6836, map to IFC_ADC_SOURCE_N15VA in TR-6850
#define IFC_ADC_SOURCE_P12VR    0x0011 // for TR-6836, map to IFC_ADC_SOURCE_P15VA in TR-6850
// 97/09/04 Tom ME
/////////////////////////////////////////////////////////
//hardware counter address
#define IFC_CNT_HIGH_ADDR 0xe00  //pci_high_addr  0xe00
#define IFC_CNT_LOAD_ADDR 0x36   //hd_cnt_load_addr  0x36
#define IFC_CNT_VAL_ADDR  0x37   //hd_cnt_value_addr  0x37
#define IFC_CNT_CTRL_ADDR 0x38   //hd_cnt_ctl_addr  0x38

//hardware counter write enable
#define IFC_CNT_TIMEOUT_EN  0x80000000 //hd_cnt_timeout_en  0x80000000
#define IFC_CNT_RST_EN      0x2        //hd_cnt_rst_en  0x2
#define IFC_CNT_OUT_EN      0x8        //hd_cnt_out_en  0x8
#define IFC_CNT_EN_EN       0x20       //hd_cnt_en_en  0x20

//hardware counter offsets
#define IFC_CNT_TIMEOUT_OFFSET 0 //hd_cnt_timeout_offset  0
#define IFC_CNT_RST_OFFSET     0 //hd_cnt_rst_offset  0
#define IFC_CNT_OUT_OFFSET     2 //hd_cnt_out_offset  2
#define IFC_CNT_EN_OFFSET      4 //hd_cnt_en_offset  4
////////////////////////////////////////////////////////

#define EXPORTDLL __declspec(dllexport)


 extern "C"
{

EXPORTDLL void ATE_DelayTimeMS(double tm);
EXPORTDLL int PCIINF_Init(char *filePath,char *msg);
EXPORTDLL void IFC_SystemReset(void);
//----------------------------------------------------------------------------------
EXPORTDLL void ATE_ResetIfcBoard();
EXPORTDLL bool ATE_IsIfcBoardID(); 
EXPORTDLL int  ATE_RdIfcBoardID();
EXPORTDLL int  ATE_RdModuleID(int slot);
EXPORTDLL int  ATE_RdAc64ModuleID(int slot);
// 2014/09/24 Tom MB : new PPMU chip
EXPORTDLL int  ATE_RdAc64ModulePPMUID(int slot);
// 2014/09/24 Tom ME

EXPORTDLL int ATE_IfcLoadFPGA(char *filePath,char *msg);
EXPORTDLL bool ATE_IsLoadIfcFPGA();
//----------------------------------------------------------------------------------
EXPORTDLL void ATE_WrStartTriggerEdge(int trigEdge);
EXPORTDLL void ATE_WrEowTriggerEdge(int trigEdge);

EXPORTDLL void ATE_WrBinActiveState(int activeState);
EXPORTDLL void ATE_WrEotActiveState(int activeState);
EXPORTDLL void ATE_WrRejActiveState(int activeState);

EXPORTDLL void ATE_WrParallelBinOut(int siteBN[], bool mode=0);
EXPORTDLL void ATE_WrSerialBinOut(int Site, int bin, bool mode=0);
EXPORTDLL int ATE_RdStartLatchSignal(int start[], int eow[], int onWf[]);
EXPORTDLL void ATE_ClearLatchStart();
EXPORTDLL void ATE_ClearLatchEow();

//----------------------------------------------------------------------------------
EXPORTDLL void ATE_WrParallelEot(int onSite[]);
EXPORTDLL void ATE_WrSerialNullEot();
EXPORTDLL void ATE_WrSerialEotxM(int siteEot);
EXPORTDLL void ATE_WrParallelRej(int onSite[], int fail[]);
EXPORTDLL void ATE_WrSerialRejxM(int rejSite, int fail);
//----------------------------------------------------------------------------------

EXPORTDLL void ATE_WrBinWidth(double width);  // unit is "mS".
EXPORTDLL void ATE_WrEotWidth(double width);
EXPORTDLL void ATE_WrRejWidth(double width);

// 2011/04/06 chiu MB, add new Rej_Mode "Fixed Mode" for 8 site TTL
EXPORTDLL void ATE_WrSerialRejF(int site, bool high);
EXPORTDLL void ATE_WrParallelRejF(int onSite[], bool high[]);
// 2011/04/06 chiu ME, add new Rej_Mode "Fixed Mode" for 8 site TTL

EXPORTDLL bool ATE_CheckPower(int pwrIdx, double &value);

EXPORTDLL bool ATE_CheckPower3P3V(double &powVal);
EXPORTDLL bool ATE_CheckPowerP5VR(double &powVal);
EXPORTDLL bool ATE_CheckPowerP5V(double &powVal);
EXPORTDLL bool ATE_CheckPowerP18V(double &powVal);
EXPORTDLL bool ATE_CheckPowerN18V(double &powVal);

EXPORTDLL bool ATE_CheckPowerN18VA(double &powVal);
EXPORTDLL bool ATE_CheckPowerP18VA(double &powVal);
EXPORTDLL bool ATE_CheckPowerN26V(double &powVal);
EXPORTDLL bool ATE_CheckPowerP26V(double &powVal);
EXPORTDLL bool ATE_CheckPowerN54V(double &powVal);
EXPORTDLL bool ATE_CheckPowerP54V(double &powVal);

// 97/09/04 Tom MB
EXPORTDLL bool ATE_CheckPowerP12VR(double &powVal);
EXPORTDLL bool ATE_CheckPowerN5VR(double &powVal);
EXPORTDLL bool ATE_CheckPowerN24V(double &powVal);
EXPORTDLL bool ATE_CheckPowerP24V(double &powVal);
// 97/09/04 Tom ME

EXPORTDLL int ATE_CheckPowerVal(char *msg,bool bPowerON);




EXPORTDLL void ATE_MasterCLK(int clkIdx, bool onOff);

// 5/28/03 Eddy, for power on/off double sure
EXPORTDLL bool ATE_IFC_RWOK();         /* true: OK, false: not OK */

EXPORTDLL extern int ifc_adc_data;
EXPORTDLL extern bool ifcDlyFlagErr;


}
void ATE_WrBinWidth(int Site, double width);  // unit is "mS"
void ATE_WrEotWidth(int Site, double width);
void ATE_WrRejWidth(int Site, double width);
#endif




