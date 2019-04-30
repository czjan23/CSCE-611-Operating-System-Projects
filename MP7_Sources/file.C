/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem* _file_system, int _id) {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");

    // basic initialization
    fileSystem = _file_system;
    id = _id;
    size = 0;
    pos = 0;
    dummy = new Block(-1);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("reading from file\n");

    // if it is end of file, no read
    if (EoF()) {
        Console::puts("end of file!\n");
        return 0;
    }

    // read the file until we have nothing to read or we have read _n bytes
    unsigned int start = pos;

    unsigned char* buffer = new unsigned char[512];

    while (pos < size && pos - start < _n) {
        int blockNum = pos / 512;
        int offSet = pos - blockNum * 512;

        Block* cur = dummy->next;

        for (int i = 0; i < blockNum; i++) {
            cur = cur->next;
        }

        fileSystem->disk->read(cur->blockNo, buffer);

        int i = 0;
        while (pos < size && pos - start < _n && i < 512) {
            _buf[pos++ - start] = buffer[i++];
        }
    }

    // return the bytes that we actually read
    return pos - start;
}


void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("writing to file\n");

    unsigned int start = pos;

    unsigned char* buffer = new unsigned char[512];

    // write to the block that the file has already owned
    while (pos < size && pos - start < _n) {
        int blockNum = pos / 512;
        int offSet = pos - blockNum * 512;

        Block* cur = dummy->next;
        for (int i = 0; i < blockNum; i++) {
            cur = cur->next;
        }

        fileSystem->disk->read(cur->blockNo, buffer);

        while (pos < size && pos - start < _n && pos / 512 == blockNum) {
            buffer[pos - blockNum * 512] = _buf[pos - start];
            pos++;
        }
    }

    // if we have write all the stuff, no need to continue
    if (pos - start == _n) {
        return;
    }

    // require new block from the file system, and write to it, until we write all the things in _buf
    while (pos - start < _n) {
        unsigned long newBlockNo = fileSystem->requireBlock(id);
        Block* cur = dummy;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = new Block(newBlockNo);
        cur->next->next = NULL;

        int i = 0;
        while (pos - start < _n && i < 512) {
            buffer[i++] = _buf[pos++ - start];
        }
        size += i;

        fileSystem->disk->write(newBlockNo, buffer);
    }
}

void File::Reset() {
    Console::puts("reset current position in file\n");

    // move the pos to the beginning of the file
    pos = 0;
}

void File::Rewrite() {
    Console::puts("erase content of file\n");

    // free the blocks owned by the file, and clean the list
    fileSystem->eraseFile(id);
    dummy->next = NULL;
}


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");
    return pos == size;
}
