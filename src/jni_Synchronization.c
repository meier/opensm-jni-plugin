/*
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by Timothy Meier, meier3@llnl.gov, All rights reserved.
 * LLNL-CODE-673346
 *
 * This file is part of the OpenSM Monitoring Service (OMS) package.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (as published by
 * the Free Software Foundation) version 2.1 dated February 1999.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * OUR NOTICE AND TERMS AND CONDITIONS OF THE GNU GENERAL PUBLIC LICENSE
 *
 * Our Preamble Notice
 *
 * A. This notice is required to be provided under our contract with the U.S.
 * Department of Energy (DOE). This work was produced at the Lawrence Livermore
 * National Laboratory under Contract No.  DE-AC52-07NA27344 with the DOE.
 *
 * B. Neither the United States Government nor Lawrence Livermore National
 * Security, LLC nor any of their employees, makes any warranty, express or
 * implied, or assumes any liability or responsibility for the accuracy,
 * completeness, or usefulness of any information, apparatus, product, or
 * process disclosed, or represents that its use would not infringe privately-
 * owned rights.
 *
 * C. Also, reference herein to any specific commercial products, process, or
 * services by trade name, trademark, manufacturer or otherwise does not
 * necessarily constitute or imply its endorsement, recommendation, or favoring
 * by the United States Government or Lawrence Livermore National Security,
 * LLC. The views and opinions of authors expressed herein do not necessarily
 * state or reflect those of the United States Government or Lawrence Livermore
 * National Security, LLC, and shall not be used for advertising or product
 * endorsement purposes.
 *****************************************************************************
 *
 * jni_Synchronization.c
 *
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */
#include <jni.h>

#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include "jni_Synchronization.h"
#include "osmJniPi.h"
#include "jni_OsmNativeInterface.h"

#define OS_NAME_SIZE                  80       /* sizes of names of os specific objects */

#define OS_USECS_PER_TIC   (1000000/CLOCKS_PER_SEC) /* a clock tic is platform dependent */
#define OS_MSECS_PER_TIC   (1000/CLOCKS_PER_SEC)    /* and is 1 ms on most Windows platforms and */
                                                    /* usually 10ms on Solaris */
#define OS_MSECS_PER_MSEC             1             /* a one to one relationship */
#define OS_MSECS_PER_USEC             1000          /* 1K */
#define OS_MSECS_PER_NSEC             1000000          /* 1M */

extern plugin_data_t *       gData; /* the global plugin data */

/*
  JSZ_START_CAPTURE_SIG = 0,
  JSZ_PACKET_FOR_FILTER_SIG,
  JSZ_NEW_DATA_READY,           // not used
  JSZ_MESSAGE_LOCK,             // lock for the message class
  JSZ_STOP_CAPTURE,             // not used
  JSZ_ALERT_FOR_PROCESSING_SIG,
  JSZ_ALERT_EVENT,              // tips event, multi-sig for JVM
  JSZ_ALARM_EVENT,              // tips event, multi-sig for JVM
  JSZ_WATCH_DOG_EVENT,          // future use, timeout only
  JSZ_MSG_EVENT,                // tips event, multi-sig for JVM
  JSZ_COUNTER_LOCK,
  JSZ_TIME_FILTER_LOCK,
  JSZ_PROBE_QUEUE_PARAMS_LOCK,
  JSZ_PROBE_QUEUE_LOCK,
  JSZ_ALERT_ATTRIB_LOCK,
  JSZ_ALERT_LOCK,
  JSZ_ALARM_LOCK,
  JSZ_PROTO_STATS_LOCK,
  JSZ_CAPTURE_STATS_LOCK,
  JSZ_WATCHDOG_STATS_LOCK,
  JSZ_SYNC_UNKNOWN
*/

static char *osm_sync_names[] =
{
  "StartCaptureTask",
  "PacketForFilter",
  "NewDataReady",
  "MessageLock",
  "StopCaptureTask",
  "AlertToProcess",
  "AlertEvent",
  "AlarmEvent",
  "WatchDogEvent",
  "MessageEvent",
  "CounterLock",
  "TimeFilterLock",
  "ProbeQueueParamsLock",
  "ProbeQueueLock",
  "AlertAttributeLock",
  "AlertClassLock",
  "AlarmClassLock",
  "ProtocolStatsLock",
  "CaptureStatsLock",
  "WatchDogStatsLock",
  "SyncUknown",
};

static int osm_sync_timeouts[] =
{
  20000, // StartCaptureTask
//  OS_INFINITE, // StartCaptureTask
  OS_INFINITE, // PacketForFilter
  501, // NewDataReady
  102, // Message
  103, // StopCaptureTask
  OS_INFINITE, // AlertToProcess
  104, // Alert
  105, // Alarm
  5000, // WatchDog (use timeout at its normal watching period 5 secs)
  106, // Message
  40, // Counter SM
  41, // Time Filter SM
  42, // Probe Queue Params SM
  42, // Probe Queue Lock
  43, // Alert Attrib SM
  40, // JSZ_ALERT_LOCK,
  41, // JSZ_ALARM_LOCK,
  42, // JSZ_PROTO_STATS_LOCK,
  43, // JSZ_CAPTURE_STATS_LOCK,
  44, // JSZ_WATCHDOG_STATS_LOCK,
  107, // SyncUnknown
};

static int osm_timeouts_err[] =
{
  200, // StartCaptureTask
  201, // PacketForFilter
  202, // NewDataReady
  203, // Message
  204, // StopCaptureTask
  205, // AlertToProcess
  222, // Alert
  206, // Alarm
  666, // WatchDog
  207, // Message
  210, // Counter SM
  211, // Time Filter SM
  212, // Probe Queue Params SM
  212, // Probe Queue Lock
  213, // Alert Attrib SM
  300, // JSZ_ALERT_LOCK,
  301, // JSZ_ALARM_LOCK,
  302, // JSZ_PROTO_STATS_LOCK,
  303, // JSZ_CAPTURE_STATS_LOCK,
  304, // JSZ_WATCHDOG_STATS_LOCK,
  208, // SyncUknown
};

// the different types of alerts that are listened for by the JVM
static int osm_multi_alert_types[] =
{
  JSZ_ALERT_EVENT, // alert index or id
  JSZ_ALARM_EVENT, // alarm index or id
  JSZ_MSG_EVENT,   // message index or id
};

// an array of pointers to sync objects (events or mutexes)
OS_MUTEX_Handle      *MultiOsmMutexHandles[JSZ_MULTI_EVENT_COUNT];
OS_MUTEX_Attrs       *MultiOsmMutexAtts[JSZ_MULTI_EVENT_COUNT];
OS_MULTI_SYNC_Attrs  OsmMultiSync;

OS_CONDITION_Object  OsmConditionObjs[JSZ_NUM_SYNC_OBJECTS];
OS_CONDITION_Event   OsmConditions[JSZ_NUM_SYNC_OBJECTS];

OS_MUTEX_Attrs       OsmMutexAtts[JSZ_NUM_SYNC_OBJECTS];
OS_MUTEX_ATTR_Object OsmMutexAttsObj[JSZ_NUM_SYNC_OBJECTS];
OS_MUTEX_Object      OsmMutexObjs[JSZ_NUM_SYNC_OBJECTS];
OS_MUTEX_Handle      OsmMutexs[JSZ_NUM_SYNC_OBJECTS];

static int JSZ_MultiSyncNum = -1;



int OS_gettime(struct timespec *ts)
{
  int rtnVal = 0;

  rtnVal = clock_gettime(CLOCK_REALTIME, ts);   // the current time

  return rtnVal;
}
/*-----------------------------------------------------------------------*/


/******************************************************************************
*** Function: OS_addtime
***
*** This function returns a high res time.  Note, Linux does not yet support
*** the clock_gettime() function, so uses gettimeofday() instead.
*** <p>
***
*** created:  8/13/2004 (4:30:12 PM)
***
***   Parameters:  a pointer to a timespec is used to recieve the time.
***
***   Returns:  the 0 if all succeeds
***
******************************************************************************/
int OS_addtime(struct timespec *tb,  int millisecs)
{
  int rtnVal = 0;

  struct timespec ts;
  long msec_overflow;

  // calculate the new time, adding the # of msecs rolling everything up
  msec_overflow = tb->tv_nsec/OS_MSECS_PER_NSEC + millisecs;
  ts.tv_nsec = (tb->tv_nsec%OS_MSECS_PER_NSEC) + (msec_overflow%OS_MSECS_PER_USEC)*OS_MSECS_PER_NSEC;
  ts.tv_sec = tb->tv_sec + msec_overflow/OS_MSECS_PER_USEC;

  // copy over the results
  tb->tv_nsec = ts.tv_nsec;
  tb->tv_sec  = ts.tv_sec;

  return rtnVal;
}
/*-----------------------------------------------------------------------*/



int OS_pendOnMutex( OS_MUTEX_Attrs *pMutexAttr, int timeout_in_millisecs)
{
  OS_MUTEX_Handle MutexHandle = pMutexAttr->hMutex;
  DWORD ErrVal = 0;
  int RtnVal   = OS_FALSE;   /* False, unavailable */

  // not defined in Windows time.h, for some stupid reason
  struct timespec tb;         // time base for the timed lock
  int j;
  j = 0;


  // if the timeout is a negative number, then it has special meaning, INFINITE or DEFAULT
  if(timeout_in_millisecs < 0)
  {
    timeout_in_millisecs = timeout_in_millisecs == OS_INFINITE ? INFINITE: pMutexAttr->msTimeout;
  }


  // the Linux Way (Posix Native Thread Library)

  // if the timeout is INIFINITE, then just lock, otherwise use timedlock or trylock (depends on availablility)
  if(timeout_in_millisecs == INFINITE)
  {
    ErrVal = pthread_mutex_lock(MutexHandle);
  }
  else
  {
    // set up the timespec to reprepresent a timeout period
    OS_gettime(&tb);  // the current time

    // calculate the new time, adding the # of msecs rolling everything up
    OS_addtime(&tb, timeout_in_millisecs);

    ErrVal = pthread_mutex_timedlock(MutexHandle, &tb);
//    ErrVal = pthread_mutex_lock(MutexHandle);
      // this makes performance worse - more spinning.... check its usage
//    ErrVal = pthread_mutex_trylock(MutexHandle);

  }

  if(ErrVal == WAIT_OBJECT_0)
  {
    // SUCCESS!!! return quickly
    RtnVal = OS_TRUE;
  }
  else if(ErrVal == EDEADLK)
  {
    RtnVal = OS_FALSE;
    J_LOG(gData, OSM_LOG_FUNCS, "ERROR: Could not wait on Mutex (%s), already locked\n", pMutexAttr->name);
  }
  else if(ErrVal == EINVAL)
  {
    RtnVal = OS_FALSE;
    J_LOG(gData, OSM_LOG_FUNCS, "Mutex wait argument illegal (%s)\n", pMutexAttr->name);
  }
  else if(ErrVal == ETIMEDOUT)
  {
    RtnVal = OS_FALSE;
    J_LOG(gData, OSM_LOG_FUNCS, "Mutex wait timed out (%s)\n", pMutexAttr->name);
  }
  else if(ErrVal == EAGAIN)
  {
    RtnVal = OS_FALSE;
    J_LOG(gData, OSM_LOG_FUNCS, "Mutex wait failed due to recursion error (%s)\n", pMutexAttr->name);
  }
  else
  {
    J_LOG(gData, OSM_LOG_FUNCS, "Mutex Pend UNKNOWN (%s)(%d) (%d)**!!\n", pMutexAttr->name, (int)ErrVal, EINTR);
    RtnVal = OS_FALSE;
  }

  return RtnVal;
}


/******************************************************************************
*** Function Name:
***   OS_postMutex
***
*** Global Variables Used:
***     None
**/
/**
*** This routine increments the count of the specified mutex object.  If
*** the mutexs count was 0 (unavailable) this routine increments it to 1
*** (available) and releases any threads that may be pending on it.
*** <p>
***
*** <dl><dt><b>Side Effects:</b></dt>
***   <dd>None
*** </dd></dl>
***
***
*** @param    pMutexAttr  a reference to the data structure that describes
***                           the mutex.
***
*** @return   An error code. Should be zero if Mutex was posted.
******************************************************************************/

int OS_postMutex( OS_MUTEX_Attrs *pMutexAttr)
{
  OS_MUTEX_Handle MutexHandle = pMutexAttr->hMutex;
  int ErrVal = 0;

  if(!MutexHandle)
  {
    J_LOG(gData, OSM_LOG_FUNCS, "Invalid Mutex Handle!!!\n");
  }
  else
  {
  // the Linux Way (Posix Native Thread Library)

    ErrVal = pthread_mutex_unlock(MutexHandle);
    if(ErrVal)
    {
      if(ErrVal == EINVAL)
      {
        // the value specified in the argument is illegal
        J_LOG(gData, OSM_LOG_ERROR, "Could not POST Mutex (%s) (%d)\n", pMutexAttr->name, ErrVal);
      }
      else if(ErrVal == EPERM)
      {
        // if here, the mutex is not currentlly held by the caller
        J_LOG(gData, OSM_LOG_ERROR, "*** ERROR POSTing Mutex (%s) (err %d)\n", pMutexAttr->name, ErrVal);
      }
      else
      {
        // Unknown...
        J_LOG(gData, OSM_LOG_ERROR, "*** ERROR POSTing Mutex (%s) (err %d)\n", pMutexAttr->name, ErrVal);
        //        DBG_printErrorString(DBG_ALL, ErrVal);
      }
    }
    else
    {
      // success, return quickly
      //DBG_printf( DBG_ALL, "Releasing or Posting Mutex - UNLOCKED (%s)\n", pMutexAttr->name);
    }

  }
  return ErrVal;
}




/******************************************************************************
*** Function Name:
***   OS_createMutex
***
*** Global Variables Used:
***     None
**/
/**
*** This function creates a "Named" Mutex using the supplied attributes.
*** <p>
***
*** <dl><dt><b>Side Effects:</b></dt>
***   <dd>GetLastError() returns additional error information.
*** </dd></dl>
***
***
*** @param    mutex_attrs  A data structure that contains the Mutexs
***                            desired attributtes.
***
*** @return   A Handle to the mutex is returned if successfully created or if
***           it already existed, otherwise NULL is returned.
***           GetLastError() contains additional info.
******************************************************************************/

OS_MUTEX_Handle OS_createMutex(OS_MUTEX_Attrs* mutex_attrs )
{
  /*
  Create a NAMED mutex with specific attributes.

  Attempt to create it first (may already exist).
  If it does not exist, then return its handle after POSTing or releasing it.  If it
  already existed, then just return the handle without changing its availability.
  */
  char MutexName[OS_NAME_SIZE];
  OS_MUTEX_Handle MutexHandle;
  OS_MUTEX_ATTR_Handle MutexAttrHandle;

  DWORD err_code = 0;

  strncpy(MutexName, (char *)mutex_attrs->name, OS_NAME_SIZE);

  J_LOG(gData, OSM_LOG_FUNCS, "Attempting to create the Mutex with name (%s)\n", MutexName);

  MutexAttrHandle = mutex_attrs->hMutexAttr;
  MutexHandle     = mutex_attrs->hMutex;


  // first check to see if the Mutex already exists, and if so, skip
  if(mutex_attrs->initialized != OS_TRUE)
  {
    pthread_mutexattr_init(MutexAttrHandle);
    err_code = pthread_mutex_init(MutexHandle, MutexAttrHandle);

    switch(err_code)
    {
    case 0:
      J_LOG(gData, OSM_LOG_FUNCS, "Successfully created the Mutex!\n");
      mutex_attrs->initialized = OS_TRUE;
      break;

    case EINVAL:
      J_LOG(gData, OSM_LOG_FUNCS, "Mutex creation argument invalid!\n");
      break;

    case ENOMEM:
      J_LOG(gData, OSM_LOG_FUNCS, "No resources for creating the Mutex!\n");
      break;

    default:
      J_LOG(gData, OSM_LOG_ERROR, "UNKNOWN Error while creating the Mutex! (%d)\n", (int)err_code);
      break;
    }
  }
  else
  {
    J_LOG(gData, OSM_LOG_FUNCS, "Mutex already exists!\n");
  }
  J_LOG(gData, OSM_LOG_FUNCS, "Initializing Mutex now!\n");
  OS_postMutex( mutex_attrs );
  return MutexHandle;
}

/******************************************************************************
*** Function Name:
***   OS_deleteMutex
***
*** Global Variables Used:
***     None
**/
/**
*** Eliminates your reference to a mutex.  If no other Thread has a reference
*** then the mutex is closed and deleted.
*** <p>
***
*** <dl><dt><b>Side Effects:</b></dt>
***   <dd>None
*** </dd></dl>
***
***
*** @param    mutex_attrs a reference to the data structure that describes
***                           the mutex.
***
*** @return   probably should return an error code,... nah
******************************************************************************/

void OS_deleteMutex( OS_MUTEX_Attrs* mutex_attrs )
{
  int errVal = 0;  // 0 means no error
  OS_MUTEX_Handle      MutexHandle     = mutex_attrs->hMutex;
  OS_MUTEX_ATTR_Handle MutexAttrHandle = mutex_attrs->hMutexAttr;

  pthread_mutexattr_destroy(MutexAttrHandle);
  errVal = pthread_mutex_destroy(MutexHandle);

  if(errVal)
    J_LOG(gData, OSM_LOG_FUNCS, "Could not Close the mutex (%s)!\n", mutex_attrs->name);
  else
    J_LOG(gData, OSM_LOG_FUNCS, "Deleting Mutex (%s)\n", mutex_attrs->name);
  return;
}

/******************************************************************************
*** Function: OS_createCondition
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/16/2003 (3:55:25 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int OS_createCondition(OS_CONDITION_Event* pCondition)
{
  int returnVal = 0;
  OS_CONDITION_Handle pC   = pCondition->hCondition;

  if(pCondition->initialized)
  {
    J_LOG(gData, OSM_LOG_FUNCS, "Mutex Condition already initialized\n");
  }
  else
  {
    pCondition->initialized  = OS_TRUE;
    pCondition->bConditionMet = OS_FALSE;

    J_LOG(gData, OSM_LOG_FUNCS, "Mutex Condition is being Initialized\n");
    returnVal = pthread_cond_init(pC, NULL);
  }

  return returnVal;
}
/*-----------------------------------------------------------------------*/


/******************************************************************************
*** Function: OS_deleteCondition
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/16/2003 (4:32:32 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int OS_deleteCondition(OS_CONDITION_Event* pCondition)
{
  int returnVal = 0;
  OS_CONDITION_Handle pC   = pCondition->hCondition;
  pCondition->initialized  = OS_FALSE;
  pCondition->bConditionMet = OS_FALSE;

  J_LOG(gData, OSM_LOG_FUNCS, "Destroying Mutex Condition\n");
  returnVal = pthread_cond_destroy(pC);

  return 1;
}
/*-----------------------------------------------------------------------*/
/******************************************************************************
*** Function: OS_signalCondition
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/16/2003 (4:32:49 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int OS_signalCondition(OS_CONDITION_Event* pCondition)
{
  int returnVal = 0;
  OS_CONDITION_Handle  pC   = pCondition->hCondition;
  OS_MUTEX_Attrs*      pMa  = pCondition->hMutexAtts;
//  OS_MUTEX_Handle      pM   = pMa->hMutex;
//  OS_MUTEX_ATTR_Handle pA   = pMa->hMutexAttr;

  returnVal = OS_pendOnMutex( pMa, 2); // time out quickly
//  returnVal = pthread_mutex_lock(pM);

  // check the condition flag.  It should be FALSE if we aren't signaling too fast
  if( pCondition->bConditionMet == TRUE)
  {
    // should always be FALSE, (if previous signal was caught)
    // so this represents a condition when signals are generated faster than
    // they can be consumed.
    //
    // Queues are built into the system to handle the speed difference between
    // the major worker threads.  Report this condition as a normal occurrence

//    J_LOG(gData, OSM_LOG_ERROR, "Unhandled Mutex Condition (%s), send again?\n", pCondition->hMutexAtts->name);
  }

  // The condition is signaled, set the flag and wake up any waiting threads
  pCondition->bConditionMet = TRUE;

//  J_LOG(gData, OSM_LOG_FUNCS, "Wake up waiting thread... (%s)\n", pCondition->hMutexAtts->name);
  returnVal = pthread_cond_signal(pC);
//  returnVal = pthread_cond_broadcast(pC);

  returnVal = OS_postMutex(pMa);
//  returnVal = pthread_mutex_unlock(pM);
  return returnVal;
}
/*-----------------------------------------------------------------------*/



/******************************************************************************
*** Function: OS_timedwaitForCondition
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/16/2003 (4:33:01 PM)
***
***   Parameters:
***
***   Returns:  TRUE if wait succeeded, and FALSE otherwise
***
******************************************************************************/
int OS_timedwaitForCondition(OS_CONDITION_Event* pCondition, int timeout_in_millisecs)
{
  int returnVal = 0;     // returns
  int waitVal = 0;     // returns
  OS_CONDITION_Handle  pC   = pCondition->hCondition;
  OS_MUTEX_Attrs*      pMa  = pCondition->hMutexAtts;
  OS_MUTEX_Handle      pM   = pMa->hMutex;
//  OS_MUTEX_ATTR_Handle pA   = pMa->hMutexAttr;

  int timedOut = FALSE;   // true if the wait returned a timed out value

  struct timespec tb;         // time base for the timed condition

  OS_gettime(&tb);  // the current time
//  clock_gettime(CLOCK_REALTIME, &tb);   // the current time

  // calculate the new time, adding the # of msecs rolling everything up
  OS_addtime(&tb, timeout_in_millisecs);

//  J_LOG(gData, OSM_LOG_FUNCS, "Mutex Condition is waiting (%s)\n", pCondition->hMutexAtts->name);
  OS_pendOnMutex( pMa, 2); // time out quickly
//  returnVal = pthread_mutex_lock(pM);

  while (!pCondition->bConditionMet && !timedOut)
  {
    // waiting for condition
//    returnVal = pthread_cond_wait(pC, pM);
    waitVal = pthread_cond_timedwait(pC, pM, &tb);

    J_LOG(gData, OSM_LOG_FUNCS,"Condition TimedWait recieved, (%s) (%d) [%d]\n", pCondition->hMutexAtts->name, pCondition->bConditionMet, waitVal);
    if(waitVal == ETIMEDOUT)
    {
      timedOut = pCondition->bConditionMet ? FALSE : TRUE; // only considered timed out if condition is still not met
    }
  }
  // reset everything for the next time
  pCondition->bConditionMet = FALSE;
  OS_postMutex(pMa);

//  returnVal = pthread_mutex_unlock(pM);

  // assume that if it gets here, everything is hunky dorey
  // three possible return values
  //
  // 0         - everything worked!
  // EINVAL    - condition does not refer to an initialized variable
  // EFAULT    - invalid pointer
  // ETIMEDOUT - absolute time has passed
  //
  // return 0 (or positive #) for good or OS_WAIT_TIMEOUT for a timeout, negative # is an error

  returnVal = waitVal == 0 ? 0: -1;
  returnVal = timedOut ? OS_WAIT_TIMEOUT: returnVal;

  return returnVal;
}
/*-----------------------------------------------------------------------*/
/******************************************************************************
*** Function: OS_waitForCondition
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/16/2003 (4:33:01 PM)
***
***   Parameters:
***
***   Returns:  TRUE if wait succeeded, and FALSE otherwise
***
******************************************************************************/
int OS_waitForCondition(OS_CONDITION_Event* pCondition, int timeout_in_millisecs)
{
  int returnVal = 0;     // returns
  OS_CONDITION_Handle  pC   = pCondition->hCondition;
  OS_MUTEX_Attrs*      pMa  = pCondition->hMutexAtts;
  OS_MUTEX_Handle      pM   = pMa->hMutex;
//  OS_MUTEX_ATTR_Handle pA   = pMa->hMutexAttr;

  // if the timeout is a negative number, then it has special meaning, INFINITE or DEFAULT
  if(timeout_in_millisecs < 0)
  {
    timeout_in_millisecs = timeout_in_millisecs == OS_INFINITE ? INFINITE: pMa->msTimeout;
  }

  // if the timeout is INIFINITE, then just lock, otherwise use timedlock or trylock (depends on availablility)
  if(timeout_in_millisecs == INFINITE)
  {
    OS_pendOnMutex( pMa, 2); // time out quickly

    while (!pCondition->bConditionMet)
    {
      // wait indefinately for condition
//      J_LOG(gData, OSM_LOG_FUNCS,"Condition Wait recieved, (%s)\n", pCondition->hMutexAtts->name);

      returnVal = pthread_cond_wait(pC, pM);
      returnVal = TRUE;
    }
    // reset everything for the next time
    pCondition->bConditionMet = FALSE;
    OS_postMutex(pMa);
  }
  else
  {
    // wait with a timeout
    returnVal = OS_timedwaitForCondition(pCondition, timeout_in_millisecs);
    returnVal = !returnVal ? TRUE: FALSE;
  }
  return returnVal;
}
/*-----------------------------------------------------------------------*/


/******************************************************************************
*** Function: JSZ_createMultiSyncObject
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  7/21/2003 (9:05:35 AM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/

// map the desired events or mutexes into the multi object
int JSZ_createMultiSyncObject(void)
{
  int success = 1;           // okay by default
  int i;

    // re-use existing mutexes
    for(i = 0; i < JSZ_MULTI_EVENT_COUNT; i++)
    {
      J_LOG(gData, OSM_LOG_FUNCS, "****** MSMO: loop # %d *******\n", i);
      MultiOsmMutexAtts[i]    = &(OsmMutexAtts[osm_multi_alert_types[i]]);
      MultiOsmMutexHandles[i] = &(MultiOsmMutexAtts[i]->hMutex);
    }

    OsmMultiSync.name       = "MultipleOsmMutex";
    OsmMultiSync.nCount     = JSZ_MULTI_EVENT_COUNT;

    OsmMultiSync.lpAttribute = MultiOsmMutexAtts;
    OsmMultiSync.lpHandles   = MultiOsmMutexHandles;

    OsmMultiSync.bWaitAll    = (BOOLEAN) 0;  // trigger on any event (don't need ALL)
    OsmMultiSync.msTimeout   = 200;          // 200ms by default
  return success;
}
/*-----------------------------------------------------------------------*/


int JSZ_createMasterSyncObjects(int syncType)
{
  int i;
  int success = 1;            // okay, by default

     /* Open the EXISTING mutexes */
    for(i = 0; i < JSZ_NUM_SYNC_OBJECTS; i++)
    {
      OsmMutexAtts[i].name         = (LPCTSTR)osm_sync_names[i];
      OsmMutexAtts[i].idNum        = i;
      OsmMutexAtts[i].msTimeout    = osm_sync_timeouts[i];
      OsmMutexAtts[i].security     = NULL;   /* Not used */
      OsmMutexAtts[i].hMutex       = &(OsmMutexObjs[i]);  // initialize with defaults
      OsmMutexAtts[i].hMutexAttr   = &(OsmMutexAttsObj[i]);  // initialize with defaults

      OsmMutexs[i] = OS_createMutex( &(OsmMutexAtts[i]) );
      if ( OsmMutexs[i] == 0 )
      {
        J_LOG(gData, OSM_LOG_ERROR, "****** Could not open the %s Mutex\n", osm_sync_names[i]);
        success = 0;            // all or nothing!
      }

      // create the Conditions that are linked with the Mutexes
      OsmConditions[i].hMutexAtts   = &(OsmMutexAtts[i]);
      OsmConditions[i].hCondition   = &(OsmConditionObjs[i]);
      OS_createCondition(&(OsmConditions[i]));
    }
  /* create the multi-event object, hard code this */
  JSZ_createMultiSyncObject();

  return success;
}

/*-----------------------------------------------------------------------*/


/******************************************************************************
*** Function: JSZ_releaseMasterSyncObjects
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  7/21/2003 (9:05:55 AM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/

int JSZ_releaseMasterSyncObjects(void)
{
  int i;

    for(i = 0; i < JSZ_NUM_SYNC_OBJECTS; i++)
    {
      OS_deleteMutex(& (OsmMutexAtts[i]));
      OS_deleteCondition(&(OsmConditions[i]));
    }
  return 1;
}

/******************************************************************************
*** Function: JSZ_lock
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  7/21/2003 (9:06:23 AM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/

int JSZ_lock(int ObjectNum)
{
  int rtn_val = 0;
  if((ObjectNum >= 0) && (ObjectNum < JSZ_NUM_SYNC_OBJECTS))
  {
    rtn_val = OS_pendOnMutex(& (OsmMutexAtts[ObjectNum]), OS_DEFAULT);
    if ( rtn_val == OS_FALSE )
    {
      rtn_val = osm_timeouts_err[ObjectNum];
    }
    else
    {
      rtn_val = JSZ_WAIT_OK;
    }
  }
  else
  {
    J_LOG(gData, OSM_LOG_FUNCS, "Invalid Sync Object Number: object(%d)\n", ObjectNum);
  }
  return rtn_val;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
*** Function: JSZ_unlock
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  7/21/2003 (9:06:47 AM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/

int JSZ_unlock(int ObjectNum)
{
  int rtn_val = 0;
  if((ObjectNum >= 0) && (ObjectNum < JSZ_NUM_SYNC_OBJECTS))
  {
    OS_postMutex(& (OsmMutexAtts[ObjectNum]));
  }
  else
  {
    J_LOG(gData, OSM_LOG_FUNCS, "Invalid Sync Object Number:  object(%d)\n", ObjectNum);
  }
return rtn_val;
}
/*-----------------------------------------------------------------------*/
/******************************************************************************
*** Function: JSZ_isMultiSignal
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/17/2003 (4:35:52 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int JSZ_isMultiSignal(int SyncNum)
{
  int isMultiSignal = FALSE;
  int i;

  // loop through the multisync objects and test to see if this is one of them
  for(i = 0; i < JSZ_MULTI_EVENT_COUNT; i++)
  {
    if(SyncNum == osm_multi_alert_types[i])
    {
      i = JSZ_MULTI_EVENT_COUNT;  // break out now
      isMultiSignal = TRUE;
    }
  }
  return isMultiSignal;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
*** Function: JSZ_saveMultiSignal
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/17/2003 (4:37:07 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int JSZ_saveMultiSignal(int SyncNum)
{
  return (JSZ_MultiSyncNum = SyncNum);
}
/*-----------------------------------------------------------------------*/


/******************************************************************************
*** Function: JSZ_getMultiSignal
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/17/2003 (4:37:36 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int JSZ_getMultiSignal(void)
{
  return JSZ_MultiSyncNum;
}
/*-----------------------------------------------------------------------*/
/******************************************************************************
*** Function: JSZ_sendSignal
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
*** created:  12/16/2003 (11:20:09 AM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int JSZ_sendSignal(int ObjectNum)
{
  int rtn_val = 0;

    // use conditions with Mutexes
    if((ObjectNum >= 0) && (ObjectNum < JSZ_NUM_SYNC_OBJECTS))
    {
      // if a multisignal, then send TSZ_NEW_DATA_READY, otherwise send requested value
      if( JSZ_isMultiSignal(ObjectNum) )
      {
        JSZ_saveMultiSignal(ObjectNum);   // save so we can decode the new sync later

        // replace with new sync type that represents the multi syncs
        ObjectNum = JSZ_NEW_DATA_READY;
      }
      OS_signalCondition(&(OsmConditions[ObjectNum]));
//      OS_postMutex(& (TipsMutexAtts[ObjectNum]));
    }
    else
    {
      J_LOG(gData, OSM_LOG_FUNCS, "Invalid Sync Object Number: object(%d)\n", ObjectNum);
    }
  return rtn_val;
}
/*-----------------------------------------------------------------------*/
/******************************************************************************
*** Function: JSZ_waitForOsmEvent
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/

int JSZ_waitForOsmEvent(int timeout_in_millisecs)
{
  // waits for any one of the events that JAVA is interested in
  // and returns a value associated with the Event, and then resets
  // the Event or Mutex
  //
  // negative return values indicate errors!
  //
  int rtn_val = -1;

    // The NEW_DATA_READY signal is used as a multi-signal, and then decoded it,
    // This should only be activated in the Linux Version!
    //
      rtn_val = OS_waitForCondition(&(OsmConditions[JSZ_NEW_DATA_READY]), timeout_in_millisecs);
//      rtn_val = JSZ_waitForSignal( JSZ_NEW_DATA_READY );
      if ( rtn_val < 0 )
      {
        // no need to report timeout errors
        if(rtn_val != OS_WAIT_TIMEOUT)
          J_LOG(gData, OSM_LOG_FUNCS, "Multiple Wait Error: val(%d)\n", rtn_val);
      }
      else
      {
        // JSZ_NEW_DATA_READY is a special multi-sync signal, that saves the
        // actual type in the GLOBAL mulitsignal variable, so we must get it and
        // return it
        rtn_val = JSZ_getMultiSignal();
      }
  return rtn_val;
}

