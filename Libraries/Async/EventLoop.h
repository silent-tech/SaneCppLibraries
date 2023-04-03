// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once

#include "../Foundation/Atomic.h"
#include "../Foundation/Function.h"
#include "../Foundation/IntrusiveDoubleLinkedList.h"
#include "../Foundation/Limits.h"
#include "../Foundation/Optional.h"
#include "../Foundation/Span.h"
#include "../Foundation/TaggedUnion.h"
#include "../Foundation/Time.h"
#include "../InputOutput/FileDescriptor.h"

namespace SC
{
struct EventLoop;
struct Async;
struct AsyncTimeout;
struct AsyncRead;
struct AsyncWakeUp;
struct AsyncResult;
struct EventObject;
} // namespace SC

struct SC::AsyncResult
{
    EventLoop& loop;
    Async&     async;
    // TODO: Add AsyncResult error
};

struct SC::Async
{
    struct Timeout
    {
        IntegerMilliseconds timeout; // not needed, but keeping just for debugging
        TimeCounter         expirationTime;
    };

    struct Read
    {
        FileDescriptorNative fileDescriptor;
        Span<uint8_t>        readBuffer;
    };

    struct WakeUp
    {
        EventObject* eventObject = nullptr;
        Atomic<bool> pending     = false;
        EventLoop*   eventLoop   = nullptr; // Maybe this should be moved as part of every async
    };

    union Operation
    {
        enum class Type
        {
            Timeout,
            Read,
            WakeUp
        };

        Operation() {}
        ~Operation() {}

        Timeout timeout;
        Read    read;
        WakeUp  wakeUp;

        using FieldsTypes =
            TypeList<TaggedField<Operation, Type, decltype(timeout), &Operation::timeout, Type::Timeout>,
                     TaggedField<Operation, Type, decltype(read), &Operation::read, Type::Read>,
                     TaggedField<Operation, Type, decltype(wakeUp), &Operation::wakeUp, Type::WakeUp>>;
    };

    enum class State
    {
        Free,       // not in any queue
        Active,     // when monitored by OS syscall
        Submitting, // when in submission queue
        Cancelling  // when in cancellation queue (TBD)
    };

    Async* next  = nullptr;
    Async* prev  = nullptr;
    State  state = State::Free;

    TaggedUnion<Operation>       operation;
    Function<void(AsyncResult&)> callback;
};

struct SC::AsyncTimeout : public Async
{
};

struct SC::AsyncRead : public Async
{
};

struct SC::AsyncWakeUp : public Async
{
    [[nodiscard]] ReturnCode wakeUp();
};

struct SC::EventLoop
{
    // Creation
    [[nodiscard]] ReturnCode create();
    [[nodiscard]] ReturnCode close();

    // Execution
    [[nodiscard]] ReturnCode run();
    [[nodiscard]] ReturnCode runOnce();

    // Async Operations
    [[nodiscard]] ReturnCode addTimeout(AsyncTimeout& async, IntegerMilliseconds expiration,
                                        Function<void(AsyncResult&)>&& callback);

    [[nodiscard]] ReturnCode addRead(AsyncRead& async, FileDescriptorNative fileDescriptor, Span<uint8_t> readBuffer);

    [[nodiscard]] ReturnCode addWakeUp(AsyncWakeUp& async, Function<void(AsyncResult&)>&& callback,
                                       EventObject* eventObject = nullptr);

    // WakeUp support
    [[nodiscard]] ReturnCode wakeUpFromExternalThread(Async::WakeUp& wakeUp);

    [[nodiscard]] ReturnCode wakeUpFromExternalThread();

  private:
    IntrusiveDoubleLinkedList<Async> submission;
    IntrusiveDoubleLinkedList<Async> stagedHandles;
    IntrusiveDoubleLinkedList<Async> activeTimers;
    IntrusiveDoubleLinkedList<Async> activeWakeUps;

    TimeCounter loopTime;

    struct Internal;
    struct KernelQueue;
    static constexpr int InternalSize      = 1024;
    static constexpr int InternalAlignment = alignof(void*);
    template <typename T, int N, int Alignment>
    friend struct OpaqueFunctions;
    OpaqueUniqueObject<Internal, InternalSize, InternalAlignment> internal;

    void invokeExpiredTimers();
    void updateTime() { loopTime.snap(); }
    void submitAsync(Async& async);

    void runCompletionForNotifiers();

    [[nodiscard]] bool               shouldQuit();
    [[nodiscard]] const TimeCounter* findEarliestTimer() const;
    [[nodiscard]] ReturnCode         stageSubmissions(KernelQueue& queue);
};
