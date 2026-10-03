#ifndef THREAD_H
#define THREAD_H

#include "common.h"

/*
 * ============================================================================
 *  EE THREADING NOTES / RESEARCH
 * ============================================================================
 *
 *  NativeHooks contains a small wrapper around the PlayStation 2 EE kernel
 *  threading API. Thread creation and execution have been tested successfully
 *  in SOCOM; however, persistent worker threads are currently NOT recommended
 *  as the primary mechanism for running per-frame/gameplay logic.
 *
 *
 *  THREAD CREATION
 *  --------------------------------------------------------------------------
 *
 *  EE threads are created using:
 *
 *      CreateThread()
 *      StartThread()
 *
 *  and described by an ee_thread_t structure:
 *
 *      +0x00  status
 *      +0x04  func
 *      +0x08  stack
 *      +0x0C  stack_size
 *      +0x10  gp_reg
 *      +0x14  initial_priority
 *      +0x18  current_priority
 *      +0x1C  attr
 *      +0x20  option
 *
 *  Each simultaneously active thread requires its own stack.
 *
 *  Thread stacks used by NativeHooks are allocated from g_ThreadStack[] and
 *  should remain 16-byte aligned.
 *
 *
 *  GLOBAL POINTER ($gp)
 *  --------------------------------------------------------------------------
 *
 *  SOCOM relies on a valid MIPS $gp when executing many game functions.
 *
 *  Testing showed that a NativeHooks thread created with:
 *
 *      gp_reg = NULL
 *
 *  could execute ordinary NativeHooks code, but calls into SOCOM such as:
 *
 *      ftsGetPlayer()
 *
 *  returned invalid/NULL results.
 *
 *  Creating the same thread with the current SOCOM $gp allowed those calls to
 *  work correctly.
 *
 *  The observed SOCOM $gp is:
 *
 *      0x00494670
 *
 *  Memory_GetGP() can be used to inherit the current $gp when creating a
 *  thread. Thread_CreateThreadGP() exists as a convenience wrapper for this.
 *
 *
 *  THREAD PRIORITY
 *  --------------------------------------------------------------------------
 *
 *  EE thread priorities range from 0-127.
 *
 *      LOWER NUMBER = HIGHER PRIORITY
 *
 *  Priority 0 is reserved by the kernel and should not be used by NativeHooks.
 *
 *  SOCOM has been observed using several relatively high-priority threads,
 *  including priorities around:
 *
 *      3, 5, 6, ...
 *
 *  NativeHooks experiments have included priorities 1-10 and 0x40.
 *
 *  Changing priority alone did NOT provide a reliable periodic execution
 *  rate for a NativeHooks worker.
 *
 *
 *  EE SCHEDULING
 *  --------------------------------------------------------------------------
 *
 *  The EE thread scheduler is cooperative and should not be treated like a
 *  conventional preemptive/time-sliced desktop OS scheduler.
 *
 *  A READY thread is not guaranteed to receive CPU time at a regular interval.
 *
 *  This was confirmed experimentally in SOCOM.
 *
 *  A NativeHooks worker could remain:
 *
 *      THS_READY (0x02)
 *
 *  for long periods during an active mission while making little or no
 *  progress. The same thread could then execute extremely rapidly during
 *  menus, loading screens, mission transitions, or other game-state changes.
 *
 *  In an experiment with multiple equal-priority NativeHooks threads and no
 *  yielding, the first runnable thread could execute continuously while later
 *  threads remained READY. Once the running thread exited, the next thread
 *  began executing.
 *
 *  Therefore:
 *
 *      READY != regularly scheduled
 *
 *  and a persistent worker must not assume that it will execute once per
 *  frame, once per millisecond, or at any other stable cadence.
 *
 *
 *  THREAD STATUS
 *  --------------------------------------------------------------------------
 *
 *  ReferThreadStatus() (EE syscall 0x30) was used to inspect both SOCOM and
 *  NativeHooks threads.
 *
 *  Observed/documented status values:
 *
 *      THS_RUN          0x01
 *      THS_READY        0x02
 *      THS_WAIT         0x04
 *      THS_SUSPEND      0x08
 *      THS_WAITSUSPEND  0x0C
 *      THS_DORMANT      0x10
 *
 *  The ReferThreadStatus output structure is 0x30 bytes:
 *
 *      +0x00  status
 *      +0x04  func
 *      +0x08  stack
 *      +0x0C  stack_size
 *      +0x10  gp_reg
 *      +0x14  initial_priority
 *      +0x18  current_priority
 *      +0x1C  attr
 *      +0x20  option
 *      +0x24  waitType
 *      +0x28  waitId
 *      +0x2C  wakeupCount
 *
 *
 *  YIELDING / DELAYS
 *  --------------------------------------------------------------------------
 *
 *  mcDelayThread(1) was tested as a way of allowing other equal-priority
 *  threads to execute.
 *
 *  It successfully prevented a single NativeHooks thread from permanently
 *  monopolizing execution and allowed multiple worker threads to progress.
 *
 *  However, it did NOT provide a stable execution cadence.
 *
 *  During active gameplay a worker could still execute very infrequently,
 *  while the same worker could execute extremely rapidly in menus and during
 *  transitions.
 *
 *  Removing mcDelayThread() did not eliminate the underlying scheduling
 *  behavior, confirming that the delay itself was not the root cause.
 *
 *
 *  CURRENT DESIGN DECISION
 *  --------------------------------------------------------------------------
 *
 *  Persistent NativeHooks worker threads are currently considered unsuitable
 *  for normal per-frame gameplay processing.
 *
 *  Game logic should instead execute from hooks placed at appropriate points
 *  in SOCOM's existing update pipeline.
 *
 *  For example, CAppCamera::Tick has proven to be a reliable location for
 *  local-player processing. Moving PlayerSeal_Tick() there also corrected
 *  update-order problems seen when similar work was performed asynchronously
 *  (HUD ammo state, recoil/crosshair state, etc.).
 *
 *  In general:
 *
 *      game/frame logic       -> execute from an appropriate SOCOM hook
 *      input logic            -> execute from an input hook
 *      rendering/UI logic     -> execute from the corresponding game hook
 *      one-shot initialization -> Bootstrap()
 *
 *  The thread subsystem is being retained because EE threads may still be
 *  useful for isolated/background operations where exact frame synchronization
 *  and execution cadence are not important.
 *
 *  Do NOT assume that creating a persistent worker thread gives NativeHooks an
 *  independent, regularly scheduled game loop.
 *
 *
 *  VERIFIED SOCOM EE SYSCALL WRAPPERS
 *  --------------------------------------------------------------------------
 *
 *      CreateThread          0x0015A6E0
 *      DeleteThread          0x0015A6F0
 *      StartThread           0x0015A700
 *      ExitThread            0x0015A710
 *      ExitDeleteThread      0x0015A720
 *      GetThreadId           0x0015A7D0   (syscall 0x2F)
 *      ReferThreadStatus     0x0015A7E0   (syscall 0x30)
 *
 *  These addresses are specific to the currently researched SOCOM retail
 *  executable and should not be assumed valid for other builds.
 *
 * ============================================================================
 */

static volatile s32 g_TestThreadId = -1;
static volatile s32 g_CurrentThreadId = -1;
static volatile u32 g_ThreadCounter = 0;
static volatile s32 g_ThreadStatusResult[9] = { 0 };
static volatile ee_thread_status_t g_ThreadStatus[9] = { 0 };

//
// ============================================================================
//  ENUMS
// ============================================================================
//

typedef s32 THREAD_PRIORITY;
enum
{
    THREAD_PRIORITY_0  = 0x00,
    THREAD_PRIORITY_1  = 0x01,
    THREAD_PRIORITY_2  = 0x02,
    THREAD_PRIORITY_3  = 0x03,
    THREAD_PRIORITY_4  = 0x04,
    THREAD_PRIORITY_5  = 0x05,
    THREAD_PRIORITY_6  = 0x06,
    THREAD_PRIORITY_7  = 0x07,
    THREAD_PRIORITY_8  = 0x08,

    THREAD_PRIORITY_10 = 0x10,
    THREAD_PRIORITY_20 = 0x20,
    THREAD_PRIORITY_40 = 0x40
};


//
// ============================================================================
//  TYPES
// ============================================================================
//

typedef void (*ThreadFunction_t)(void* arg);

typedef s32  (*sceCreateThread_t)(ee_thread_t* thread);
typedef s32  (*sceDeleteThread_t)(s32 thread_id);
typedef s32  (*sceStartThread_t)(s32 thread_id, void* args);
typedef void (*sceExitThread_t)(void);
typedef void (*sceExitDeleteThread_t)(void);
typedef s32  (*sceTerminateThread_t)(s32 thread_id);
typedef s32  (*sceChangeThreadPriority_t)(s32 thread_id, s32 priority);
typedef s32  (*sceGetThreadId_t)(void);
typedef s32  (*sceSleepThread_t)(void);
typedef s32  (*sceWakeupThread_t)(s32 thread_id);
typedef s32  (*sceSuspendThread_t)(s32 thread_id);
typedef s32  (*sceResumeThread_t)(s32 thread_id);

//
// ============================================================================
//  SETUP
// ============================================================================
//

void Thread_SetCreateThread(sceCreateThread_t function);
void Thread_SetDeleteThread(sceDeleteThread_t function);
void Thread_SetStartThread(sceStartThread_t function);
void Thread_SetExitThread(sceExitThread_t function);
void Thread_SetExitDeleteThread(sceExitDeleteThread_t function);
void Thread_SetTerminateThread(sceTerminateThread_t function);
void Thread_SetChangeThreadPriority(sceChangeThreadPriority_t function);
void Thread_SetGetThreadId(sceGetThreadId_t function);
void Thread_SetSleepThread(sceSleepThread_t function);
void Thread_SetWakeupThread(sceWakeupThread_t function);
void Thread_SetSuspendThread(sceSuspendThread_t function);
void Thread_SetResumeThread(sceResumeThread_t function);

//
// ============================================================================
//  THREAD MANAGEMENT
// ============================================================================
//

s32 Thread_CreateThread(ThreadFunction_t function, u32 gp, THREAD_PRIORITY priority);
s32 Thread_CreateThreadGP(ThreadFunction_t function, THREAD_PRIORITY priority);

s32 Thread_DeleteThread(s32 thread_id);
s32 Thread_StartThread(s32 thread_id, void* args);
s32 Thread_TerminateThread(s32 thread_id);

s32 Thread_ChangePriority(s32 thread_id, s32 priority);
s32 Thread_GetThreadId(void);

s32 Thread_Sleep(void);
s32 Thread_Wakeup(s32 thread_id);

s32 Thread_Suspend(s32 thread_id);
s32 Thread_Resume(s32 thread_id);

void Thread_Exit(void);
void Thread_ExitDelete(void);

#endif // THREAD_H