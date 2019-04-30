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

Scheduler::Scheduler() {
    // basic initialization
    dummy = new ThreadNode();
    tail = new ThreadNode();
    dummy->next = tail;
    tail->prev = dummy;

    Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {

    // disable interrupts
    if (Machine::interrupts_enabled()) {
        Machine::disable_interrupts();
    }

    // get the first node in the queue
    ThreadNode* cur = dummy->next;

    // let dummy node point to the second node in the queue
    dummy->next = cur->next;

    // erase the first node from the queue
    cur->next = NULL;
    cur->prev = NULL;

    // dispatch to the first thread in the queue
    Thread::dispatch_to(cur->thread);
}

void Scheduler::resume(Thread * _thread) {
    // resume does the same thing as add
    add(_thread);
}

void Scheduler::add(Thread * _thread) {
    // disable interrupts
    if (!Machine::interrupts_enabled()) {
        Machine::enable_interrupts();
    }

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
