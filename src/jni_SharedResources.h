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
 * jni_SharedResources.h
 *
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */

#ifndef JNI_SHAREDRESOURCES_H_
#define JNI_SHAREDRESOURCES_H_

#include <complib/cl_types_osd.h>
#include <stddef.h>
#include <stdint.h>

//#include "/hdd1/meier3/javaRepo/jdk1.6.0_41/jre/include/jni.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <iba/ib_types.h>
#include <jni.h>

  /** =========================================================================
   * Port Error Counters
   */

/* TODO replace these to enable static memory allocation */
#define MAX_NUM_NODES       (5000)
#define MAX_NUM_SWITCHES    (4000)
#define MAX_NUM_PORTS      (20000)
#define MAX_NUM_ROUTERS       (32)
#define MAX_NUM_MANAGERS       (8)
#define MAX_NUM_PARTITIONS     (8)
#define MAX_NUM_MCGROUPS    (8192)  /* max mgid value, normally total num ports */
#define MAX_NUM_PLUGINS       (16)

#define MAX_NODE_NAME_SIZE    (65)
#define MAX_STRING_SIZE      (128)  /* TODO redundant, eliminate one */

#define EVENT_QUEUE_SIZE       (5)    /* 5 is the smallest queue possible */

  // indexes for the port counter arrays  (keep this order)
  enum PORT_COUNTER_TYPE
  {
    SYMBOL_ERR_CNT = 0,
    LINK_ERR_RECOVER,
    LINK_DOWNED,
    RCV_ERR,
    RCV_REM_PHYS_ERR,
    RCV_SWITCH_RELAY_ERR,
    XMIT_DISCARDS,
    XMIT_CONSTRAINT_ERR,
    RCV_CONSTRAINT_ERR,
    LINK_INTEGRITY,
    BUFFER_OVERRUN,
    VL15_DROPPED,
    XMIT_DATA,
    RCV_DATA,
    XMIT_PKTS,
    RCV_PKTS,
    UNICAST_XMIT_PKTS,
    UNICAST_RCV_PKTS,
    MULTICAST_XMIT_PKTS,
    MULTICAST_RCV_PKTS,
    XMIT_WAIT,
    NUM_PORT_COUNTERS    // always last, this enum defines the order!!
  };
//
//  typedef struct ib_port_errors
//{
//  uint64_t symbol_err_cnt;
//  uint64_t link_err_recover;
//  uint64_t link_downed;
//  uint64_t rcv_err;
//  uint64_t rcv_rem_phys_err;
//  uint64_t rcv_switch_relay_err;
//  uint64_t xmit_discards;
//  uint64_t xmit_constraint_err;
//  uint64_t rcv_constraint_err;
//  uint64_t link_integrity;
//  uint64_t buffer_overrun;
//  uint64_t vl15_dropped;
//  time_t time_stamp;
//} jsr_PortError_t;
//
//typedef struct ib_port_counters
//{
//  uint64_t xmit_data;
//  uint64_t rcv_data;
//  uint64_t xmit_pkts;
//  uint64_t rcv_pkts;
//  uint64_t unicast_xmit_pkts;
//  uint64_t unicast_rcv_pkts;
//  uint64_t multicast_xmit_pkts;
//  uint64_t multicast_rcv_pkts;
//  time_t time_stamp;
//} jsr_PortCounter_t;
//
//typedef struct ib_port_wait_counters
//{
//  uint64_t xmit_wait;
//  time_t time_stamp;
//} jsr_PortWaitCounter_t;
//
//typedef struct ib_port
//{
//  uint64_t node_guid;
//  uint64_t guid;
//  ib_net16_t lid;
//  uint8_t port_num;
//  ib_net16_t pkey;
//  jsr_PortError_t errors;
//  jsr_PortCounter_t counters;
//  jsr_PortWaitCounter_t wait_cnt;
//} jsr_Port_t;
//
//typedef struct ib_node
//{
//  uint64_t guid;
//  char *name;
//  uint8_t num_ports;
//  uint8_t node_type;
//  ib_net64_t sys_guid;
//  ib_net16_t device_id;
//  jsr_Port_t port[1];
//} jsr_Node_t;
//
typedef struct pm_node
{
  uint64_t node_guid;
  char node_name[MAX_NODE_NAME_SIZE];
  uint8_t num_ports;
  boolean_t esp0;
  boolean_t active;
} pm_Node_t;

/* from the perf mgr */
typedef struct pm_port
{
  uint64_t node_guid;
  uint8_t port_num;
  uint64_t port_counters[NUM_PORT_COUNTERS];
  time_t counter_ts;
  time_t error_ts;
  time_t wait_ts;
} pm_Port_t;


/* the new 12-18-2018 struct
 */

//typedef struct osm_subn_opt {
//	const char *config_file;
//	ib_net64_t guid;
//	ib_net64_t m_key;
//	ib_net64_t sm_key;
//	ib_net64_t sa_key;
//	ib_net64_t subnet_prefix;
//	ib_net16_t m_key_lease_period;
//	uint8_t m_key_protect_bits;
//	boolean_t m_key_lookup;
//	uint32_t sweep_interval;
//	uint32_t max_wire_smps;
//	uint32_t max_wire_smps2;
//	uint32_t max_smps_timeout;
//	uint32_t transaction_timeout;
//	uint32_t transaction_retries;
//	uint32_t long_transaction_timeout;
//	uint8_t sm_priority;
//	uint8_t lmc;
//	boolean_t lmc_esp0;
//	uint8_t max_op_vls;
//	uint8_t force_link_speed;
//	uint8_t force_link_speed_ext;
//	uint8_t force_link_width;
//	uint8_t fdr10;
//	boolean_t reassign_lids;
//	boolean_t ignore_other_sm;
//	boolean_t single_thread;
//	boolean_t disable_multicast;
//	boolean_t force_log_flush;
//	uint8_t subnet_timeout;
//	uint8_t packet_life_time;
//	uint8_t vl_stall_count;
//	uint8_t leaf_vl_stall_count;
//	uint8_t head_of_queue_lifetime;
//	uint8_t leaf_head_of_queue_lifetime;
//	uint8_t local_phy_errors_threshold;
//	uint8_t overrun_errors_threshold;
//	boolean_t use_mfttop;
//	uint32_t sminfo_polling_timeout;
//	uint32_t polling_retry_number;
//	uint32_t max_msg_fifo_timeout;
//	boolean_t force_heavy_sweep;
//	uint8_t log_flags;
//	char *dump_files_dir;
//	char *log_file;
//	uint32_t log_max_size;
//	char *partition_config_file;
//	boolean_t no_partition_enforcement;
//	char *part_enforce;
//	osm_partition_enforce_type_enum part_enforce_enum;
//	boolean_t allow_both_pkeys;
//	boolean_t keep_pkey_indexes;
//	uint8_t sm_assigned_guid;
//	boolean_t qos;
//	char *qos_policy_file;
//	boolean_t suppress_sl2vl_mad_status_errors;
//	boolean_t accum_log_file;
//	char *console;
//	uint16_t console_port;
//	char *port_prof_ignore_file;
//	char *hop_weights_file;
//	char *port_search_ordering_file;
//	boolean_t port_profile_switch_nodes;
//	boolean_t sweep_on_trap;
//	char *routing_engine_names;
//	boolean_t avoid_throttled_links;
//	boolean_t use_ucast_cache;
//	boolean_t connect_roots;
//	char *lid_matrix_dump_file;
//	char *lfts_file;
//	char *root_guid_file;
//	char *cn_guid_file;
//	char *io_guid_file;
//	boolean_t port_shifting;
//	uint32_t scatter_ports;
//	uint16_t max_reverse_hops;
//	char *ids_guid_file;
//	char *guid_routing_order_file;
//	boolean_t guid_routing_order_no_scatter;
//	char *sa_db_file;
//	boolean_t sa_db_dump;
//	char *torus_conf_file;
//	boolean_t do_mesh_analysis;
//	boolean_t exit_on_fatal;
//	boolean_t honor_guid2lid_file;
//	boolean_t daemon;
//	boolean_t sm_inactive;
//	boolean_t babbling_port_policy;
//	boolean_t drop_event_subscriptions;
//	boolean_t ipoib_mcgroup_creation_validation;
//	boolean_t mcgroup_join_validation;
//	boolean_t use_original_extended_sa_rates_only;
//	boolean_t use_optimized_slvl;
//	boolean_t fsync_high_avail_files;
//	osm_qos_options_t qos_options;
//	osm_qos_options_t qos_ca_options;
//	osm_qos_options_t qos_sw0_options;
//	osm_qos_options_t qos_swe_options;
//	osm_qos_options_t qos_rtr_options;
//	boolean_t congestion_control;
//	ib_net64_t cc_key;
//	uint32_t cc_max_outstanding_mads;
//	ib_net32_t cc_sw_cong_setting_control_map;
//	uint8_t cc_sw_cong_setting_victim_mask[IB_CC_PORT_MASK_DATA_SIZE];
//	uint8_t cc_sw_cong_setting_credit_mask[IB_CC_PORT_MASK_DATA_SIZE];
//	uint8_t cc_sw_cong_setting_threshold;
//	uint8_t cc_sw_cong_setting_packet_size;
//	uint8_t cc_sw_cong_setting_credit_starvation_threshold;
//	osm_cct_entry_t cc_sw_cong_setting_credit_starvation_return_delay;
//	ib_net16_t cc_sw_cong_setting_marking_rate;
//	ib_net16_t cc_ca_cong_setting_port_control;
//	ib_net16_t cc_ca_cong_setting_control_map;
//	osm_cacongestion_entry_t cc_ca_cong_entries[IB_CA_CONG_ENTRY_DATA_SIZE];
//	osm_cct_t cc_cct;
//	boolean_t enable_quirks;
//	boolean_t no_clients_rereg;
//#ifdef ENABLE_OSM_PERF_MGR
//	boolean_t perfmgr;
//	boolean_t perfmgr_redir;
//	uint16_t perfmgr_sweep_time_s;
//	uint32_t perfmgr_max_outstanding_queries;
//	boolean_t perfmgr_ignore_cas;
//	char *event_db_dump_file;
//	int perfmgr_rm_nodes;
//	boolean_t perfmgr_log_errors;
//	boolean_t perfmgr_query_cpi;
//	boolean_t perfmgr_xmit_wait_log;
//	uint32_t perfmgr_xmit_wait_threshold;
//#endif				/* ENABLE_OSM_PERF_MGR */
//	char *event_plugin_name;
//	char *event_plugin_options;
//	char *node_name_map_name;
//	char *prefix_routes_file;
//	char *log_prefix;
//	boolean_t consolidate_ipv6_snm_req;
//	struct osm_subn_opt *file_opts; /* used for update */
//	uint8_t lash_start_vl;			/* starting vl to use in lash */
//	uint8_t sm_sl;			/* which SL to use for SM/SA communication */
//	uint8_t nue_max_num_vls;	/* maximum #VLs to use in nue */
//	boolean_t nue_include_switches;	/* control how nue treats switches */
//	char *per_module_logging_file;
//	boolean_t quasi_ftree_indexing;
//} osm_subn_opt_t;
//


/* from the subnets option struct */
typedef struct sr_options
{
  jboolean lmc_esp0;
  jboolean reassign_lids;
  jboolean ignore_other_sm;
  jboolean single_thread;
  jboolean disable_multicast;
  jboolean force_log_flush;
  jboolean use_mfttop;
  jboolean force_heavy_sweep;
  jboolean no_partition_enforcement;
  jboolean qos;
  jboolean accum_log_file;
  jboolean port_profile_switch_nodes;
  jboolean sweep_on_trap;
  jboolean use_ucast_cache;
  jboolean connect_roots;
  jboolean sa_db_dump;
  jboolean do_mesh_analysis;
  jboolean exit_on_fatal;
  jboolean honor_guid2lid_file;
  jboolean daemon;
  jboolean sm_inactive;
  jboolean babbling_port_policy;
  jboolean use_optimized_slvl;
  jboolean enable_quirks;
  jboolean no_clients_rereg;
  jboolean perfmgr;
  jboolean perfmgr_redir;
  jboolean consolidate_ipv6_snm_req;

  jboolean  m_key_lookup;
  jboolean  allow_both_pkeys;
  jboolean  port_shifting;
  jboolean  remote_guid_sorting;
  jboolean  guid_routing_order_no_scatter;
  jboolean  drop_event_subscriptions;
  jboolean  fsync_high_avail_files;
  jboolean  congestion_control;
  jboolean  perfmgr_ignore_cas;
  jboolean  perfmgr_log_errors;
  jboolean  perfmgr_query_cpi;
  jboolean  perfmgr_xmit_wait_log;


  jshort   sm_priority;
  jshort   lmc;
  jshort   max_op_vls;
  jshort   force_link_speed;
  jshort   subnet_timeout;
  jshort   packet_life_time;
  jshort   vl_stall_count;
  jshort   leaf_vl_stall_count;
  jshort   head_of_queue_lifetime;
  jshort   leaf_head_of_queue_lifetime;
  jshort   local_phy_errors_threshold;
  jshort   overrun_errors_threshold;
  jshort   log_flags;
  jshort   lash_start_vl;
  jshort   sm_sl;

  jshort m_key_protect_bits;
  jshort force_link_speed_ext;
  jshort fdr10;
  jshort sm_assigned_guid;
  jshort cc_sw_cong_setting_threshold;
  jshort cc_sw_cong_setting_packet_size;
  jshort cc_sw_cong_setting_credit_starvation_threshold;

  jint     m_key_lease_period;
  jint     sweep_interval;
  jint     max_wire_smps;
  jint     max_wire_smps2;
  jint     max_smps_timeout;
  jint     transaction_timeout;
  jint     transaction_retries;
  jint     sminfo_polling_timeout;
  jint     polling_retry_number;
  jint     max_msg_fifo_timeout;
  jint     console_port;
  jint     max_reverse_hops;
  jint     perfmgr_sweep_time_s;
  jint     perfmgr_max_outstanding_queries;

  jint ca_port; /* alternative to guid */
  jint part_enforce_enum;
  jint scatter_ports;
  jint cc_max_outstanding_mads;
  jint cc_sw_cong_setting_control_map;
  jint cc_sw_cong_setting_marking_rate;
  jint cc_ca_cong_setting_port_control;
  jint cc_ca_cong_setting_control_map;
  jint  perfmgr_rm_nodes;
  jint  perfmgr_xmit_wait_threshold;

  jlong    guid;
  jlong    m_key;
  jlong    sm_key;
  jlong    sa_key;
  jlong    subnet_prefix;
  jlong    log_max_size;

  jlong cc_key;

  char    config_file[MAX_STRING_SIZE];
  char    dump_files_dir[MAX_STRING_SIZE];
  char    log_file[MAX_STRING_SIZE];
  char    partition_config_file[MAX_STRING_SIZE];
  char    qos_policy_file[MAX_STRING_SIZE];
  char    console[MAX_STRING_SIZE];
  char    port_prof_ignore_file[MAX_STRING_SIZE];
  char    hop_weights_file[MAX_STRING_SIZE];
  char    routing_engine_names[MAX_STRING_SIZE];
  char    lid_matrix_dump_file[MAX_STRING_SIZE];
  char    lfts_file[MAX_STRING_SIZE];
  char    root_guid_file[MAX_STRING_SIZE];
  char    cn_guid_file[MAX_STRING_SIZE];
  char    io_guid_file[MAX_STRING_SIZE];
  char    ids_guid_file[MAX_STRING_SIZE];
  char    guid_routing_order_file[MAX_STRING_SIZE];
  char    sa_db_file[MAX_STRING_SIZE];
  char    torus_conf_file[MAX_STRING_SIZE];
  char    event_db_dump_file[MAX_STRING_SIZE];
  char    event_plugin_name[MAX_STRING_SIZE];
  char    event_plugin_options[MAX_STRING_SIZE];
  char    node_name_map_name[MAX_STRING_SIZE];
  char    prefix_routes_file[MAX_STRING_SIZE];
  char    log_prefix[MAX_STRING_SIZE];

  char  ca_name[MAX_STRING_SIZE];
  char  force_link_speed_file[MAX_STRING_SIZE];
  char  part_enforce[MAX_STRING_SIZE];
  char  port_search_ordering_file[MAX_STRING_SIZE];
  char  per_module_logging_file[MAX_STRING_SIZE];;


} sr_Options_t;

/* from the subnets switch tbl */
typedef struct sr_switch
{
  uint64_t guid;

  uint16_t max_lid_ho;
  uint8_t num_ports;
  uint16_t num_hops;
  jshort hops[MAX_NUM_NODES];  // these are going into jshort arrays, so need to be 16 (instead of 8)
  jshort lft[MAX_NUM_NODES];   // ditto
  uint8_t new_lft;
  uint16_t lft_size;
  int32_t mft_block_num;
  uint32_t mft_position;
  unsigned endport_links;
  unsigned need_update;
  uint32_t num_of_mcm;
  uint8_t is_mc_member;
} sr_Switch_t;

/* from the subnets router tbl */
typedef struct sr_router
{
  uint64_t guid;
} sr_Router_t;

/* from the subnets manager tbl */
typedef struct sr_manager
{
  uint64_t guid;

  jshort  pri_state;
  jint    act_count;

  jlong   sm_key;
  char State[MAX_STRING_SIZE];

} sr_Manager_t;

/* from the subnet */
typedef struct sr_subnet
{
  jboolean             ignore_existing_lfts;
  jboolean             subnet_initialization_error;
  jboolean             force_heavy_sweep;
  jboolean             force_reroute;
  jboolean             in_sweep_hop_0;
  jboolean             first_time_master_sweep;
  jboolean             set_client_rereg_on_sweep;
  jboolean             coming_out_of_standby;
  jboolean             sweeping_enabled;

  jshort               min_ca_mtu;
  jshort               min_ca_rate;
  jshort               min_data_vls;
  jshort               min_sw_data_vls;
  jshort               need_update;
  jshort               sm_state;
  jshort               last_sm_port_state;

  jint                 max_ucast_lid_ho;
  jint                 max_mcast_lid_ho;
  jint                 master_sm_base_lid;
  jint                 sm_base_lid;

  jlong                sm_port_guid;
} sr_Subnet_t;

/* from the subnets pkey tbl */
typedef struct sr_pkey
{
  ib_net16_t pkey;
  uint8_t sl;
  char name[MAX_STRING_SIZE];
  ib_net16_t mlid;
  boolean_t well_known;
  unsigned full_members;
  unsigned num_full_membs;
  unsigned num_part_membs;
  uint64_t full_guid_array[MAX_NUM_NODES];
  uint64_t part_guid_array[MAX_NUM_NODES];
} sr_PKey_t;

/* from the partitions mcast group tbl */
typedef struct sr_mcgroups
{
  ib_net16_t mlid;
  boolean_t well_known;
  unsigned port_members;
  jshort port_num_array[MAX_NUM_NODES];  // this is going into jshort arrays, so need to be 16 (instead of 8)
  uint64_t port_guid_array[MAX_NUM_NODES];
} sr_MCGroups_t;

/* from the ib_mlnx_ext_port_info struct */
typedef struct mlnx_port_info
{
        uint8_t state_change_enable;
        uint8_t link_speed_supported;
        uint8_t link_speed_enabled;
        uint8_t link_speed_active;
} mlnx_port_info_t;


/* from the port tbl & osm_physp_t (maps to SBN_Port) */
typedef struct pt_port
{
  uint64_t node_guid;
  uint64_t port_guid;
  uint8_t port_num;
  ib_port_info_t port_info;

  mlnx_port_info_t ext_port_info;

  uint64_t linked_node_guid;
  uint64_t linked_port_guid;
  uint8_t linked_port_num;
} pt_Port_t;

/* from the node tbl & osm_node_t*/
typedef struct pt_node
{
  ib_node_info_t node_info;
  uint8_t description[IB_NODE_DESCRIPTION_SIZE];
} pt_Node_t;

typedef struct jst_stats
{
  /* natively 32 bits, should they be 64? */
  jlong qp0_mads_outstanding;
  jlong qp0_mads_outstanding_on_wire;
  jlong qp0_mads_rcvd;
  jlong qp0_mads_sent;
  jlong qp0_unicasts_sent;
  jlong qp0_mads_rcvd_unknown;
  jlong sa_mads_outstanding;
  jlong sa_mads_rcvd;
  jlong sa_mads_sent;
  jlong sa_mads_rcvd_unknown;
  jlong sa_mads_ignored;
} jst_Stats_t;

typedef struct jsi_port_desc {
  struct jsi_port_desc *next;
        uint64_t node_guid;
        uint8_t port_num;
        char print_desc[IB_NODE_DESCRIPTION_SIZE + 1];
} jsi_PortDesc_t;

/* this is basically a clone of fabric_stats_t in osm_console.c */
typedef struct jsi_port_stats{
        uint8_t node_type_lim;  /* limit the results; 0 == ALL */
        uint64_t total_nodes;
        uint64_t total_ports;
        uint64_t ports_down;
        uint64_t ports_active;
        uint64_t ports_disabled;
        jsi_PortDesc_t *disabled_ports;
        uint64_t ports_1X;
        uint64_t ports_4X;
        uint64_t ports_8X;
        uint64_t ports_12X;
        uint64_t ports_unknown_width;
        uint64_t ports_unenabled_width;
        jsi_PortDesc_t *unenabled_width_ports;
        uint64_t ports_reduced_width;
        jsi_PortDesc_t *reduced_width_ports;
        uint64_t ports_sdr;
        uint64_t ports_ddr;
        uint64_t ports_qdr;
        uint64_t ports_fdr10;
        uint64_t ports_fdr;
        uint64_t ports_edr;
        uint64_t ports_unknown_speed;
        uint64_t ports_unenabled_speed;
        jsi_PortDesc_t *unenabled_speed_ports;
        uint64_t ports_reduced_speed;
        jsi_PortDesc_t *reduced_speed_ports;
} jsi_PortStats_t;

typedef struct jsi_system_info{
      jint SM_Priority;
      jint PM_SweepTime;
      jint PM_OutstandingQueries;
      jint PM_MaximumQueries;
      jint numPlugins;

      char OpenSM_Version[MAX_STRING_SIZE];
      char OsmJpi_Version[MAX_STRING_SIZE];
      char SM_State[MAX_STRING_SIZE];
      char SA_State[MAX_STRING_SIZE];
      char PM_State[MAX_STRING_SIZE];
      char PM_SweepState[MAX_STRING_SIZE];
      char RoutingEngine[MAX_STRING_SIZE];
      char EventPlugins[MAX_NUM_PLUGINS][MAX_STRING_SIZE];
} jsi_SystemInfo_t;


typedef struct jpi_plugin_info
{
  /*  */
  jint update_period;
  jint report_period;
  jint event_timeout_ms;
  jlong update_count;
  jlong event_count;
} jpi_Plugin_t;


/* fifo Q counters (these are dynamic values) */
typedef struct SharedFifoQueueStats
{
  int id;
  unsigned long size;
  unsigned long numInQueue;
  unsigned long maxInQueue;
  unsigned long numDropped;
}jsr_FifoQueueStats;

typedef struct OsmEventType
{
  int EventId;
  int trapType;
  int trapNum;
  int trapLID;
  jsi_PortDesc_t PortDescription;
}jsr_OsmEvent;


char * sstrncpy(char *dest, const char *src, size_t n);
int jsr_initialize(void);
  const char* jsr_getPluginName(void);
  const char* jsr_getPluginVersion(void);
  const char* jsr_getOsmVersion(void);

  int jsr_waitForNextEvent(void * pJenv, int timeOutInMs);
  int jsr_signalNextEvent(jsr_OsmEvent * pEvent );
  const char* jsr_invokeCommand(int cmdType, const char* cmdArgs);



  jsi_SystemInfo_t * sr_getSysInfo(void);

  int jsr_UpdateSharedResources(int update_period, int report_period);

  int pm_getNumPM_Nodes(void);
  int pm_getNumPM_Ports(void);
  int pm_getNumPM_Esp0Ports(void);
  int pm_getNumPT_Ports(void);
  int pm_getNumPT_Nodes(void);
  pt_Node_t * pm_getPT_Nodes(void);
  pm_Node_t * pm_getPM_Nodes(void);
  pt_Port_t * pm_getPT_Ports(void);
  pm_Port_t * pm_getPM_Ports(void);

  int sr_getNumSwitches(void);
  int sr_getNumRouters(void);
  int sr_getNumManagers(void);
  int sr_getNumPKeys(void);
  int sr_getNumMCGroups(void);

  sr_Subnet_t * sr_getSubnet(void);
  sr_Options_t * sr_getOptions(void);
  sr_Switch_t  * sr_getSwitches(void);
  sr_Router_t  * sr_getRouters(void);
  sr_Manager_t * sr_getManagers(void);
  sr_PKey_t    * sr_getPKeys(void);
  sr_MCGroups_t * sr_getMCGroups(void);

  jst_Stats_t * sr_getStats(void);

  jsi_PortStats_t * sr_getPortStats(int type);


  int sr_getRouterTable(void);
  int sr_getSwitchTable(void);
  int sr_getPKeyTable(void);
  int sr_getManagerTable(void);
  int sr_getSubnetOptions(void);
  int sr_getSubnetAttributes(void);
  int sr_getMCGroupTable(void);


  jpi_Plugin_t * sr_getPluginInfo();
  int sr_getNativePluginInfo(int update_period, int report_period, long count);


//  int jsr_setPortCounters(uint64_t node_guid, uint8_t port_num, jsr_PortCounter_t jpc);
//  int jsr_setPortWaitCounters(uint64_t node_guid, uint8_t port_num, jsr_PortWaitCounter_t jpc);
//  int jsr_setPortErrors(uint64_t node_guid, uint8_t port_num, jsr_PortError_t jpe);

  int jsr_destroy(void);

#ifdef __cplusplus
}
#endif
#endif /* JNI_SHAREDRESOURCES_H_ */
