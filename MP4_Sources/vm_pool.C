/*
 File: vm_pool.C

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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    // Basic Initialization
    page_table = _page_table;
    base_address = _base_address;
    frame_pool = _frame_pool;
    size = _size;

    // use the first page of the pool to store region descriptors
    region_descriptors = (RegionDescriptors*)base_address;
    last_address = base_address + PageTable::PAGE_SIZE;
    regions_count = 0;
    total_regions_size = 0;
    page_table->register_pool(this);

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    // Limitation check
    if (regions_count == REGIONS_LIMIT || total_regions_size + _size > size - PageTable::PAGE_SIZE) {
        Console::puts("Cannot allocate this region!\n");
        return 0;
    }

    // Allocate the new region
    region_descriptors[regions_count].address = last_address;
    region_descriptors[regions_count].length = _size;
    total_regions_size += _size;
    regions_count += 1;
    last_address += _size;

    Console::puts("Allocated region of memory.\n");

    return last_address - _size;
}

void VMPool::release(unsigned long _start_address) {
    // Look for the region to release
    unsigned long index;
    for (index = 0; index < regions_count; index++) {
        if (region_descriptors[index].address == _start_address) {
            break;
        }
    }

    // Free the whole region
    unsigned long cur_addr = _start_address;
    while (cur_addr < _start_address + region_descriptors[index].length) {
        page_table->free_page(cur_addr);
        cur_addr += PageTable::PAGE_SIZE;
    }

    regions_count -= 1;
    total_regions_size -= region_descriptors[index].length;

    // If the region freed is the last region, we need to update last_address
    if (region_descriptors[index].address + region_descriptors[index].length == last_address) {
        last_address = region_descriptors[index].address;
    }

    // Update region_descriptors
    region_descriptors[index].address = region_descriptors[regions_count].address;
    region_descriptors[index].length = region_descriptors[regions_count].length;

    page_table->load();
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    for (unsigned long i = 0; i < regions_count; i++) {
        if (_address >= region_descriptors[i].address && _address <= region_descriptors[i].length + region_descriptors[i].address) {
            Console::puts("Checked whether address is part of an allocated region.\n");
            return true;
        }
    }
    Console::puts("Checked whether address is part of an allocated region.\n");
    return false;
}




