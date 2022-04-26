// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool FileHeader::createNextIndexingNode(BitMap *freeMap)
{
    FileHeader *hdr;
    hdr = new FileHeader;
    // BitMap* freeMap = fileSystem->getBitMap();
    int sector = freeMap->Find(); // find a sector to hold the file header
    if (sector == -1)
        return false; // no free block for file header
    
    hdr->WriteBack(sector);
    hdr->Allocate(freeMap, 1);
    delete hdr;
    nextSector=sector;
    // fileSystem->setBitMap(freeMap);
    printf("new file hdr sector:%d\n",sector);
    return true;
}

// // lintondoes (v2).
// bool FileHeader::Allocate(BitMap *freeMap, int fileSize, int incrementBytes)
// {
//     printf("----------------------------------------------secs:%d\n",numSectors);
//     //让他留下8个字节的额外空间，用于记录文件头的其他内容
//     // if (numSectors > 22) {
       
//     // }

//     if ((fileSize == 0) && (incrementBytes > 0))
//     {
//         if (freeMap->NumClear() < 1)
//             return false;
//         dataSectors[0] = freeMap->Find();
//         numSectors = 1;
//         numBytes = 0;
//     }

//     numBytes = fileSize;
//     int offset = numSectors * SectorSize - numBytes;
//     int newSectorBytes = incrementBytes - offset;
//     if (newSectorBytes <= 0)
//     {
//         numBytes = numBytes + incrementBytes;
//         return true;
//     }

//     int moreSectors = divRoundUp(newSectorBytes, SectorSize);
       
//     if (freeMap->NumClear() < moreSectors)
//         return false;

//     //让他留下8个字节的额外空间，用于记录文件头的其他内容
//     if (numSectors + moreSectors > 22)
//     {
//         printf("----------------------------------------------numsec>22\n");
//         if (!createNextIndexingNode(freeMap))
//             return false;

//         for (int i = numSectors; i <= 22; i++)
//             dataSectors[i] = freeMap->Find();

//         FileHeader *hdr;
//         hdr = new FileHeader;
//         hdr->FetchFrom(nextSector);

//         for (int i = 23; i < numSectors + moreSectors - 22; i++)
//             hdr->dataSectors[i] = freeMap->Find();
//     }else{
//         for (int i = numSectors; i < numSectors + moreSectors; i++)
//             dataSectors[i] = freeMap->Find();
//     }
    
//     numBytes = numBytes + incrementBytes;
//     numSectors = numSectors + moreSectors;
//     return true;
// }

//lintondoes.
bool FileHeader::Allocate(BitMap *freeMap, int fileSize,int incrementBytes){
//    if(numSectors>30)return false;
if (numSectors > 22) //让他留下3个字节的额外空间，用于记录文件头的其他内容
    return false;
if ((fileSize == 0) && (incrementBytes > 0))
{
    if (freeMap->NumClear() < 1)
        return false;
    dataSectors[0] = freeMap->Find();
    numSectors = 1;
    numBytes = 0;
   }
   numBytes=fileSize;
   int offset=numSectors*SectorSize-numBytes;
   int newSectorBytes=incrementBytes-offset;
   if(newSectorBytes<=0){
       numBytes=numBytes+incrementBytes;
       return true;
   }
   int moreSectors=divRoundUp(newSectorBytes,SectorSize);
//    if(numSectors+moreSectors>30)return false;
   if (numSectors + moreSectors > 22) //让他留下3个字节的额外空间，用于记录文件头的其他内容
       return false;
   if(freeMap->NumClear()<moreSectors)return false;
   for(int i=numSectors;i<numSectors+moreSectors;i++)dataSectors[i]=freeMap->Find();
   numBytes=numBytes+incrementBytes;
   numSectors=numSectors+moreSectors;
   return true;
}

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    for (int i = 0; i < numSectors; i++)
	dataSectors[i] = freeMap->Find();
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

FileHeader::FileHeader(){
    numBytes=0;
    numSectors=0;
    type=0;//默认初始化为文件头
    nextSector=-1;//默认不存在下一个索引链结点
    for(int i=0;i<NumDirect;i++)dataSectors[i]=0;
}

void FileHeader::setType(int t_){
    type=t_;
    // printf("setting type. type:%d\n",type);
}

void FileHeader::Deallocate(BitMap *freeMap)
{
    for (int i = 0; i < numSectors; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    return(dataSectors[offset / SectorSize]);
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void FileHeader::Cat()
{
    int i, j, k;
    char *data = new char[SectorSize];

    for (i = k = 0; i < numSectors; i++)
    {
        synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++)
        {
            if ('\040' <= data[j] && data[j] <= '\176') // isprint(data[j])
                printf("%c", data[j]);
            else
                printf("\\%x", (unsigned char)data[j]);
        }
        printf("\n");
    }
    delete[] data;
}

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("type: %d ",type);
    if(type==0)printf("—— this is a file.\n");
    else printf("this is a dir.\n");
    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}
