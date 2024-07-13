/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _RTOS_H_
#define _RTOS_H_

#include "definitions.h"

#ifdef __cplusplus
extern "C++" {
#endif

#if defined(RTOS_COOS)
#ifdef __cplusplus
  extern "C" {
#endif
    #include <CoOS.h>
#ifdef __cplusplus
  }
#endif

  #define RTOS_MS_PER_TICK              (1000 / CFG_SYSTICK_FREQ)  // RTOS timer tick length in ms (currently 1 for STM32, 2 for others)

  typedef OS_TID RTOS_TASK_HANDLE;
  typedef OS_MutexID RTOS_MUTEX_HANDLE;
  typedef OS_FlagID RTOS_FLAG_HANDLE;
  typedef OS_EventID RTOS_EVENT_HANDLE;

  static inline void RTOS_INIT()
  {
    CoInitOS();
  }

  static inline void RTOS_START()
  {
    CoStartOS();
  }

  static inline void RTOS_WAIT_MS(uint32_t x)
  {
    if (!x)
      return;
    if ((x = x / RTOS_MS_PER_TICK) < 1)
      x = 1;
    CoTickDelay(x);
  }

  static inline void RTOS_WAIT_TICKS(uint32_t x)
  {
    CoTickDelay(x);
  }

  #define RTOS_CREATE_TASK(taskId, task, name, stackStruct, stackSize, priority)   \
                                        taskId = CoCreateTask(task, NULL, priority, &stackStruct.stack[stackSize-1], stackSize)

#ifdef __cplusplus
  static inline void RTOS_CREATE_MUTEX(OS_MutexID &mutex)
  {
    mutex = CoCreateMutex();
  }

  static inline void RTOS_LOCK_MUTEX(OS_MutexID &mutex)
  {
    CoEnterMutexSection(mutex);
  }

  static inline void RTOS_UNLOCK_MUTEX(OS_MutexID &mutex)
  {
    CoLeaveMutexSection(mutex);
  }
#endif  // __cplusplus

  static inline uint32_t getStackAvailable(void * address, uint32_t size)
  {
    uint32_t * array = (uint32_t *)address;
    uint32_t i = 0;
    while (i < size && array[i] == 0x55555555) {
      i++;
    }
    return i;
  }

  extern int _estack;
  extern int _main_stack_start;
  static inline uint32_t stackSize()
  {
    return ((unsigned char *)&_estack - (unsigned char *)&_main_stack_start) / 4;
  }

  static inline uint32_t stackAvailable()
  {
    return getStackAvailable(&_main_stack_start, stackSize());
  }

  #define RTOS_CREATE_FLAG(flag)        flag = CoCreateFlag(false, false)
  #define RTOS_SET_FLAG(flag)           (void)CoSetFlag(flag)
  #define RTOS_CLEAR_FLAG(flag)         (void)CoClearFlag(flag)
  #define RTOS_WAIT_FLAG(flag,timeout)  (CoWaitForSingleFlag(flag,timeout) == E_TIMEOUT)

  static inline void RTOS_ISR_SET_FLAG(RTOS_FLAG_HANDLE flag)
  {
    CoEnterISR();
    CoSchedLock();
    isr_SetFlag(flag);
    CoSchedUnlock();
    CoExitISR();
  }
  
#ifdef __cplusplus
  template<int SIZE>
  class TaskStack
  {
    public:
      TaskStack()
      {
      }

      void paint()
      {
        for (uint32_t i=0; i<SIZE; i++) {
          stack[i] = 0x55555555;
        }
      }

      uint32_t size()
      {
        return SIZE * 4;
      }

      uint32_t available()
      {
        return getStackAvailable(stack, SIZE);
      }

      OS_STK stack[SIZE];
  };
#endif // __cplusplus

  static inline uint32_t RTOS_GET_TIME(void)
  {
    return (uint32_t)CoGetOSTime();
  }

  static inline uint32_t RTOS_GET_MS(void)
  {
    return (RTOS_GET_TIME() * RTOS_MS_PER_TICK);
  }

  #define RTOS_DEFINE_STACK(name, size) TaskStack<size> __ALIGNED(8) name // stack must be aligned to 8 bytes otherwise printf for %f does not work!

  #define TASK_FUNCTION(task)           void task(void * pdata)
  #define TASK_RETURN()                 return

#else // no RTOS
  static inline void RTOS_START()
  {
  }

  static inline void RTOS_WAIT_MS(unsigned x)
  {
  }

  static inline void RTOS_WAIT_TICKS(unsigned x)
  {
  }
#endif  // RTOS type

#ifdef __cplusplus
}
#endif

#endif // _RTOS_H_
