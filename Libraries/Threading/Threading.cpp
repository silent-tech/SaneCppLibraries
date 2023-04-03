// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#include "Threading.h"

#include "../Foundation/StringNative.h"

#if SC_PLATFORM_WINDOWS
#include "ThreadingInternalWindows.inl"
#else
#include "ThreadingInternalPosix.inl"
#endif

struct SC::Thread::CreateParams
{
    CreateParams(Thread* thread) : thread(thread) {}

    EventObject event;
    Action*     callback;
    Action*     syncCallback;
    Thread*     thread;
    StringView  nameNullTerminated;

    Internal::NativeHandle threadHandle;

    static Internal::CallbackReturnType threadFunc(void* argument)
    {
        CreateParams& self = *static_cast<CreateParams*>(argument);

        if (not self.nameNullTerminated.isEmpty())
        {
            Internal::setThreadName(self.threadHandle, self.nameNullTerminated);
        }
        if (self.syncCallback)
        {
            (*self.syncCallback)();
        }
        // Note: If cb has captured heap allocated objects, they will get freed on the thread
        auto callback = self.callback;
        self.event.signal();
        if (callback)
        {
            (*callback)();
        }
        return 0;
    }
};

SC::Thread::~Thread() { SC_DEBUG_ASSERT(not thread.hasValue() && "Forgot to call join() or detach()"); }

SC::ReturnCode SC::Thread::start(StringView name, Action* func, Action* syncFunc)
{
    if (thread.hasValue())
        return "Error thread already started"_a8;
    CreateParams self(this);
    self.callback     = func;
    self.syncCallback = syncFunc;
    StringNative<128> nameNative;
    SC_TRY_IF(nameNative.convertNullTerminateFastPath(name, self.nameNullTerminated));
    OpaqueThread opaqueThread;
    SC_TRY_IF(Internal::createThread(self, opaqueThread, self.threadHandle, &CreateParams::threadFunc));
    thread.assign(move(opaqueThread));
    self.event.wait();
    return true;
}

SC::ReturnCode SC::Thread::join()
{
    OpaqueThread* threadNative;
    SC_TRY_IF(thread.get(threadNative));
    SC_TRY_IF(Internal::joinThread(threadNative));
    thread.clear();
    return true;
}

SC::ReturnCode SC::Thread::detach()
{
    OpaqueThread* threadNative;
    SC_TRY_IF(thread.get(threadNative));
    SC_TRY_IF(Internal::detachThread(threadNative));
    thread.clear();
    return true;
}

void SC::EventObject::wait()
{
    mutex.lock();
    while (not isSignaled)
    {
        cond.wait(mutex);
    }
    if (autoReset)
    {
        isSignaled = false;
    }
    mutex.unlock();
}

void SC::EventObject::signal()
{
    mutex.lock();
    isSignaled = true;
    mutex.unlock();
    cond.signal();
}
