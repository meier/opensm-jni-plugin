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
 *
 * fifo.c
 *
 *  Created on: Aug 2, 2011
 *      Author: meier3
 */

#include "fifo.h"

#include <complib/cl_types.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>

#include "osmJniPi.h"

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           Module Defines                                            !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           Module Type Definitions                                   !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static unsigned int FQ_ID = 0;             // the value assigned to queues

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           External Variable Definitions                             !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

extern plugin_data_t * gData; /* the global plugin data */

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           Module Variable Definitions                               !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           Local Function Declarations                               !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           Function Definitions                                      !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/******************************************************************************
 *** Function: FQ_init
 ***
 *** Creates and initializes a FIFO queue by setting its maximum size and doing
 *** some other bookkeeping.
 *** <p>
 ***
 *** created:  8/1/2011 (9:34:02 AM)
 ***
 ***   Parameters:  the size of the queue (currently not implemented)
 ***
 ***   Returns:  a pointer to the newly constructed queue or NULL if a problem was
 ***             encountered
 ***
 ******************************************************************************/
FIFO_QUEUE*
FQ_init(long sizeOfData, long sizeOfQueue)
{
  long n;
  char *Q;

  // allocate memory for pointers
  FIFO_QUEUE *fq = (FIFO_QUEUE *) malloc(sizeof(FIFO_QUEUE));

  sizeOfQueue =
      ((sizeOfQueue > FQ_MIN_QUEUE_SIZE) && (sizeOfQueue <= FQ_MAX_QUEUE_SIZE)) ? sizeOfQueue : FQ_MAX_QUEUE_SIZE;

  // Allocate memory for the actual storage
  Q = (char *) calloc((size_t) sizeOfQueue, (size_t) sizeOfData);

  if ((fq != NULL) && (Q != NULL))
  {
    fq->id = FQ_ID++;        // automatically increment it for the next one
    fq->sizeOfData = sizeOfData;     // must be greater than zero, dummy
    fq->sizeOfQueue = sizeOfQueue;
    fq->next = 0;
    fq->numInQueue = 0L;
    fq->maxInQueue = 0L;
    fq->oldest = 0;
    fq->numErrors = 0L;
    fq->queueArray[fq->oldest] = NULL;

    // all the pointers actually point to real memory!
    for (n = 0; n < sizeOfQueue; n++)
    {
      fq->queueArray[n] = (void *) Q;
      Q += sizeOfData;
    }

    // create the lock object
    fq->lock = NULL;
    fq->lock = (OS_MUTEX_Attrs *) malloc(sizeof(OS_MUTEX_Attrs));
    fq->lock->hMutex = (OS_MUTEX_Object *) malloc(sizeof(OS_MUTEX_Object));
    fq->lock->hMutexAttr = (OS_MUTEX_ATTR_Object *) malloc(sizeof(OS_MUTEX_ATTR_Object));
    fq->lock->name = "Fifo Mutex Lock";
    fq->lock->idNum = fq->id;
    fq->lock->security = NULL; /* not used */
//    fq->lock->msTimeout  = 100;
    fq->lock->msTimeout = 1000;
//    fq->lock->msTimeout  = OS_INFINITE;

    if (OS_createMutex(fq->lock) == NULL)
    {
      J_LOG(gData, OSM_LOG_ERROR, "****** Could not open the %s Mutex # %d\n", fq->lock->name, fq->id);
    }
    else
    {
      J_LOG(gData, OSM_LOG_FUNCS, "****** Created the %s Mutex # %d\n", fq->lock->name, fq->id);
    }
  }
  else
  {
    // severe error allocating memory, free up anything that I can and return null
    if (fq != NULL)
      free(fq);
    if (Q != NULL)
      free(Q);
    return NULL;
  }
  return fq;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_reset
 ***
 *** Clears and resets the FIFO.  This method sets up the FIFO for initial use,
 *** or reuse.  It does this by putting all values in a known and valid initial
 *** state.
 *** <p>
 ***
 *** created:  8/1/2011 (9:34:02 AM)
 ***
 ***   Parameters:
 ***
 ***   Returns:  -1 if an error occurred during reset, otherwise returns the state
 ***             of the FIFOs lock.  It should be unlocked (FALSE or 0).
 ***
 ******************************************************************************/
int
FQ_reset(FIFO_QUEUE *fq)
{
  int success = -1;

  if (fq != NULL)
  {
    J_LOG(gData, OSM_LOG_FUNCS, "****** Resetting the FIFO and %s Mutex # %d\n", fq->lock->name, fq->id);

    // don't try to grab the lock, because it may block, just blast through !!!
    fq->next = 0;
    fq->numInQueue = 0L;
    fq->maxInQueue = 0L;
    fq->oldest = 0;
    fq->numErrors = 0L;

    // the lock is managed by a higher process.....
  }
  else
  {
  }
  return success;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_deinit
 ***
 *** Releases and destroys the queue.
 *** <p>
 ***
 *** created:  8/1/2011 (10:17:43 AM)
 ***
 ***   Parameters: a pointer to the queue to be released.
 ***
 ***   Returns: always 1
 ***
 ******************************************************************************/
int
FQ_deinit(FIFO_QUEUE *fq)
{
  if (fq != NULL)
  {
    OS_deleteMutex(fq->lock);

    // free up the storage for the queue
    free(&(fq->queueArray[0]));

    // free up the fifo
    free(fq);
  }
  return 1;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_size
 ***
 *** Returns the physical size of the Queue.
 *** <p>
 ***
 *** created:  11/12/2011 (8:25:17 PM)
 ***
 ***   Parameters: a pointer to the queue.
 ***
 ***   Returns: the size of the queue.
 ***
 ******************************************************************************/
long
FQ_size(FIFO_QUEUE *fq)
{
  return fq->sizeOfQueue;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_used
 ***
 *** Returns the number of elements in the queue.
 *** <p>
 ***
 *** created:  11/12/2011 (8:25:32 PM)
 ***
 ***   Parameters: a pointer to the queue.
 ***
 ***   Returns: the number of elements in the queue.
 ***
 ******************************************************************************/
long
FQ_used(FIFO_QUEUE *fq)
{
  return fq->numInQueue;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_isFull
 ***
 *** Returns true (1) if the queue is full.
 *** <p>
 ***
 *** created:  11/12/2011 (8:25:32 PM)
 ***
 ***   Parameters: a pointer to the queue.
 ***
 ***   Returns: true or false
 ***
 ******************************************************************************/
int
FQ_isFull(FIFO_QUEUE *fq)
{
  return (fq->numInQueue >= fq->sizeOfQueue);
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_isEmpty
 ***
 *** Returns true (1) if the queue is empty.
 *** <p>
 ***
 *** created:  11/12/2011 (8:25:32 PM)
 ***
 ***   Parameters: a pointer to the queue.
 ***
 ***   Returns: true or false
 ***
 ******************************************************************************/
int
FQ_isEmpty(FIFO_QUEUE *fq)
{
  return (fq->numInQueue <= 0);
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_isEmpty
 ***
 *** Returns the number of elements in the queue.
 *** <p>
 ***
 *** created:  11/12/2011 (8:25:32 PM)
 ***
 ***   Parameters: a pointer to the queue.
 ***
 ***   Returns: the number of elements in the queue.
 ***
 ******************************************************************************/
int
FQ_isLocked(FIFO_QUEUE *fq)
{
  return fq->isLocked;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_max
 ***
 *** Returns the maximum number of elements that have occupied the queue since the
 *** last time it this value was reset.  This method can also be used to reset
 *** this value back to zero, by providing a zero as a clear argument.
 *** <p>
 ***
 *** created:  2/19/2011 (10:36:35 AM)
 ***
 ***   Parameters: a pointer to the queu, and a clear flag.  If the clear flag is
 ***              zero, then the value is set to zero.
 ***
 ***   Returns:  the maximum number in the queue, this is like a high water mark
 ***             that can be used to determine if queue periodically fills up.
 ***
 ******************************************************************************/
long
FQ_max(FIFO_QUEUE *fq, int clear)
{
  int max = fq->maxInQueue;

  if (clear == 0)
    fq->maxInQueue = 0;
  return max;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_errors
 ***
 *** Returns the number of errors that have occurred in the queue, and provides
 *** a way to clear the error count, by providing a zero to the clear argument.
 *** <p>
 ***
 *** created:  11/13/2011 (11:40:32 AM)
 ***
 ***   Parameters:  a pointer to the queue, and a clear argument which, if zero
 ***               will reset the error counter to zero.
 ***
 ***   Returns:  the number of queue errors that have accumulated since the last
 ***             time the counter was reset.
 ***
 ******************************************************************************/
long
FQ_errors(FIFO_QUEUE *fq, int clear)
{
  long err = fq->numErrors;

  if (clear == 0)
    fq->numErrors = 0L;
  return err;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_getAddPtr
 ***
 *** Returns a pointer to the memory where the next element will be placed when
 *** added.
 *** <p>
 ***
 *** created:  8/1/2011 (9:26:50 AM)
 ***
 ***   Parameters:  a pointer to the element to receive the pointer, and the queue
 ***
 ***   Returns:  -1 if add not possible, otherwise returns the number in the queue
 ***
 ******************************************************************************/
long
FQ_getAddPtr(FIFO_QUEUE *fq, void** addPtr)
{
  if ((fq == NULL) || (fq->numInQueue >= fq->sizeOfQueue))
  {
    if (fq != NULL)
      fq->numErrors++;  // could not add
    return -1;
  }

  *addPtr = fq->queueArray[fq->next];
  return fq->numInQueue;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_nextAdd
 ***
 *** Does fifo queue bookkeeping after an add.  Updates the internal queue
 *** fields to reflect that a new element was just copied into a queue location.
 *** <p>
 ***
 *** created:  8/1/2011 (9:26:50 AM)
 ***
 ***   Parameters:  a pointer to the queue
 ***
 ***   Returns:  -1 if not added, otherwise returns the number in the queue
 ***
 ******************************************************************************/
long
FQ_nextAdd(FIFO_QUEUE *fq)
{
  if (fq == NULL)
    return -1;

  fq->numInQueue++;

  // update the next index, wrap if necessary
  fq->next = (fq->next + 1) % fq->sizeOfQueue;
//  fq->next = ((fq->next + 1) >= fq->sizeOfQueue) ? 0 : (fq->next + 1);

  // update max if necessary
  fq->maxInQueue = (fq->maxInQueue < fq->numInQueue) ? fq->numInQueue : fq->maxInQueue;

  return fq->numInQueue;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_add
 ***
 *** Adds an element to the end of the queue.  If the queue is full, then the
 *** element will not be added, and -1 will be returned.
 *** <p>
 ***
 *** created:  8/1/2011 (9:26:50 AM)
 ***
 ***   Parameters:  a pointer to the element to add (it will be copied)
 ***
 ***   Returns:  -1 if not added, otherwise returns the number in the queue
 ***
 ******************************************************************************/
long
FQ_add(FIFO_QUEUE *fq, void* element)
{
  long result = 0;
  void* mem = NULL;

  result = FQ_getAddPtr(fq, &mem);

  if (result < 0)
    return result;

  memcpy(mem, element, fq->sizeOfData);

  return FQ_nextAdd(fq);
}
/*-----------------------------------------------------------------------*/
/******************************************************************************
 *** Function: FQ_addOld
 ***
 *** Adds an element to the end of the queue.  If the queue is full, then the
 *** element will not be added, and -1 will be returned.
 *** <p>
 ***
 *** created:  8/1/2011 (9:26:50 AM)
 ***
 ***   Parameters:  a pointer to the element to add (it will be copied)
 ***
 ***   Returns:  -1 if not added, otherwise returns the number in the queue
 ***
 ******************************************************************************/
long
FQ_addOld(FIFO_QUEUE *fq, void* element)
{
  unsigned int next;

  if ((fq == NULL) || (element == NULL) || (fq->numInQueue >= fq->sizeOfQueue))
  {
    if (fq != NULL)
      fq->numErrors++;  // could not add
    return -1;
  }

  next = fq->next;
  fq->numInQueue++;

  // copy the element into the queue, the size was previously established
  memcpy(fq->queueArray[next], element, fq->sizeOfData);

  // update the next index, wrap if necessary
  fq->next = ((next + 1) >= fq->sizeOfQueue) ? 0 : (next + 1);

  // update max if necessary
  fq->maxInQueue = (fq->maxInQueue < fq->numInQueue) ? fq->numInQueue : fq->maxInQueue;

  return fq->numInQueue;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_remove
 ***
 *** Removes the oldest element from the queue, and copies its contents into
 *** the element associated with the argument pointer.  If the
 *** queue was empty, or some other error occurs, then -1 is returned, and
 *** nothing is copied.
 *** <p>
 ***
 *** created:  8/1/2011 (9:30:23 AM)
 ***
 ***   Parameters: a pointer to the queue
 ***
 ***   Returns:  the number remaining in the queue, or -1 if could not be removed
 ***
 ******************************************************************************/
long
FQ_remove(FIFO_QUEUE *fq, void* element)
{
  unsigned int oldest;

  if ((fq == NULL) || (element == NULL) || (fq->numInQueue <= 0))
    return -1;

  oldest = fq->oldest;
  fq->numInQueue--;

  // copy the oldest element in the queue into the location pointed at by the argument
  memcpy(element, fq->queueArray[oldest], fq->sizeOfData);

  // update the oldest to the next oldest, and wrap if necessary
  fq->oldest = ((oldest + 1) >= fq->sizeOfQueue) ? 0 : (oldest + 1);

  return fq->numInQueue;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
 *** Function: FQ_lock
 ***
 *** Attempts to lock the fifo mutex.  May block, if it is already locked.  If
 *** it blocks, it may timeout, depending upon the timeout period.  The timeout
 *** period can be INFINITE, in which case it will block indefinately.
 *** <p>
 ***
 *** created:  8/1/2011 (9:30:23 AM)
 ***
 ***   Parameters: a pointer to the queue
 ***
 ***   Returns:  the lockness of the lock.  TRUE if locked, FALSE otherwise
 ***
 ******************************************************************************/
int
FQ_lock(FIFO_QUEUE *fq)
{
  if (fq == NULL)
    return -1;

  fq->isLocked = OS_pendOnMutex(fq->lock, OS_DEFAULT);
  return fq->isLocked;
}
/*-----------------------------------------------------------------------*/
/******************************************************************************
 *** Function: FQ_unlock
 ***
 *** Attempts to unlock the fifo queues lock.  It should be locked.
 *** <p>
 ***
 *** created:  8/1/2011 (9:30:23 AM)
 ***
 ***   Parameters: a pointer to the queue
 ***
 ***   Returns:  TRUE if locked, FALSE if the method succeeds and is unlocked
 ***
 ******************************************************************************/
int
FQ_unlock(FIFO_QUEUE *fq)
{
  if (fq == NULL)
    return -1;

  if (OS_postMutex(fq->lock) == 0)
    fq->isLocked = FALSE;
  return fq->isLocked;
}
/*-----------------------------------------------------------------------*/

