#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    // Initialization
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    // Page directory is stored in kernel_mem_pool
    page_directory = (unsigned long*) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);

    // First 4 MB is directly mapped.
    unsigned long* kernel_page_table = (unsigned long*) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);

    // Mark the shared portion of memory to supervisor level, read/write, present
    unsigned long address = 0;
    for (unsigned int i = 0; i < ENTRIES_PER_PAGE; i++) {
        kernel_page_table[i] = address | 3;
        address += PAGE_SIZE;
    }

    // Fill the first entry of the page directory
    page_directory[0] = (unsigned long) kernel_page_table;
    page_directory[0] = page_directory[0] | 3;

    // Set the oher 1023 entries to supervisor level, read/write, not present
    for (unsigned int i = 1; i < ENTRIES_PER_PAGE; i++) {
        page_directory[i] = 0 | 2;
    }

    Console::puts("Constructed Page Table object\n");
}

void PageTable::load()
{
    // Load current page table
    current_page_table = this;
    write_cr3((unsigned long) this->page_directory);

    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    // Enable paging
    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000);

    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    if ((_r->err_code & 1) == 1) {
        // The exception is caused by protection fault
        Console::puts("Reference denied for protection!\n");
        Console::puts("handled page fault\n");
        return;
    }

    // The exception is caused by the page not valid
    // Read the current page directory
    unsigned long* current_directory = (unsigned long*) read_cr3();

    // Read the page fault address
    unsigned long fault_address = read_cr2();

    // Get the page table number
    unsigned long page_table_number = (fault_address >> 22) & 0x3FF;

    // Get the page number
    unsigned long page_number = (fault_address >> 12) & 0x3FF;

    unsigned long* page_table;

    if ((current_directory[page_table_number] & 1) == 1) {
        // If the page table which fault address belongs to is in memory, get it
        page_table = (unsigned long*) (current_directory[page_table_number] & 0xFFFFF000);
    } else {
        // If the page table which fault address belongs to is not in memory, allocate one
        current_directory[page_table_number] = (kernel_mem_pool->get_frames(1) * PAGE_SIZE) | 3;

        // Get the page table
        page_table = (unsigned long*) (current_directory[page_table_number] & 0xFFFFF000);

        // Mark the entries in the page table to supervisor level, read/write, not valid
        for (unsigned int i = 0; i < ENTRIES_PER_PAGE; i++) {
            page_table[i] = 0 | 2;
        }
    }

    // Allocate a frame for the fault address, mark it to supervisor level, read/write, valid
    page_table[page_number] = (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;

    Console::puts("handled page fault\n");
}



