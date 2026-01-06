#include "FenceGPU.hpp"

#include <iostream>
#include <Utils.hpp>

FenceGPU::FenceGPU()
{
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

std::string FenceGPU::toString()
{
    switch (r) 
    {
        case GL_ALREADY_SIGNALED : 
            return TERMINAL_OK + "ALREADY_SIGNALED";
        
        case GL_TIMEOUT_EXPIRED : 
            return TERMINAL_ERROR + "TIMEOUT_EXPIRED";

        default :
            return "NOT UPDATED";
    }
}

void FenceGPU::printState()
{
    // system("clear");

    for(auto &i : list)
    {
        i.second.r = glClientWaitSync(i.second.fence, 0, 0);
        // std::cout << i.first << "\n\t" << i.second.toString() << "\n\n" << TERMINAL_RESET;

        glDeleteSync(i.second.fence);
    }
}