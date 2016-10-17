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
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */

#include "jni_SharedResources.h"

#include <complib/cl_byteswap_osd.h>
#include <complib/cl_fleximap.h>
#include <complib/cl_map.h>
#include <complib/cl_passivelock.h>
#include <complib/cl_qlist.h>
#include <complib/cl_qmap.h>
#include <complib/cl_types.h>
#include <opensm/osm_base.h>
#include <opensm/osm_event_plugin.h>
#include <opensm/osm_helper.h>
#include <opensm/osm_log.h>
#include <opensm/osm_mcm_port.h>
#include <opensm/osm_multicast.h>
#include <opensm/osm_node.h>
#include <opensm/osm_opensm.h>
#include <opensm/osm_partition.h>
#include <opensm/osm_perfmgr.h>
#include <opensm/osm_perfmgr_db.h>
#include <opensm/osm_port.h>
#include <opensm/osm_remote_sm.h>
#include <opensm/osm_router.h>
#include <opensm/osm_sa.h>
#include <opensm/osm_sm.h>
#include <opensm/osm_stats.h>
#include <opensm/osm_switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo.h"
#include "jni_Port.h"
#include "osmJniPi.h"
#include "osmJniPi_version.h"

extern plugin_data_t * gData; /* the global plugin data */
extern void osm_update_node_desc(IN osm_opensm_t *osm);

// flag, indicating if this module (and its resources) have been initialized
static int jsr_isInitialized = 0;

static FIFO_QUEUE *pEventQueue; /* a shared queue that holds event types*/

// the build date and time, force this to create a "version"
volatile const char version_date[] = __DATE__;
volatile const char version_time[] = __TIME__;

/* refer to the port struct and the node struct */
static char *portCounterNames[] =
  { "symbol_err_cnt", "link_err_recover", "link_downed", "rcv_err", "rcv_rem_phys_err", "rcv_switch_relay_err",
      "xmit_discards", "xmit_constraint_err", "rcv_constraint_err", "link_integrity", "buffer_overrun", "vl15_dropped",
      "xmit_data", "rcv_data", "xmit_pkts", "rcv_pkts", "unicast_xmit_pkts", "unicast_rcv_pkts", "multicast_xmit_pkts",
      "multicast_rcv_pkts", "xmit_wait", };

/* TODO fix this later with something more intelligent */
static pm_Node_t AllNodes[MAX_NUM_NODES];
static pm_Port_t AllPorts[MAX_NUM_PORTS];
static pt_Port_t AllPT_Ports[MAX_NUM_PORTS];
static pt_Node_t AllPT_Nodes[MAX_NUM_NODES];

static jst_Stats_t OSM_Stats;

static sr_Subnet_t OSM_Subnet;
static sr_Options_t OSM_Options;
static sr_Switch_t All_Switches[MAX_NUM_SWITCHES];
static sr_Router_t All_Routers[MAX_NUM_ROUTERS];
static sr_Manager_t All_Managers[MAX_NUM_MANAGERS];
static sr_PKey_t All_PKeys[MAX_NUM_PARTITIONS];
static sr_MCGroups_t All_MCGroups[MAX_NUM_MCGROUPS];

static jsi_SystemInfo_t SystemInfo;
static jpi_Plugin_t PluginInfo;

/* array of all (sw, ca, rt) ports that are disabled, reduced width, reduced speed, etc */
static jsi_PortDesc_t PortDescriptions[MAX_NUM_PORTS]; /** TODO should be max_num_ports_per_node ?? **/
static jsi_PortDesc_t * pPortDescription = &PortDescriptions[0];

static jsi_PortStats_t PortStatistics[IB_NODE_TYPE_ROUTER]; /* storage for CA, SW, and RT */

static int ActualNumberNodes = 0;
static int ActualNumberPorts = 0;
static int ActualSwitchPortZero = 0;
static int ActualNumberPT_Ports = 0;
static int ActualNumberPT_Nodes = 0;

static int ActualNumberSwitches = 0;
static int ActualNumberRouters = 0;
static int ActualNumberManagers = 0;
static int ActualNumberPartitions = 0;
static int ActualNumberMCGroups = 0;

static long EventCount = 0;
static int EventTimeout = 1;

jsi_PortDesc_t *
getNextAvailPortDesc(void)
{
  jsi_PortDesc_t * pRtn = pPortDescription++;
  return pRtn;
}

jsi_PortDesc_t *
resetPortDesc()
{
  int n = 0;
  pPortDescription = &PortDescriptions[0];
  jsi_PortStats_t * pPS = &PortStatistics[0];

  /* clear the PortStatistics, because an iterator is used to increment */
  for (n = 0; n < IB_NODE_TYPE_ROUTER; n++)
  {
    pPS->total_nodes = 0;
    pPS->total_ports = 0;
    pPS->ports_down = 0;
    pPS->ports_active = 0;
    pPS->ports_disabled = 0;
    pPS->ports_1X = 0;
    pPS->ports_4X = 0;
    pPS->ports_8X = 0;
    pPS->ports_12X = 0;
    pPS->ports_unknown_width = 0;
    pPS->ports_unenabled_width = 0;
    pPS->ports_reduced_width = 0;
    pPS->ports_sdr = 0;
    pPS->ports_ddr = 0;
    pPS->ports_qdr = 0;
    pPS->ports_fdr10 = 0;
    pPS->ports_fdr = 0;
    pPS->ports_edr = 0;
    pPS->ports_unknown_speed = 0;
    pPS->ports_unenabled_speed = 0;
    pPS->ports_reduced_speed = 0;
    pPS++;
  }
  return pPortDescription;
}

jsi_PortStats_t *
sr_getPortStats(int type)
{
  return &(PortStatistics[type - 1]);
}

jsi_SystemInfo_t *
sr_getSysInfo()
{
  return &SystemInfo;
}

jpi_Plugin_t *
sr_getPluginInfo()
{
  return &PluginInfo;
}

char *
sstrncpy(char *dest, const char *src, size_t n)
{
  /* safe string copy */
  /* destination must exist, ... a char[n] array */
  /* source may, or may not exist (can be a null pointer)
   * and can be larger or smaller than dest, and should be
   * null terminated but doesn't have to be.
   *
   * copies at most n chars from src to dest, and null
   * terminates
   */
  if (src == NULL)
    *dest = '\0';
  else
  {
    strncpy(dest, src, n);
    dest[n - 1] = '\0';
  }
  return dest;
}

int
pm_getNumPM_Nodes()
{
  return ActualNumberNodes;
}

int
pm_getNumPM_Ports()
{
  return ActualNumberPorts;
}

int
pm_getNumPM_Esp0Ports()
{
  return ActualSwitchPortZero;
}

int
pm_getNumPT_Ports()
{
  return ActualNumberPT_Ports;
}

int
pm_getNumPT_Nodes()
{
  return ActualNumberPT_Nodes;
}

int
sr_getNumSwitches()
{
  return ActualNumberSwitches;
}
int
sr_getNumRouters()
{
  return ActualNumberRouters;
}
int
sr_getNumManagers()
{
  return ActualNumberManagers;
}
int
sr_getNumPKeys()
{
  return ActualNumberPartitions;
}
int
sr_getNumMCGroups()
{
  return ActualNumberMCGroups;
}

pt_Node_t *
pm_getPT_Nodes()
{
  return &AllPT_Nodes[0];
}
pm_Node_t *
pm_getPM_Nodes()
{
  return &AllNodes[0];
}

pt_Port_t *
pm_getPT_Ports()
{
  return &AllPT_Ports[0];
}
pm_Port_t *
pm_getPM_Ports()
{
  return &AllPorts[0];
}

jst_Stats_t *
sr_getStats()
{
  return &OSM_Stats;
}

sr_Subnet_t *
sr_getSubnet()
{
  return &OSM_Subnet;
}

sr_Options_t *
sr_getOptions()
{
  return &OSM_Options;
}

sr_Switch_t *
sr_getSwitches()
{
  return &All_Switches[0];
}

sr_Router_t *
sr_getRouters()
{
  return &All_Routers[0];
}

sr_Manager_t *
sr_getManagers()
{
  return &All_Managers[0];
}

sr_PKey_t *
sr_getPKeys()
{
  return &All_PKeys[0];
}

sr_MCGroups_t *
sr_getMCGroups()
{
  return &All_MCGroups[0];
}

/******************************************************************************
 *** Function: jsr_waitForNextEvent
 ***
 *** Returns the oldest event from the event queue.  If the event queue is empty,
 *** it waits until the next event occurs or until the timeout expires.
 *** <p>
 ***
 ***   Parameters:
 ***
 ***   Returns:
 ***
 ******************************************************************************/
int
jsr_waitForNextEvent(void * pJenv, int timeOutInMs)
{
  int eventType = -2;
  int secs = timeOutInMs / 1000;
  jsr_OsmEvent osmEvent;
  long num = 0;

  /* if the queue is non-empty, remove one and return immediately */
  /* else, wait here until a signal arrives indicating a non-empty queue */
  EventTimeout = timeOutInMs;
  if (FQ_isEmpty(pEventQueue))
    eventType = thread_wait(gData, secs);

  // we either timed out, got a signal, or queue is not empty
  FQ_lock(pEventQueue);
  num = FQ_remove(pEventQueue, &osmEvent);
  FQ_unlock(pEventQueue);

  return osmEvent.EventId;
}

/******************************************************************************
 *** Function: jsr_signalNextEvent
 ***
 *** adds the current event to the queue, and signals any waiting threads.
 ***
 *** NOTE:  This Queue will block on removes (if empty), but the adds will either
 *** work, or return an error because the queue is full.  It does not block based
 *** on the queue, but could block (pause) via the queue bookkeeping locks.  In
 *** any case, this should be fast.
 ***
 *** <p>
 ***
 ***   Parameters:
 ***
 ***   Returns:
 ***
 ******************************************************************************/
int
jsr_signalNextEvent(jsr_OsmEvent * pEvent)
{
  int status = 0;
  long num = 0;

  /* add a copy of the provided event object to the queue.  If the
   * queue is full, return -1, indicating it did not add, otherwise
   * return the number in the queue.
   */
  FQ_lock(pEventQueue);
  num = FQ_add(pEventQueue, pEvent);
  EventCount++;
  FQ_unlock(pEventQueue);

  // signal the non-emptyness to waiting threads
  thread_signal(gData);
  return status;
}

/* TODO - copied from osm_console.c, eliminate redundancy */
static const char *
sm_state_str(int state)
{
  switch (state)
  {
    case IB_SMINFO_STATE_DISCOVERING:
      return "Discovering";
    case IB_SMINFO_STATE_STANDBY:
      return "Standby    ";
    case IB_SMINFO_STATE_NOTACTIVE:
      return "Not Active ";
    case IB_SMINFO_STATE_MASTER:
      return "Master     ";
  }
  return "UNKNOWN    ";
}
/* TODO - copied from osm_console.c, eliminate redundancy */
static const char *
sa_state_str(osm_sa_state_t state)
{
  switch (state)
  {
    case OSM_SA_STATE_INIT:
      return "Init";
    case OSM_SA_STATE_READY:
      return "Ready";
  }
  return "UNKNOWN";
}

/* TODO - copied from osm_console.c, refer to __tag_port_report() */
static void
setPortDesc(jsi_PortDesc_t ** head, uint64_t node_guid, uint8_t port_num, char *print_desc)
{
  /* grab the next available mem location from the PortDesciption array pool */
  jsi_PortDesc_t * pPortDesc = getNextAvailPortDesc();
  pPortDesc->node_guid = node_guid;
  pPortDesc->port_num = port_num;
  memcpy(pPortDesc->print_desc, print_desc, IB_NODE_DESCRIPTION_SIZE + 1);
  pPortDesc->next = NULL;
  if (*head)
  {
    pPortDesc->next = *head;
    *head = pPortDesc;
  }
  else
    *head = pPortDesc;
}

/**
 * iterator function to get portstatus on each node (copied directly from osm_console.c __get_stats() )
 */
static void
getPortStats(cl_map_item_t * const p_map_item, void *context)
{
  jsi_PortStats_t *fs = (jsi_PortStats_t *) context;
  osm_node_t *node = (osm_node_t *) p_map_item;
  osm_physp_t *physp0;
  ib_port_info_t *pi0;
  uint8_t num_ports = osm_node_get_num_physp(node);
  uint8_t port = 0;

  /* Skip nodes we are not interested in */
  if (fs->node_type_lim != 0 && fs->node_type_lim != node->node_info.node_type)
    return;

  fs->total_nodes++;

  if (osm_node_get_type(node) == IB_NODE_TYPE_SWITCH)
  {
    physp0 = osm_node_get_physp_ptr(node, 0);
    pi0 = &physp0->port_info;
  }
  else
    pi0 = NULL;

  for (port = 1; port < num_ports; port++)
  {
    osm_physp_t *phys = osm_node_get_physp_ptr(node, port);
    ib_port_info_t *pi = NULL;
    ib_mlnx_ext_port_info_t *epi = NULL;
    uint8_t active_speed = 0;
    uint8_t enabled_speed = 0;
    uint8_t active_width = 0;
    uint8_t enabled_width = 0;
    uint8_t port_state = 0;
    uint8_t port_phys_state = 0;

    if (!phys)
      continue;

    pi = &phys->port_info;
    epi = &phys->ext_port_info;
    if (!pi0)
      pi0 = pi;
    active_speed = ib_port_info_get_link_speed_active(pi);
    enabled_speed = ib_port_info_get_link_speed_enabled(pi);
    active_width = pi->link_width_active;
    enabled_width = pi->link_width_enabled;
    port_state = ib_port_info_get_port_state(pi);
    port_phys_state = ib_port_info_get_port_phys_state(pi);

    if (port_state == IB_LINK_DOWN)
      fs->ports_down++;
    else if (port_state == IB_LINK_ACTIVE)
      fs->ports_active++;
    if (port_phys_state == IB_PORT_PHYS_STATE_DISABLED)
    {
      setPortDesc(&(fs->disabled_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
      fs->ports_disabled++;
    }

    fs->total_ports++;

    if (port_state == IB_LINK_DOWN)
      continue;

    if (!(active_width & enabled_width))
    {
      setPortDesc(&(fs->unenabled_width_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
      fs->ports_unenabled_width++;
    }
    else if ((enabled_width ^ active_width) > active_width)
    {
      setPortDesc(&(fs->reduced_width_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
      fs->ports_reduced_width++;
    }

    /* unenabled speed usually due to problems with force_link_speed */
    if (!(active_speed & enabled_speed))
    {
      setPortDesc(&(fs->unenabled_speed_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
      fs->ports_unenabled_speed++;
    }
    else if ((enabled_speed ^ active_speed) > active_speed)
    {
      setPortDesc(&(fs->reduced_speed_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
      fs->ports_reduced_speed++;
    }

    switch (active_speed)
    {
      case IB_LINK_SPEED_ACTIVE_2_5:
        fs->ports_sdr++;
        break;
      case IB_LINK_SPEED_ACTIVE_5:
        fs->ports_ddr++;
        break;
      case IB_LINK_SPEED_ACTIVE_10:
        if (!(pi0->capability_mask & IB_PORT_CAP_HAS_EXT_SPEEDS)
            || ((pi0->capability_mask & IB_PORT_CAP_HAS_EXT_SPEEDS) && !ib_port_info_get_link_speed_ext_active(pi)))
        {
          if (epi->link_speed_active & FDR10)
            fs->ports_fdr10++;
          else
          {
            fs->ports_qdr++;
            /* check for speed reduced from FDR10 */
            if (epi->link_speed_enabled & FDR10)
            {
              setPortDesc(&(fs->reduced_speed_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
              fs->ports_reduced_speed++;
            }
          }
        }
        break;
//      case IB_LINK_SPEED_ACTIVE_EXTENDED:
      case 0:  // TODO, fix with above
        break;
      default:
//        setPortDesc(&(fs->unknown_speed_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
        fs->ports_unknown_speed++;
        break;
    }
    if (pi0->capability_mask & IB_PORT_CAP_HAS_EXT_SPEEDS && ib_port_info_get_link_speed_ext_sup(pi) &&
    (enabled_speed = ib_port_info_get_link_speed_ext_enabled(pi)) != IB_LINK_SPEED_EXT_DISABLE &&
    active_speed == IB_LINK_SPEED_ACTIVE_10)
    {
      active_speed = ib_port_info_get_link_speed_ext_active(pi);
      if (!(active_speed & enabled_speed))
      {
        setPortDesc(&(fs->unenabled_speed_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
        fs->ports_unenabled_speed++;
      }
      else if ((enabled_speed ^ active_speed) > active_speed)
      {
        setPortDesc(&(fs->reduced_speed_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
        fs->ports_reduced_speed++;
      }
      switch (active_speed)
      {
        case IB_LINK_SPEED_EXT_ACTIVE_14:
          fs->ports_fdr++;
          break;
        case IB_LINK_SPEED_EXT_ACTIVE_25:
          fs->ports_edr++;
          break;
        case IB_LINK_SPEED_EXT_ACTIVE_NONE:
          break;
        default:
//          setPortDesc(&(fs->unknown_speed_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
          fs->ports_unknown_speed++;
          break;
      }
    }
    switch (active_width)
    {
      case IB_LINK_WIDTH_ACTIVE_1X:
        fs->ports_1X++;
        break;
      case IB_LINK_WIDTH_ACTIVE_4X:
        fs->ports_4X++;
        break;
      case IB_LINK_WIDTH_ACTIVE_8X:
        fs->ports_8X++;
        break;
      case IB_LINK_WIDTH_ACTIVE_12X:
        fs->ports_12X++;
        break;
      default:
        //          setPortDesc(&(fs->unknown_width_ports), cl_ntoh64(node->node_info.node_guid), port, node->print_desc);
        fs->ports_unknown_width++;
        break;
    }
  }
}

static void
getPortStatistics(void)
{
  osm_opensm_t * p_osm = gData->p_osm;
  int ndex = 0;
  jsi_PortStats_t *fs;

  resetPortDesc(); /* reset the pointers and clear the data structs */

  /* 0 is unknown, 1 is CA, 2 is SW, and 3 is IB_NODE_TYPE_ROUTER, but I am indexing them one less */

  cl_plock_acquire(&p_osm->lock);
  for (ndex = 0; ndex < IB_NODE_TYPE_ROUTER; ndex++)
  {
    fs = &(PortStatistics[ndex]);
    fs->node_type_lim = ndex + 1;

    cl_qmap_apply_func(&(p_osm->subn.node_guid_tbl), getPortStats, (void *) fs);
  }
  cl_plock_release(&p_osm->lock);
}

static void
sr_getSystemInfo()
{
  osm_opensm_t * p_osm = gData->p_osm;
  SystemInfo.numPlugins = 0;

  cl_list_item_t *item;
  const char *re_str;

  cl_plock_acquire(&p_osm->lock);

  re_str =
      p_osm->routing_engine_used ? osm_routing_engine_type_str(p_osm->routing_engine_used->type) :
          osm_routing_engine_type_str(OSM_ROUTING_ENGINE_TYPE_NONE);

  SystemInfo.SM_Priority = p_osm->subn.opt.sm_priority;
  SystemInfo.PM_SweepTime = osm_perfmgr_get_sweep_time_s(&p_osm->perfmgr);
  SystemInfo.PM_OutstandingQueries = p_osm->perfmgr.outstanding_queries;
  SystemInfo.PM_MaximumQueries = p_osm->perfmgr.max_outstanding_queries;

  sstrncpy(SystemInfo.OpenSM_Version, p_osm->osm_version, MAX_STRING_SIZE);
  sstrncpy(SystemInfo.OsmJpi_Version, jsr_getPluginVersion(), MAX_STRING_SIZE);
  sstrncpy(SystemInfo.SM_State, sm_state_str(p_osm->subn.sm_state),
  MAX_STRING_SIZE);
  sstrncpy(SystemInfo.SA_State, sa_state_str(p_osm->sa.state), MAX_STRING_SIZE);
  sstrncpy(SystemInfo.PM_State, osm_perfmgr_get_state_str(&p_osm->perfmgr),
  MAX_STRING_SIZE);
  sstrncpy(SystemInfo.PM_SweepState, osm_perfmgr_get_sweep_state_str(&p_osm->perfmgr), MAX_STRING_SIZE);
  sstrncpy(SystemInfo.RoutingEngine, re_str, MAX_STRING_SIZE);

  /* count the plugins, should be at least one, me! */
  if (cl_qlist_head(&p_osm->plugin_list) == cl_qlist_end(&p_osm->plugin_list))
  {
    J_LOG(gData, OSM_LOG_ERROR, "Plugin count is ZERO, should be at least one!\n");
  }
  for (item = cl_qlist_head(&p_osm->plugin_list); item != cl_qlist_end(&p_osm->plugin_list); item = cl_qlist_next(item))
  {
    sstrncpy(&(SystemInfo.EventPlugins[SystemInfo.numPlugins][0]), ((osm_epi_plugin_t *) item)->plugin_name,
    MAX_STRING_SIZE);
    SystemInfo.numPlugins++;
    if (SystemInfo.numPlugins >= MAX_NUM_PLUGINS)
    {
      J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER PLUGINS REACHED - ARE YOU KIDDING ME?** (%d)\n", MAX_NUM_PLUGINS);
      break;
    }
  }
  cl_plock_release(&p_osm->lock);
}

/******************************************************************************
 *** Function: jsr_initialize
 ***
 *** This method is responsible for obtaining and initializing all persistent resources
 *** (memory) that will be shared between different execution threads.
 *** <p>
 ***
 ***
 *** <dt><b>Miscellaneous Comments:</b></dt>
 ***   <dd>It is expected that this method is called only once, before its
 ***       resources are needed.
 *** </dd>
 ***
 *** <dt><b>External or Global Variables:</b></dt>
 ***   <dd>None
 *** </dd>
 ***
 ***  Parameters: none
 ***
 ***  Returns: 1 is returned if initialization was successful, and zero otherwise
 ***
 ******************************************************************************/
int
jsr_initialize()
{
  // initialize parts of the data structures that need default values

  if (jsr_isInitialized != 1)
  {
    pEventQueue = FQ_init(sizeof(jsr_OsmEvent), EVENT_QUEUE_SIZE);
    jsr_isInitialized = 1;
  }
  return jsr_isInitialized;
}
/*-----------------------------------------------------------------------*/

/**********************************************************************
 * Internal call db->lock should be held when calling
 **********************************************************************/
static inline db_node_t *
getNodeFromDb(perfmgr_db_t * db, uint64_t guid)
{
  cl_map_item_t *rc = cl_qmap_get(&db->pc_data, guid);
  const cl_map_item_t *end = cl_qmap_end(&db->pc_data);
  cl_map_item_t *rnd = cl_qmap_head(&db->pc_data);
  int count = cl_qmap_count(&db->pc_data);
  db_node_t * p_node = NULL;

  J_LOG(gData, OSM_LOG_INFO, "Node Map Size is (%d)\n", count);
  if ((count > 0) && (rnd != NULL))
  {
    p_node = (db_node_t *) rnd;
    J_LOG(gData, OSM_LOG_INFO, "Node guid 0x%" PRIx64 "\n", p_node->node_guid);
    J_LOG(gData, OSM_LOG_INFO, "Number of ports %d\n", p_node->num_ports);
    J_LOG(gData, OSM_LOG_INFO, "Name of node: %s\n", p_node->node_name);
  }

  if (rc == end)
    return NULL;
  return (db_node_t *) rc;
}

static inline perfmgr_db_err_t
bad_node_port(db_node_t * node, uint8_t port)
{
  if (!node)
    return PERFMGR_EVENT_DB_GUIDNOTFOUND;
  if (port >= node->num_ports || (!node->esp0 && port == 0))
    return PERFMGR_EVENT_DB_PORTNOTFOUND;
  return PERFMGR_EVENT_DB_SUCCESS;
}

const char*
jsr_getOsmState()
{
  osm_opensm_t * p_osm = gData->p_osm;
  osm_sm_t * p_sm = &(p_osm->sm);

  // this appears to be master/slave, not what I was looking for
  //

  // should be, up, down, sweeping, routing, etc...
  return osm_get_sm_mgr_state_str(p_sm->p_subn->sm_state);
}

int
pm_printPM_Nodes()
{
  int n_ndex = 0;
  /* TODO these need locks */
  pm_Node_t * p_pmNode = &AllNodes[0];

  J_LOG(gData, OSM_LOG_DEBUG, "Total # of PM Nodes: %d\n", ActualNumberNodes);
  for (n_ndex = 0; n_ndex < ActualNumberNodes; n_ndex++, p_pmNode++)
  {
    /*   */
    J_LOG(gData, OSM_LOG_INFO, "PM Node: %d\n", n_ndex);
    J_LOG(gData, OSM_LOG_INFO, "PM Node guid 0x%" PRIxLEAST64 "\n", p_pmNode->node_guid);
    J_LOG(gData, OSM_LOG_INFO, "PM Number of ports %d\n", p_pmNode->num_ports);
    J_LOG(gData, OSM_LOG_INFO, "PM Name of node: %s\n", p_pmNode->node_name);
  }
  return n_ndex;
}

int
pm_printPM_Ports()
{
  int p_ndex = 0;
  int c_ndex = 0;
  char *timeStr;

  /* TODO these need locks */
  pm_Port_t * p_pmPort = &AllPorts[0];

  J_LOG(gData, OSM_LOG_DEBUG, "Total # of PM Ports: %d\n", ActualNumberPorts);
  for (p_ndex = 0; p_ndex < ActualNumberPorts; p_ndex++, p_pmPort++)
  {
    /*   */
    J_LOG(gData, OSM_LOG_INFO, "PM Port: %d\n", p_ndex);
    J_LOG(gData, OSM_LOG_INFO, "PM Port guid: 0x%" PRIxLEAST64 "\n", p_pmPort->node_guid);
    J_LOG(gData, OSM_LOG_INFO, "PM Port Num :%d\n", p_pmPort->port_num);

    /* print out the name and values */
    for (c_ndex = 0; c_ndex < NUM_PORT_COUNTERS; c_ndex++)
    {
      J_LOG(gData, OSM_LOG_INFO, "    %s: = %" PRIu64 "\n", portCounterNames[c_ndex], p_pmPort->port_counters[c_ndex]);
    }

    timeStr = ctime(&p_pmPort->counter_ts);
    timeStr[strlen(timeStr) - 1] = '\0'; /* remove \n */
    J_LOG(gData, OSM_LOG_INFO, "    Counter timestamp: %s\n", timeStr);
    timeStr = ctime(&p_pmPort->error_ts);
    timeStr[strlen(timeStr) - 1] = '\0'; /* remove \n */
    J_LOG(gData, OSM_LOG_INFO, "    Error   timestamp: %s\n", timeStr);

    /* TODO print out the three timestamps */
//    p_pmPort->counter_ts;
//    p_pmPort->error_ts;
//    p_pmPort->wait_ts;
  }
  return p_ndex;
}

int
pt_printPT_Nodes()
{
  int n_ndex = 0;
  ib_node_info_t * p_ni;

  /* TODO these need locks */
  pt_Node_t * p_ptNode = &AllPT_Nodes[0];

  J_LOG(gData, OSM_LOG_DEBUG, "Total # of PT Nodes: %d\n", ActualNumberPT_Nodes);
  for (n_ndex = 0; n_ndex < ActualNumberPT_Nodes; n_ndex++, p_ptNode++)
  {
    p_ni = &(p_ptNode->node_info);

    J_LOG(gData, OSM_LOG_INFO,
        "NodeInfo dump:\n" "\t\t\t\tbase_version............0x%X\n" "\t\t\t\tclass_version...........0x%X\n" "\t\t\t\tnode_type...............%s\n" "\t\t\t\tnum_ports...............%u\n" "\t\t\t\tsys_guid................0x%016" PRIx64 "\n" "\t\t\t\tnode_guid...............0x%016" PRIx64 "\n" "\t\t\t\tport_guid...............0x%016" PRIx64 "\n" "\t\t\t\tpartition_cap...........0x%X\n" "\t\t\t\tdevice_id...............0x%X\n" "\t\t\t\trevision................0x%X\n" "\t\t\t\tport_num................%u\n" "\t\t\t\tvendor_id...............0x%X\n" "\t\t\t\tNodeDescription\n" "\t\t\t\t%s\n",
        p_ni->base_version, p_ni->class_version, ib_get_node_type_str(p_ni->node_type), p_ni->num_ports,
        cl_ntoh64(p_ni->sys_guid), cl_ntoh64(p_ni->node_guid), cl_ntoh64(p_ni->port_guid),
        cl_ntoh16(p_ni->partition_cap), cl_ntoh16(p_ni->device_id), cl_ntoh32(p_ni->revision),
        ib_node_info_get_local_port_num(p_ni), cl_ntoh32(ib_node_info_get_vendor_id(p_ni)), p_ptNode->description);

  }
  return n_ndex;
}

int
pt_getPortTable()
{
  /* collect all the ports from the node table.  Each node contains a
   * list of their ports, so iterate through the node table and
   * aggregate all the information about ports into a single all
   * inclusive array of ports.
   *
   * Use the information from the PortTable when it seems more accurate
   * for example getting the lids.
   *
   * refer to the original version (pt_getPortTableOrig())
   */
  int rc = 0;
  int j = 0;
  osm_opensm_t * p_osm = gData->p_osm;
  osm_node_t * p_node = NULL;
  int n_ndex = 0;
  osm_physp_t * pp_port = NULL;
  int p_ndex = 0;

  /* TODO this need locks */
  pt_Node_t * p_ptNode = &AllPT_Nodes[0];
  pt_Port_t * p_ptPort = &AllPT_Ports[0];

  /* loop through the node table and copy the info */

  CL_PLOCK_ACQUIRE(p_osm->sm.p_lock);
  cl_qmap_t *p_node_guid_tbl = &(p_osm->sm.p_subn->node_guid_tbl);
  cl_qmap_t *p_port_guid_tbl = &(p_osm->sm.p_subn->port_guid_tbl);

  osm_node_t * p_next_node = (osm_node_t *) cl_qmap_head(p_node_guid_tbl);
  while ((p_next_node != (osm_node_t *) cl_qmap_end(p_node_guid_tbl)) && (n_ndex < MAX_NUM_NODES))
  {
    p_node = p_next_node;
    p_next_node = (osm_node_t *) cl_qmap_next(&p_next_node->map_item);

    p_ptNode->node_info = p_node->node_info;
    /* use the print_desc instead of description, because description may not be unique or valid */
//      sstrncpy((char*) p_ptNode->description, (char*) p_node->node_desc.description, sizeof(p_ptNode->description));
    sstrncpy((char*) p_ptNode->description, (char*) p_node->print_desc, sizeof(p_ptNode->description));

    /* done getting node table information, now get the ports for this node */
    for (j = 1; (j < p_node->physp_tbl_size) && (p_ndex < MAX_NUM_PORTS); j++)
    {
      /* the physical port table is indexed by port number.  Since there is
       * no port zero, I can start at 1.
       */
      pp_port = osm_node_get_physp_ptr(p_node, j);
      if (pp_port != NULL)
      {

        osm_port_t * p_port = (osm_port_t *) cl_qmap_get(p_port_guid_tbl, pp_port->port_guid);
        ib_port_info_t * p_pi = NULL;
        if (p_port != NULL)
        {
          /* found a port, so extract the port info */
          p_pi = &(p_port->p_physp->port_info);

        }

        if (pp_port->p_node != NULL)
          p_ptPort->node_guid = pp_port->p_node->node_info.node_guid;
        else
          p_ptPort->node_guid = p_ptNode->node_info.node_guid;

        p_ptPort->port_guid = pp_port->port_guid;
        p_ptPort->port_info = pp_port->port_info;
        p_ptPort->port_num = pp_port->port_num;

        /* copy the desired melanox extended members */
        p_ptPort->ext_port_info.link_speed_active = pp_port->ext_port_info.link_speed_active;
        p_ptPort->ext_port_info.link_speed_enabled = pp_port->ext_port_info.link_speed_enabled;
        p_ptPort->ext_port_info.link_speed_supported = pp_port->ext_port_info.link_speed_supported;
        p_ptPort->ext_port_info.state_change_enable = pp_port->ext_port_info.state_change_enable;

//              J_LOG(gData, OSM_LOG_ERROR, "Compare two LIDS   : (%d & %d) %u and %u\n", pp_port->port_num, j, cl_ntoh16(pp_port->port_info.base_lid), cl_ntoh16(p_pi->base_lid));
//              J_LOG(gData, OSM_LOG_ERROR, "Compare two SM LIDS: (%d & %d) %u and %u\n", pp_port->port_num, j, cl_ntoh16(pp_port->port_info.master_sm_base_lid), cl_ntoh16(p_pi->master_sm_base_lid));

        // the port info from the node table is mostly right, but not the lids, so replace them here
        if (p_pi != NULL)
        {
          p_ptPort->port_info.base_lid = p_pi->base_lid;
          p_ptPort->port_info.master_sm_base_lid = p_pi->master_sm_base_lid;
        }

        // the "linked" port is the remote one, if it exists
        osm_physp_t * rp_port = pp_port->p_remote_physp;

        if (rp_port != NULL)
        {
          /* found a remote port, so extract the remote ports identity */
          p_ptPort->linked_node_guid = rp_port->p_node->node_info.node_guid;
          p_ptPort->linked_port_guid = rp_port->port_guid;
          p_ptPort->linked_port_num = rp_port->port_num;
        }
        else
        {
          // set everything to NULL?
        }
        /* increment both the port index counter and pointer */
        p_ndex++;
        p_ptPort++;
      }
    }

    n_ndex++;
    p_ptNode++;
  }

  CL_PLOCK_RELEASE(p_osm->sm.p_lock);
  ActualNumberPT_Nodes = n_ndex;
  ActualNumberPT_Ports = p_ndex;

  if (p_ndex >= MAX_NUM_PORTS)
    J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER PORTS REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_PORTS);

  if (n_ndex >= MAX_NUM_NODES)
    J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER NODES REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_NODES);

  return rc;
}
int
pt_printPT_Ports()
{
  int p_ndex = 0;
  ib_port_info_t * p_pi;

  /* TODO these need locks */
  pt_Port_t * p_ptPort = &AllPT_Ports[0];

  J_LOG(gData, OSM_LOG_DEBUG, "Total # of PT Ports: %d\n", ActualNumberPT_Ports);
  for (p_ndex = 0; p_ndex < ActualNumberPT_Ports; p_ndex++, p_ptPort++)
  {
    p_pi = &(p_ptPort->port_info);
    /*   */
    J_LOG(gData, OSM_LOG_INFO, "PT Port: %d\n", p_ndex);
    J_LOG(gData, OSM_LOG_INFO, "PT Node guid: 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_ptPort->node_guid));
    J_LOG(gData, OSM_LOG_INFO, "PT Port guid: 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_ptPort->port_guid));
    J_LOG(gData, OSM_LOG_INFO, "PT Port Num :%d\n", p_ptPort->port_num);
    J_LOG(gData, OSM_LOG_INFO, "PT L Node guid: 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_ptPort->linked_node_guid));
    J_LOG(gData, OSM_LOG_INFO, "PT L Port guid: 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_ptPort->linked_port_guid));
    J_LOG(gData, OSM_LOG_INFO, "PT L Port Num :%d\n", p_ptPort->linked_port_num);

    J_LOG(gData, OSM_LOG_INFO,
        "(pt_print)PortInfo dump:\n" "\t\t\t\tm_key...................0x%016" PRIx64 "\n" "\t\t\t\tsubnet_prefix...........0x%016" PRIx64 "\n" "\t\t\t\tbase_lid................%u\n" "\t\t\t\tbase_lid................%u\n" "\t\t\t\tmaster_sm_base_lid......%u\n" "\t\t\t\tcapability_mask.........0x%X\n" "\t\t\t\tcapability_mask2........0x%X\n" "\t\t\t\tdiag_code...............0x%X\n" "\t\t\t\tm_key_lease_period......0x%X\n" "\t\t\t\tlocal_port_num..........%u\n" "\t\t\t\tlink_width_enabled......0x%X\n" "\t\t\t\tlink_width_supported....0x%X\n" "\t\t\t\tlink_width_active.......0x%X\n" "\t\t\t\tlink_speed_supported....0x%X\n" "\t\t\t\tport_state..............%s\n" "\t\t\t\tstate_info2.............0x%X\n" "\t\t\t\tm_key_protect_bits......0x%X\n" "\t\t\t\tlmc.....................0x%X\n" "\t\t\t\tlink_speed..............0x%X\n" "\t\t\t\tlink_speed_ext..........0x%X\n" "\t\t\t\tlink_speed_ext_enabled..0x%X\n" "\t\t\t\tmtu_smsl................0x%X\n" "\t\t\t\tvl_cap_init_type........0x%X\n" "\t\t\t\tvl_high_limit...........0x%X\n" "\t\t\t\tvl_arb_high_cap.........0x%X\n" "\t\t\t\tvl_arb_low_cap..........0x%X\n" "\t\t\t\tinit_rep_mtu_cap........0x%X\n" "\t\t\t\tvl_stall_life...........0x%X\n" "\t\t\t\tvl_enforce..............0x%X\n" "\t\t\t\tm_key_violations........0x%X\n" "\t\t\t\tp_key_violations........0x%X\n" "\t\t\t\tq_key_violations........0x%X\n" "\t\t\t\tguid_cap................0x%X\n" "\t\t\t\tclient_reregister.......0x%X\n" "\t\t\t\tmcast_pkey_trap_suppr...0x%X\n" "\t\t\t\tsubnet_timeout..........0x%X\n" "\t\t\t\tresp_time_value.........0x%X\n" "\t\t\t\terror_threshold.........0x%X\n" "\t\t\t\tmax_credit_hint.........0x%X\n" "\t\t\t\tlink_round_trip_latency.0x%X\n",
        cl_ntoh64(p_pi->m_key), cl_ntoh64(p_pi->subnet_prefix), cl_ntoh16(p_pi->base_lid), p_pi->base_lid,
        cl_ntoh16(p_pi->master_sm_base_lid), cl_ntoh32(p_pi->capability_mask), cl_ntoh32(p_pi->capability_mask2),
        cl_ntoh16(p_pi->diag_code), cl_ntoh16(p_pi->m_key_lease_period), p_pi->local_port_num, p_pi->link_width_enabled,
        p_pi->link_width_supported, p_pi->link_width_active, ib_port_info_get_link_speed_sup(p_pi),
        ib_get_port_state_str(ib_port_info_get_port_state(p_pi)), p_pi->state_info2, ib_port_info_get_mpb(p_pi),
        ib_port_info_get_lmc(p_pi), p_pi->link_speed, p_pi->link_speed_ext, p_pi->link_speed_ext_enabled,
        p_pi->mtu_smsl, p_pi->vl_cap, p_pi->vl_high_limit, p_pi->vl_arb_high_cap, p_pi->vl_arb_low_cap, p_pi->mtu_cap,
        p_pi->vl_stall_life, p_pi->vl_enforce, cl_ntoh16(p_pi->m_key_violations), cl_ntoh16(p_pi->p_key_violations),
        cl_ntoh16(p_pi->q_key_violations), p_pi->guid_cap, ib_port_info_get_client_rereg(p_pi),
        ib_port_info_get_mcast_pkey_trap_suppress(p_pi), ib_port_info_get_timeout(p_pi), p_pi->resp_time_value,
        p_pi->error_threshold, cl_ntoh16(p_pi->max_credit_hint), cl_ntoh32(p_pi->link_rt_latency));

  }
  return p_ndex;
}

int
sr_getManagerTable()
{
  int rc = 0;
  osm_opensm_t * p_osm = gData->p_osm;
  osm_subn_t *p_subn = &p_osm->subn;
  osm_remote_sm_t *p_rsm;
  int n_ndex = 0;

  sr_Manager_t * p_Manager = sr_getManagers();

  /* copy the primary sm first, then do remotes */
  p_Manager->guid = cl_ntoh64(p_subn->sm_port_guid);
  p_Manager->pri_state = (jshort) p_subn->opt.sm_priority;

  sstrncpy(p_Manager->State, sm_state_str(p_subn->sm_state), sizeof(p_Manager->State));

  n_ndex++;
  p_Manager++;

  CL_PLOCK_ACQUIRE(p_osm->sm.p_lock);
  p_rsm = (osm_remote_sm_t *) cl_qmap_head(&p_subn->sm_guid_tbl);
  while ((p_rsm != (osm_remote_sm_t *) cl_qmap_end(&p_subn->sm_guid_tbl)) && (n_ndex < MAX_NUM_MANAGERS))
  {
    p_Manager->guid = cl_ntoh64(p_rsm->smi.guid);
    p_Manager->pri_state = (jshort) ib_sminfo_get_priority(&p_rsm->smi); // just priority, state is in string
    p_Manager->sm_key = (jlong) p_rsm->smi.sm_key;
    p_Manager->act_count = (jint) p_rsm->smi.act_count;

    sstrncpy(p_Manager->State, sm_state_str(ib_sminfo_get_state(&p_rsm->smi)), sizeof(p_Manager->State));

    p_rsm = (osm_remote_sm_t *) cl_qmap_next(&p_rsm->map_item);
    n_ndex++;
    p_Manager++;
  }
  if (n_ndex >= MAX_NUM_MANAGERS)
    J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER MANAGERS REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_MANAGERS);

  CL_PLOCK_RELEASE(p_osm->sm.p_lock);
  ActualNumberManagers = n_ndex;
  return rc;
}

int
sr_getRouterTable()
{
  int rc = 0;
  osm_opensm_t * p_osm = gData->p_osm;
  osm_router_t * p_router = NULL;
  int n_ndex = 0;

  sr_Router_t * p_Router = sr_getRouters();

  CL_PLOCK_ACQUIRE(p_osm->sm.p_lock);
  cl_qmap_t *p_router_guid_tbl = &(p_osm->sm.p_subn->rtr_guid_tbl);

  /* normally routers are rare, check for the tables existence */
  if (p_router_guid_tbl != NULL)
  {
    /* loop through the table and copy the info */
    osm_router_t * p_next_router = (osm_router_t *) cl_qmap_head(p_router_guid_tbl);
    while ((p_next_router != (osm_router_t *) cl_qmap_end(p_router_guid_tbl)) && (n_ndex < MAX_NUM_ROUTERS))
    {
      // this is a guid table, I just want the key (guid)
      p_router = p_next_router;
      p_next_router = (osm_router_t *) cl_qmap_next(&p_next_router->map_item);

      // copy everything I want
      p_Router->guid = cl_ntoh64(cl_qmap_key((cl_map_item_t * )p_router));

      n_ndex++;
      p_Router++;
    }
  }
  if (n_ndex >= MAX_NUM_ROUTERS)
    J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER ROUTERS REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_ROUTERS);

  CL_PLOCK_RELEASE(p_osm->sm.p_lock);
  ActualNumberRouters = n_ndex;
  return rc;
}

int
sr_getSwitchTable(void)
{
  int rc = 0;
  osm_opensm_t * p_osm = gData->p_osm;
  osm_switch_t * p_switch = NULL;
  int n_ndex = 0;

  sr_Switch_t * p_Switch = sr_getSwitches();

  /* loop through the table and copy the info */

  CL_PLOCK_ACQUIRE(p_osm->sm.p_lock);
  cl_qmap_t *p_switch_guid_tbl = &(p_osm->sm.p_subn->sw_guid_tbl);

  J_LOG(gData, OSM_LOG_DEBUG, "Getting Switches now\n");

  osm_switch_t * p_next_switch = (osm_switch_t *) cl_qmap_head(p_switch_guid_tbl);
  while ((p_next_switch != (osm_switch_t *) cl_qmap_end(p_switch_guid_tbl)) && (n_ndex < MAX_NUM_SWITCHES))
  {
    // this is a guid table, I just want the key (guid)
    p_switch = p_next_switch;
    p_next_switch = (osm_switch_t *) cl_qmap_next(&p_next_switch->map_item);

    // copy everything I want
    p_Switch->guid = cl_ntoh64(cl_qmap_key((cl_map_item_t * )p_switch));

    p_Switch->endport_links = p_switch->endport_links;
    p_Switch->is_mc_member = p_switch->is_mc_member;
    p_Switch->lft_size = p_switch->lft_size;
    p_Switch->max_lid_ho = p_switch->max_lid_ho;
    p_Switch->mft_block_num = p_switch->mft_block_num;
    p_Switch->mft_position = p_switch->mft_position;
    p_Switch->need_update = p_switch->need_update;
    p_Switch->num_hops = p_switch->num_hops;
    p_Switch->num_of_mcm = p_switch->num_of_mcm;
    p_Switch->num_ports = p_switch->num_ports;

    /* the hops table, turn it into an array indexed by lid, and value is min hops */
    if (p_switch->hops != NULL)
    {
//     J_LOG(gData, OSM_LOG_INFO, "%d Switch guid 0x%" PRIxLEAST64 " has a good Hop Table, size %d\n", n_ndex, p_Switch->guid, p_Switch->num_hops);
      int n = 0;
      int maxSize = p_Switch->num_hops > MAX_NUM_NODES ? MAX_NUM_NODES : p_Switch->num_hops;
      for (n = 0; n < maxSize; n++)
      {
        // in this case, n is the lid, and the value should be the min # hops, or 255 (meaning no path)
        if ((n > p_switch->max_lid_ho) || (!p_switch->hops[n]))
        {
          // bad or empty table should just return 255, which probably corresponds to no LID or no route
          p_Switch->hops[n] = (jshort) 255;
        }
        else
        {
          // more than one entry probably reflects more than one route.  The fist one is all I care about
          p_Switch->hops[n] = (jshort) (p_switch->hops[n][0]);
        }
      }
    }

    /* the linear forwarding table, its an array (index is lid, value is port) */
    if (p_switch->lft != NULL)
    {
//       J_LOG(gData, OSM_LOG_INFO, "%d Switch guid 0x%" PRIxLEAST64 " has a good LFT, size %d\n", n_ndex, p_Switch->guid, p_Switch->lft_size);
      /* just copy the known size, leave the rest */
      int n = 0;
      int maxSize = p_Switch->lft_size > MAX_NUM_NODES ? MAX_NUM_NODES : p_Switch->lft_size;
      uint8_t* pptr = p_switch->lft;
      for (n = 0; n < maxSize; n++)
      {
        // in this case, n is the lid, and the value should be the port number, or 255 (meaning no path)
        p_Switch->lft[n] = (jshort) *pptr;
        pptr++;
      }
    }

    if (p_switch->new_lft != NULL)
    {
      p_Switch->new_lft = (uint8_t) *(p_switch->new_lft);
    }
    n_ndex++;    // the switch counter
    p_Switch++;  // the switch pointer
  }
  if (n_ndex >= MAX_NUM_SWITCHES)
    J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER SWITCHES REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_SWITCHES);

  CL_PLOCK_RELEASE(p_osm->sm.p_lock);
  ActualNumberSwitches = n_ndex;
  return rc;
}

int
sr_getStatistics()
{
  int rc = 0;
  osm_stats_t * p_stats = &(gData->p_osm->stats);

  CL_PLOCK_ACQUIRE(&(gData->p_osm->lock));

  OSM_Stats.qp0_mads_outstanding = (jlong) p_stats->qp0_mads_outstanding;
  OSM_Stats.qp0_mads_outstanding_on_wire = (jlong) p_stats->qp0_mads_outstanding_on_wire;
  OSM_Stats.qp0_mads_rcvd = (jlong) p_stats->qp0_mads_rcvd;
  OSM_Stats.qp0_mads_sent = (jlong) p_stats->qp0_mads_sent;
  OSM_Stats.qp0_unicasts_sent = (jlong) p_stats->qp0_unicasts_sent;
  OSM_Stats.qp0_mads_rcvd_unknown = (jlong) p_stats->qp0_mads_rcvd_unknown;
  OSM_Stats.sa_mads_outstanding = (jlong) p_stats->sa_mads_outstanding;
  OSM_Stats.sa_mads_rcvd = (jlong) p_stats->sa_mads_rcvd;
  OSM_Stats.sa_mads_sent = (jlong) p_stats->sa_mads_sent;
  OSM_Stats.sa_mads_rcvd_unknown = (jlong) p_stats->sa_mads_rcvd_unknown;
  OSM_Stats.sa_mads_ignored = (jlong) p_stats->sa_mads_ignored;

  CL_PLOCK_RELEASE(&(gData->p_osm->lock));
  return rc;
}

int
sr_getNativePluginInfo(int update_period, int report_period, long count)
{
  /* fill in PluginInfo FIXME - get a legitimate lock, don't use osm */

  int rc = 0;
  PluginInfo.update_period = (jint) update_period;
  PluginInfo.report_period = (jint) report_period;
  PluginInfo.update_count = (jlong) count;

  PluginInfo.event_timeout_ms = (jint) EventTimeout;
  PluginInfo.event_count = (jlong) EventCount;

  return rc;
}

int
sr_getSubnetAttributes()
{
  int rc = 0;
  osm_subn_t * p_subn = &(gData->p_osm->subn);

  CL_PLOCK_ACQUIRE(&(gData->p_osm->lock));

  OSM_Subnet.coming_out_of_standby = (jboolean) p_subn->coming_out_of_standby;
  OSM_Subnet.first_time_master_sweep = (jboolean) p_subn->first_time_master_sweep;
//  OSM_Subnet.set_client_rereg_on_sweep = (jboolean) p_subn->set_client_rereg_on_sweep;
  OSM_Subnet.set_client_rereg_on_sweep = (jboolean) 0;
  OSM_Subnet.ignore_existing_lfts = (jboolean) p_subn->ignore_existing_lfts;
  OSM_Subnet.subnet_initialization_error = (jboolean) p_subn->subnet_initialization_error;
  OSM_Subnet.force_heavy_sweep = (jboolean) p_subn->force_heavy_sweep;
  OSM_Subnet.force_reroute = (jboolean) p_subn->force_reroute;
  OSM_Subnet.in_sweep_hop_0 = (jboolean) p_subn->in_sweep_hop_0;
  OSM_Subnet.sweeping_enabled = (jboolean) p_subn->sweeping_enabled;

  OSM_Subnet.min_ca_mtu = (jshort) p_subn->min_ca_mtu;
  OSM_Subnet.min_ca_rate = (jshort) p_subn->min_ca_rate;
  OSM_Subnet.min_data_vls = (jshort) p_subn->min_data_vls;
  OSM_Subnet.min_sw_data_vls = (jshort) p_subn->min_sw_data_vls;
  OSM_Subnet.need_update = (jshort) p_subn->need_update;
  OSM_Subnet.sm_state = (jshort) p_subn->sm_state;
  OSM_Subnet.last_sm_port_state = (jshort) p_subn->last_sm_port_state;

  OSM_Subnet.max_ucast_lid_ho = (jint) p_subn->max_ucast_lid_ho;
  OSM_Subnet.max_mcast_lid_ho = (jint) p_subn->max_mcast_lid_ho;
  OSM_Subnet.master_sm_base_lid = (jint) p_subn->master_sm_base_lid;
  OSM_Subnet.sm_base_lid = (jint) p_subn->sm_base_lid;

  OSM_Subnet.sm_port_guid = (jlong) cl_ntoh64(p_subn->sm_port_guid);

  CL_PLOCK_RELEASE(&(gData->p_osm->lock));

  return rc;
}

int
sr_getSubnetOptions()
{
  /* copy the subnet options into its peer class */
  int rc = 0;
  osm_subn_opt_t * p_opt = &(gData->p_osm->subn.opt);

  CL_PLOCK_ACQUIRE(&(gData->p_osm->lock));

  /* all the booleans */
  OSM_Options.accum_log_file = (jboolean) p_opt->accum_log_file;
  OSM_Options.lmc_esp0 = (jboolean) p_opt->lmc_esp0;
  OSM_Options.reassign_lids = (jboolean) p_opt->reassign_lids;
  OSM_Options.ignore_other_sm = (jboolean) p_opt->ignore_other_sm;
  OSM_Options.single_thread = (jboolean) p_opt->single_thread;
  OSM_Options.disable_multicast = (jboolean) p_opt->disable_multicast;
  OSM_Options.force_log_flush = (jboolean) p_opt->force_log_flush;
  OSM_Options.force_heavy_sweep = (jboolean) p_opt->force_heavy_sweep;
  OSM_Options.no_partition_enforcement = (jboolean) p_opt->no_partition_enforcement;
  OSM_Options.qos = (jboolean) p_opt->qos;
  OSM_Options.accum_log_file = (jboolean) p_opt->accum_log_file;
  OSM_Options.port_profile_switch_nodes = (jboolean) p_opt->port_profile_switch_nodes;
  OSM_Options.sweep_on_trap = (jboolean) p_opt->sweep_on_trap;
  OSM_Options.use_ucast_cache = (jboolean) p_opt->use_ucast_cache;
  OSM_Options.connect_roots = (jboolean) p_opt->connect_roots;
  OSM_Options.sa_db_dump = (jboolean) p_opt->sa_db_dump;
  OSM_Options.do_mesh_analysis = (jboolean) p_opt->do_mesh_analysis;
  OSM_Options.exit_on_fatal = (jboolean) p_opt->exit_on_fatal;
  OSM_Options.honor_guid2lid_file = (jboolean) p_opt->honor_guid2lid_file;
  OSM_Options.daemon = (jboolean) p_opt->daemon;
  OSM_Options.sm_inactive = (jboolean) p_opt->sm_inactive;
  OSM_Options.babbling_port_policy = (jboolean) p_opt->babbling_port_policy;
  OSM_Options.use_optimized_slvl = (jboolean) p_opt->use_optimized_slvl;
  OSM_Options.enable_quirks = (jboolean) p_opt->enable_quirks;
  OSM_Options.no_clients_rereg = (jboolean) p_opt->no_clients_rereg;
  OSM_Options.perfmgr = (jboolean) p_opt->perfmgr;
  OSM_Options.perfmgr_redir = (jboolean) p_opt->perfmgr_redir;
  OSM_Options.consolidate_ipv6_snm_req = (jboolean) p_opt->consolidate_ipv6_snm_req;
  OSM_Options.m_key_lookup = (jboolean) p_opt->m_key_lookup;
  OSM_Options.allow_both_pkeys = (jboolean) p_opt->allow_both_pkeys;
  OSM_Options.port_shifting = (jboolean) p_opt->port_shifting;
  OSM_Options.remote_guid_sorting = (jboolean) p_opt->remote_guid_sorting;
  OSM_Options.guid_routing_order_no_scatter = (jboolean) p_opt->guid_routing_order_no_scatter;
  OSM_Options.drop_event_subscriptions = (jboolean) p_opt->drop_event_subscriptions;
  OSM_Options.fsync_high_avail_files = (jboolean) p_opt->fsync_high_avail_files;
  OSM_Options.congestion_control = (jboolean) p_opt->congestion_control;
  OSM_Options.perfmgr_ignore_cas = (jboolean) p_opt->perfmgr_ignore_cas;
  OSM_Options.perfmgr_log_errors = (jboolean) p_opt->perfmgr_log_errors;
  OSM_Options.perfmgr_query_cpi = (jboolean) p_opt->perfmgr_query_cpi;
  OSM_Options.perfmgr_xmit_wait_log = (jboolean) p_opt->perfmgr_xmit_wait_log;

  /* all the shorts */
  OSM_Options.sm_priority = (jshort) p_opt->sm_priority;
  OSM_Options.lmc = (jshort) p_opt->lmc;
  OSM_Options.max_op_vls = (jshort) p_opt->max_op_vls;
  OSM_Options.force_link_speed = (jshort) p_opt->force_link_speed;
  OSM_Options.subnet_timeout = (jshort) p_opt->subnet_timeout;
  OSM_Options.packet_life_time = (jshort) p_opt->packet_life_time;
  OSM_Options.vl_stall_count = (jshort) p_opt->vl_stall_count;
  OSM_Options.leaf_vl_stall_count = (jshort) p_opt->leaf_vl_stall_count;
  OSM_Options.head_of_queue_lifetime = (jshort) p_opt->head_of_queue_lifetime;
  OSM_Options.leaf_head_of_queue_lifetime = (jshort) p_opt->leaf_head_of_queue_lifetime;
  OSM_Options.local_phy_errors_threshold = (jshort) p_opt->local_phy_errors_threshold;
  OSM_Options.overrun_errors_threshold = (jshort) p_opt->overrun_errors_threshold;
  OSM_Options.log_flags = (jshort) p_opt->log_flags;
  OSM_Options.lash_start_vl = (jshort) p_opt->lash_start_vl;
  OSM_Options.sm_sl = (jshort) p_opt->sm_sl;
  OSM_Options.m_key_protect_bits = (jshort) p_opt->m_key_protect_bits;
  OSM_Options.force_link_speed_ext = (jshort) p_opt->force_link_speed_ext;
  OSM_Options.fdr10 = (jshort) p_opt->fdr10;
  OSM_Options.sm_assigned_guid = (jshort) p_opt->sm_assigned_guid;
  OSM_Options.cc_sw_cong_setting_threshold = (jshort) p_opt->cc_sw_cong_setting_threshold;
  OSM_Options.cc_sw_cong_setting_packet_size = (jshort) p_opt->cc_sw_cong_setting_packet_size;
  OSM_Options.cc_sw_cong_setting_credit_starvation_threshold =
      (jshort) p_opt->cc_sw_cong_setting_credit_starvation_threshold;

  /* all the ints */
  OSM_Options.console_port = (jint) p_opt->console_port;
  OSM_Options.m_key_lease_period = (jint) p_opt->m_key_lease_period;
  OSM_Options.sweep_interval = (jint) p_opt->sweep_interval;
  OSM_Options.max_wire_smps = (jint) p_opt->max_wire_smps;
  OSM_Options.max_wire_smps2 = (jint) p_opt->max_wire_smps2;
  OSM_Options.max_smps_timeout = (jint) p_opt->max_smps_timeout;
  OSM_Options.transaction_timeout = (jint) p_opt->transaction_timeout;
  OSM_Options.transaction_retries = (jint) p_opt->transaction_retries;
  OSM_Options.sminfo_polling_timeout = (jint) p_opt->sminfo_polling_timeout;
  OSM_Options.polling_retry_number = (jint) p_opt->polling_retry_number;
  OSM_Options.max_msg_fifo_timeout = (jint) p_opt->max_msg_fifo_timeout;
  OSM_Options.max_reverse_hops = (jint) p_opt->max_reverse_hops;
  OSM_Options.perfmgr_sweep_time_s = (jint) p_opt->perfmgr_sweep_time_s;
  OSM_Options.perfmgr_max_outstanding_queries = (jint) p_opt->perfmgr_max_outstanding_queries;
  OSM_Options.ca_port = (jint) p_opt->ca_port;
  OSM_Options.part_enforce_enum = (jint) p_opt->part_enforce_enum;
  OSM_Options.scatter_ports = (jint) p_opt->scatter_ports;
  OSM_Options.cc_max_outstanding_mads = (jint) p_opt->cc_max_outstanding_mads;
  OSM_Options.cc_sw_cong_setting_control_map = (jint) p_opt->cc_sw_cong_setting_control_map;
  OSM_Options.cc_sw_cong_setting_marking_rate = (jint) p_opt->cc_sw_cong_setting_marking_rate;
  OSM_Options.cc_ca_cong_setting_port_control = (jint) p_opt->cc_ca_cong_setting_port_control;
  OSM_Options.cc_ca_cong_setting_control_map = (jint) p_opt->cc_ca_cong_setting_control_map;
  OSM_Options.perfmgr_rm_nodes = (jint) p_opt->perfmgr_rm_nodes;
  OSM_Options.perfmgr_xmit_wait_threshold = (jint) p_opt->perfmgr_xmit_wait_threshold;

  /* all the longs */
  OSM_Options.log_max_size = (jlong) p_opt->log_max_size;
  OSM_Options.guid = (jlong) p_opt->guid;
  OSM_Options.m_key = (jlong) p_opt->m_key;
  OSM_Options.sm_key = (jlong) p_opt->sm_key;
  OSM_Options.sa_key = (jlong) p_opt->sa_key;
  OSM_Options.subnet_prefix = (jlong) p_opt->subnet_prefix;
  OSM_Options.cc_key = (jlong) p_opt->cc_key;

  /* do all the strings */
  sstrncpy(OSM_Options.config_file, p_opt->config_file, sizeof(OSM_Options.config_file));
  sstrncpy(OSM_Options.dump_files_dir, p_opt->dump_files_dir, sizeof(OSM_Options.dump_files_dir));
  sstrncpy(OSM_Options.log_file, p_opt->log_file, sizeof(OSM_Options.log_file));
  sstrncpy(OSM_Options.partition_config_file, p_opt->partition_config_file, sizeof(OSM_Options.partition_config_file));
  sstrncpy(OSM_Options.qos_policy_file, p_opt->qos_policy_file, sizeof(OSM_Options.qos_policy_file));
  sstrncpy(OSM_Options.console, p_opt->console, sizeof(OSM_Options.console));
  sstrncpy(OSM_Options.port_prof_ignore_file, p_opt->port_prof_ignore_file, sizeof(OSM_Options.port_prof_ignore_file));
  sstrncpy(OSM_Options.hop_weights_file, p_opt->hop_weights_file, sizeof(OSM_Options.hop_weights_file));
  sstrncpy(OSM_Options.routing_engine_names, p_opt->routing_engine_names, sizeof(OSM_Options.routing_engine_names));
  sstrncpy(OSM_Options.lid_matrix_dump_file, p_opt->lid_matrix_dump_file, sizeof(OSM_Options.lid_matrix_dump_file));
  sstrncpy(OSM_Options.lfts_file, p_opt->lfts_file, sizeof(OSM_Options.lfts_file));
  sstrncpy(OSM_Options.root_guid_file, p_opt->root_guid_file, sizeof(OSM_Options.root_guid_file));
  sstrncpy(OSM_Options.cn_guid_file, p_opt->cn_guid_file, sizeof(OSM_Options.cn_guid_file));
  sstrncpy(OSM_Options.io_guid_file, p_opt->io_guid_file, sizeof(OSM_Options.io_guid_file));
  sstrncpy(OSM_Options.ids_guid_file, p_opt->ids_guid_file, sizeof(OSM_Options.ids_guid_file));
  sstrncpy(OSM_Options.guid_routing_order_file, p_opt->guid_routing_order_file,
      sizeof(OSM_Options.guid_routing_order_file));
  sstrncpy(OSM_Options.sa_db_file, p_opt->sa_db_file, sizeof(OSM_Options.sa_db_file));
  sstrncpy(OSM_Options.torus_conf_file, p_opt->torus_conf_file, sizeof(OSM_Options.torus_conf_file));
  sstrncpy(OSM_Options.event_db_dump_file, p_opt->event_db_dump_file, sizeof(OSM_Options.event_db_dump_file));
  sstrncpy(OSM_Options.event_plugin_name, p_opt->event_plugin_name, sizeof(OSM_Options.event_plugin_name));
  sstrncpy(OSM_Options.event_plugin_options, p_opt->event_plugin_options, sizeof(OSM_Options.event_plugin_options));
  sstrncpy(OSM_Options.node_name_map_name, p_opt->node_name_map_name, sizeof(OSM_Options.node_name_map_name));
  sstrncpy(OSM_Options.prefix_routes_file, p_opt->prefix_routes_file, sizeof(OSM_Options.prefix_routes_file));
  sstrncpy(OSM_Options.log_prefix, p_opt->log_prefix, sizeof(OSM_Options.log_prefix));
  sstrncpy(OSM_Options.ca_name, p_opt->ca_name, sizeof(OSM_Options.ca_name));
  sstrncpy(OSM_Options.force_link_speed_file, p_opt->force_link_speed_file, sizeof(OSM_Options.force_link_speed_file));
  sstrncpy(OSM_Options.part_enforce, p_opt->part_enforce, sizeof(OSM_Options.part_enforce));
  sstrncpy(OSM_Options.port_search_ordering_file, p_opt->port_search_ordering_file,
      sizeof(OSM_Options.port_search_ordering_file));
  sstrncpy(OSM_Options.per_module_logging_file, p_opt->per_module_logging_file,
      sizeof(OSM_Options.per_module_logging_file));

  CL_PLOCK_RELEASE(&(gData->p_osm->lock));
  return rc;
}

int
sr_getMCGroupTable()
{
  int rc = 0;
  osm_opensm_t * p_osm = gData->p_osm;
  osm_sa_t *p_sa = &(p_osm->sa);
  osm_mgrp_t *p_mgrp;

  int m_ndex = 0;

  /*       mgrp_mgid_tbl
   *               Container of pointers to all Multicast group objects in
   *               the subnet. Indexed by MGID.
   */
  sr_MCGroups_t * p_MCGroups = sr_getMCGroups();

  /* loop through the table and copy the info */
  CL_PLOCK_ACQUIRE(p_sa->p_lock);

  /* simply go over all MCGs and match */
  for (p_mgrp = (osm_mgrp_t *) cl_fmap_head(&p_sa->p_subn->mgrp_mgid_tbl);
      (p_mgrp != (osm_mgrp_t *) cl_fmap_end(&p_sa->p_subn->mgrp_mgid_tbl) && (m_ndex < MAX_NUM_MCGROUPS)); p_mgrp =
          (osm_mgrp_t *) cl_fmap_next(&p_mgrp->map_item))
  {
    // do the copy here
    p_MCGroups->port_members = p_mgrp->full_members;
    p_MCGroups->mlid = cl_ntoh16(p_mgrp->mlid);
    p_MCGroups->well_known = p_mgrp->well_known;

    /* a list of port guids in this multicast group */
    int p_ndex = 0;
    osm_mcm_port_t *p_mcm_port, *p_next_mcm_port;

    p_next_mcm_port = (osm_mcm_port_t *) cl_qmap_head(&p_mgrp->mcm_port_tbl);
    while (p_next_mcm_port != (osm_mcm_port_t *) cl_qmap_end(&p_mgrp->mcm_port_tbl))
    {
      p_mcm_port = p_next_mcm_port;
      p_next_mcm_port = (osm_mcm_port_t *) cl_qmap_next(&p_mcm_port->map_item);
      p_MCGroups->port_num_array[p_ndex] = (jshort) (p_mcm_port->port->p_physp->port_num);
      p_MCGroups->port_guid_array[p_ndex] = cl_ntoh64(p_mcm_port->port->p_physp->port_guid);
      p_ndex++;
    }
    p_MCGroups->port_members = p_ndex;
    m_ndex++;
    p_MCGroups++;
  }
  ActualNumberMCGroups = m_ndex;
  if (m_ndex >= MAX_NUM_MCGROUPS)
  {
    m_ndex = 0;
    // how many mcgroups are there?????
    for (p_mgrp = (osm_mgrp_t *) cl_fmap_head(&p_sa->p_subn->mgrp_mgid_tbl);
        p_mgrp != (osm_mgrp_t *) cl_fmap_end(&p_sa->p_subn->mgrp_mgid_tbl);
        p_mgrp = (osm_mgrp_t *) cl_fmap_next(&p_mgrp->map_item))
    {
      m_ndex++;
    }
  }
  CL_PLOCK_RELEASE(p_sa->p_lock);
  if (m_ndex >= MAX_NUM_MCGROUPS)
  {
    J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER MCGROUPS REACHED - MCGROUP TABLE INCOMPLETE** (%d vs %d)\n",
        MAX_NUM_MCGROUPS, m_ndex);
  }
  return rc;
}

int
sr_getPKeyTable()
{
  int rc = 0;
  osm_opensm_t * p_osm = gData->p_osm;
  osm_prtn_t * p_pkey = NULL;
  cl_map_iterator_t i, i_next;
  const cl_map_t *p_mem_tbl;

  osm_physp_t *p_physp;

  int n_ndex = 0;
  int p_ndex = 0;

  sr_PKey_t * p_PKey = sr_getPKeys();

  CL_PLOCK_ACQUIRE(p_osm->sm.p_lock);
  cl_qmap_t *p_pkey_guid_tbl = &(p_osm->sm.p_subn->prtn_pkey_tbl);

  /* continue if the partition table exists */
  if (p_pkey_guid_tbl != NULL)
  {

    J_LOG(gData, OSM_LOG_DEBUG, "Getting Partitions now\n");

    /* loop through the table and copy the info */
    osm_prtn_t * p_next_pkey = (osm_prtn_t *) cl_qmap_head(p_pkey_guid_tbl);
    while ((p_next_pkey != (osm_prtn_t *) cl_qmap_end(p_pkey_guid_tbl)) && (n_ndex < MAX_NUM_PARTITIONS))
    {
      p_pkey = p_next_pkey;
      p_next_pkey = (osm_prtn_t *) cl_qmap_next(&p_next_pkey->map_item);

      /* copy everything I want */
      p_PKey->pkey = cl_ntoh16(p_pkey->pkey);
      p_PKey->sl = p_pkey->sl;

      sstrncpy(p_PKey->name, p_pkey->name, sizeof(p_PKey->name));

      /* copy the two member arrays */
      p_ndex = 0;
      p_mem_tbl = &(p_pkey->full_guid_tbl);
      i_next = cl_map_head(p_mem_tbl);
      while ((i_next != cl_map_end(p_mem_tbl)) && (p_ndex < MAX_NUM_NODES))
      {
        i = i_next;
        i_next = cl_map_next(i);
        p_physp = cl_map_obj(i);
        if (p_physp)
        {
          p_PKey->full_guid_array[p_ndex++] = cl_ntoh64(p_physp->port_guid);
        }
      }
      p_PKey->num_full_membs = p_ndex;
      if (p_ndex >= MAX_NUM_NODES)
        J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER NODES REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_NODES);

      p_ndex = 0;
      p_mem_tbl = &(p_pkey->part_guid_tbl);
      i_next = cl_map_head(p_mem_tbl);
      while ((i_next != cl_map_end(p_mem_tbl)) && (p_ndex < MAX_NUM_NODES))
      {
        i = i_next;
        i_next = cl_map_next(i);
        p_physp = cl_map_obj(i);
        if (p_physp)
        {
          p_PKey->part_guid_array[p_ndex++] = cl_ntoh64(p_physp->port_guid);
        }
      }
      p_PKey->num_part_membs = p_ndex;
      if (p_ndex >= MAX_NUM_NODES)
        J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER NODES REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_NODES);

      n_ndex++;
      p_PKey++;
    }
    if (n_ndex >= MAX_NUM_PARTITIONS)
      J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER PARTITIONS REACHED - LOSS OF DATA PROBABLE** (%d)\n",
          MAX_NUM_PARTITIONS);

  }
  CL_PLOCK_RELEASE(p_osm->sm.p_lock);
  ActualNumberPartitions = n_ndex;
  return rc;
}

int
pm_getPM_DB(perfmgr_db_t * db)
{
  /* this method never called if db null, no need for checks */
  /* loop through the database and fill in the peer classes */

  db_port_t *p_port = NULL;
  db_node_t * p_node = NULL;
  int rc = 0;
  int n_ndex = 0;
  int p_ndex = 0;
  int tot_ports = 0;
  int esp0_ports = 0;

  /* TODO these need locks */
  pm_Node_t * p_pmNode = &AllNodes[0];
  pm_Port_t * p_pmPort = &AllPorts[0];

  cl_plock_acquire(&db->lock);
  p_node = (db_node_t *) cl_qmap_head(&db->pc_data);
  while ((p_node != (db_node_t *) cl_qmap_end(&db->pc_data)) && (n_ndex < MAX_NUM_NODES))
  {
//        if this node is a switch and the esp0 is one, then use port 0,
//        otherwise skip it, and start at port 1

    /* fill in the static array of node guids (don't exceed max)*/
    p_pmNode->node_guid = p_node->node_guid;
    sstrncpy(p_pmNode->node_name, p_node->node_name, MAX_NODE_NAME_SIZE);
    p_pmNode->esp0 = p_node->esp0 ? 1 : 0;

    /* how many ports does this node really have? */
    p_pmNode->num_ports = 0; /* count esp0 ports and skip invalid ports (see for loop) */
    esp0_ports += p_pmNode->esp0; /* keep a local running total of esp0 */

    for (p_ndex = p_node->esp0 ? 0 : 1; (p_ndex < p_node->num_ports) && (tot_ports < MAX_NUM_PORTS); p_ndex++)
    {
      // if this port isn't valid, then skip it
      p_port = &p_node->ports[p_ndex];
      if (p_port->valid)
      {
        // use totals - previous values are for internal use
        p_pmPort->node_guid = p_pmNode->node_guid;
        p_pmPort->port_num = p_ndex;

        p_pmPort->port_counters[SYMBOL_ERR_CNT] = p_port->err_total.symbol_err_cnt;
        p_pmPort->port_counters[LINK_ERR_RECOVER] = p_port->err_total.link_err_recover;
        p_pmPort->port_counters[LINK_DOWNED] = p_port->err_total.link_downed;
        p_pmPort->port_counters[RCV_ERR] = p_port->err_total.rcv_err;
        p_pmPort->port_counters[RCV_REM_PHYS_ERR] = p_port->err_total.rcv_rem_phys_err;
        p_pmPort->port_counters[RCV_SWITCH_RELAY_ERR] = p_port->err_total.rcv_switch_relay_err;
        p_pmPort->port_counters[XMIT_DISCARDS] = p_port->err_total.xmit_discards;
        p_pmPort->port_counters[XMIT_CONSTRAINT_ERR] = p_port->err_total.xmit_constraint_err;
        p_pmPort->port_counters[RCV_CONSTRAINT_ERR] = p_port->err_total.rcv_constraint_err;
        p_pmPort->port_counters[LINK_INTEGRITY] = p_port->err_total.link_integrity;
        p_pmPort->port_counters[BUFFER_OVERRUN] = p_port->err_total.buffer_overrun;
        p_pmPort->port_counters[VL15_DROPPED] = p_port->err_total.vl15_dropped;
        p_pmPort->error_ts = p_port->err_total.time;
        p_pmPort->port_counters[XMIT_DATA] = p_port->dc_total.xmit_data;
        p_pmPort->port_counters[RCV_DATA] = p_port->dc_total.rcv_data;
        p_pmPort->port_counters[XMIT_PKTS] = p_port->dc_total.xmit_pkts;
        p_pmPort->port_counters[RCV_PKTS] = p_port->dc_total.rcv_pkts;
        p_pmPort->port_counters[UNICAST_XMIT_PKTS] = p_port->dc_total.unicast_xmit_pkts;
        p_pmPort->port_counters[UNICAST_RCV_PKTS] = p_port->dc_total.unicast_rcv_pkts;
        p_pmPort->port_counters[MULTICAST_XMIT_PKTS] = p_port->dc_total.multicast_xmit_pkts;
        p_pmPort->port_counters[MULTICAST_RCV_PKTS] = p_port->dc_total.multicast_rcv_pkts;
        p_pmPort->counter_ts = p_port->dc_total.time;

        /* TODO fill these in, where do they come from? */
        p_pmPort->port_counters[XMIT_WAIT] = p_port->err_total.xmit_wait;
//        p_pmPort->port_counters[XMIT_WAIT] = 0;

        /* update the port counters and pointer */
        p_pmNode->num_ports++; /* the actual number of ports for this node */
        p_pmPort++;
        tot_ports++;
      }
    }
    /* all done with this node and its ports */
    p_node = (db_node_t *) cl_qmap_next(&p_node->map_item);
    p_pmNode++;
    n_ndex++;
//        J_LOG(gData, OSM_LOG_INFO, "@@ Node: %3d, Ports: %3d, CumPorts: %5d @@\n", n_ndex, p_ndex, tot_ports);
    if (tot_ports >= MAX_NUM_PORTS)
      J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER PORTS REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_PORTS);
  }
  cl_plock_release(&db->lock);
  ActualNumberNodes = n_ndex;
  ;
  ActualNumberPorts = tot_ports;
  ActualSwitchPortZero = esp0_ports;
  if (n_ndex >= MAX_NUM_NODES)
    J_LOG(gData, OSM_LOG_ERROR, "**MAX NUMBER NODES REACHED - LOSS OF DATA PROBABLE** (%d)\n", MAX_NUM_NODES);

  return rc;
}

perfmgr_db_err_t
jsr_getPortCounters(perfmgr_db_t * db, uint64_t guid, uint8_t port)
{
  db_port_t *p_port = NULL;
  db_node_t *node = NULL;
  perfmgr_db_data_cnt_reading_t counters;
  perfmgr_db_err_reading_t errors;
  perfmgr_db_err_t rc = PERFMGR_EVENT_DB_SUCCESS;

  cl_plock_acquire(&db->lock);
  node = getNodeFromDb(db, cl_ntoh64(guid));

  if ((rc = bad_node_port(node, port)) != PERFMGR_EVENT_DB_SUCCESS)
  {
    J_LOG(gData, OSM_LOG_INFO, "**COULD NOT GET NODE from DB** (%d)\n", rc);
    J_LOG(gData, OSM_LOG_INFO, "Port Info:   guid 0x%" PRIxLEAST64 ", port# %d\n", cl_ntoh64(guid), port);
    if (node == NULL)
      J_LOG(gData, OSM_LOG_INFO, "Returned node is NULL\n");

    cl_plock_release(&db->lock);
    return rc;
  }

  p_port = &node->ports[port];

  /* I have the port, get a copy of the absolute values (most current) */
  counters = p_port->dc_total;
  errors = p_port->err_total;

  cl_plock_release(&db->lock);

  /* lock released, so can do time intensive operations */

  /* print out the counters and errors */
  J_LOG(gData, OSM_LOG_INFO, "xd %" PRIu64 "\n", counters.xmit_data);
  J_LOG(gData, OSM_LOG_INFO, "rd %" PRIu64 "\n", counters.rcv_data);
  J_LOG(gData, OSM_LOG_INFO, "xp %" PRIu64 "\n", counters.xmit_pkts);
  J_LOG(gData, OSM_LOG_INFO, "rp %" PRIu64 "\n", counters.rcv_pkts);
//    J_LOG(gData, OSM_LOG_INFO, "rp %" PRIu64 "\n",counters.time);

  J_LOG(gData, OSM_LOG_INFO, "ebo %" PRIu64 "\n", errors.buffer_overrun);
  J_LOG(gData, OSM_LOG_INFO, "eld %" PRIu64 "\n", errors.link_downed);
  J_LOG(gData, OSM_LOG_INFO, "eler %" PRIu64 "\n", errors.link_err_recover);
  J_LOG(gData, OSM_LOG_INFO, "eli %" PRIu64 "\n", errors.link_integrity);
  J_LOG(gData, OSM_LOG_INFO, "erc %" PRIu64 "\n", errors.rcv_constraint_err);
  J_LOG(gData, OSM_LOG_INFO, "er %" PRIu64 "\n", errors.rcv_err);
  J_LOG(gData, OSM_LOG_INFO, "err %" PRIu64 "\n", errors.rcv_rem_phys_err);
  J_LOG(gData, OSM_LOG_INFO, "ersr %" PRIu64 "\n", errors.rcv_switch_relay_err);
  J_LOG(gData, OSM_LOG_INFO, "es %" PRIu64 "\n", errors.symbol_err_cnt);
  J_LOG(gData, OSM_LOG_INFO, "evd %" PRIu64 "\n", errors.vl15_dropped);
  J_LOG(gData, OSM_LOG_INFO, "exc %" PRIu64 "\n", errors.xmit_constraint_err);
  J_LOG(gData, OSM_LOG_INFO, "exd %" PRIu64 "\n", errors.xmit_discards);
  J_LOG(gData, OSM_LOG_INFO, "exw %" PRIu64 "\n", errors.xmit_wait);
//    J_LOG(gData, OSM_LOG_INFO, "el %" PRIu64 "\n",errors.time);

  return rc;
}

long
jsr_getNumPorts(void)
{
  osm_opensm_t * p_osm = gData->p_osm;
  osm_sm_t * p_sm = &(p_osm->sm);
  osm_subn_t * p_subn = p_sm->p_subn;
  cl_qmap_t * p_tbl = &(p_subn->port_guid_tbl);

  // how big is the table, thats how many ports we have
  return (long) p_tbl->count;
}

long
jsr_getNumNodes()
{
  osm_opensm_t * p_osm = gData->p_osm;
  osm_sm_t * p_sm = &(p_osm->sm);
  osm_subn_t * p_subn = p_sm->p_subn;
  cl_qmap_t * p_tbl = &(p_subn->node_guid_tbl);

  // how big is the table, thats how many ports we have
  return (long) p_tbl->count;
}

void
jsr_printNodeGuids()
{
  osm_opensm_t * p_osm = gData->p_osm;
  cl_qmap_t *p_node_guid_tbl;
  osm_node_t *p_node, *p_next_node;
  ib_node_info_t * p_ni;

  CL_PLOCK_ACQUIRE(p_osm->sm.p_lock);
  p_node_guid_tbl = &(p_osm->sm.p_subn->node_guid_tbl);

  p_next_node = (osm_node_t *) cl_qmap_head(p_node_guid_tbl);
  while (p_next_node != (osm_node_t *) cl_qmap_end(p_node_guid_tbl))
  {
    p_node = p_next_node;
    p_next_node = (osm_node_t *) cl_qmap_next(&p_next_node->map_item);

    J_LOG(gData, OSM_LOG_INFO, "Node Info: node type # %d\n", (uint8_t )(p_node->node_info.node_type));
    J_LOG(gData, OSM_LOG_INFO, "Node Info: sys  guid 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_node->node_info.sys_guid));
    J_LOG(gData, OSM_LOG_INFO, "Node Info: node guid 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_node->node_info.node_guid));
    J_LOG(gData, OSM_LOG_INFO, "Node Info: port guid 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_node->node_info.port_guid));
    J_LOG(gData, OSM_LOG_INFO, "Node Info: # ports %d\n", (uint8_t )(p_node->node_info.num_ports));
    J_LOG(gData, OSM_LOG_INFO, "Node Info: talbe size %d\n", (uint8_t )(p_node->physp_tbl_size));
//    J_LOG(gData, OSM_LOG_INFO, "Switch Info: dimn ports %d\n", (uint8_t)(p_node->sw.dimn_ports));
//    J_LOG(gData, OSM_LOG_INFO, "Switch Info: endpoint links %d\n", (uint8_t)(p_node->sw.endport_links));
//    J_LOG(gData, OSM_LOG_INFO, "Switch Info: talbe size %d\n", (uint8_t)(p_node->sw.switch_info.));

    p_ni = &(p_node->node_info);
    osm_dump_node_info(&(gData->jpilog), p_ni, OSM_LOG_INFO);

  }
  CL_PLOCK_RELEASE(p_osm->sm.p_lock);
}

void
jsr_printPortGuids()
{
  osm_opensm_t * p_osm = gData->p_osm;
  cl_qmap_t *p_port_guid_tbl;
  osm_port_t *p_port, *p_next_port;
  ib_port_info_t * p_pi;
  ib_net64_t node_guid;
  ib_net64_t port_guid;
  int8_t port_num;

  CL_PLOCK_ACQUIRE(p_osm->sm.p_lock);
  p_port_guid_tbl = &(p_osm->sm.p_subn->port_guid_tbl);

  p_next_port = (osm_port_t *) cl_qmap_head(p_port_guid_tbl);
  while (p_next_port != (osm_port_t *) cl_qmap_end(p_port_guid_tbl))
  {
    p_port = p_next_port;
    p_next_port = (osm_port_t *) cl_qmap_next(&p_next_port->map_item);

    J_LOG(gData, OSM_LOG_INFO, "Port Info:   guid 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_port->guid));
//    J_LOG(gData, OSM_LOG_INFO, "Port Info:   lid %d\n", p_port->lid);
    J_LOG(gData, OSM_LOG_INFO, "Port Info:   lid %d\n", cl_ntoh16(p_port->p_physp->port_info.base_lid));
    J_LOG(gData, OSM_LOG_INFO, "Port Info: p guid 0x%" PRIxLEAST64 "\n", cl_ntoh64(p_port->p_physp->port_guid));
    J_LOG(gData, OSM_LOG_INFO, "Port Info: p port # %d\n", p_port->p_physp->port_num);

    p_pi = &(p_port->p_physp->port_info);
    node_guid = p_port->p_node->node_info.node_guid;
//    node_guid = p_port->p_node->node_info.node_guid;
//    node_guid = p_port->p_node->node_info.port_guid;
//    port_guid = p_port->p_node->node_info.sys_guid;
//    port_guid = p_port->p_node->node_info.node_guid;
//    port_guid = p_port->p_node->node_info.port_guid;
    port_guid = p_port->p_physp->port_guid;
//     node_guid = p_port->guid;
//      port_guid = p_port->p_physp->port_guid;
    port_num = p_port->p_physp->port_num;
    port_num = p_pi->local_port_num;

    jp_dump_port_info(node_guid, port_guid, port_num);
    pt_printPT_Ports();

//    osm_dump_port_info(p_log, node_guid, port_guid, port_num, p_pi,log_level);
//    jsr_getPortCounters(db, port_guid, port_num);
  }

  CL_PLOCK_RELEASE(p_osm->sm.p_lock);
}

const char*
jsr_getPluginName(void)
{
  /* the name of this plugin, as seen by the OSM */
  cl_qlist_t plugin_list = gData->p_osm->plugin_list;
  osm_epi_plugin_t *pi;
  cl_list_item_t *itor;

  itor = cl_qlist_head(&plugin_list);
  pi = (osm_epi_plugin_t *) itor;
  return pi->plugin_name;
}

const char*
jsr_getPluginVersion(void)
{
  static char VersionString[MAX_STRING_SIZE];

  sprintf(VersionString, "%s (%s at %s)", OSM_JNI_PI_VERSION, version_date, version_time);

  return VersionString;
}

const char*
jsr_getOsmVersion(void)
{
  osm_opensm_t * p_osm = gData->p_osm;
  return p_osm->osm_version;
}

const char*
jsr_setPrfMgrSweepPeriod(const char* cmdArg)
{
  // returning a pointer to a status string, so make it static
  static char ResultString[MAX_STRING_SIZE];
  J_LOG(gData, OSM_LOG_INFO, "Native PrfMgr Sweep Period (%s)\n", cmdArg);

  osm_opensm_t * p_osm = gData->p_osm;

  // attempt to convert the cmdArg into an integer.
  uint16_t time_s = atoi(cmdArg);
  if (time_s > 9)
  {
    osm_perfmgr_set_sweep_time_s(&p_osm->perfmgr, time_s);
    return "Success";
  }
  else
  {
    return "Native PrfMgr Sweep Period requires an integer greater than 9";
  }
  return ResultString;
}

const char*
jsr_setLogLevel(const char* cmdArg)
{
  J_LOG(gData, OSM_LOG_INFO, "Native Set Log Level (%s)\n", cmdArg);

  osm_opensm_t * p_osm = gData->p_osm;
  int level;

  /* Handle x, 0x, and decimal specification of log level */
  if (!strncmp(cmdArg, "x", 1))
  {
    cmdArg++;
    level = strtoul(cmdArg, NULL, 16);
  }
  else
  {
    if (!strncmp(cmdArg, "0x", 2))
    {
      cmdArg += 2;
      level = strtoul(cmdArg, NULL, 16);
    }
    else
      level = strtol(cmdArg, NULL, 10);
  }
  if ((level >= 0) && (level < 256))
  {
    J_LOG(gData, OSM_LOG_INFO, "Setting log level to 0x%x\n", level);
    osm_log_set_level(&p_osm->log, level);
    return "Success";
  }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Invalid log level 0x%x\n", level);
  return "Fail";
}

const char*
get_firstArg(const char* cmdArgs)
{
  static char buff[MAX_STRING_SIZE];
  int bNdex = 0;
  int eNdex = 0;

  // the first arg is the command, and the second arg should be the true arg
  strncpy(buff, cmdArgs, MAX_STRING_SIZE);
  int len = strlen(buff);
  int i = 0;
  for (i = 0; i < len; i++)
  {
    if (buff[i] == ' ')
    {
      if (bNdex == 0)
        bNdex = i + 1;
      else if (eNdex == 0)
      {
        eNdex = i;
        break;
      }
    }
  }
  // check the value of the two index
  if (bNdex > 1)
  {
    if (eNdex != 0)
      buff[eNdex] = 0;  // make sure its null terminated
    return &(buff[bNdex]);
  }
  return NULL;
}

const char*
jsr_invokeCommand(int cmdType, const char* cmdArgs)
{
  osm_opensm_t * p_osm = gData->p_osm;
  // returning a pointer to a status string, so make it static
  static char ResultString[MAX_STRING_SIZE];

  // for right now, just echo it back
  sprintf(ResultString, "CmdType: %d, CmdArgs: %s", cmdType, cmdArgs);
  J_LOG(gData, OSM_LOG_INFO, "Native Command (%s)\n", ResultString);

  if (cmdType == 0)
    J_LOG(gData, OSM_LOG_INFO, "Native Echo\n");
  else if (cmdType == 1)
  {
    J_LOG(gData, OSM_LOG_INFO, "Native Light Sweep\n");
    osm_opensm_sweep(p_osm);
    return "Success";
  }
  else if (cmdType == 2)
  {
    J_LOG(gData, OSM_LOG_INFO, "Native Heavy Sweep\n");
    p_osm->subn.force_heavy_sweep = TRUE;
    osm_opensm_sweep(p_osm);
    return "Success";
  }
  else if (cmdType == 3)
  {
    J_LOG(gData, OSM_LOG_INFO, "Native Reroute\n");
    p_osm->subn.force_reroute = TRUE;
    osm_opensm_sweep(p_osm);
    return "Success";
  }
  else if (cmdType == 4)
  {
    return jsr_setLogLevel(get_firstArg(cmdArgs));
  }
  else if (cmdType == 5)
  {
    J_LOG(gData, OSM_LOG_INFO, "Native Update Description\n");
    osm_update_node_desc(p_osm);
    return "Success";
  }
  else if (cmdType == 6)
  {
    J_LOG(gData, OSM_LOG_INFO, "Native PrfMgr Sweep\n");
    osm_sm_signal(&p_osm->sm, OSM_SIGNAL_PERFMGR_SWEEP);
    return "Success";
  }
  else if (cmdType == 7)
  {
    return jsr_setPrfMgrSweepPeriod(get_firstArg(cmdArgs));
  }
  else if (cmdType == 8)
  {
    J_LOG(gData, OSM_LOG_INFO, "Native PrfMgr Clear Counters\n");
    osm_perfmgr_clear_counters(&p_osm->perfmgr);
    return "Success";
  }
  return ResultString;
}

/*
 * This method gets periodically called to safely copy everything
 * from opensm, into a shared memory region, which will be used
 * by the JNI layer for the service
 *
 */
int
jsr_UpdateSharedResources(int update_period, int report_period)
{
  osm_opensm_t * p_osm = gData->p_osm;
  static long counter = 0;

  J_LOG(gData, OSM_LOG_DEBUG, "Updating Shared Resources (%ld)\n", counter);

  if (p_osm)
  {
    if (p_osm->perfmgr.db)
      pm_getPM_DB(p_osm->perfmgr.db);
    else
      J_LOG(gData, OSM_LOG_DEBUG, "The PerfMgr database appears to be null.\n");

    pt_getPortTable();
    sr_getSubnetAttributes();
    sr_getStatistics();
    sr_getRouterTable();
    sr_getSwitchTable();
    sr_getPKeyTable();

    sr_getMCGroupTable();

    sr_getManagerTable();
    sr_getSubnetOptions();

    sr_getSystemInfo();

    sr_getNativePluginInfo(update_period, report_period, counter++);

    getPortStatistics();
  }

  return 0;
}

/******************************************************************************
 *** Function: jsr_destroy
 ***
 *** This method is responsible for releasing the shared resources.
 *** <p>
 ***
 ***
 *** <dt><b>Miscellaneous Comments:</b></dt>
 ***   <dd>  This method does nothing if the resources were not initialized.
 *** </dd>
 ***
 *** <dt><b>External or Global Variables:</b></dt>
 ***   <dd>None
 *** </dd>
 ***
 ***  Parameters: none
 ***
 ***  Returns:  0 is returned, indicating the shared resources are no longer available.
 ***
 ******************************************************************************/
int
jsr_destroy(void)
{
  if (jsr_isInitialized == 1)
  {
    // FQ_deinit(pEventQueue);
    jsr_isInitialized = 0;

  }
  return jsr_isInitialized;
}
/*-----------------------------------------------------------------------*/
