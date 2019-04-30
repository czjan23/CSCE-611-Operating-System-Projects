/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

unsigned int ContFramePool::pool_num = 0;
ContFramePool** ContFramePool::pool_list;

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    // Bitmap must fir in a single frame!
    assert(_n_frames <= FRAME_SIZE * 4 );

    //Initialize parameters
    base_frame_no = _base_frame_no;
    n_frames = _n_frames;
    n_free_frames = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;

    // If _info_frame_no is zero then we keep management info in the first
    // frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char*) (base_frame_no * FRAME_SIZE);
        n_free_frames -= 1;
    }
    else {
        bitmap = (unsigned char*) (info_frame_no * FRAME_SIZE);
    }

    //Construct the pool list
    ContFramePool::pool_list[ContFramePool::pool_num] = this;
    ContFramePool::pool_num += 1;

    // Number of frames must be "fill" the bitmap!
    assert(n_frames % 4 == 0);

    // Everything ok. Proceed to mark all bits as unallocated in the bitmap
    for(int i = 0; i * 4 < n_frames; i++) {
        bitmap[i] = 0x00;
    }

    // Mark the first frame as being used and is the head of the first contiguous frames
    if(info_frame_no == 0) {
        bitmap[0] = 0x40;
        n_free_frames -= 1;
    }

    Console::puts("Frame Pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    // The number of frames to allocate should be a positive number
    assert(_n_frames > 0);

    // There should be enough free frames
    if(n_free_frames < _n_frames) {
        return 0;
    }

    // Allocate the frames based on first fit strategy
    unsigned int start = 0;
    while(start <= n_frames - _n_frames) {

        unsigned int count_frames = 0;
        unsigned int now = start;
        unsigned int r = start / 4;
        unsigned int c = start % 4;
        unsigned char mask = 0xC0 >> 2 * c;
        if((bitmap[r] & mask) == 0x00) {
            count_frames = 1;
            now = start + 1;
            while(count_frames < _n_frames) {
                r = now / 4;
                c = now % 4;
                mask = 0xC0 >> 2 * c;
                if((bitmap[r] & mask) == 0x00) {
                    count_frames++;
                    now++;
                }
                else {
                    break;
                }
            }
        }

        if(count_frames < _n_frames) {
            start = now + 1;
        }
        else {
            r = start / 4;
            c = start % 4;
            mask = 0x40 >> 2 * c;
            bitmap[r] = bitmap[r] | mask;
            for (int i = start + 1; i < start + _n_frames; i++) {
                r = i / 4;
                c = i % 4;
                mask = 0xC0 >> 2 * c;
                bitmap[r] = bitmap[r] | mask;
            }
            n_free_frames -= _n_frames;
            return base_frame_no + start;
        }
    }

    return 0;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // Mark all frames in the range as being used.
    unsigned int now = _base_frame_no - base_frame_no;
    unsigned int end = now + _n_frames;
    unsigned int r = now / 4;
    unsigned int c = now % 4;
    unsigned char mask = 0x40 >> 2 * c;
    bitmap[r] = bitmap[r] | mask;

    now += 1;
    while(now < end) {
        r = now / 4;
        c = now % 4;
        mask = 0xC0 >> 2 * c;
        bitmap[r] = bitmap[r] | mask;
        now += 1;
    }

    n_free_frames -= _n_frames;
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // Find the frame pool which the frames needed to be released belong to
    unsigned int pool_num = 0;
    while(pool_list[pool_num]->n_frames + pool_list[pool_num]->base_frame_no < _first_frame_no) {
        pool_num += 1;
    }

    // Call the corresponding frame pool's release_healper function to release frames
    pool_list[pool_num]->release_helper(_first_frame_no);

}

void ContFramePool::release_helper(unsigned long _first_frame_no)
{
    // Release the contiguous frames that were allocated and start with frame with number of _first_frame_no
    unsigned int now = _first_frame_no - base_frame_no;
    unsigned int r = now / 4;
    unsigned int c = now % 4;
    unsigned char mask = 0xC0 >> 2 * c;
//    if((bitmap[r] & mask) << 2 * c != 0x40) {
//        Console::puts("Invalid release operation!");
//        assert(false);
//    }
    bitmap[r] = bitmap[r] ^ mask;

    now += 1;
    while(true) {
        r = now / 4;
        c = now % 4;
        mask = 0xC0 >> 2 * c;
        if((bitmap[r] & mask) << 2 * c != 0xC0) {
            break;
        }
        bitmap[r] = bitmap[r] ^ mask;
    }
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    return _n_frames / (4 * FRAME_SIZE) + (_n_frames % (4 * FRAME_SIZE) > 0 ? 1 : 0);
}
