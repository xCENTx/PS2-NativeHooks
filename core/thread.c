#include "thread.h"
#include "memory.h"

//
// ============================================================================
//  CONSTANTS
// ============================================================================
//

#define THREAD_MAX_COUNT            4
#define THREAD_STACK_SIZE    0x4000

//
// ============================================================================
//  STATIC DATA
// ============================================================================
//

__attribute__((section(".thread_stack"), aligned(16)))
static u8 g_ThreadStack[THREAD_MAX_COUNT][THREAD_STACK_SIZE];

static s32 g_ThreadCount = 0;

//
// ============================================================================
//  THREAD FUNCTIONS
// ============================================================================
//

static sceCreateThread_t m_createThread = NULL;
static sceDeleteThread_t m_deleteThread = NULL;
static sceStartThread_t m_startThread = NULL;
static sceExitThread_t m_exitThread = NULL;
static sceExitDeleteThread_t m_exitDeleteThread = NULL;
static sceTerminateThread_t m_terminateThread = NULL;
static sceChangeThreadPriority_t m_changeThreadPriority = NULL;
static sceGetThreadId_t m_getThreadId = NULL;
static sceSleepThread_t m_sleepThread = NULL;
static sceWakeupThread_t m_wakeupThread = NULL;
static sceSuspendThread_t m_suspendThread = NULL;
static sceResumeThread_t m_resumeThread = NULL;

//
// ============================================================================
//  SETUP
// ============================================================================
//

void Thread_SetCreateThread(sceCreateThread_t function)
{
    m_createThread = function;
}

void Thread_SetDeleteThread(sceDeleteThread_t function)
{
    m_deleteThread = function;
}

void Thread_SetStartThread(sceStartThread_t function)
{
    m_startThread = function;
}

void Thread_SetExitThread(sceExitThread_t function)
{
    m_exitThread = function;
}

void Thread_SetExitDeleteThread(sceExitDeleteThread_t function)
{
    m_exitDeleteThread = function;
}

void Thread_SetTerminateThread(sceTerminateThread_t function)
{
    m_terminateThread = function;
}

void Thread_SetChangeThreadPriority(sceChangeThreadPriority_t function)
{
    m_changeThreadPriority = function;
}

void Thread_SetGetThreadId(sceGetThreadId_t function)
{
    m_getThreadId = function;
}

void Thread_SetSleepThread(sceSleepThread_t function)
{
    m_sleepThread = function;
}

void Thread_SetWakeupThread(sceWakeupThread_t function)
{
    m_wakeupThread = function;
}

void Thread_SetSuspendThread(sceSuspendThread_t function)
{
    m_suspendThread = function;
}

void Thread_SetResumeThread(sceResumeThread_t function)
{
    m_resumeThread = function;
}

//
// ============================================================================
//  THREAD MANAGEMENT
// ============================================================================
//

s32 Thread_CreateThread(ThreadFunction_t function, u32 gp, THREAD_PRIORITY priority)
{
    if (function == NULL)
        return -1;

    if (m_createThread == NULL ||
        m_startThread == NULL ||
        m_deleteThread == NULL)
    {
        return -1;
    }
    if (g_ThreadCount >= THREAD_MAX_COUNT)
        return -1;

    s32 slot = g_ThreadCount;

    ee_thread_t thread = { 0 };
    
    thread.func             = (void*)function;
    thread.stack            = g_ThreadStack[slot];
    thread.stack_size       = THREAD_STACK_SIZE;
    thread.gp_reg           = (void*)gp;
    thread.initial_priority = priority;

    s32 threadId = m_createThread(&thread);

    if (threadId < 0)
        return threadId;

    s32 result = m_startThread(threadId, NULL);

    if (result < 0)
    {
        m_deleteThread(threadId);
        return result;
    }

    g_ThreadCount++;

    return threadId;
}

s32 Thread_CreateThreadGP(ThreadFunction_t function, THREAD_PRIORITY priority)
{
    return Thread_CreateThread(function, Memory_GetGP(), priority);
}

s32 Thread_DeleteThread(s32 thread_id)
{
    if (m_deleteThread == NULL)
        return -1;

    return m_deleteThread(thread_id);
}

s32 Thread_StartThread(s32 thread_id, void* args)
{
    if (m_startThread == NULL)
        return -1;

    return m_startThread(thread_id, args);
}

s32 Thread_TerminateThread(s32 thread_id)
{
    if (m_terminateThread == NULL)
        return -1;

    return m_terminateThread(thread_id);
}

s32 Thread_ChangePriority(s32 thread_id, s32 priority)
{
    if (m_changeThreadPriority == NULL)
        return -1;

    return m_changeThreadPriority(thread_id, priority);
}

s32 Thread_GetThreadId(void)
{
    if (m_getThreadId == NULL)
        return -1;

    return m_getThreadId();
}

s32 Thread_Sleep(void)
{
    if (m_sleepThread == NULL)
        return -1;

    return m_sleepThread();
}

s32 Thread_Wakeup(s32 thread_id)
{
    if (m_wakeupThread == NULL)
        return -1;

    return m_wakeupThread(thread_id);
}

s32 Thread_Suspend(s32 thread_id)
{
    if (m_suspendThread == NULL)
        return -1;

    return m_suspendThread(thread_id);
}

s32 Thread_Resume(s32 thread_id)
{
    if (m_resumeThread == NULL)
        return -1;

    return m_resumeThread(thread_id);
}

void Thread_Exit(void)
{
    if (m_exitThread != NULL)
        m_exitThread();
}

void Thread_ExitDelete(void)
{
    if (m_exitDeleteThread != NULL)
        m_exitDeleteThread();
}