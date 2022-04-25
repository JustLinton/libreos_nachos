#include "pwdStack.h"
#include <memory.h>
#include <stdio.h>

// template <class T>
PwdStack::PwdStack()
{
    ind=0;
}

// template<class T>
bool PwdStack::push(PwdEntry ele)
{
    if(ind>=STACK_SIZE-1)return false;
    stk[++ind] = ele;
    // printf("push: index=%d\n",ind);
    return true;
}

// template <class T>
PwdEntry PwdStack::top()
{
    if(isEmpty())return PwdEntry();
    return stk[ind];
}

// template <class T>
bool PwdStack::isEmpty()
{
    return ind <= 0;
}

bool PwdStack::pop()
{
    if(isEmpty())return false;
    ind--;
    return true;
}

PwdEntry::PwdEntry(char *name_, int headSec_)
{
    name = new char[255];
    strcpy(name, name_);
    headSector = headSec_;
}

PwdEntry::PwdEntry()
{
    //根目录
    name = "";
    headSector = 1;
}