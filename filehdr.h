// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "bitmap.h"

// #define NumDirect 	((SectorSize - 2 * sizeof(int)) / sizeof(int))
#define NumDirect ((SectorSize - 4 * sizeof(int)) / sizeof(int)) //让他留下8个字节的额外空间，用于记录文件头的其他内容
#define MaxFileSize 	(NumDirect * SectorSize)

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
    FileHeader();

    bool Allocate(BitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data

    bool Allocate(BitMap *freeMap, int fileSize, int incrementBytes);
    
    void Deallocate(BitMap *bitMap); // De-allocate this file's
                                                                                                 //  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

    void Cat();

    void setType(int t_);
    int getType(){return type;}

    bool createNextIndexingNode(BitMap *freeMap);

    int getNextSector(){
      return nextSector;
    }

    int getNumSectors(){
      return numSectors;
    }

    int getNumBytes(){
      return numBytes;
    }

  private:
    int numBytes;			//4B Number of bytes in the file
    int numSectors;			//4B Number of data sectors in the file
    int dataSectors[NumDirect];		//4B*22 Disk sector numbers for each data 
					// block in the file
    int type; //4B 类型。0是文件，1是目录，2是设备
    int nextSector; //4B 链接索引头所在扇区。 
};

#endif // FILEHDR_H
