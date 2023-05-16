#pragma once
#ifndef PCTALK_HPP
#define PCTALK_HPP

#include <mutex>
#include <switch.h>

int PcTalks();

class DeShakkar {
    private:
    static std::mutex debugLock;

    public:
    static Result shakDebugActiveProcess(Handle *debug, u64 processID) {
        debugLock.lock();
        return svcDebugActiveProcess(debug, processID);
    }
    static Result shakCloseHandle(Handle handle){
        debugLock.unlock();
        return svcCloseHandle(handle);
    }
};
//extern std::mutex DeShakkar::debugLock;

#endif