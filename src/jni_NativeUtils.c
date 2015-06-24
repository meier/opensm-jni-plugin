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
 * jni_NativeUtils.c
 *
 *  Created on: Aug 2, 2014
 *      Author: meier3
 */
#include <stddef.h>

#include <jni.h>
#include "jni_OsmNativeInterface.h"
#include "jni_PeerClass.h"
#include "osmJniPi.h"
#include "osmJniPi_version.h"

#define NUM_NATIVE_METHODS (9)

static char interface_class_name[] = "gov/llnl/lc/infiniband/opensm/plugin/OsmNativeInterface";

static char getOsmNodes_method[] = "getOsmNodes";
static char getOsmPorts_method[] = "getOsmPorts";
static char getOsmStats_method[] = "getOsmStats";
static char getOsmSubnet_method[] = "getOsmSubnet";
static char getOsmSysInfo_method[] = "getOsmSysInfo";
static char getOsmPluginInfo_method[] = "getPluginInfo";
static char wait_for_event_method[] = "wait_for_event";
static char getVersion_method[] = "getVersion";
static char invokeCommand_method[] = "invokeCommand";

static char getOsmNodes_signature[] = "()L"OSM_NODE_CLASS_NAME";";
static char getOsmPorts_signature[] = "()L"OSM_PORT_CLASS_NAME";";
static char getOsmStats_signature[] = "()L"OSM_STAT_CLASS_NAME";";
static char getOsmSubnet_signature[] = "()L"OSM_SUBNET_CLASS_NAME";";
static char getOsmSysInfo_signature[] = "()L"OSM_SYSINFO_CLASS_NAME";";
static char getOsmPluginInfo_signature[] = "()L"OSM_PLUGININFO_CLASS_NAME";";
static char wait_for_event_signature[] = "(I)I";
static char getVersion_signature[] = "()Ljava/lang/String;";
static char invokeCommand_signature[] = "(ILjava/lang/String;)Ljava/lang/String;";

static JNINativeMethod jniNativeMethods[NUM_NATIVE_METHODS];
static JNINativeMethod * wait_nm = &jniNativeMethods[0];
static JNINativeMethod * getVersion_nm = &jniNativeMethods[1];
static JNINativeMethod * getOsmNodes_nm = &jniNativeMethods[2];
static JNINativeMethod * getOsmPorts_nm = &jniNativeMethods[3];
static JNINativeMethod * getOsmStats_nm = &jniNativeMethods[4];
static JNINativeMethod * getOsmSubnet_nm = &jniNativeMethods[5];
static JNINativeMethod * getOsmSysInfo_nm = &jniNativeMethods[6];
static JNINativeMethod * getOsmPluginInfo_nm = &jniNativeMethods[7];
static JNINativeMethod * invokeCommand_nm = &jniNativeMethods[8];

int
jnu_registerAllNatives(JNIEnv * pJEnv, plugin_data_t *pData)
{
  JNIEnv *env = pJEnv;
  jclass cls;

  /* initialize the native interface class */
  cls = (*env)->FindClass(env, interface_class_name);
  if (cls == NULL)
  {
    J_LOG(pData, OSM_LOG_ERROR, "Can't find Class (%s) at (%s)\n", interface_class_name, pData->class_path);
    return -1;
  }

  /* register all the native functions so the jvm can find them */
  wait_nm->name = wait_for_event_method;
  wait_nm->signature = wait_for_event_signature;
  wait_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_wait_1for_1event;

  getVersion_nm->name = getVersion_method;
  getVersion_nm->signature = getVersion_signature;
  getVersion_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getVersion;

  getOsmNodes_nm->name = getOsmNodes_method;
  getOsmNodes_nm->signature = getOsmNodes_signature;
  getOsmNodes_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmNodes;

  getOsmPorts_nm->name = getOsmPorts_method;
  getOsmPorts_nm->signature = getOsmPorts_signature;
  getOsmPorts_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmPorts;

  getOsmStats_nm->name = getOsmStats_method;
  getOsmStats_nm->signature = getOsmStats_signature;
  getOsmStats_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmStats;

  getOsmSysInfo_nm->name = getOsmSysInfo_method;
  getOsmSysInfo_nm->signature = getOsmSysInfo_signature;
  getOsmSysInfo_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmSysInfo;

  getOsmPluginInfo_nm->name = getOsmPluginInfo_method;
  getOsmPluginInfo_nm->signature = getOsmPluginInfo_signature;
  getOsmPluginInfo_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getPluginInfo;

  getOsmSubnet_nm->name = getOsmSubnet_method;
  getOsmSubnet_nm->signature = getOsmSubnet_signature;
  getOsmSubnet_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_getOsmSubnet;

  invokeCommand_nm->name = invokeCommand_method;
  invokeCommand_nm->signature = invokeCommand_signature;
  invokeCommand_nm->fnPtr = Java_gov_llnl_lc_infiniband_opensm_plugin_OsmNativeInterface_invokeCommand;

  (*env)->RegisterNatives(env, cls, jniNativeMethods, NUM_NATIVE_METHODS);

  jpc_initJniReferences(pJEnv);

  return 0;
}

/******************************************************************************
 *** Function: JNI_OnLoad
 ***
 *** This gets called automatically when the native library is loaded via
 *** Java.  We are loading the JVM from the native library, so this method
 *** never gets called from the JVM (native library already loaded).
 ***
 *** Called manually by C, to maintain standard practice.
 ***
 *** <p>
 ***
 *** created:  5/22/2011 (3:05:33 PM)
 ***
 ***   Parameters:
 ***
 ***   Returns:
 ***
 ******************************************************************************/
jint
JNI_OnLoad(JavaVM *vm, void *reserved)
{
  JNIEnv * pJenv;
  jint rtnVal = (*vm)->GetEnv(vm, (void **) &pJenv, (jint) JNI_VERSION_1_4);

  if (pJenv != NULL)
  {
    // initialize all references to peer class fields and constructors
    jpc_initJniReferences(pJenv);

    // anything else?  Do it here
  }
  rtnVal = (jint) JNI_VERSION_1_4;
  return rtnVal;
}

