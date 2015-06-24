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
 * jni_SharedResources.c
 *
 *  jni_Synchronization.h
 *      Author: meier3
 */

#ifndef JNI_SYNCHRONIZATION_H_
#define JNI_SYNCHRONIZATION_H_

#define _MULTI_THREADED  // must be before pthread.h
//  #define _GNU_SOURCE      // see features.h (defined as gcc preprocessor option -D_GNU_SOURCE

#include <netinet/in.h>
#include <string.h>
#include <asm/errno.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif

#define JSZ_WAIT_OK                   55
#define OSM_OK                        60

#define JSZ_NUM_SYNC_OBJECTS  (JSZ_SYNC_UNKNOWN)  // the total number of envent types
#define JSZ_MULTI_EVENT_COUNT (3)                 // the number of events in the multi-event


  enum MASTER_SYNC_TYPES
  {
    JSZ_START_CAPTURE_SIG = 0,
    JSZ_PACKET_FOR_FILTER_SIG,
    JSZ_NEW_DATA_READY,
    JSZ_MESSAGE_LOCK,         // lock for the message class
    JSZ_STOP_CAPTURE,
    JSZ_ALERT_FOR_PROCESSING_SIG,  // an alert signal for the alert processing loop
    JSZ_ALERT_EVENT,   // an alert event for the JVM
    JSZ_ALARM_EVENT,   // an alarm event for the JVM
    JSZ_WATCH_DOG_EVENT,
    JSZ_MSG_EVENT,     // a message event for the JVM
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
  };


  enum OS_WAIT_ERRS
  {
    OS_WAIT_FAIL       = -10,
    OS_WAIT_TIMEOUT    = -8,
    OS_WAIT_ABANDONED  = -6,
    OS_WAIT_UNKNOWN    = -4
  };

  enum OS_TIME_OUT
  {
    OS_DEFAULT    = -2,
    OS_INFINITE   = -1,
    OS_IMMEDIATE  = 0
  };

   enum OS_STREAM_TYPES
   {
     OS_NULLSTREAM = -1,
     OS_STDIN      = 0,
     OS_STDOUT     = 1,
     OS_STDERR     = 2
   };

   // define types for Linux (Posix Native Thread Library)

   #define OS_TRUE          1
   #define OS_FALSE         0
   #define INFINITE      0xFFFFFFFF
   #define WAIT_OBJECT_0 0

   typedef void* OS_TSK_RTN_TYPE;
   typedef OS_TSK_RTN_TYPE (*OS_TSK)(void *);

   #define OS_TSK_RTN OS_TSK_RTN_TYPE

   typedef void* HANDLE;
   typedef void* LPSECURITY_ATTRIBUTES;
   typedef char* LPCTSTR;
   typedef char BOOLEAN;
   typedef unsigned long DWORD;

   typedef pthread_cond_t OS_CONDITION_Object;
   typedef pthread_cond_t* OS_CONDITION_Handle;
   typedef pthread_mutex_t OS_MUTEX_Object;
   typedef pthread_mutex_t* OS_MUTEX_Handle;
   typedef pthread_mutexattr_t OS_MUTEX_ATTR_Object;
   typedef pthread_mutexattr_t* OS_MUTEX_ATTR_Handle;
   typedef HANDLE OS_EVENT_Handle;
   typedef HANDLE OS_TSK_Handle;
   typedef pthread_attr_t OS_TSK_ATTR_Object;
   typedef pthread_attr_t* OS_TSK_ATTR_Handle;

   typedef struct OS_Mutex_Attributes
   {
     OS_MUTEX_Handle         hMutex;
     OS_MUTEX_ATTR_Handle    hMutexAttr;
     LPSECURITY_ATTRIBUTES   security;
       BOOLEAN                 owned;
       BOOLEAN                 initialized;
     LPCTSTR                 name;
     int                     msTimeout;
     int                     idNum;
   } OS_MUTEX_Attrs;

   typedef struct OS_Multi_Mutex_Attributes
   {
     DWORD            nCount;          // number of handles & attribs in array
     OS_MUTEX_Handle  **lpHandles;    // object-handle array
     BOOLEAN             bWaitAll;        // wait option
     OS_MUTEX_Attrs   **lpAttribute;  // object-attribute array
     LPCTSTR          name;            // the name of the multi-event
     int              msTimeout;
   } OS_MULTI_MUTEX_Attrs;

   typedef struct OS_Event_Attributes
   {
     OS_EVENT_Handle         hEvent;
     LPSECURITY_ATTRIBUTES   security;
     BOOLEAN                 manualReset;
     BOOLEAN                 initialState;
       BOOLEAN                 initialized;
     LPCTSTR                 name;
     int                     msTimeout;
     int                     idNum;
   } OS_EVENT_Attrs;

   typedef struct OS_Multi_Event_Attributes
   {
     DWORD            nCount;          // number of handles & attribs in array
     OS_EVENT_Handle  **lpHandles;    // object-handle array
     BOOLEAN             bWaitAll;        // wait option
     OS_EVENT_Attrs   **lpAttribute;  // object-attribute array
     LPCTSTR          name;            // the name of the multi-event
     int              msTimeout;
   } OS_MULTI_EVENT_Attrs;

   typedef struct OS_Task_Attributes
   {
     int stack;
     int stacksize;
     int stackseg;
     int environ1;
     char* name;
     int exitflag;
     unsigned long TaskID;
     OS_TSK_Handle  hTask;
     OS_TSK_ATTR_Handle  hTaskAttr;
   } OS_TSK_Attrs;

   typedef struct OS_Condition_Event
   {
     BOOLEAN                 initialized;
     BOOLEAN                 bConditionMet;
     OS_MUTEX_Attrs*         hMutexAtts;
     OS_CONDITION_Handle     hCondition;
   } OS_CONDITION_Event;


 /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
 /*~~~           External Variable Declarations                        !!!*/
 /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


 /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
 /*~~~           Function Declarations                                 !!!*/
 /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

   // define types for Linux (Posix Native Thread Library)
#define OS_MULTI_SYNC_Attrs OS_MULTI_MUTEX_Attrs

int JSZ_createMasterSyncObjects(int syncType);
int JSZ_releaseMasterSyncObjects(void);
int JSZ_waitForOsmEvent(int timeout_in_millisecs);
int JSZ_lock(int ObjectNum);
int JSZ_unlock(int ObjectNum);
int JSZ_sendSignal(int ObjectNum);
int OS_pendOnMutex( OS_MUTEX_Attrs *pMutexAttr, int timeout_in_millisecs);
int OS_postMutex( OS_MUTEX_Attrs *pMutexAttr);
OS_MUTEX_Handle OS_createMutex(OS_MUTEX_Attrs* mutex_attrs );
void OS_deleteMutex( OS_MUTEX_Attrs* mutex_attrs );


#ifdef __cplusplus
}
#endif
#endif /* JNI_SYNCHRONIZATION_H_ */
