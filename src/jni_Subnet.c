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
 * jni_Subnet.c
 *
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */

#include <unistd.h>
#include <jni.h>

#include "osmJniPi.h"
#include "jni_PeerClass.h"
#include "jni_Subnet.h"
#include "jni_SharedResources.h"

extern plugin_data_t *       gData; /* the global plugin data */
extern JPC_CLASS PeerClassArray[];


void jsn_printPartitions(void)
{
  int ndex = 0;
  sr_PKey_t * p_PKey = sr_getPKeys();

  for(ndex = 0; ndex < sr_getNumPKeys(); ndex++, p_PKey++)
    {
      J_LOG(gData, OSM_LOG_INFO, "  PKey %d has key of 0x%d\n", ndex, p_PKey->pkey);
      J_LOG(gData, OSM_LOG_INFO, "  PKey %d has name of (%s)\n", ndex, p_PKey->name);
    }
}

void jsn_printLFTforSwitch(sr_Switch_t * p_Switch)
{
  J_LOG(gData, OSM_LOG_INFO, "LID DISPLAY for \n");
  J_LOG(gData, OSM_LOG_INFO, "Switch guid 0x%" PRIxLEAST64 "\n", p_Switch->guid);
  J_LOG(gData, OSM_LOG_INFO, "Switch Number of ports %d\n", p_Switch->num_ports);
  J_LOG(gData, OSM_LOG_INFO, "Switch Number of hops: %d\n", p_Switch->num_hops);
  J_LOG(gData, OSM_LOG_INFO, "Switch LFT size: %d\n", p_Switch->lft_size);
  int maxSize = p_Switch->lft_size > 200 ? 200: p_Switch->lft_size;

  int l = 0;

  for(l = 0; l < maxSize; l++)
  {
       J_LOG(gData, OSM_LOG_INFO, "LFT for lid %d is %d \n", l, p_Switch->lft[l]);
  }
}

void jsn_printHopTableforSwitch(sr_Switch_t * p_Switch)
{
  J_LOG(gData, OSM_LOG_INFO, "HOP DISPLAY for \n");
  J_LOG(gData, OSM_LOG_INFO, "Switch guid 0x%" PRIxLEAST64 "\n", p_Switch->guid);
  J_LOG(gData, OSM_LOG_INFO, "Switch Number of ports %d\n", p_Switch->num_ports);
  J_LOG(gData, OSM_LOG_INFO, "Switch Number of hops: %d\n", p_Switch->num_hops);
  int maxSize = p_Switch->num_hops > 200 ? 200: p_Switch->num_hops;

  int l = 0;

  for(l = 0; l < maxSize; l++)
  {
      short hops = p_Switch->hops[l];
      if(hops < 255)
         J_LOG(gData, OSM_LOG_INFO, "HOPs for lid %d is %d \n", l, hops);
  }
}

void jsn_printMultiCastGroups(void)
{
  int ndex = 0;
  sr_MCGroups_t * p_MCGs = sr_getMCGroups();

  for(ndex = 0; ndex < sr_getNumMCGroups(); ndex++, p_MCGs++)
    {
      J_LOG(gData, OSM_LOG_INFO, "  MCG %4d has mlid of %d  %d, #members %d\n", ndex, p_MCGs->mlid, p_MCGs->well_known, p_MCGs->port_members);
    }
}

void jsn_printSwitches(void)
{
  int ndex = 0;
  sr_Switch_t * p_Switch = sr_getSwitches();

  for(ndex = 0; ndex < sr_getNumSwitches(); ndex++, p_Switch++)
    {
      J_LOG(gData, OSM_LOG_INFO, "  Switch %d has guid of 0x%" PRIxLEAST64 "\n", ndex, p_Switch->guid);
      J_LOG(gData, OSM_LOG_INFO, "  Switch %d has num_ports of %d\n", ndex, p_Switch->num_ports);
      J_LOG(gData, OSM_LOG_INFO, "  Switch %d has num_hops of %d\n", ndex, p_Switch->num_hops);
//      J_LOG(gData, OSM_LOG_INFO, "  Switch %d has endport_links of %d\n", ndex, p_Switch->endport_links);
      if(ndex == 7)
        {
          jsn_printLFTforSwitch(p_Switch);
//          jsn_printHopTableforSwitch(p_Switch);
        }
    }
}
void jsn_printRouters(void)
{
  int ndex = 0;
  sr_Router_t * p_Router = sr_getRouters();

  for(ndex = 0; ndex < sr_getNumRouters(); ndex++, p_Router++)
    {
      J_LOG(gData, OSM_LOG_INFO, "  Router %d has guid of 0x%" PRIxLEAST64 "\n", ndex, p_Router->guid);
    }

}

void jsn_printManagers(void)
{
  int ndex = 0;
  sr_Manager_t * p_Manager = sr_getManagers();

  for(ndex = 0; ndex < sr_getNumManagers(); ndex++, p_Manager++)
    {
      J_LOG(gData, OSM_LOG_INFO, "  Manager %d has guid of 0x%" PRIxLEAST64 "\n", ndex, p_Manager->guid);
      J_LOG(gData, OSM_LOG_INFO, "  Manager %d has state of %s\n", ndex, p_Manager->State);
      J_LOG(gData, OSM_LOG_INFO, "  Manager %d has sm_key of %ld\n", ndex, p_Manager->sm_key);
      J_LOG(gData, OSM_LOG_INFO, "  Manager %d has pri_state of %d\n", ndex, p_Manager->pri_state);
      J_LOG(gData, OSM_LOG_INFO, "  Manager %d has act_count of %d\n", ndex, p_Manager->act_count);
    }

}

void jsn_printOptions(void)
{
  sr_Options_t * p_Options = sr_getOptions();

  J_LOG(gData, OSM_LOG_INFO, "  Option config_file: %s\n", p_Options->config_file);
  J_LOG(gData, OSM_LOG_INFO, "  Option console: %s\n", p_Options->console);
  J_LOG(gData, OSM_LOG_INFO, "  Option console_port: %d\n", p_Options->console_port);
  J_LOG(gData, OSM_LOG_INFO, "  Option event_plugin_name: %s\n", p_Options->event_plugin_name);
  J_LOG(gData, OSM_LOG_INFO, "  Option event_plugin_options: %s\n", p_Options->event_plugin_options);
  J_LOG(gData, OSM_LOG_INFO, "  Option log_max_size: %ld\n", p_Options->log_max_size);
  J_LOG(gData, OSM_LOG_INFO, "  Option node_name_map_name: %s\n", p_Options->node_name_map_name);

}

void jsn_getOsmSubnetPrint(void * pJEnv)
{
  J_LOG(gData, OSM_LOG_INFO, "Native Subnet activation\n");
  sr_getSubnetAttributes();
  sr_Subnet_t * p_Subnet = sr_getSubnet();

  J_LOG(gData, OSM_LOG_INFO, "  Subnet guid of 0x%" PRIxLEAST64 "\n", p_Subnet->sm_port_guid);
  J_LOG(gData, OSM_LOG_INFO, "  Subnet sm_state: %d\n", p_Subnet->sm_state);
  J_LOG(gData, OSM_LOG_INFO, "  Subnet coming_out_of_standby: %d\n", p_Subnet->coming_out_of_standby);
  J_LOG(gData, OSM_LOG_INFO, "  Subnet master_sm_base_lid: %d\n", p_Subnet->master_sm_base_lid);
  J_LOG(gData, OSM_LOG_INFO, "  Subnet need_update: %d\n", p_Subnet->need_update);

  J_LOG(gData, OSM_LOG_INFO, "Number of Routers is %d\n", sr_getNumRouters());
  if(sr_getNumRouters() > 0)
    jsn_printRouters();
  J_LOG(gData, OSM_LOG_INFO, "Number of Switches is %d\n", sr_getNumSwitches());
  if(sr_getNumSwitches() > 0)
    jsn_printSwitches();
  J_LOG(gData, OSM_LOG_INFO, "Number of PKeys is %d\n", sr_getNumPKeys());
  if(sr_getNumPKeys() > 0)
    jsn_printPartitions();
  J_LOG(gData, OSM_LOG_INFO, "Number of Managers is %d\n", sr_getNumManagers());
  if(sr_getNumManagers() > 0)
    jsn_printManagers();
  J_LOG(gData, OSM_LOG_INFO, "Number of MultiCast Groups is %d\n", sr_getNumMCGroups());
  if(sr_getNumMCGroups() > 0)
    jsn_printMultiCastGroups();
  jsn_printOptions();

}

/* the OSM_Subnet object consists of 5 other objects */

jobject * jsn_getOsmOptions(void * pJenv)
{
  // returning a pointer to an object, so make it static
    static jobject      currentObject;
    JNIEnv * pJEnv        = (JNIEnv *) pJenv;
    JPC_CLASS OPT_Class = PeerClassArray[JPC_SBN_OPTIONS_CLASS];

    /* TODO need a lock ? */
    sr_Options_t * p_O = sr_getOptions();

    J_LOG(gData, OSM_LOG_DEBUG, "  Option config_file: %s\n", p_O->config_file);

    // construct the options container object
    currentObject = (*pJEnv)->NewObject(pJEnv, OPT_Class.jpcClass, OPT_Class.constructorMethod->methodID,
        /* booleans */
        p_O->lmc_esp0,
        p_O->reassign_lids,
        p_O->ignore_other_sm,
        p_O->single_thread,
        p_O->disable_multicast,
        p_O->force_log_flush,
        p_O->use_mfttop,
        p_O->force_heavy_sweep,
        p_O->no_partition_enforcement,
        p_O->qos,
        p_O->accum_log_file,
        p_O->port_profile_switch_nodes,
        p_O->sweep_on_trap,
        p_O->use_ucast_cache,
        p_O->connect_roots,
        p_O->sa_db_dump,
        p_O->do_mesh_analysis,
        p_O->exit_on_fatal,
        p_O->honor_guid2lid_file,
        p_O->daemon,
        p_O->sm_inactive,
        p_O->babbling_port_policy,
        p_O->use_optimized_slvl,
        p_O->enable_quirks,
        p_O->no_clients_rereg,
        p_O->perfmgr,
        p_O->perfmgr_redir,
        p_O->consolidate_ipv6_snm_req,
        p_O->m_key_lookup,
        p_O->allow_both_pkeys,
        p_O->port_shifting,
        p_O->remote_guid_sorting,
        p_O->guid_routing_order_no_scatter,
        p_O->drop_event_subscriptions,
        p_O->fsync_high_avail_files,
        p_O->congestion_control,
        p_O->perfmgr_ignore_cas,
        p_O->perfmgr_log_errors,
        p_O->perfmgr_query_cpi,
        p_O->perfmgr_xmit_wait_log,

        /* shorts */
        p_O->sm_priority,
        p_O->lmc,
        p_O->max_op_vls,
        p_O->force_link_speed,
        p_O->subnet_timeout,
        p_O->packet_life_time,
        p_O->vl_stall_count,
        p_O->leaf_vl_stall_count,
        p_O->head_of_queue_lifetime,
        p_O->leaf_head_of_queue_lifetime,
        p_O->local_phy_errors_threshold,
        p_O->overrun_errors_threshold,
        p_O->log_flags,
        p_O->lash_start_vl,
        p_O->sm_sl,
        p_O->m_key_protect_bits,
        p_O->force_link_speed_ext,
        p_O->fdr10,
        p_O->sm_assigned_guid,
        p_O->cc_sw_cong_setting_threshold,
        p_O->cc_sw_cong_setting_packet_size,
        p_O->cc_sw_cong_setting_credit_starvation_threshold,

        /* ints */
        p_O->m_key_lease_period,
        p_O->sweep_interval,
        p_O->max_wire_smps,
        p_O->max_wire_smps2,
        p_O->max_smps_timeout,
        p_O->transaction_timeout,
        p_O->transaction_retries,
        p_O->sminfo_polling_timeout,
        p_O->polling_retry_number,
        p_O->max_msg_fifo_timeout,
        p_O->console_port,
        p_O->max_reverse_hops,
        p_O->perfmgr_sweep_time_s,
        p_O->perfmgr_max_outstanding_queries,
        p_O->ca_port,
        p_O->part_enforce_enum,
        p_O->scatter_ports,
        p_O->cc_max_outstanding_mads,
        p_O->cc_sw_cong_setting_control_map,
        p_O->cc_sw_cong_setting_marking_rate,
        p_O->cc_ca_cong_setting_port_control,
        p_O->cc_ca_cong_setting_control_map,
        p_O->perfmgr_rm_nodes,
        p_O->perfmgr_xmit_wait_threshold,

        /* longs */
        p_O->guid,
        p_O->m_key,
        p_O->sm_key,
        p_O->sa_key,
        p_O->subnet_prefix,
        p_O->log_max_size,
        p_O->cc_key,

        /* strings */
        (*pJEnv)->NewStringUTF(pJEnv, p_O->config_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->dump_files_dir),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->log_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->partition_config_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->qos_policy_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->console),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->port_prof_ignore_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->hop_weights_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->routing_engine_names),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->lid_matrix_dump_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->lfts_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->root_guid_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->cn_guid_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->io_guid_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->ids_guid_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->guid_routing_order_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->sa_db_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->torus_conf_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->event_db_dump_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->event_plugin_name),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->event_plugin_options),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->node_name_map_name),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->prefix_routes_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->log_prefix),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->ca_name),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->force_link_speed_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->part_enforce),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->port_search_ordering_file),
        (*pJEnv)->NewStringUTF(pJEnv, p_O->per_module_logging_file));


    if (currentObject == NULL)
    {
      J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an Options Object\n");
    }


    return &currentObject;
}


jobjectArray * jsn_getOsmManagers(void * pJenv)
{
  // returning a pointer to an array of objects, so make it static
  static jobject currentObject;
  static jobjectArray objectArray;
  JNIEnv * pJEnv = (JNIEnv *) pJenv;

  /* build an array of managers from the subnet */
  int numObjects = sr_getNumManagers();
  int n_ndex = 0;

  /* TODO these need locks */
  sr_Manager_t * p_Objects = sr_getManagers(); // use the latest from the subnet

  JPC_CLASS Object_Class = PeerClassArray[JPC_SBN_MANAGER_CLASS];

  // create an array to hold the Managers
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of Managers: %d\n", numObjects);

  objectArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numObjects, Object_Class.jpcClass, NULL);
  if (objectArray != NULL)
    {
      // have the array, now create and set the objects

      for (n_ndex = 0; n_ndex < numObjects; n_ndex++, p_Objects++)
        {
          /*  create the object, using the field constructor */
          currentObject = (*pJEnv)->NewObject(pJEnv, Object_Class.jpcClass, Object_Class.constructorMethod->methodID,
              (jshort) p_Objects->pri_state, (jint) p_Objects->act_count, (jlong) p_Objects->guid,
              (jlong)p_Objects->sm_key, (*pJEnv)->NewStringUTF(pJEnv, p_Objects->State));

          if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a Router Object for populating the array: %d\n", n_ndex);
            }
          else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, objectArray, (jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
        }
    }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for Managers\n");

  return &objectArray;
}

jobjectArray * jsn_getOsmRouters(void * pJenv)
{
  // returning a pointer to an array of objects, so make it static
  static jobject currentObject;
  static jobjectArray objectArray;
  JNIEnv * pJEnv = (JNIEnv *) pJenv;

  /* build an array of routers from the subnet */
  int numObjects = sr_getNumRouters();
  int n_ndex = 0;

  /* TODO these need locks */
  sr_Router_t * p_Objects = sr_getRouters(); // use the latest from the subnet

  JPC_CLASS Object_Class = PeerClassArray[JPC_SBN_ROUTER_CLASS];

  // create an array to hold the Routers
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of Routers: %d\n", numObjects);

  objectArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numObjects, Object_Class.jpcClass, NULL);
  if (objectArray != NULL)
    {
      // have the array, now create and set the objects

      for (n_ndex = 0; n_ndex < numObjects; n_ndex++, p_Objects++)
        {
          /*  create the object, using the field constructor */
          currentObject = (*pJEnv)->NewObject(pJEnv, Object_Class.jpcClass, Object_Class.constructorMethod->methodID, (jlong) p_Objects->guid);

          if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a Router Object for populating the array: %d\n", n_ndex);
            }
          else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, objectArray, (jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
        }
    }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for Routers\n");

  return &objectArray;
}

jobjectArray * jsn_getOsmSwitches(void * pJenv)
{
  // returning a pointer to an array of objects, so make it static
  static jobject currentObject;
  static jobjectArray objectArray;
  static jshortArray lftArray;
  static jshortArray hopArray;

  JNIEnv * pJEnv = (JNIEnv *) pJenv;

  /* build an array of switches from the subnet */
  int numObjects = sr_getNumSwitches();
  int n_ndex = 0;

  /* TODO these need locks */
  sr_Switch_t * p_Objects = sr_getSwitches(); // use the latest from the subnet

  JPC_CLASS Object_Class = PeerClassArray[JPC_SBN_SWITCH_CLASS];

  // create an array to hold the Switches
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of Switches: %d\n", numObjects);

  objectArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numObjects, Object_Class.jpcClass, NULL);
  if (objectArray != NULL)
    {
      // have the array, now create and set the objects

      for (n_ndex = 0; n_ndex < numObjects; n_ndex++, p_Objects++)
        {
          lftArray = (jshortArray) (*pJEnv)->NewShortArray(pJenv, (jsize) p_Objects->lft_size);
          hopArray = (jshortArray) (*pJEnv)->NewShortArray(pJenv, (jsize) p_Objects->num_hops);
          if ((lftArray == NULL) || (hopArray == NULL))
          {
            /* holly crap batman, throw an error and get out */
            J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an array for holding the linear forwarding table or hops\n");
          }
          else
          {
              (*pJEnv)->SetShortArrayRegion(pJEnv, lftArray, (jsize) 0, (jsize) p_Objects->lft_size, (jshort *) (p_Objects->lft));
              (*pJEnv)->SetShortArrayRegion(pJEnv, hopArray, (jsize) 0, (jsize) p_Objects->num_hops, (jshort *) (p_Objects->hops));

              /*  create the object, using the field constructor */
              currentObject = (*pJEnv)->NewObject(pJEnv, Object_Class.jpcClass, Object_Class.constructorMethod->methodID,
                  (jshort) p_Objects->num_ports,
                  hopArray, lftArray, (jshort) p_Objects->new_lft, (jshort) p_Objects->is_mc_member,
                  (jint) p_Objects->max_lid_ho, (jint) p_Objects->num_hops, (jint) p_Objects->lft_size,
                  (jint) p_Objects->mft_block_num, (jint) p_Objects->mft_position, (jint) p_Objects->endport_links,
                  (jint) p_Objects->need_update, (jint) p_Objects->num_of_mcm,
                  (jlong) p_Objects->guid);

            if (currentObject == NULL)
            {
                J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a Switch Object for populating the array: %d\n", n_ndex);
            }
            else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, objectArray,(jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
            (*pJEnv)->DeleteLocalRef(pJEnv, lftArray);
            (*pJEnv)->DeleteLocalRef(pJEnv, hopArray);
          }
        }
    }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for Switches\n");

  return &objectArray;
}

jobjectArray * jsn_getOsmMultiCastGroups(void * pJenv)
{
  // returning a pointer to an array of objects, so make it static
  static jobject currentObject;
  static jobjectArray objectArray;
  static jshortArray portNumArray;
  static jlongArray portGuidArray;
  JNIEnv * pJEnv = (JNIEnv *) pJenv;

  /* build an array of multicast groups from the subnet */
  int numObjects = sr_getNumMCGroups();
  int n_ndex = 0;

  sr_MCGroups_t * p_Objects = sr_getMCGroups(); // use the latest from the subnet

  // create an array to hold the Multicast Groups
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of MCGroups: %d\n", numObjects);

  JPC_CLASS Object_Class = PeerClassArray[JPC_SBN_MCGROUP_CLASS];

  objectArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numObjects, Object_Class.jpcClass, NULL);
  if (objectArray != NULL)
    {
      // have the array, now create and set the objects
      for (n_ndex = 0; n_ndex < numObjects; n_ndex++, p_Objects++)
        {
          /* each mcgroup has a bool, int, int, and two arrays */
          portNumArray = (jshortArray) (*pJEnv)->NewShortArray(pJenv, (jsize) p_Objects->port_members);
          portGuidArray = (jlongArray) (*pJEnv)->NewLongArray(pJenv, (jsize) p_Objects->port_members);
          if ((portNumArray == NULL) || (portGuidArray == NULL))
          {
            /* holly crap batman, throw an error and get out */
            J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an array for holding the mcgroup members\n");
          }
          else
          {
              (*pJEnv)->SetShortArrayRegion(pJEnv, portNumArray, (jsize) 0, (jsize) p_Objects->port_members, (jshort *) (p_Objects->port_num_array));
              (*pJEnv)->SetLongArrayRegion(pJEnv, portGuidArray, (jsize) 0, (jsize) p_Objects->port_members, (jlong *) (p_Objects->port_guid_array));

              /*  create a MCGroup object */
            currentObject = (*pJEnv)->NewObject(pJEnv, Object_Class.jpcClass,Object_Class.constructorMethod->methodID,
                (jboolean) p_Objects->well_known, (jint) p_Objects->mlid, (jint) p_Objects->port_members,
                portNumArray, portGuidArray);

            if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a MCGroup Object for populating the array: %d\n", n_ndex);
            }
            else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, objectArray,(jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
            (*pJEnv)->DeleteLocalRef(pJEnv, portNumArray);
            (*pJEnv)->DeleteLocalRef(pJEnv, portGuidArray);
          }
        }
    }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for MCGroups\n");

  return &objectArray;
}

jobjectArray * jsn_getOsmPartitionKeys(void * pJenv)
{
  // returning a pointer to an array of objects, so make it static
  static jobject currentObject;
  static jobjectArray objectArray;
  static jlongArray fullGuidArray;
  static jlongArray partGuidArray;
  JNIEnv * pJEnv = (JNIEnv *) pJenv;

  /* build an array of partitions from the subnet */
  int numObjects = sr_getNumPKeys();
  int n_ndex = 0;

  /* TODO these need locks */
  sr_PKey_t * p_Objects = sr_getPKeys(); // use the latest from the subnet

  JPC_CLASS Object_Class = PeerClassArray[JPC_SBN_PKEY_CLASS];

  // create an array to hold the Partitions
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of PKeys: %d\n", numObjects);

  objectArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numObjects, Object_Class.jpcClass, NULL);
  if (objectArray != NULL)
    {
      // have the array, now create and set the objects

      for (n_ndex = 0; n_ndex < numObjects; n_ndex++, p_Objects++)
        {
          /* each partition key has members, plus two other guid arrays */

          /* arrays for membership */
          fullGuidArray = (jlongArray) (*pJEnv)->NewLongArray(pJenv, (jsize) p_Objects->num_full_membs);
          partGuidArray = (jlongArray) (*pJEnv)->NewLongArray(pJenv, (jsize) p_Objects->num_part_membs);
          if ((fullGuidArray == NULL) || (partGuidArray == NULL))
          {
            /* holly crap batman, throw an error and get out */
            J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an array for holding the partition members\n");
          }
          else
          {
              (*pJEnv)->SetLongArrayRegion(pJEnv, fullGuidArray, (jsize) 0, (jsize) p_Objects->num_full_membs, (jlong *) (p_Objects->full_guid_array));
              (*pJEnv)->SetLongArrayRegion(pJEnv, partGuidArray, (jsize) 0, (jsize) p_Objects->num_part_membs, (jlong *) (p_Objects->part_guid_array));

              /*  create a PKey object */
            currentObject = (*pJEnv)->NewObject(pJEnv, Object_Class.jpcClass,Object_Class.constructorMethod->methodID,
                (jboolean) p_Objects->well_known, (jshort) p_Objects->sl,
                (jint) p_Objects->pkey, (jint) p_Objects->mlid, (jint) p_Objects->full_members,
                partGuidArray, fullGuidArray, (*pJEnv)->NewStringUTF(pJEnv, p_Objects->name));

            if (currentObject == NULL)
            {
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a PKey Object for populating the array: %d\n", n_ndex);
            }
            else
            {
              (*pJEnv)->SetObjectArrayElement(pJEnv, objectArray,(jsize) n_ndex, currentObject);
              (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
            }
            (*pJEnv)->DeleteLocalRef(pJEnv, fullGuidArray);
            (*pJEnv)->DeleteLocalRef(pJEnv, partGuidArray);
          }
        }
    }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for PKeys\n");

  return &objectArray;
}


void * jsn_getOsmSubnet(void * pJenv)
{
  // returning a pointer to an object, so make it static
    static jobject      currentObject;
    JNIEnv * pJEnv        = (JNIEnv *) pJenv;
    JPC_CLASS SUBN_Class         = PeerClassArray[JPC_OSM_SUBNET_CLASS];
    jobject * p_optObject        = jsn_getOsmOptions(pJEnv);
    jobjectArray * p_mgrArray    = jsn_getOsmManagers(pJEnv);
    jobjectArray * p_rtrArray    = jsn_getOsmRouters(pJEnv);
    jobjectArray * p_swtArray    = jsn_getOsmSwitches(pJEnv);
    jobjectArray * p_keyArray    = jsn_getOsmPartitionKeys(pJEnv);
    jobjectArray * p_mgroupArray = jsn_getOsmMultiCastGroups(pJEnv);

    sr_Subnet_t * p_Subnet = sr_getSubnet();

    /* TODO need a lock ? */

    // construct the subnet container object
    currentObject = (*pJEnv)->NewObject(pJEnv, SUBN_Class.jpcClass, SUBN_Class.constructorMethod->methodID,
        p_Subnet->ignore_existing_lfts, p_Subnet->subnet_initialization_error, p_Subnet->force_heavy_sweep,
        p_Subnet->force_reroute, p_Subnet->in_sweep_hop_0, p_Subnet->first_time_master_sweep, p_Subnet->set_client_rereg_on_sweep,
        p_Subnet->coming_out_of_standby, p_Subnet->sweeping_enabled, p_Subnet->min_ca_mtu,
        p_Subnet->min_ca_rate, p_Subnet->min_data_vls, p_Subnet->min_sw_data_vls, p_Subnet->need_update,
        p_Subnet->sm_state, p_Subnet->last_sm_port_state, p_Subnet->max_ucast_lid_ho, p_Subnet->max_mcast_lid_ho,
        p_Subnet->master_sm_base_lid, p_Subnet->sm_base_lid, p_Subnet->sm_port_guid,
        *p_optObject,
        *p_mgrArray,
        *p_rtrArray,
        *p_swtArray,
        *p_keyArray,
        *p_mgroupArray);

    if (currentObject == NULL)
    {
      J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a Subnet Object\n");
    }

    // fully constructed, or failed, either way, clean up
    (*pJEnv)->DeleteLocalRef(pJEnv, *p_optObject);
    (*pJEnv)->DeleteLocalRef(pJEnv, *p_mgrArray);
    (*pJEnv)->DeleteLocalRef(pJEnv, *p_rtrArray);
    (*pJEnv)->DeleteLocalRef(pJEnv, *p_swtArray);
    (*pJEnv)->DeleteLocalRef(pJEnv, *p_keyArray);
    (*pJEnv)->DeleteLocalRef(pJEnv, *p_mgroupArray);

    return (void *) &currentObject;
}

