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
 * jni_Node.c
 *
 *  Created on: Jun 2, 2011
 *      Author: meier3
 *
 *  Peer Class for Node Objects:
 *    This Node Class contains all the information gathered from all the sources
 *    of Node data.  PerfMgr, SubnetMgr, etc.
 *
 */

#include "jni_Node.h"

#include <complib/cl_byteswap_osd.h>
#include <stddef.h>

#include <jni.h>
#include "jni_PeerClass.h"
#include "jni_SharedResources.h"
#include "osmJniPi.h"

extern plugin_data_t *       gData; /* the global plugin data */
extern JPC_CLASS PeerClassArray[];


void * jnd_getOsmNodes(void * pJenv)
{
// returning a pointer to an object, so make it static
  static jobject      currentObject;
  static jobjectArray pfmArray;
  static jobjectArray sbnArray;
  JNIEnv * pJEnv        = (JNIEnv *) pJenv;

// this contains two arrays, that represent all the nodes

  /* build an array of nodes from the perfmgr */
  int numPfmNodes = pm_getNumPM_Nodes();
  int numSbnNodes = pm_getNumPT_Nodes();
  int n_ndex = 0;

  /* TODO these need locks */
  pm_Node_t * p_pmNode = pm_getPM_Nodes();  // use the latest from the DB (updated elsewhere)
  pt_Node_t * p_ptNode = pm_getPT_Nodes();  // use the latest from the subnet

  JPC_CLASS PFM_Class = PeerClassArray[JPC_PFM_NODE_CLASS];
  JPC_CLASS SBN_Class = PeerClassArray[JPC_SBN_NODE_CLASS];
  JPC_CLASS OSM_Class = PeerClassArray[JPC_OSM_NODE_CLASS];


  // create an array to hold the PM Nodes
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of PM Nodes: %d\n", numPfmNodes);

  pfmArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numPfmNodes, PFM_Class.jpcClass, NULL);
  if (pfmArray != NULL)
  {
    // have the array, now create and set the objects

    for (n_ndex = 0; n_ndex < numPfmNodes; n_ndex++, p_pmNode++)
    {
      /*  create a PFM node object */
      currentObject = (*pJEnv)->NewObject(pJEnv, PFM_Class.jpcClass, PFM_Class.constructorMethod->methodID,
          (*pJEnv)->NewStringUTF(pJEnv, p_pmNode->node_name), (jshort)p_pmNode->num_ports, (jlong)p_pmNode->node_guid,
          (jboolean)p_pmNode->esp0, (jboolean)p_pmNode->active);

      if (currentObject == NULL)
      {
        J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a PM Node Object for populating the array: %d\n", n_ndex);

      }
      else
      {
        (*pJEnv)->SetObjectArrayElement(pJEnv, pfmArray, (jsize) n_ndex, currentObject);
        (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
      }
    }

  }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for PFM_Nodes\n");

  // create an array to hold the SBN Nodes
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of SBN Nodes: %d\n", numSbnNodes);

  sbnArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numSbnNodes, SBN_Class.jpcClass, NULL);
  if (sbnArray != NULL)
  {
    // have the array, now create and set the objects


    for (n_ndex = 0; n_ndex < numSbnNodes; n_ndex++, p_ptNode++)
    {
      /*  create a PFM node object */

      currentObject = (*pJEnv)->NewObject(pJEnv, SBN_Class.jpcClass, SBN_Class.constructorMethod->methodID,
          (*pJEnv)->NewStringUTF(pJEnv, (char *)(p_ptNode->description)),
          (jshort)p_ptNode->node_info.base_version,
          (jshort)p_ptNode->node_info.class_version, (jshort)p_ptNode->node_info.node_type,
          (jshort)p_ptNode->node_info.num_ports, (jint)cl_ntoh16(p_ptNode->node_info.partition_cap),
          (jint)cl_ntoh16(p_ptNode->node_info.device_id), (jint)cl_ntoh32(p_ptNode->node_info.revision),
          (jint)cl_ntoh32(p_ptNode->node_info.port_num_vendor_id),
          (jlong)cl_ntoh64(p_ptNode->node_info.sys_guid),
          (jlong)cl_ntoh64(p_ptNode->node_info.node_guid), (jlong)cl_ntoh64(p_ptNode->node_info.port_guid));

      if (currentObject == NULL)
      {
        J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a PT Node Object for populating the array: %d\n", n_ndex);

      }
      else
      {
        (*pJEnv)->SetObjectArrayElement(pJEnv, sbnArray, (jsize) n_ndex, currentObject);
        (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
      }
    }
  }
  else J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for SBN_Nodes\n");

// done with both arrays, so construct the container object
  currentObject = (*pJEnv)->NewObject(pJEnv, OSM_Class.jpcClass, OSM_Class.constructorMethod->methodID, pfmArray, sbnArray);
  if (currentObject == NULL)
  {
    J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a OSM Node Object for holding the arrays\n");
  }

  // fully constructed, or failed, either way, clean up
  (*pJEnv)->DeleteLocalRef(pJEnv, pfmArray);
  (*pJEnv)->DeleteLocalRef(pJEnv, sbnArray);

  return (void *) &currentObject;
}
