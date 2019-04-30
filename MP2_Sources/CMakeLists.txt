cmake_minimum_required(VERSION 3.9)
project(MP2_Sources)

set(CMAKE_CXX_STANDARD 11)

add_executable(MP2_Sources
        assert.C
        assert.H
        console.C
        console.H
        cont_frame_pool.C
        cont_frame_pool.H
        kernel.C
        machine.C
        machine.H
        machine_low.H
        simple_frame_pool.C
        simple_frame_pool.H
        utils.C
        utils.H)
