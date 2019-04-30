/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

ThreadNode::ThreadNode() {
    thread = NULL;
    next = NULL;
    prev = NULL;
}

ThreadNode::ThreadNode(Thread* _thread) {
    thread = _thread;
    next = NULL;
    prev = NULL;
}

ThreadNode* Scheduler::dummy = NULL;
ThreadNode* Scheduler::tail = NULL;
ThreadNode* Scheduler::dummyBlock = NULL;
ThreadNode* Scheduler::tailBlock = NULL;

Scheduler::Scheduler() {
    // basic initialization for ready queue
    dummy = new ThreadNode();
    tail = new ThreadNode();
    dummy->next = tail;
    tail->prev = dummy;

    // basic initialization for block queue
    dummyBlock = new ThreadNode();
    tailBlock = new ThreadNode();
    dummyBlock->next = tailBlock;
    tailBlock->prev = dummyBlock;

    Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    // get the first node in the queue
    ThreadNode* cur = dummy->next;

    // erase the first node from the queue
    dummy->next = cur->next;
    cur->next->prev = dummy;

    // dispatch to the first thread in the queue
    Thread::dispatch_to(cur->thread);
    delete cur;
}

void Scheduler::resume(Thread * _thread) {
    // resume does the same thing as add at first
    add(_thread);

    // check if the block queue is empty or not
    if (dummyBlock->next == tailBlock) {
        return;
    }

    // check if the disk is ready
    if (!((Machine::inportb(0x1F7) & 0x08) != 0)) {
        return;
    }

    // remove the first node in the block queue from block queue
    ThreadNode* temp = dummyBlock->next;
    dummyBlock->next = temp->next;
    temp->next->prev = dummyBlock;

    // add the corresponding thread to ready queue
    add(temp->thread);
    delete temp;
}

void Scheduler::add(Thread * _thread) {
    // create the new thread and its corresponding data structure
    ThreadNode* newNode = new ThreadNode(_thread);

    // append the newNode to the end of the queue
    ThreadNode* temp = tail->prev;
    temp->next = newNode;
    newNode->prev = temp;
    newNode->next = tail;
    tail->prev = newNode;
}

void Scheduler::terminate(Thread * _thread) {
    // there is no thread in the queue
    if (dummy->next == tail) {
        Console::puts("There is no thread to terminate.");
        return;
    }

    // find the thread need to be terminated
    ThreadNode* temp = dummy->next;
    while (temp != tail) {
        if (temp->thread->ThreadId() == _thread->ThreadId()) {
            temp->prev->next = temp->next;
            temp->next->prev = temp->prev;
            delete temp;
            break;
        }
        temp = temp->next;
    }
}

void Scheduler::addToBlock(Thread *_thread) {
    // create the new thread and its corresponding data structure
    ThreadNode* newNode = new ThreadNode(_thread);

    // append the newNode to the end of the block queue
    ThreadNode* temp = tailBlock->prev;
    temp->next = newNode;
    newNode->prev = temp;
    newNode->next = tailBlock;
    tailBlock->prev = newNode;
}


