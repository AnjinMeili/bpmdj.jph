/****
 Active Objects v4.3
 Copyright (C) 2006-2012 Werner Van Belle
 http://active.yellowcouch.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
****/
#ifndef __loaded__lock_h__
#define __loaded__lock_h__
using namespace std;
#line 1 "lock.h++"
#include <string>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <queue>
#include <time.h>
#include "cmpxchg.h"
using namespace std;

/**
 * @brief Atomic non-recursive waiting or non-waiting lock.
 * 
 * The lock class implements an atomic lock through a cmpxchg.
 * It does not provide a counting lock. The main reason for this is that
 * it forces the programmer to think about his locking system.
 * If he needs to avoid locking the same object multiple times then 
 * the performance of the program in general improves.
 * The lock will remember who currently obtained the lock through a string
 * that must be passed when locking
 * @see AutoLock for a handy one-liner.
 */
class Lock
{
public:
  /**
   * can be used as a boolean. NULL is unlocked, another value otherwise.
   */
  volatile void * locked;
  string who;
  bool deleted;
  Lock() : locked(NULL)
  {
    deleted=false;
  };

  /**
   * Will try to lock the object. Returns true under success, false otherwise.
   */
  bool try_lock(string w);
  /**
   * Will lock the object, with a complete disregard who asks the lock. 
   * If the lock is asked by the same thread that has the lock then it will still
   * be a deadlock. I believe this annoyance forces the programmer to think and
   * improve his program. If wait is true then the call will try for
   * 60s to acquire the lock. If that fails, it will assert itself.
   * if wait is false, the lock return true or false depending on a successful
   * acquisition.
   */
  void wait_lock(string w);
  /**
   * Straightforwardly unlocks the lock.
   */
  void unlock();
  virtual ~Lock()
  {
    deleted=true;
  }
};

/**
 * @brief Helper class to lock an object within a certain scope.
 * 
 * The Synchronized(Lock) macro will allocate an AutoLock in the current scope.
 * As soon as the control flow goes out of scope, the lock will automatically 
 * be released. E.g
 * Synchronized(active-object) will lock the object
 * Synchronized(this) will lock this object for the duration of the scope.
 */
class AutoLock
{
  Lock& lock;
public:
  /**
   * waits until lock is locked
   */
  AutoLock(Lock&lock, string l): lock(lock)
  {
    lock.wait_lock(l);
  }
  /**
   * waits until lock is locked
   */
  AutoLock(Lock*lock, string l): lock(*lock)
  {
    lock->wait_lock(l);
  };
  /**
   * unlocks the underlying lock
   */
  ~AutoLock()
  {
    lock.unlock();
  }
};

#define DamnPreprocessor(a) #a
#define Synchronized_(a,b,c) AutoLock tmp_auto_locker(a,#a "_at_" b DamnPreprocessor(c))
#define Synchronized(a) Synchronized_(a,__FILE__, __LINE__)

#endif // __loaded__lock_h__
