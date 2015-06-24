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
 * jni_OsmNativeInterface.c
 *
 *  Created on: Jun 2, 2015
 *      Author: meier3
 */
#include <jni.h>

#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include "osmJniPi.h"
#include "jni_OsmNativeInterface.h"
#include "jni_Synchronization.h"
#include "jni_SharedResources.h"
#include "jni_Node.h"
#include "jni_Port.h"
#include "jni_Stats.h"
#include "jni_SysInfo.h"
#include "jni_Subnet.h"

/*
 *  Require JNI version 1.6 or higher, otherwise ....
 */
/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    getVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getVersion
  (JNIEnv * pJEnv, jobject jObj)
{
  return (*pJEnv)->NewStringUTF(pJEnv, jsr_getOsmVersion());
}

/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    wait_for_event
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_wait_1for_1event
  (JNIEnv * pJEnv, jobject jObj, jint timeout)
{
  return (jint) jsr_waitForNextEvent((void *) pJEnv, timeout);
}

/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    getOsmNodes
 * Signature: ()Lgov/llnl/lc/infiniband/opensm/plugin/data/OSM_Nodes;
 */
JNIEXPORT jobject JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmNodes
  (JNIEnv * pJEnv, jobject jObj)
{
  return  * (jobject *)(jnd_getOsmNodes((void *) pJEnv));
}

/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    getOsmPorts
 * Signature: ()Lgov/llnl/lc/infiniband/opensm/plugin/data/OSM_Ports;
 */
JNIEXPORT jobject JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmPorts
(JNIEnv * pJEnv, jobject jObj)
{
  return  * (jobject *)(jpt_getOsmPorts((void *) pJEnv));
}


/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    getOsmStats
 * Signature: ()Lgov/llnl/lc/infiniband/opensm/plugin/data/OSM_Stats;
 */
JNIEXPORT jobject JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmStats
(JNIEnv * pJEnv, jobject jObj)
{
  return  * (jobject *)(jst_getOsmStats((void *) pJEnv));
}


/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    getOsmSysInfo
 * Signature: ()Lgov/llnl/lc/infiniband/opensm/plugin/data/OSM_SysInfo;
 */
JNIEXPORT jobject JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmSysInfo
(JNIEnv * pJEnv, jobject jObj)
{
  return  * (jobject *)(jsi_getOsmSysInfo((void *) pJEnv));
}

/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    getPluginInfo
 * Signature: ()Lgov/llnl/lc/infiniband/opensm/plugin/data/OSM_PluginInfo;
 */
JNIEXPORT jobject JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getPluginInfo
(JNIEnv * pJEnv, jobject jObj)
{
  return  * (jobject *)(jpi_getPluginInfo((void *) pJEnv));
}

/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    getOsmSubnet
 * Signature: ()Lgov/llnl/lc/infiniband/opensm/plugin/data/OSM_Subnet;
 */
JNIEXPORT jobject JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmSubnet
(JNIEnv * pJEnv, jobject jObj)
{
  return  * (jobject *)(jsn_getOsmSubnet((void *) pJEnv));
}

/*
 * Class:     gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface
 * Method:    invokeCommand
 * Signature: (ILjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_invokeCommand
  (JNIEnv * pJEnv, jobject jObj, jint cmdType, jstring cmdArgs)
{
  const char* cArgs = (char*) (*pJEnv)->GetStringUTFChars(pJEnv, cmdArgs, NULL);

  jstring rtnString = (*pJEnv)->NewStringUTF(pJEnv, jsr_invokeCommand(cmdType, cArgs));

  (*pJEnv)->ReleaseStringUTFChars(pJEnv, cmdArgs, cArgs);
  return rtnString;
}



