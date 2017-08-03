#ifndef __ESSENTIAL_CMD__
#define __ESSENTIAL_CMD__
#include "ICmd.h"
#include "drv_peb64.h"


class EssentialCMD : public IAPICmd
{
    public:
            ~virtual EssentialCMD()
            {

            }

            virtual void Execute()
            {
                //Retrieve Board info

                int curSlot=1;
                const int EndSlotdx=6;
                for(;curSlot<EndSlotdx;curSlot++)
                {
                    int  curID=drvRdMainBrdId( curSlot );
                    if(curID==PEB64_ID)
                    {
                            int x=10;
                    }
                }
                
            }
}

#endif