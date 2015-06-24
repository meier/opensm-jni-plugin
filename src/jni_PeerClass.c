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
 * jni_PeerClass.c
 *
 *  Created on: Jun 2, 2011
 *      Author: meier3
 */
#include <jni.h>
#include <iba/ib_types.h>
#include <opensm/osm_helper.h>
#include <opensm/osm_perfmgr_db.h>

#include "osmJniPi.h"
#include "jni_PeerClass.h"

extern plugin_data_t *       gData; /* the global plugin data */

 JPC_CLASS PeerClassArray[JPC_NUM_PEER_CLASSES];
 JPC_FID   FieldIdArray[JPC_NUM_PEER_CLASSES][MAX_NUM_FIELDS];
 JPC_MID   MethodIdArray[JPC_NUM_PEER_CLASSES][MAX_NUM_METHODS];

/* these are the peer classes, and this ORDER must be maintained for all below */
/* see JPC_PEER_CLASS_TYPE enum in header file. (DEPENDENT CLASSES GO FIRST, keep order!) */
static char *java_peer_class_names[] =
{
    SBN_NODE_CLASS_NAME,
    PFM_NODE_CLASS_NAME,
    OSM_NODE_CLASS_NAME,
    SBN_PORTINFO_CLASS_NAME,
    MLX_PORTINFO_CLASS_NAME,
    SBN_PORT_CLASS_NAME,
    PFM_PORT_CLASS_NAME,
    OSM_PORT_CLASS_NAME,
    OSM_STAT_CLASS_NAME,
    SBN_OPTIONS_CLASS_NAME,
    SBN_MANAGER_CLASS_NAME,
    SBN_ROUTER_CLASS_NAME,
    SBN_SWITCH_CLASS_NAME,
    SBN_PKEY_CLASS_NAME,
    SBN_MCGROUP_CLASS_NAME,
    OSM_SUBNET_CLASS_NAME,
    IB_PORT_CLASS_NAME,
    SBN_NPORTSTAT_CLASS_NAME,
    OSM_SYSINFO_CLASS_NAME,
    OSM_PLUGININFO_CLASS_NAME,
    OSM_EVENT_CLASS_NAME,
    STRING_CLASS_NAME,
};

static int methods_in_class[] =
{
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_ZERO_METHODS,
    NUM_STRING_METHODS,
};

static int fields_in_class[] =
{
    NUM_SBN_NODE_FIELDS,
    NUM_PFM_NODE_FIELDS,
    NUM_OSM_NODE_FIELDS,
    NUM_SBN_PORTINFO_FIELDS,
    NUM_MLX_PORTINFO_FIELDS,
    NUM_SBN_PORT_FIELDS,
    NUM_PFM_PORT_FIELDS,
    NUM_OSM_PORT_FIELDS,
    NUM_OSM_STAT_FIELDS,
    NUM_SBN_OPTIONS_FIELDS,
    NUM_SBN_MANAGER_FIELDS,
    NUM_SBN_ROUTER_FIELDS,
    NUM_SBN_SWITCH_FIELDS,
    NUM_SBN_PKEY_FIELDS,
    NUM_SBN_MCGROUP_FIELDS,
    NUM_OSM_SUBNET_FIELDS,
    NUM_IB_PORT_FIELDS,
    NUM_SBN_NPORTSTAT_FIELDS,
    NUM_OSM_SYSINFO_FIELDS,
    NUM_OSM_PLUGININFO_FIELDS,
    NUM_OSM_EVENT_FIELDS,
    NUM_STRING_FIELDS,
};

static char constructor_method_name[] = "<init>";  // default - class name

/* the order has to match everywhere, but also classes must be defined prior to
 * being used in a container or parent class */
static char *constructor_method_signatures[] =
{
  "(Ljava/lang/String;SSSSIIIIJJJ)V",                               // SBN_Node
  "(Ljava/lang/String;SJZZ)V",                                      // PFM_Node
  "([L"PFM_NODE_CLASS_NAME";[L"SBN_NODE_CLASS_NAME";)V",            // OSM_Nodes
  "(SSSSSSSSSSSSSSSSSSSSSSIIIIIIIIIIIJJ)V",                         // SBN_PortInfo
  "(SSSS)V",                                                        // MLX_ExtPortInfo
  "(SSJJJJL"SBN_PORTINFO_CLASS_NAME";L"MLX_PORTINFO_CLASS_NAME";)V",// SBN_Port
  "(SJJJJ[J)V",                                                     // PFM_Port
  "([L"PFM_PORT_CLASS_NAME";[L"SBN_PORT_CLASS_NAME";)V",            // OSM_Ports
  "(JJJJJJJJJJJ)V",                                                 // OSM_Stats
  // SBN_Options (start - kill me now!)
  "(ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"
  "SSSSSSSSSSSSSSSSSSSSSS"
  "IIIIIIIIIIIIIIIIIIIIIIII"
  "JJJJJJJ"
  "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
  "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
  "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
  "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
  "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
  "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
  // SBN_Options (end)
  "(SIJJLjava/lang/String;)V",                            // SBN_Manager
  "(J)V",                                                 // SBN_Router
  "(S[S[SSSIIIIIIIIJ)V",                                   // SBN_Switch
  "(ZSIII[J[JLjava/lang/String;)V",                       // SBN_PartitionKey
  "(ZII[S[J)V",                                           // SBN_MulticastGroup
  // OSM_Subnet (start)
  "(ZZZZZZZZZSSSSSSSIIIIJ"
  "L"SBN_OPTIONS_CLASS_NAME";[L"SBN_MANAGER_CLASS_NAME";[L"SBN_ROUTER_CLASS_NAME";[L"SBN_SWITCH_CLASS_NAME";[L"SBN_PKEY_CLASS_NAME";[L"SBN_MCGROUP_CLASS_NAME";)V",
  // OSM_Subnet (end)
  "(JILjava/lang/String;)V",                              // IB_Port
  "(JJJJJJJJJJJJJJJJJJJJJ"                                    // SBN_NodePortStatus
  "[L"IB_PORT_CLASS_NAME";[L"IB_PORT_CLASS_NAME";[L"IB_PORT_CLASS_NAME";[L"IB_PORT_CLASS_NAME";[L"IB_PORT_CLASS_NAME";Ljava/lang/String;)V",
  "(IIII"                                // OSM_SysInfo
  "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
  "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;"
  "L"SBN_NPORTSTAT_CLASS_NAME";L"SBN_NPORTSTAT_CLASS_NAME";L"SBN_NPORTSTAT_CLASS_NAME";)V",
  "(IIIJJ)V",                                           // OSM_PluginInfo
  "(IIIIL"IB_PORT_CLASS_NAME";)V",                      // OSM_EventObject
  "()V",                                                  // string
};

/* TODO finish these, for now just implement the constructors */
static char * method_names[][MAX_NUM_CONSTRUCTORS] =
{
    {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,},
    {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,},
    {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,},
    {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,},
    {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,}, {constructor_method_name,},
    {constructor_method_name,}, {constructor_method_name,},
};

static char *method_signatures[][MAX_NUM_CONSTRUCTORS] =
{
    {"()V",}, {"()V",}, {"()V",}, {"()V",}, {"()V",},
    {"()V",}, {"()V",}, {"()V",}, {"()V",}, {"()V",},
    {"()V",}, {"()V",}, {"()V",}, {"()V",}, {"()V",},
    {"()V",}, {"()V",}, {"()V",}, {"()V",}, {"()V",},
    {"()V",}, {"()V",},
};

static char * field_names[][MAX_NUM_FIELDS] =
{
  {
    "description",                    // SBN_Node
    "base_version",
    "class_version",
    "node_type",
    "num_ports",
    "partition_cap",
    "device_id",
    "revision",
    "port_num_vendor_id",
    "sys_guid",
    "node_guid",
    "port_guid",
  },
  {
      "node_name",          // PFM_Node
      "num_ports",
      "node_guid",
      "esp0",
      "active",
  },
  {
      "PerfMgrNodes",      // OSM_Nodes
      "SubnNodes",
  },
  {
      "local_port_num",      // SBN_PortInfo
      "link_width_enabled",
      "link_width_supported",
      "link_width_active",
      "state_info1",
      "state_info2",
      "mkey_lmc",
      "link_speed",
      "link_speed_ext",
      "link_speed_ext_enabled",
      "mtu_smsl",
      "vl_cap",
      "vl_high_limit",
      "vl_arb_high_cap",
      "vl_arb_low_cap",
      "mtu_cap",
      "vl_stall_life",
      "vl_enforce",
      "guid_cap",
      "subnet_timeout",
      "resp_time_value",
      "error_threshold",
      "base_lid",
      "master_sm_base_lid",
      "capability_mask",
      "capability_mask2",
      "diag_code",
      "m_key_lease_period",
      "m_key_violations",
      "p_key_violations",
      "q_key_violations",
      "max_credit_hint",
      "link_rt_latency",
      "m_key",
      "subnet_prefix",
  },
  {
      "state_change_enable",      // MLX_ExtPortInfo
      "link_speed_supported",
      "link_speed_enabled",
      "link_speed_active",
  },
  {
      "port_num",      // SBN_Port
      "linked_port_num",
      "node_guid",
      "port_guid",
      "linked_node_guid",
      "linked_port_guid",
      "port_info",
      "ext_port_info",
  },
  {
      "port_num",      // PFM_Port
      "node_guid",
      "counter_ts",
      "error_ts",
      "wait_ts",
      "port_counters",
  },
  {
      "PerfMgrPorts",      // OSM_Ports
      "SubnPorts",
  },
  {
      "qp0_mads_outstanding",  // OSM_Stats
      "qp0_mads_outstanding_on_wire",
      "qp0_mads_rcvd",
      "qp0_mads_sent",
      "qp0_unicasts_sent",
      "qp0_mads_rcvd_unknown",
      "sa_mads_outstanding",
      "sa_mads_rcvd",
      "sa_mads_sent",
      "sa_mads_rcvd_unknown",
      "sa_mads_ignored",
  },
  {
      "lmc_esp0",      // SBN_Options
      "reassign_lids",
      "ignore_other_sm",
      "single_thread",
      "disable_multicast",
      "force_log_flush",
      "use_mfttop",
      "force_heavy_sweep",
      "no_partition_enforcement",
      "qos",
      "accum_log_file",
      "port_profile_switch_nodes",
      "sweep_on_trap",
      "use_ucast_cache",
      "connect_roots",
      "sa_db_dump",
      "do_mesh_analysis",
      "exit_on_fatal",
      "honor_guid2lid_file",
      "daemon",
      "sm_inactive",
      "babbling_port_policy",
      "use_optimized_slvl",
      "enable_quirks",
      "no_clients_rereg",
      "perfmgr",
      "perfmgr_redir",
      "consolidate_ipv6_snm_req",
      "m_key_lookup",
      "allow_both_pkeys",
      "port_shifting",
      "remote_guid_sorting",
      "guid_routing_order_no_scatter",
      "drop_event_subscriptions",
      "fsync_high_avail_files",
      "congestion_control",
      "perfmgr_ignore_cas",
      "perfmgr_log_errors",
      "perfmgr_query_cpi",
      "perfmgr_xmit_wait_log",

      "sm_priority",
      "lmc",
      "max_op_vls",
      "force_link_speed",
      "subnet_timeout",
      "packet_life_time",
      "vl_stall_count",
      "leaf_vl_stall_count",
      "head_of_queue_lifetime",
      "leaf_head_of_queue_lifetime",
      "local_phy_errors_threshold",
      "overrun_errors_threshold",
      "log_flags",
      "lash_start_vl",
      "sm_sl",
      "m_key_protect_bits",
      "force_link_speed_ext",
      "fdr10",
      "sm_assigned_guid",
      "cc_sw_cong_setting_threshold",
      "cc_sw_cong_setting_packet_size",
      "cc_sw_cong_setting_credit_starvation_threshold",

      "m_key_lease_period",
      "sweep_interval",
      "max_wire_smps",
      "max_wire_smps2",
      "max_smps_timeout",
      "transaction_timeout",
      "transaction_retries",
      "sminfo_polling_timeout",
      "polling_retry_number",
      "max_msg_fifo_timeout",
      "console_port",
      "max_reverse_hops",
      "perfmgr_sweep_time_s",
      "perfmgr_max_outstanding_queries",
      "ca_port",
      "part_enforce_enum",
      "scatter_ports",
      "cc_max_outstanding_mads",
      "cc_sw_cong_setting_control_map",
      "cc_sw_cong_setting_marking_rate",
      "cc_ca_cong_setting_port_control",
      "cc_ca_cong_setting_control_map",
      "perfmgr_rm_nodes",
      "perfmgr_xmit_wait_threshold",

      "guid",
      "m_key",
      "sm_key",
      "sa_key",
      "subnet_prefix",
      "log_max_size",
      "cc_key",

      "config_file",
      "dump_files_dir",
      "log_file",
      "partition_config_file",
      "qos_policy_file",
      "console",
      "port_prof_ignore_file",
      "hop_weights_file",
      "routing_engine_names",
      "lid_matrix_dump_file",
      "lfts_file",
      "root_guid_file",
      "cn_guid_file",
      "io_guid_file",
      "ids_guid_file",
      "guid_routing_order_file",
      "sa_db_file",
      "torus_conf_file",
      "event_db_dump_file",
      "event_plugin_name",
      "event_plugin_options",
      "node_name_map_name",
      "prefix_routes_file",
      "log_prefix",
      "ca_name", /* alternative to guid */
      "force_link_speed_file",
      "part_enforce",
      "port_search_ordering_file",
      "per_module_logging_file",
  },
  {
      "pri_state",        // SBN_Manager
      "act_count",

      "guid",
      "sm_key",
      "State",
  },
  {
      "guid",            // SBN_Router
  },
  {
      "num_ports",       // SBN_Switch
      "hops",
      "lft",
      "new_lft",
      "is_mc_member",

      "max_lid_ho",
      "num_hops",
      "lft_size",
      "mft_block_num",
      "mft_position",
      "endport_links",
      "need_update",
      "num_of_mcm",

      "guid",
  },
  {
      "well_known",                 // SBN_PartitionKey

      "sl",
      "pkey",
      "mlid",
      "full_members",

      "partial_member_guids",
      "full_member_guids",

      "Name",
  },
  {
      "well_known",                 // SBN_MulticastGroup
      "mlid",
      "port_members",
      "port_number",
      "port_guids",
  },
  {
      "ignore_existing_lfts",        // OSM_Subnet
      "subnet_initialization_error",
      "force_heavy_sweep",
      "force_reroute",
      "in_sweep_hop_0",
      "first_time_master_sweep",
      "set_client_rereg_on_sweep",
      "coming_out_of_standby",
      "sweeping_enabled",

      "min_ca_mtu",
      "min_ca_rate",
      "min_data_vls",
      "min_sw_data_vls",
      "need_update",
      "sm_state",
      "last_sm_port_state",

      "max_ucast_lid_ho",
      "max_mcast_lid_ho",
      "master_sm_base_lid",
      "sm_base_lid",

      "sm_port_guid",

      "Options",
      "Managers",
      "Routers",
      "Switches",
      "PKeys",
      "MCGroups",
  },
  {                     // IB_Port
  },
  {
      "total_nodes",    // SBN_NPortStatus
      "total_ports",
      "ports_down",
      "ports_active",
      "ports_disabled",
      "ports_1X",
      "ports_4X",
      "ports_8X",
      "ports_12X",
      "ports_unknown_width",
      "ports_reduced_width",
      "ports_sdr",
      "ports_ddr",
      "ports_qdr",
      "ports_unknown_speed",
      "ports_reduced_speed",
      "ports_unenabled_width",
      "ports_fdr10",
      "ports_fdr",
      "ports_edr",
      "ports_unenabled_speed",

      "disabled_ports",
      "reduced_width_ports",
      "reduced_speed_ports",
      "unenabled_width_ports",
      "unenabled_speed_ports",

      "NodeType",
  },
  {
      "SM_Priority",    // OSM_SysInfo
      "PM_SweepTime",
      "PM_OutstandingQueries",
      "PM_MaximumQueries",

       "OpenSM_Version",
       "OsmJpi_Version",
       "SM_State",
       "SA_State",
       "PM_State",
       "PM_SweepState",
       "RoutingEngine",
       "EventPlugins",

      "CA_PortStatus",
      "SW_PortStatus",
      "RT_PortStatus",
  },
  {                     // OSM_PluginInfo
      "NativeUpdatePeriodSecs",
      "NativeReportPeriodSecs",
      "NativeEventTimeoutMsecs",
      "NativeUpdateCount",
      "NativeEventCount",
  },
  {                     // OSM_EventObject
      "EventId",
      "TrapType",
      "TrapNum",
      "TrapLID",
      "Port",
  },
  {
    "stringID",
  },
};
static char *field_signatures[][MAX_NUM_FIELDS] =
{
  {
      "Ljava/lang/String;",                    // SBN_Node
      "S",
      "S",
      "S",
      "S",
      "I",
      "I",
      "I",
      "I",
      "J",
      "J",
      "J",
  },
  {
      "Ljava/lang/String;",                    // PFM_Node
      "S",
      "J",
      "Z",
      "Z",
  },
  {
      "[L"PFM_NODE_CLASS_NAME";",   // OSM_Nodes
      "[L"SBN_NODE_CLASS_NAME";",
  },
  {
      "S",                    // SBN_PortInfo
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "J",
      "J",
  },
  {
      "S",                    // MLX_ExtPortInfo
      "S",
      "S",
      "S",
  },
  {
      "S",                    // SBN_Port
      "S",
      "J",
      "J",
      "J",
      "J",
      "L"SBN_PORTINFO_CLASS_NAME";",
      "L"MLX_PORTINFO_CLASS_NAME";",
  },
  {
      "S",                    // PFM_Port
      "J",
      "J",
      "J",
      "J",
      "[J",
  },
  {
      "[L"PFM_PORT_CLASS_NAME";",                    // OSM_Ports
      "[L"SBN_PORT_CLASS_NAME";",
  },
  {
      "J",                    // OSM_Stats
      "J",
      "J",
      "J",
      "J",
      "J",
      "J",
      "J",
      "J",
      "J",
      "J",
  },
  {
      "Z","Z","Z","Z","Z","Z","Z","Z","Z","Z", // SBN_Options
      "Z","Z","Z","Z","Z","Z","Z","Z","Z","Z",
      "Z","Z","Z","Z","Z","Z","Z","Z","Z","Z",
      "Z","Z","Z","Z","Z","Z","Z","Z","Z","Z",
      "S","S","S","S","S","S","S","S","S","S",
      "S","S","S","S","S","S","S","S","S","S",
      "S","S",
      "I","I","I","I","I","I","I","I","I","I",
      "I","I","I","I","I","I","I","I","I","I",
      "I","I","I","I",
      "J","J","J","J","J","J","J",
      "Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;",
      "Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;",
      "Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;",
      "Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;",
      "Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;",
      "Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;","Ljava/lang/String;",
  },
  {
      "S",                                    // SBN_Manager
      "I",
      "J",
      "J",
      "Ljava/lang/String;",
  },
  {
      "J",                                    // SBN_Router
  },
  {
      "S",                                    // SBN_Switch
      "[S",
      "[S",
      "S",
      "S",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "I",
      "J",
  },
  {
      "Z",                                     // SBN_PartitionKey
      "S",
      "I",
      "I",
      "I",
      "[J",
      "[J",
      "Ljava/lang/String;",
  },
  {
      "Z",                                     // SBN_MulticastGroup
      "I",
      "I",
      "[S",
      "[J",
  },
  {
      "Z",                                     // OSM_Subnet
      "Z",
      "Z",
      "Z",
      "Z",
      "Z",
      "Z",
      "Z",
      "Z",

      "S",
      "S",
      "S",
      "S",
      "S",
      "S",
      "S",

      "I",
      "I",
      "I",
      "I",
      "J",

      "L"SBN_OPTIONS_CLASS_NAME";",
      "[L"SBN_MANAGER_CLASS_NAME";",
      "[L"SBN_ROUTER_CLASS_NAME";",
      "[L"SBN_SWITCH_CLASS_NAME";",
      "[L"SBN_PKEY_CLASS_NAME";",
      "[L"SBN_MCGROUP_CLASS_NAME";",
  },
  {
      // IB_Port
  },
  {
      // SBN_NPortStatus
      "J","J","J","J","J",
      "J","J","J","J","J",
      "J","J","J","J","J",
      "J","J","J","J","J",
      "J",
      "[L"IB_PORT_CLASS_NAME";","[L"IB_PORT_CLASS_NAME";","[L"IB_PORT_CLASS_NAME";","[L"IB_PORT_CLASS_NAME";","[L"IB_PORT_CLASS_NAME";",
      "Ljava/lang/String;",
  },
  {
      "I",     // OSM_SysInfo
      "I",
      "I",
      "I",
      "Ljava/lang/String;",
      "Ljava/lang/String;",
      "Ljava/lang/String;",
      "Ljava/lang/String;",
      "Ljava/lang/String;",
      "Ljava/lang/String;",
      "Ljava/lang/String;",
      "[Ljava/lang/String;",
      "L"SBN_NPORTSTAT_CLASS_NAME";",
      "L"SBN_NPORTSTAT_CLASS_NAME";",
      "L"SBN_NPORTSTAT_CLASS_NAME";",
  },
  {
      // OSM_PluginInfo
      "I",
      "I",
      "I",
      "J",
      "J",
  },
  {
      // OSM_EventObject
      "I",
      "I",
      "I",
      "I",
      "L"IB_PORT_CLASS_NAME";",
  },
  {
     "J",
  },
};


/*
 * ======================================================================
 */

/************************************************************************
*** Function: jpc_initFieldID
***
*** This is a JNI optimization.  Finds the references to a known field in a
*** class, and caches it for later use.
***
*** Called by jpc_initAllFields()
*** <p>
***
*** created:  9/22/2003 (3:05:33 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int jpc_initFieldID(JNIEnv * pJEnv, jclass jClass, JPC_FID * fidStruct)
{
  int success = 0;
  jfieldID tempFID = NULL;

  tempFID = (*pJEnv)->GetFieldID(pJEnv, jClass, fidStruct->fieldName , fidStruct->fieldSignature);

  if(tempFID == NULL)
  {
    J_LOG(gData, OSM_LOG_ERROR,"JNI cannot create a field Id for (%s) using signature (%s)!\n", fidStruct->fieldName, fidStruct->fieldSignature);
  }
  else
  {
    fidStruct->fieldID = tempFID;
    success = 1;
  }
  return success;
}
/*-----------------------------------------------------------------------*/


/******************************************************************************
*** Function: jpc_initAllFields
***
*** This is a JNI optimization.  Finds the references to the known fields in a
*** class, and caches them for later use.  Primarily used by the default
*** constructor of a class.
***
*** Called by jpc_initPeerClass()
*** <p>
***
*** created:  9/22/2003 (3:05:33 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int jpc_initAllFields(JNIEnv * pJEnv, JPC_CLASS * classStruct)
{
  int success                     = 1;
  int index                       = classStruct->classIndex;
  int j;
  int numFields                   = classStruct->numFields;
  jclass jClass                   = classStruct->jpcClass;

  JPC_FID *pFID;

  for(j = 0; j <  numFields; j++)
  {
    pFID = &(classStruct->fieldArray[j]);
    pFID->fieldName      = field_names[index][j];
    pFID->fieldSignature = field_signatures[index][j];
    if( !jpc_initFieldID( pJEnv, jClass, pFID) )
    {
      J_LOG(gData, OSM_LOG_ERROR,"JNI cannot create a field Id for (%s)!\n", pFID->fieldName);
      success = 0;
    }
  }
  return success;
}
/*-----------------------------------------------------------------------*/


/******************************************************************************
*** Function: jpc_initConstructorID
***
*** This is a JNI optimization.  Finds the references to the known methods in a
*** class, and caches them for later use.
***
*** Called by jpc_initPeerClass()
*** <p>
***
*** created:  9/22/2003 (5:04:23 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int jpc_initConstructorID(JNIEnv * pJEnv, jclass jClass, JPC_MID * midStruct)
{
  int success = 0;
  jmethodID tempMID = NULL;

  tempMID = (*pJEnv)->GetMethodID(pJEnv, jClass, midStruct->methodName , midStruct->methodSignature);

  if(tempMID == NULL)
  {
    J_LOG(gData, OSM_LOG_ERROR,"JNI cannot create a method Id for (%s) {%s}!\n", midStruct->methodName, midStruct->methodSignature);
  }
  else
  {
    midStruct->methodID = tempMID;
    success = 1;
  }
  return success;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
*** Function: jpc_initMethodID
***
*** This is a JNI optimization.  Finds the references to the known methods in a
*** class, and caches them for later use.
***
*** Called by jpc_initPeerClass()
*** <p>
***
*** created:  9/22/2003 (5:04:23 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int jpc_initMethodID(JNIEnv * pJEnv, jclass jClass, JPC_MID * midStruct)
{
  int success = 0;
  jmethodID tempMID = NULL;

  tempMID = (*pJEnv)->GetMethodID(pJEnv, jClass, midStruct->methodName , midStruct->methodSignature);

  if(tempMID == NULL)
  {
    J_LOG(gData, OSM_LOG_ERROR,"JNI cannot create a method Id for (%s)!\n", midStruct->methodName);
  }
  else
  {
    midStruct->methodID = tempMID;
    success = 1;
  }
  return success;
}
/*-----------------------------------------------------------------------*/

/******************************************************************************
*** Function: jpc_initAllMethods
***
*** This is a JNI optimization.  Finds the references to the known methods in a
*** class, and caches them for later use.
***
*** Called by jpc_initPeerClass()
*** <p>
***
*** created:  9/22/2003 (3:05:33 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int jpc_initAllMethods(JNIEnv * pJEnv, JPC_CLASS * classStruct)
{
  int success                     = 1;
  int index                       = classStruct->classIndex;
  int j;
  int numMethods                  = classStruct->numMethods;
  jclass jClass                   = classStruct->jpcClass;

  JPC_MID *pMID;

  for(j = 0; j <  numMethods; j++)
  {
    pMID = &(classStruct->methodArray[j]);
    pMID->methodName      = method_names[index][j];
    pMID->methodSignature = method_signatures[index][j];
    if( !jpc_initMethodID( pJEnv, jClass, pMID) )
    {
      J_LOG(gData, OSM_LOG_ERROR,"JNI cannot create a method Id for (%s)!\n", pMID->methodName);
      success = 0;
    }
  }
  return success;
}
/*-----------------------------------------------------------------------*/



/******************************************************************************
*** Function: jpc_initPeerClasses
***
*** This is a JNI optimization.  Finds the references to the known classes that
*** will be used within C, and cache them for later use.
***
*** Called by jpc_initJniReferences()
*** <p>
***
*** created:  9/22/2003 (4:46:46 PM)
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int jpc_initPeerClass(JNIEnv * pJEnv, JPC_CLASS * classStruct)
{
  int success = 0;
  int index   = classStruct->classIndex;

  jclass localClassRef;

  classStruct->className = java_peer_class_names[index];

  localClassRef = (*pJEnv)->FindClass(pJEnv, classStruct->className );
  if(localClassRef == NULL)
  {
    J_LOG(gData, OSM_LOG_ERROR,"JNI cannot create an local reference for (%s)!\n", classStruct->className);
  }
  else
  {
    classStruct->jpcClass = (*pJEnv)->NewGlobalRef(pJEnv, localClassRef);
    if(classStruct->jpcClass == NULL)
    {
      J_LOG(gData, OSM_LOG_ERROR,"JNI cannot create an global class reference for (%s)!\n", classStruct->className);
    }
    else
    {
      // success so far, now get a reference to the constructor
      classStruct->constructorMethod->methodName      = constructor_method_name;
      classStruct->constructorMethod->methodSignature = constructor_method_signatures[index];
      if(jpc_initConstructorID( pJEnv, classStruct->jpcClass, classStruct->constructorMethod ))
      {

        success = jpc_initAllMethods(pJEnv, classStruct);

        // finally, fill in all the field ids
        success = jpc_initAllFields(pJEnv, classStruct);
      }
    }
    // get rid of the local references
    (*pJEnv)->DeleteLocalRef(pJEnv, localClassRef);
  }
  return success;
}
/*-----------------------------------------------------------------------*/


/**************************************************************************
*** Method Name:
***     jpc_initJniReferences
**/
/**
*** This is a JNI optimization.  Find all the references to the known data
*** structures and methods, and cache them for later use.  This causes the
*** initialization phase (one-time event) to be a bit slower, and consume
*** memory, with the benefit of much faster execution times.
*** <p>
***
*** @see          Method_related_to_this_method
***
*** @param        Parameter_name  Description_of_method_parameter__Delete_if_none
***
*** @return       Description_of_method_return_value__Delete_if_none
***
*** @throws       Class_name  Description_of_exception_thrown__Delete_if_none
**************************************************************************/

int jpc_initJniReferences(void * pJenv)
{
  int success = 1;
  int numClasses = JPC_NUM_PEER_CLASSES;  // JPC_NUM_PEER_CLASSES
  int j;
  JNIEnv * pJEnv = (JNIEnv *)pJenv;

  for(j = 0; j < numClasses; j++)
  {
    // initialize as much of the class as possible
    PeerClassArray[j].classIndex        = j;
    PeerClassArray[j].className         = java_peer_class_names[j];
    PeerClassArray[j].constructorMethod = &(MethodIdArray[j][0]);
    PeerClassArray[j].methodArray       = &(MethodIdArray[j][0]);
    PeerClassArray[j].fieldArray        = &(FieldIdArray[j][0]);
    PeerClassArray[j].numMethods        = methods_in_class[j];
    PeerClassArray[j].numFields         = fields_in_class[j];

    // and then go get the references for fields and methods in each class
    //  *** note:  if a class depends upon another class, then they need to be
    //  *** initialized in order.  Preserve the original array order, dependencies
    //  *** are satisfied!

    if(!jpc_initPeerClass(pJEnv, &PeerClassArray[j]) )
    {
      J_LOG(gData, OSM_LOG_ERROR,"JNI cannot create an global reference for (%s)!\n", PeerClassArray[j].className);
      success = 0;
    }
  }
  return success;
}
/*-----------------------------------------------------------------------*/

/**************************************************************************
*** Method Name:
***     jpc_printPeerClass
**/
/**
*** Print out the constructor, method, and field information about this
*** class.  For diagnostic use.
*** <p>
***
*** @see          Method_related_to_this_method
***
*** @param        Parameter_name  Description_of_method_parameter__Delete_if_none
***
*** @return       Description_of_method_return_value__Delete_if_none
***
*** @throws       Class_name  Description_of_exception_thrown__Delete_if_none
**************************************************************************/

int jpc_printPeerClass(JPC_CLASS * peerClass)
{
  int success = 1;
  int j;
  JPC_MID * p_cmid = peerClass->constructorMethod;
  JPC_MID * p_mid  = peerClass->methodArray;
  JPC_FID * p_fid  = peerClass->fieldArray;

  J_LOG(gData, OSM_LOG_INFO,"%2d) Peer Class (%s)\n", peerClass->classIndex, peerClass->className);
  J_LOG(gData, OSM_LOG_INFO,"\tConstructor name (%s)\n", p_cmid->methodName);
  J_LOG(gData, OSM_LOG_INFO,"\tConstructor signature (%s)\n", p_cmid->methodSignature);

  J_LOG(gData, OSM_LOG_INFO,"\tNumber of methods (%d)\n", peerClass->numMethods);
  for(j = 0; j < peerClass->numMethods; j++, p_mid++)
  {
      J_LOG(gData, OSM_LOG_INFO,"\tMethod name (%s)\n", p_mid->methodName);
      J_LOG(gData, OSM_LOG_INFO,"\tMethod signature (%s)\n", p_mid->methodSignature);
  }

  J_LOG(gData, OSM_LOG_INFO,"\tNumber of fields (%d)\n", peerClass->numFields);
  for(j = 0; j < peerClass->numFields; j++, p_fid++)
  {
      J_LOG(gData, OSM_LOG_INFO,"\tField name (%s)\n", p_fid->fieldName);
      J_LOG(gData, OSM_LOG_INFO,"\tField signature (%s)\n", p_fid->fieldSignature);
  }
  return success;
}
/*-----------------------------------------------------------------------*/

/**************************************************************************
*** Method Name:
***     jpc_printJniReferences
**/
/**
*** Print out all the Peer Class names, constructor and method signatures
*** and field names and types.  For diagnostic use.
*** <p>
***
*** @see          Method_related_to_this_method
***
*** @param        Parameter_name  Description_of_method_parameter__Delete_if_none
***
*** @return       Description_of_method_return_value__Delete_if_none
***
*** @throws       Class_name  Description_of_exception_thrown__Delete_if_none
**************************************************************************/

int jpc_printJniReferences()
{
  int success = 1;
  int numClasses = JPC_NUM_PEER_CLASSES;  // JPC_NUM_PEER_CLASSES
  int j;

  for(j = 0; j < numClasses; j++)
  {
    if(!jpc_printPeerClass(&PeerClassArray[j]) )
    {
      J_LOG(gData, OSM_LOG_ERROR,"JNI cannot print the references for Peer Class (%s)!\n", PeerClassArray[j].className);
      success = 0;
    }
  }
  return success;
}
/*-----------------------------------------------------------------------*/

