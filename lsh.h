#ifndef LSH_H
#define LSH_H

#include "pwdStack.h"

class LibreShell
{
    public:
        void startShell();
        LibreShell();
        PwdStack* getPwdStack();
    private :
        //当前工作目录的字符串表达
        PwdStack *pwdStack;
};

#endif