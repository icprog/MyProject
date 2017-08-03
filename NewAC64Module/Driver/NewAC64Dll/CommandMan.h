#ifndef __COMMAND_MAN__
#define __COMMAND_MAN__

#include "ICmd.h"
#include <map>


enum CommandEnum
{
    Command_SCAN_Board=0,
};

class CommandCtrl
{
public:
    CommandCtrl();
    ~CommandCtrl();
    void ExecuteCommand(CommandEnum input);
    
private:
    IAPICmd* GetCommand(CommandEnum input);
    void InitCMD();
    void ClearCMD();
    std::map<CommandEnum, IAPICmd*> _cmdmap;
};

#endif