/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system form disk\n");

    // basic initialization
    disk = _disk;
    size = disk->size();
    totalBlockNum = (int)size / 512;

    // initialize the fileNode list
    blockOwner = new int[totalBlockNum];
    memset(blockOwner, 0, sizeof(blockOwner));
    dummy = new FileNode(-1);
    return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
    Console::puts("formatting disk\n");

    // delete the old one, create a new one
    SimpleDisk* temp = _disk;
    _disk = new SimpleDisk(MASTER, _size);
    delete temp;
    return true;
}

File * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file\n");

    // find the fileNode based on the _file_id, and return the file if found
    FileNode* cur = dummy->next;
    while (cur != NULL) {
        if (cur->id ==_file_id) {
            return cur->file;
        }
        cur = cur->next;
    }
    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file\n");

    // create a new file and append it to the list
    FileNode* cur = dummy;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new FileNode(_file_id);
    cur = cur->next;
    cur->file = new File(this, _file_id);
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file\n");

    // delete the file and remove it from the list
    FileNode* cur = dummy->next;
    FileNode* prev = dummy;
    while (cur != NULL && cur->id != _file_id) {
        cur = cur->next;
        prev = prev->next;
    }
    if (cur == NULL) {
        Console::puts("no such file!\n");
        return true;
    }
    File* file = cur->file;
    Block* curBlock = file->dummy->next;
    while (curBlock != NULL) {
        blockOwner[curBlock->blockNo] = 0;
        curBlock = curBlock->next;
    }
    prev->next = cur->next;
    delete cur;
    delete file;
    return true;
}

void FileSystem::eraseFile(int _id) {
    // free the blocks owned by the file
    for (int i = 0; i < totalBlockNum; i++) {
        if (blockOwner[i] == _id) {
            blockOwner[i] = 0;
        }
    }
}

unsigned long FileSystem::requireBlock(int _id) {
    // allocate a free block for the file
    for (int i = 0; i < totalBlockNum; i++) {
        if (blockOwner[i] == 0) {
            blockOwner[i] = _id;
            return (unsigned long)i;
        }
    }
}
