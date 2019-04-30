cmake_minimum_required(VERSION 3.9)
project(MP4_Sources)

set(CMAKE_CXX_STANDARD 11)

add_executable(MP4_Sources
        assert.C
        assert.H
        console.C
        console.H
        cont_frame_pool.C
        cont_frame_pool.H
        exceptions.C
        exceptions.H
        gdt.C
        gdt.H
        idt.C
        idt.H
        interrupts.C
        interrupts.H
        irq.C
        irq.H
        kernel.C
        machine.C
        machine.H
        machine_low.H
        page_table.C
        page_table.H
        paging_low.H
        simple_keyboard.C
        simple_keyboard.H
        simple_timer.C
        simple_timer.H
        utils.C
        utils.H
        vm_pool.C
        vm_pool.H)
