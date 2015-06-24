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
 * jni_PeerClass.h
 *
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */

#ifndef JNI_PEERCLASS_H_
#define JNI_PEERCLASS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define STRING_CLASS_NAME         "Ljava/lang/String;"

#define SBN_NODE_CLASS_NAME       "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_Node"
#define PFM_NODE_CLASS_NAME       "gov/llnl/lc/infiniband/opensm/plugin/data/PFM_Node"
#define OSM_NODE_CLASS_NAME       "gov/llnl/lc/infiniband/opensm/plugin/data/OSM_Nodes"
#define SBN_PORT_CLASS_NAME       "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_Port"
#define SBN_PORTINFO_CLASS_NAME   "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_PortInfo"
#define MLX_PORTINFO_CLASS_NAME   "gov/llnl/lc/infiniband/opensm/plugin/data/MLX_ExtPortInfo"
#define PFM_PORT_CLASS_NAME       "gov/llnl/lc/infiniband/opensm/plugin/data/PFM_Port"
#define OSM_PORT_CLASS_NAME       "gov/llnl/lc/infiniband/opensm/plugin/data/OSM_Ports"
#define OSM_STAT_CLASS_NAME       "gov/llnl/lc/infiniband/opensm/plugin/data/OSM_Stats"
#define OSM_SYSINFO_CLASS_NAME    "gov/llnl/lc/infiniband/opensm/plugin/data/OSM_SysInfo"
#define OSM_PLUGININFO_CLASS_NAME "gov/llnl/lc/infiniband/opensm/plugin/data/OSM_PluginInfo"
#define OSM_SUBNET_CLASS_NAME     "gov/llnl/lc/infiniband/opensm/plugin/data/OSM_Subnet"
#define SBN_OPTIONS_CLASS_NAME    "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_Options"
#define SBN_SWITCH_CLASS_NAME     "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_Switch"
#define SBN_ROUTER_CLASS_NAME     "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_Router"
#define SBN_MANAGER_CLASS_NAME    "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_Manager"
#define SBN_PKEY_CLASS_NAME       "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_PartitionKey"
#define SBN_NPORTSTAT_CLASS_NAME  "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_NodePortStatus"
#define IB_PORT_CLASS_NAME        "gov/llnl/lc/infiniband/core/IB_Port"
#define OSM_EVENT_CLASS_NAME      "gov/llnl/lc/infiniband/opensm/plugin/data/OSM_EventObject"
#define SBN_MCGROUP_CLASS_NAME    "gov/llnl/lc/infiniband/opensm/plugin/data/SBN_MulticastGroup"

/* only the methods I need to access, listed in static strings above */
#define MAX_STRING_SIZE         (128)
#define MAX_NUM_FIELDS          (128)            // don't exceed this value
#define MAX_NUM_METHODS           (2)            // don't exceed this value
#define MAX_NUM_CONSTRUCTORS      (2)            // don't exceed this value

#define NUM_ZERO_METHODS          (0)
#define NUM_STRING_METHODS        (1)

/* number public fields to expose */
#define NUM_SBN_NODE_FIELDS      (12)
#define NUM_PFM_NODE_FIELDS       (5)
#define NUM_OSM_NODE_FIELDS       (2)
#define NUM_SBN_PORT_FIELDS       (8)
#define NUM_SBN_PORTINFO_FIELDS  (35)
#define NUM_MLX_PORTINFO_FIELDS   (4)
#define NUM_PFM_PORT_FIELDS       (6)
#define NUM_OSM_PORT_FIELDS       (2)
#define NUM_OSM_STAT_FIELDS      (11)
#define NUM_OSM_SYSINFO_FIELDS   (15)
#define NUM_OSM_PLUGININFO_FIELDS (5)
#define NUM_SBN_OPTIONS_FIELDS  (122)
#define NUM_SBN_SWITCH_FIELDS    (14)
#define NUM_SBN_ROUTER_FIELDS     (1)
#define NUM_SBN_MANAGER_FIELDS    (5)
#define NUM_SBN_PKEY_FIELDS       (8)
#define NUM_OSM_SUBNET_FIELDS    (27)
#define NUM_IB_PORT_FIELDS        (0)
#define NUM_SBN_NPORTSTAT_FIELDS (27)
#define NUM_OSM_EVENT_FIELDS      (5)
#define NUM_SBN_MCGROUP_FIELDS    (5)

#define NUM_STRING_FIELDS         (0)

typedef struct JpcMethodID
{
   jmethodID       methodID;  // the method ID
   char *          methodName;
   char *          methodSignature;
} JPC_MID;

typedef struct JpcFieldID
{
   jfieldID        fieldID;  // the field ID
   char *          fieldName;
   char *          fieldSignature;
} JPC_FID;

// wrap the constructors and fields all up in one
typedef struct JpcPeerClass
{
   int             classIndex;
   jclass          jpcClass;
   char*           className;          //
   JPC_MID *       constructorMethod;
   JPC_MID *       methodArray;
   JPC_FID *       fieldArray;
   int             numMethods;
   int             numFields;
} JPC_CLASS;


// indexes for the peer class array  (DEPENDENT CLASSES GO FIRST, keep order!)
enum JPC_PEER_CLASS_TYPE
{
  JPC_SBN_NODE_CLASS = 0,
  JPC_PFM_NODE_CLASS,
  JPC_OSM_NODE_CLASS,
  JPC_SBN_PORTINFO_CLASS,
  JPC_MLX_PORTINFO_CLASS,
  JPC_SBN_PORT_CLASS,
  JPC_PFM_PORT_CLASS,
  JPC_OSM_PORT_CLASS,
  JPC_OSM_STAT_CLASS,
  JPC_SBN_OPTIONS_CLASS,
  JPC_SBN_MANAGER_CLASS,
  JPC_SBN_ROUTER_CLASS,
  JPC_SBN_SWITCH_CLASS,
  JPC_SBN_PKEY_CLASS,
  JPC_SBN_MCGROUP_CLASS,
  JPC_OSM_SUBNET_CLASS,
  JPC_IB_PORT_CLASS,
  JPC_SBN_NPORTSTAT_CLASS,
  JPC_OSM_SYSINFO_CLASS,
  JPC_OSM_PLUGININFO_CLASS,
  JPC_OSM_EVENT_CLASS,
  JPC_STRING_CLASS,
  JPC_NUM_PEER_CLASSES    // always last, this enum defines the order!!
};


int jpc_initJniReferences(void * pJenv);
int jpc_printJniReferences(void);

#ifdef __cplusplus
}
#endif
#endif /* JNI_PEERCLASS_H_ */
