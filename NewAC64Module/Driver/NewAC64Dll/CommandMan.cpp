#include "CommandMan.h"

CommandCtrl::CommandCtrl()
{
    InitCMD();
}
    
CommandCtrl::~CommandCtrl()
{
    ClearCMD();
}

void CommandCtrl::ExecuteCommand(CommandEnum input)
{
    IAPICmd* curcmd=GetCommand(input);
    if(curcmd)
        curcmd->Execute();
}

IAPICmd* CommandCtrl::GetCommand(CommandEnum input)
{
    std::map<CommandEnum, IAPICmd*>::iterator pos=_cmdmap.find(input);
    if(pos!=_cmdmap.end())
        return pos->second;
    else 
    {
        return NULL;
    }
}

void CommandCtrl::InitCMD()
{
    _cmdmap[Command_SCAN_Board]=new EssentialCMD();
}   

void CommandCtrl::ClearCMD()
{
    std::map<CommandEnum, IAPICmd*>::iterator pos=_cmdmap.begin();
    std::map<CommandEnum, IAPICmd*>::iterator endpos=_cmdmap.end();

    for(;pos!=endpos;pos++)
    {
        if(pos->second)
            delete pos->second;
    }
    _cmdmap.clear();
}