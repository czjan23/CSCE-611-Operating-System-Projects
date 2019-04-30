cmake_minimum_required(VERSION 3.9)
project(MP7_Sources)

set(CMAKE_CXX_STANDARD 11)

add_executable(MP7_Sources
        assert.C
        assert.H
        console.C
        console.H
        exceptions.C
        exceptions.H
        file.C
        file.H
        file_system.C
        file_system.H
        frame_pool.C
        frame_pool.H
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
        mem_pool.C
        mem_pool.H
        simple_disk.C
        simple_disk.H
        simple_keyboard.C
        simple_keyboard.H
        simple_timer.C
        simple_timer.H
        thread.C
        thread.H
        threads_low.H
        utils.C
        utils.H)
