// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "filehdr.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

//+
void AdvancePC()
{
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));         // 前一个PC
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));         // 当前PC
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4); // 下一个PC
}

//+
void StartProcess(int spaceId)
{
    // printf("spaceId:%d\n",spaceId);
    ASSERT(currentThread->userProgramId() == spaceId);
    currentThread->space->InitRegisters(); // 设置寄存器初值
    currentThread->space->RestoreState();  // 加载页表寄存器
    machine->Run();                        // 运行
    ASSERT(FALSE);
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    // printf("exception type:%d\n", type);

    //+
    if(which == SyscallException){
        switch (type)
        {
            case SC_Halt:
            {
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            }

            //+
            case SC_SHLO:
            {
                printf("[Syscall] SC_SHLO, from pid: %d\n", (currentThread->space)->getSpaceId());
                printf("hello world!\n");
                AdvancePC();
                break;
            }

            //+
            case SC_Exec:
            {
                printf("[Syscall] SC_Exec, from pid: %d\n",(currentThread->space)->getSpaceId());
                int addr = machine->ReadRegister(4);
                char filename[50];
                for(int i = 0; ; i++){
                    machine->ReadMem(addr+i,1,(int *)&filename[i]);
                    if(filename[i] == '\0') break;
                }
                OpenFile *executable = fileSystem->Open(filename);
                if(executable == NULL) {
                    printf("Unable to open file %s\n",filename);
                    return;
                }
                // 建立新地址空间（实际上是创建进程）
                AddrSpace *space = new AddrSpace(executable);
                // space->Print();   // 输出新分配的地址空间
                delete executable;	// 关闭文件
                // 建立新核心线程，为映射用户线程做准备
                Thread *thread = new Thread(filename);
                printf("[New Kernel Thread] pid: %d, name: %s\n",space->getSpaceId(),filename);
                // 将用户进程的唯一一个线程映射到核心线程上(所谓用户进程就是那个Addrspace，映射也就是告诉那个核心线程，你可以跑这段Addrspace上的代码了。)
                thread->space = space;
                thread->Fork(StartProcess,(int)space->getSpaceId());
                machine->WriteRegister(2,space->getSpaceId());
                AdvancePC();
                break;
            }

            //+
            case SC_Exit:
            {
                printf("[Syscall] SC_Exit, from pid: %d\n", (currentThread->space)->getSpaceId());
                int exitCode = machine->ReadRegister(4);
                machine->WriteRegister(2, exitCode);
                currentThread->setExitCode(exitCode);
                // 父进程的退出码特殊标记，由 Join 的实现方式决定
                // if (exitCode == 99)
                if (exitCode == 31)
                    scheduler->emptyList(scheduler->getTerminatedList());
                delete currentThread->space; 
                currentThread->Finish();
                AdvancePC();
                break;
            }
            
            //+
            case SC_Join:
            {
                printf("[Syscall] SC_Join, from pid: %d\n", (currentThread->space)->getSpaceId());
                int SpaceId = machine->ReadRegister(4);
                currentThread->Join(SpaceId);
                // waitProcessExitCode —— 返回 Joinee 的退出码
                machine->WriteRegister(2, currentThread->waitExitCode());
                AdvancePC();
                break;
            }

            //+
            case SC_Yield:
            {
                printf("[Syscall] SC_Yield, from pid: %d\n", (currentThread->space)->getSpaceId());
                currentThread->Yield();
                AdvancePC();
                break;
            }
                

            //+
            case SC_Create:
            {
                int addr = machine->ReadRegister(4);
                char filename[128];
                for (int i = 0; i < 128; i++)
                {
                    machine->ReadMem(addr + i, 1, (int *)&filename[i]);
                    if (filename[i] == '\0')
                        break;
                }
                if (!fileSystem->Create(filename, 0))
                    printf("create file %s failed!\n", filename);
                else
                    printf("create file %s succeed!\n", filename);
                AdvancePC();
                break;
            }

            //+
            case SC_Open:
            {
                int addr = machine->ReadRegister(4), fileId;
                char filename[128];
                for (int i = 0; i < 128; i++)
                {
                    machine->ReadMem(addr + i, 1, (int *)&filename[i]);
                    if (filename[i] == '\0')
                        break;
                }
                OpenFile *openfile = fileSystem->Open(filename);
                if (openfile == NULL)
                {
                    printf("File \"%s\" not Exists, could not open it.\n", filename);
                    fileId = -1;
                }
                else
                {
                    fileId = currentThread->space->getFileDescriptor(openfile);
                    if (fileId < 0)
                        printf("Too many files opened!\n");
                    else
                        printf("file:\"%s\" open succeed, the file id is %d\n", filename, fileId);
                }
                machine->WriteRegister(2, fileId);
                AdvancePC();
                break;
            }

            //+
            case SC_Write:
            {
                // 读取寄存器信息
                int addr = machine->ReadRegister(4);   // 写入数据
                int size = machine->ReadRegister(5);   // 字节数
                int fileId = machine->ReadRegister(6); // fd

                // 创建文件
                OpenFile *openfile = new OpenFile(fileId);
                ASSERT(openfile != NULL);

                // 读取具体写入的数据
                char buffer[128];
                for (int i = 0; i < size; i++)
                {
                    machine->ReadMem(addr + i, 1, (int *)&buffer[i]);
                    if (buffer[i] == '\0')
                        break;
                }
                buffer[size] = '\0';

                // 打开文件
                openfile = currentThread->space->getFileId(fileId);
                if (openfile == NULL)
                {
                    printf("Failed to Open file \"%d\".\n", fileId);
                    AdvancePC();
                    break;
                }
                if (fileId == 1 || fileId == 2)
                {
                    openfile->WriteStdout(buffer, size);
                    delete[] buffer;
                    AdvancePC();
                    break;
                }

                // 写入数据
                int writePos = openfile->Length();
                openfile->Seek(writePos);

                // 在 writePos 后面进行数据添加
                int writtenBytes = openfile->Write(buffer, size);
                if (writtenBytes == 0)
                    printf("Write file failed!\n");
                else if (fileId != 1 & fileId != 2)
                    printf("\"%s\" has wrote in file %d succeed!\n", buffer, fileId);
                AdvancePC();
                break;
            }

            //+
            case SC_Read:
            {
                // 读取寄存器信息
                int addr = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);   // 字节数
                int fileId = machine->ReadRegister(6); // fd

                // 打开文件
                OpenFile *openfile = currentThread->space->getFileId(fileId);

                // openfile->getFileHeader()->Print();

                // 打开文件读取信息
                char buffer[size + 1];
                int readnum = 0;
                if (fileId == 0){
                    // printf("std!\n");
                    readnum = openfile->ReadStdin(buffer, size);
                }else{
                    // printf("not std!\n");
                    readnum = openfile->Read(buffer, size);
                }
                   
                // printf("readnum:%d,fileId:%d,size:%d\n",readnum,fileId,size);
                for (int i = 0; i < readnum; i++)
                    machine->WriteMem(addr, 1, buffer[i]);
                buffer[readnum] = '\0';

                for (int i = 0; i < readnum; i++)
                    if (buffer[i] >= 0 && buffer[i] <= 9)
                        buffer[i] = buffer[i] + 0x30;
                char *buf = buffer;
                if (readnum > 0)
                {
                    if (fileId != 0)
                        printf("Read file (%d) succeed! the content is \"%s\", the length is %d\n", fileId, buf, readnum);
                }
                else
                    printf("\nRead file failed!\n");
                machine->WriteRegister(2, readnum);
                AdvancePC();
                break;
            }

            //+
            case SC_Close:
            {
                int fileId = machine->ReadRegister(4);
                OpenFile *openfile = currentThread->space->getFileId(fileId);
                if (openfile != NULL)
                {
                    openfile->WriteBack(); // 将文件写入DISK
                    delete openfile;
                    currentThread->space->releaseFileDescriptor(fileId);
                    printf("File %d closed succeed!\n", fileId);
                }
                else
                    printf("Failed to Close File %d.\n", fileId);
                AdvancePC();
                break;
            }

            default:
                printf("Unexpected user mode exception %d %d\n", which, type);
                ASSERT(FALSE);
                break;
        }
    }

    // if ((which == SyscallException) && (type == SC_Halt)) {
    //     DEBUG('a', "Shutdown, initiated by user program.\n");
    //     interrupt->Halt();
    // } else {
    //     printf("Unexpected user mode exception %d %d\n", which, type);
    //     ASSERT(FALSE);
    // }
}
