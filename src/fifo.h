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
 * fifo.h
 *
 *  Created on: Aug 2, 2011
 *      Author: meier3
 */

#ifndef FIFO_H_
#define FIFO_H_

#include "jni_Synchronization.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           Defines                                               !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#define FQ_MAX_QUEUE_SIZE   (200)
#define FQ_MIN_QUEUE_SIZE     (5)

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           Type Definitions                                      !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

// Define a structure for a simple Fifo Queue
typedef struct FifoQueue
{
  unsigned int id;                            // unique queue id, an instance value
  OS_MUTEX_Attrs* lock;                          // pointer to a lock
  int isLocked;                      // boolean, attempts to indicate the lockedness
  unsigned long oldest;                        // index to oldest in queue
  unsigned long next;                          // index to newest in Queue
  unsigned long numInQueue;                    // number in queue
  unsigned long maxInQueue;                    // maximum number in queue since reset
  unsigned long sizeOfData;                    // data size of each element in the queue
  unsigned long sizeOfQueue;                   // capacity of the Queue
  unsigned long numErrors;                     // number of times an addition failed
  void* queueArray[FQ_MAX_QUEUE_SIZE]; // an array of pointers to the elements in the queue
} FIFO_QUEUE;

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           External Variable Declarations                        !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*~~~           Function Declarations                                 !!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

FIFO_QUEUE* FQ_init(long sizeOfData, long sizeOfQueue);
int FQ_deinit(FIFO_QUEUE *fq);
long FQ_add(FIFO_QUEUE *fq, void* element);
long FQ_remove(FIFO_QUEUE *fq, void* element);
long FQ_size(FIFO_QUEUE *fq);
long FQ_used(FIFO_QUEUE *fq);
long FQ_max(FIFO_QUEUE *fq, int clear);
long FQ_errors(FIFO_QUEUE *fq, int clear);
int FQ_isFull(FIFO_QUEUE *fq);
int FQ_isEmpty(FIFO_QUEUE *fq);
long FQ_getAddPtr(FIFO_QUEUE *fq, void** addPtr);
long FQ_nextAdd(FIFO_QUEUE *fq);
long FQ_addOld(FIFO_QUEUE *fq, void* element);
int FQ_lock(FIFO_QUEUE *fq);
int FQ_unlock(FIFO_QUEUE *fq);
int FQ_isLocked(FIFO_QUEUE *fq);
int FQ_reset(FIFO_QUEUE *fq);

#ifdef __cplusplus
}
#endif
#endif /* FIFO_H_ */
