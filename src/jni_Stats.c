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
 * jni_Stats.c
 *
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */

#include <unistd.h>
#include <jni.h>

#include "osmJniPi.h"
#include "jni_PeerClass.h"
#include "jni_Stats.h"

#include "jni_SharedResources.h"

extern plugin_data_t *       gData; /* the global plugin data */
extern JPC_CLASS PeerClassArray[];


void * jst_getOsmStats(void * pJenv)
{
  // returning a pointer to an object, so make it static
    static jobject      currentObject;
    JNIEnv * pJEnv        = (JNIEnv *) pJenv;
    JPC_CLASS STATS_Class = PeerClassArray[JPC_OSM_STAT_CLASS];

    jst_Stats_t * p_Stats = sr_getStats();

    /* create peer object via its field constructor */
    currentObject = (*pJEnv)->NewObject(pJEnv, STATS_Class.jpcClass, STATS_Class.constructorMethod->methodID,
        p_Stats->qp0_mads_outstanding, p_Stats->qp0_mads_outstanding_on_wire,
        p_Stats->qp0_mads_rcvd, p_Stats->qp0_mads_sent, p_Stats->qp0_unicasts_sent, p_Stats->qp0_mads_rcvd_unknown,
        p_Stats->sa_mads_outstanding, p_Stats->sa_mads_rcvd, p_Stats->sa_mads_sent,
        p_Stats->sa_mads_rcvd_unknown, p_Stats->sa_mads_ignored);

    if (currentObject == NULL)
    {
      J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a Stats Object\n");
    }
    return (void *) &currentObject;
}

