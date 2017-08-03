#ifndef __NEWAC_API_ADAPATER__
#define __NEWAC_API_ADAPATER__


class IAPICmd
{
 public:
        virtual ~IAPICmd(){}
        virtual void Execute()=0;
};


#endif