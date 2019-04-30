/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"

extern Scheduler * SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
        : SimpleDisk(_disk_id, _size) {
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
    // issue the read command
    issue_operation(READ, _block_no);

    // add the current thread to block queue, and yield the CPU
    Thread * currentThread = Thread::CurrentThread();
    SYSTEM_SCHEDULER->addToBlock(currentThread);
    SYSTEM_SCHEDULER->yield();

    // read the data, copy from simple_disk.c
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i*2]   = (unsigned char)tmpw;
        _buf[i*2+1] = (unsigned char)(tmpw >> 8);
    }
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
    // issue the write command
    issue_operation(WRITE, _block_no);

    // add the current thread to block queue, and yield the CPU
    Thread * currentThread = Thread::CurrentThread();
    SYSTEM_SCHEDULER->addToBlock(currentThread);
    SYSTEM_SCHEDULER->yield();

    // write the data, copy from simple_disk.c
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
        Machine::outportw(0x1F0, tmpw);
    }
}
