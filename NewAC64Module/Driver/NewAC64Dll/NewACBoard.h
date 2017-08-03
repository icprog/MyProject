#ifndef ___AC_BOARD___
#define ___AC_BOARD___

#include "IProperty.h"

class NewACBoard
{
public:
    NewACBoard(int slot)
    {
        Slot.Set(slot);
    }

    IProperty<int>   Slot;
};

#endif