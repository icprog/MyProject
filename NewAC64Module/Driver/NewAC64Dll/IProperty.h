#ifndef __IProperty_CLASS__
#define __IProperty_CLASS__

templat <class T>
class IProperty
{
    public:
            T  Get()
            {
                return _t;
            }
            void Set(T &input)
            {
                _t=input;
            }
    private:
             T  _t;
};

#endif