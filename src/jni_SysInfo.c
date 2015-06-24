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
 * jni_SysInfo.c
 *
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */


#include <unistd.h>
#include <jni.h>

#include "osmJniPi.h"
#include "jni_PeerClass.h"
#include "jni_SysInfo.h"
#include "jni_SharedResources.h"

extern plugin_data_t *       gData; /* the global plugin data */
extern JPC_CLASS PeerClassArray[];


jobjectArray *  jsi_getNodePortStatus(void * pJenv)
{
  jobject currentObject;
  static jobjectArray objectArray;
  jobjectArray disabledArray;
  jobjectArray reducedWidthArray;
  jobjectArray reducedSpeedArray;
  jobjectArray unenabledWidthArray;
  jobjectArray unenabledSpeedArray;
  JNIEnv * pJEnv = (JNIEnv *) pJenv;
  int type = 0;
  int n_ndex = 0;

  jsi_PortStats_t * pPortStats;
  jsi_PortDesc_t * pPortDesc = NULL;

  /* build an array of PortStatus from the node type */
  int numDisabled = 0;
  int numRedWidth = 0;
  int numRedSpeed = 0;
  int numUnWidth  = 0;
  int numUnSpeed  = 0;

  /* TODO these need locks */
  JPC_CLASS Object_Class   = PeerClassArray[JPC_SBN_NPORTSTAT_CLASS];
  JPC_CLASS PortDesc_Class = PeerClassArray[JPC_IB_PORT_CLASS];

  objectArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) IB_NODE_TYPE_ROUTER, Object_Class.jpcClass, NULL);
  if (objectArray != NULL)
  {
    /* do this for CA, SW, and RT */
    for(type = IB_NODE_TYPE_CA; type <= IB_NODE_TYPE_ROUTER; type++)
    {

    pPortStats = sr_getPortStats(type);
    pPortDesc = NULL;

    /* build an array of PortStatus from the node type */
    numDisabled = pPortStats->ports_disabled;
    numRedWidth = pPortStats->ports_reduced_width;
    numRedSpeed = pPortStats->ports_reduced_speed;
    numUnWidth  = pPortStats->ports_unenabled_width;
    numUnSpeed  = pPortStats->ports_unenabled_speed;

     // create arrays to hold the IB_Port discriptions

    disabledArray       = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numDisabled, PortDesc_Class.jpcClass, NULL);
    reducedWidthArray   = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numRedWidth, PortDesc_Class.jpcClass, NULL);
    reducedSpeedArray   = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numRedSpeed, PortDesc_Class.jpcClass, NULL);
    unenabledWidthArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numUnWidth,  PortDesc_Class.jpcClass, NULL);
    unenabledSpeedArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numUnSpeed,  PortDesc_Class.jpcClass, NULL);
    if ((disabledArray != NULL) && (reducedWidthArray != NULL) && (reducedSpeedArray) && (unenabledWidthArray != NULL) && (unenabledSpeedArray))
    {
      // have the arrays, now create and set the objects
      pPortDesc = pPortStats->disabled_ports;
      for (n_ndex = 0; n_ndex < numDisabled; n_ndex++)
      {
              /*  create a Port Description object */
            currentObject = (*pJEnv)->NewObject(pJEnv, PortDesc_Class.jpcClass, PortDesc_Class.constructorMethod->methodID,
                (jlong) pPortDesc->node_guid, (jint) pPortDesc->port_num, (*pJEnv)->NewStringUTF(pJEnv, pPortDesc->print_desc));

            if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an IB_Port Object for populating the disabled array: %d\n", n_ndex);
            }
            else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, disabledArray,(jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
            pPortDesc = pPortDesc->next;
      }

      pPortDesc = pPortStats->reduced_width_ports;
      for (n_ndex = 0; n_ndex < numRedWidth; n_ndex++)
      {
              /*  create a Port Description object */
            currentObject = (*pJEnv)->NewObject(pJEnv, PortDesc_Class.jpcClass, PortDesc_Class.constructorMethod->methodID,
                (jlong) pPortDesc->node_guid, (jint) pPortDesc->port_num, (*pJEnv)->NewStringUTF(pJEnv, pPortDesc->print_desc));

            if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an IB_Port Object for populating the reduced width array: %d\n", n_ndex);
            }
            else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, reducedWidthArray,(jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
            pPortDesc = pPortDesc->next;
      }

      pPortDesc = pPortStats->reduced_speed_ports;
      for (n_ndex = 0; n_ndex < numRedSpeed; n_ndex++)
      {
              /*  create a Port Description object */
            currentObject = (*pJEnv)->NewObject(pJEnv, PortDesc_Class.jpcClass, PortDesc_Class.constructorMethod->methodID,
                (jlong) pPortDesc->node_guid, (jint) pPortDesc->port_num, (*pJEnv)->NewStringUTF(pJEnv, pPortDesc->print_desc));

            if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an IB_Port Object for populating the reduced speed array: %d\n", n_ndex);
            }
            else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, reducedSpeedArray,(jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
            pPortDesc = pPortDesc->next;
      }

      pPortDesc = pPortStats->unenabled_width_ports;
      for (n_ndex = 0; n_ndex < numUnWidth; n_ndex++)
      {
              /*  create a Port Description object */
            currentObject = (*pJEnv)->NewObject(pJEnv, PortDesc_Class.jpcClass, PortDesc_Class.constructorMethod->methodID,
                (jlong) pPortDesc->node_guid, (jint) pPortDesc->port_num, (*pJEnv)->NewStringUTF(pJEnv, pPortDesc->print_desc));

            if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an IB_Port Object for populating the unenabled width array: %d\n", n_ndex);
            }
            else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, unenabledWidthArray,(jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
            pPortDesc = pPortDesc->next;
      }

      pPortDesc = pPortStats->unenabled_speed_ports;
      for (n_ndex = 0; n_ndex < numUnSpeed; n_ndex++)
      {
              /*  create a Port Description object */
            currentObject = (*pJEnv)->NewObject(pJEnv, PortDesc_Class.jpcClass, PortDesc_Class.constructorMethod->methodID,
                (jlong) pPortDesc->node_guid, (jint) pPortDesc->port_num, (*pJEnv)->NewStringUTF(pJEnv, pPortDesc->print_desc));

            if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an IB_Port Object for populating the unenabled speed array: %d\n", n_ndex);
            }
            else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, unenabledSpeedArray,(jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
            pPortDesc = pPortDesc->next;
      }

      J_LOG(gData, OSM_LOG_DEBUG, "Attempting to create a NodePortStats Object\n");

      /* create the remaining object elements (must match the Java Constructor order */
      currentObject = (*pJEnv)->NewObject(pJEnv, Object_Class.jpcClass, Object_Class.constructorMethod->methodID,
          (jlong) pPortStats->total_nodes, (jlong) pPortStats->total_ports, (jlong) pPortStats->ports_down, (jlong) pPortStats->ports_active,
          (jlong) pPortStats->ports_disabled,
          (jlong) pPortStats->ports_1X,
          (jlong) pPortStats->ports_4X,
          (jlong) pPortStats->ports_8X,
          (jlong) pPortStats->ports_12X,
          (jlong) pPortStats->ports_unknown_width,
          (jlong) pPortStats->ports_reduced_width,
          (jlong) pPortStats->ports_sdr,
          (jlong) pPortStats->ports_ddr,
          (jlong) pPortStats->ports_qdr,
          (jlong) pPortStats->ports_unknown_speed,
          (jlong) pPortStats->ports_reduced_speed,

          (jlong) pPortStats->ports_unenabled_width,
          (jlong) pPortStats->ports_fdr10,
          (jlong) pPortStats->ports_fdr,
          (jlong) pPortStats->ports_edr,
          (jlong) pPortStats->ports_unenabled_speed,

          disabledArray, reducedWidthArray, reducedSpeedArray, unenabledWidthArray, unenabledSpeedArray,

          (*pJEnv)->NewStringUTF(pJEnv, ib_get_node_type_str(pPortStats->node_type_lim)));

      if (currentObject == NULL)
      {
        J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an NodePortStatus Object for type: %s\n", ib_get_node_type_str(type));
      }
      else
      {
        (*pJEnv)->SetObjectArrayElement(pJEnv, objectArray,(jsize) (type - 1), currentObject);
        (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
      }

      (*pJEnv)->DeleteLocalRef(pJEnv, disabledArray);
      (*pJEnv)->DeleteLocalRef(pJEnv, reducedWidthArray);
      (*pJEnv)->DeleteLocalRef(pJEnv, reducedSpeedArray);
      (*pJEnv)->DeleteLocalRef(pJEnv, unenabledWidthArray);
      (*pJEnv)->DeleteLocalRef(pJEnv, unenabledSpeedArray);
    }
    else
      J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArrays for suspicious ports\n");
    }
  }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for NodePortStatus types\n");

  return &objectArray;
}

void jsi_printOsmSysInfo(void * pJenv)
{
  int n_ndex = 0;
  jsi_SystemInfo_t * p_sysInf = sr_getSysInfo();

  J_LOG(gData, OSM_LOG_INFO, "OsmVersion: %s\n", p_sysInf->OpenSM_Version);
  J_LOG(gData, OSM_LOG_INFO, "OsmJpi_Version: %s\n", p_sysInf->OsmJpi_Version);
  J_LOG(gData, OSM_LOG_INFO, "SM_State: %s\n", p_sysInf->SM_State);
  J_LOG(gData, OSM_LOG_INFO, "SA_State: %s\n", p_sysInf->SA_State);
  J_LOG(gData, OSM_LOG_INFO, "PM_State: %s\n", p_sysInf->PM_State);
  J_LOG(gData, OSM_LOG_INFO, "PM_SweepState: %s\n", p_sysInf->PM_SweepState);
  J_LOG(gData, OSM_LOG_INFO, "RoutingEngine: %s\n", p_sysInf->RoutingEngine);
  J_LOG(gData, OSM_LOG_INFO, "numPlugins: %d\n", p_sysInf->numPlugins);

  for(n_ndex = 0; n_ndex < p_sysInf->numPlugins; n_ndex++)
    J_LOG(gData, OSM_LOG_INFO, "%d Plugin: %s\n", n_ndex, &(p_sysInf->EventPlugins[n_ndex][0]));

}
  void * jsi_getOsmSysInfo(void * pJenv)
  {
  // returning a pointer to an object, so make it static
    static jobject      currentObject;
    jobjectArray pluginArray;
    int n_ndex = 0;
    JNIEnv * pJEnv        = (JNIEnv *) pJenv;

    jsi_SystemInfo_t * p_sysInf = sr_getSysInfo();

    JPC_CLASS String_Class    = PeerClassArray[JPC_STRING_CLASS];
    JPC_CLASS SINFO_Class     = PeerClassArray[JPC_OSM_SYSINFO_CLASS];
    jobjectArray * p_npsArray = jsi_getNodePortStatus(pJEnv);

//    jsi_printOsmSysInfo(pJenv);

    pluginArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) p_sysInf->numPlugins, String_Class.jpcClass, NULL);
    if(pluginArray == NULL)
    {
      J_LOG(gData, OSM_LOG_ERROR, "Could not create an array of Strings to hold the plugin names.\n");
    }
    else for (n_ndex = 0; n_ndex < p_sysInf->numPlugins; n_ndex++)
    {
      (*pJEnv)->SetObjectArrayElement(pJEnv, pluginArray, (jsize) n_ndex, (*pJEnv)->NewStringUTF(pJEnv, &(p_sysInf->EventPlugins[n_ndex][0])));
      n_ndex++;
    }

    J_LOG(gData, OSM_LOG_DEBUG, "Attempting to create a SysInfo Object\n");

    currentObject = (*pJEnv)->NewObject(pJEnv, SINFO_Class.jpcClass, SINFO_Class.constructorMethod->methodID,
        (jint) p_sysInf->SM_Priority, (jint) p_sysInf->PM_SweepTime, (jint) p_sysInf->PM_OutstandingQueries, (jint) p_sysInf->PM_MaximumQueries,
        (*pJEnv)->NewStringUTF(pJEnv, p_sysInf->OpenSM_Version), (*pJEnv)->NewStringUTF(pJEnv, p_sysInf->OsmJpi_Version),
        (*pJEnv)->NewStringUTF(pJEnv, p_sysInf->SM_State), (*pJEnv)->NewStringUTF(pJEnv, p_sysInf->SA_State),
        (*pJEnv)->NewStringUTF(pJEnv, p_sysInf->PM_State), (*pJEnv)->NewStringUTF(pJEnv, p_sysInf->PM_SweepState),
        (*pJEnv)->NewStringUTF(pJEnv, p_sysInf->RoutingEngine),
        pluginArray,
        (*pJEnv)->GetObjectArrayElement(pJEnv, *p_npsArray,(jsize) (IB_NODE_TYPE_CA -1)),
        (*pJEnv)->GetObjectArrayElement(pJEnv, *p_npsArray,(jsize) (IB_NODE_TYPE_SWITCH -1)),
        (*pJEnv)->GetObjectArrayElement(pJEnv, *p_npsArray,(jsize) (IB_NODE_TYPE_ROUTER -1))
        );

    if (currentObject == NULL)
    {
      J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a SysInfo Object\n");
    }
    // fully constructed, or failed, either way, clean up
    (*pJEnv)->DeleteLocalRef(pJEnv, *p_npsArray);
    (*pJEnv)->DeleteLocalRef(pJEnv, pluginArray);

    return (void *) &currentObject;
}

  void * jpi_getPluginInfo(void * pJenv)
  {
    // returning a pointer to an object, so make it static
      static jobject      currentObject;
      JNIEnv * pJEnv        = (JNIEnv *) pJenv;
      JPC_CLASS STATS_Class = PeerClassArray[JPC_OSM_PLUGININFO_CLASS];

      jpi_Plugin_t * p_Plugin = sr_getPluginInfo();

      /* create peer object via its field constructor */
      currentObject = (*pJEnv)->NewObject(pJEnv, STATS_Class.jpcClass, STATS_Class.constructorMethod->methodID,
          p_Plugin->update_period, p_Plugin->report_period, p_Plugin->event_timeout_ms, p_Plugin->update_count, p_Plugin->event_count);

      if (currentObject == NULL)
      {
        J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an OSM_PluginInfo Object\n");
      }
      return (void *) &currentObject;
  }
