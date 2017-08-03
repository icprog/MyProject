#define DLLIMPORT __declspec(dllimport)
 
extern "C" 
{
	
  DLLIMPORT extern bool PCICheckOK;

  DLLIMPORT void PCI_Wr(int addr,int data);
  DLLIMPORT int PCI_Rd(int addr);

  DLLIMPORT void PCI_IOW(int addr , int data);
  DLLIMPORT int PCI_IOR(int addr);

  DLLIMPORT void PCI_Init();
  DLLIMPORT void PCI_Close();

  DLLIMPORT void PCI_WrTimeAddr(int addr);
  DLLIMPORT void PCI_WrTimeData(int data);
  DLLIMPORT int  PCI_RdTimeData(void);
}