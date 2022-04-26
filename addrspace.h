// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#define UserStackSize		1024 	// increase this as necessary!

//l+
#define UserProgramNum  256

class AddrSpace
{
  public:
    // add: getSpaceId()
    AddrSpace(OpenFile *executable); // 创建地址空间
    ~AddrSpace();                    // 析构函数

    void InitRegisters(); // 初始化CPU寄存器
    void SaveState();     // 保存、储存地址空间
    void RestoreState();  // 恢复地址空间
    void Print();         // 打印页表
    unsigned int getSpaceId() { return spaceId; }

  // #ifdef FILESYS
    OpenFile *fileDescriptor[UserProgramNum]; // 文件描述符，0、1、2分别为stdin、stdout、stderr
    int getFileDescriptor(OpenFile *openfile);
    OpenFile *getFileId(int fd);
    void releaseFileDescriptor(int fd);
  // #endif

  private:
    // add *userMap, *pidMap,spaceId
    static BitMap *userMap, *pidMap; // 全局位图
    TranslationEntry *pageTable;     // 线性页表
    unsigned int spaceId;
    unsigned int numPages;  // 页表中的页表项
    // unsigned int nop;  // 冗余量，防止溢出
};

#endif // ADDRSPACE_H
