//自己实现一个静态栈
#ifndef PWDSTK_H
#define PWDSTK_H

#define STACK_SIZE 128

class PwdEntry
{
public:
    PwdEntry(char *name_, int headSec_);
    PwdEntry();
    int headSector; //目录文件头所在扇区号
    char *name;     //为了快速搜索，记录该层目录的名称
};

// template<class T>
class PwdStack
{
    public:
    bool push(PwdEntry ele);
    PwdEntry top();
    bool isEmpty();
    bool pop();
    PwdStack();
    int ind;

private:
    PwdEntry stk[STACK_SIZE];
   
};

#endif