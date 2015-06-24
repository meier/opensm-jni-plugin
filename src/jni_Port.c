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
 * jni_Port.c
 *
 *  Created on: May 26, 2011
 *      Author: meier3
 *
 *  Peer Class for Port Objects
 */

#include <unistd.h>
#include <jni.h>

#include "jni_PeerClass.h"

#include <iba/ib_types.h>
#include <opensm/osm_helper.h>
#include <opensm/osm_perfmgr_db.h>

#include "osmJniPi.h"
#include "jni_OsmNativeInterface.h"
#include "jni_SharedResources.h"

extern plugin_data_t *       gData; /* the global plugin data */
extern JPC_CLASS PeerClassArray[];

void * jpt_getOsmPorts(void * pJenv)
{
  // returning a pointer to an object, so make it static
  static jobject currentObject;
  static jobject pInfoObject;
  static jobject xInfoObject;
  static jlongArray counterArray;
  static jobjectArray pfmArray;
  static jobjectArray sbnArray;
  JNIEnv * pJEnv = (JNIEnv *) pJenv;
  ib_port_info_t   * p_pi;
  mlnx_port_info_t * p_epi;

  // this contains two arrays, that represent all the ports

  /* build an array of ports from the perfmgr */
  int numPfmPorts = pm_getNumPM_Ports();
  int numSbnPorts = pm_getNumPT_Ports();
  int n_ndex = 0;

  /* TODO these need locks */
  pt_Port_t * p_ptPort = pm_getPT_Ports(); // use the latest from the subnet
  pm_Port_t * p_pmPort = pm_getPM_Ports(); // use the latest from the perfmgr DB

  JPC_CLASS PFM_Class = PeerClassArray[JPC_PFM_PORT_CLASS];
  JPC_CLASS SBN_INFO_Class = PeerClassArray[JPC_SBN_PORTINFO_CLASS];
  JPC_CLASS MLX_INFO_Class = PeerClassArray[JPC_MLX_PORTINFO_CLASS];
  JPC_CLASS SBN_Class = PeerClassArray[JPC_SBN_PORT_CLASS];
  JPC_CLASS OSM_Class = PeerClassArray[JPC_OSM_PORT_CLASS];

  // create an array to hold the PM Ports
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of PM Ports: %d\n", numPfmPorts);

  pfmArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numPfmPorts, PFM_Class.jpcClass, NULL);
  if (pfmArray != NULL)
    {
      // have the array, now create and set the objects

      for (n_ndex = 0; n_ndex < numPfmPorts; n_ndex++, p_pmPort++)
        {
          /* an array for the counters and errors */
          counterArray = (jlongArray) (*pJEnv)->NewLongArray(pJenv, (jsize) NUM_PORT_COUNTERS);
          if (counterArray == NULL)
            {
              /* holly crap batman, throw an error and get out */
              J_LOG(gData, OSM_LOG_ERROR, "Couldn't create an array for holding the port counters\n");
            }
          else
            {
              (*pJEnv)->SetLongArrayRegion(pJEnv, counterArray, (jsize) 0, (jsize) NUM_PORT_COUNTERS, (jlong *) (p_pmPort->port_counters));

              /*  create a PFM node object */
              currentObject = (*pJEnv)->NewObject(pJEnv, PFM_Class.jpcClass,
                  PFM_Class.constructorMethod->methodID,
                  (jshort) p_pmPort->port_num, (jlong) p_pmPort->node_guid,
                  (jlong) p_pmPort->counter_ts, (jlong) p_pmPort->error_ts,
                  (jlong) p_pmPort->wait_ts, counterArray);

              if (currentObject == NULL)
                {
                  J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a PM Port Object for populating the array: %d\n", n_ndex);
                }
              else
                {
                  (*pJEnv)->SetObjectArrayElement(pJEnv, pfmArray,
                      (jsize) n_ndex, currentObject);
                  (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
                }
              (*pJEnv)->DeleteLocalRef(pJEnv, counterArray);

            }
        }
    }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for PFM_Ports\n");

  // create an array to hold the SBN Ports
  J_LOG(gData, OSM_LOG_DEBUG, "Total # of SBN Ports: %d\n", numSbnPorts);

  sbnArray = (jobjectArray) (*pJEnv)->NewObjectArray(pJenv, (jsize) numSbnPorts, SBN_Class.jpcClass, NULL);

  if (sbnArray != NULL)
  {
    // have the array, now create and set the objects

    for (n_ndex = 0; n_ndex < numSbnPorts; n_ndex++, p_ptPort++)
      {
        /* an info object (ahhh, it burns! )*/
        p_pi  = &(p_ptPort->port_info);
        p_epi = &(p_ptPort->ext_port_info);

        pInfoObject = (*pJEnv)->NewObject(pJEnv, SBN_INFO_Class.jpcClass,
            SBN_INFO_Class.constructorMethod->methodID,
            (jshort) p_pi->local_port_num,
            (jshort) p_pi->link_width_enabled,
            (jshort) p_pi->link_width_supported,

            (jshort) p_pi->link_width_active,
            (jshort) p_pi->state_info1,
            (jshort) p_pi->state_info2,
            (jshort) p_pi->mkey_lmc,
            (jshort) p_pi->link_speed,
            (jshort) p_pi->link_speed_ext,
            (jshort) p_pi->link_speed_ext_enabled,
            (jshort) p_pi->mtu_smsl,
            (jshort) p_pi->vl_cap,
            (jshort) p_pi->vl_high_limit,
            (jshort) p_pi->vl_arb_high_cap,
            (jshort) p_pi->vl_arb_low_cap,
            (jshort) p_pi->mtu_cap,
            (jshort) p_pi->vl_stall_life,
            (jshort) p_pi->vl_enforce,
            (jshort) p_pi->guid_cap,
            (jshort) p_pi->subnet_timeout,
            (jshort) p_pi->resp_time_value,
            (jshort) p_pi->error_threshold,

            (jint) cl_ntoh16(p_pi->base_lid),
            (jint) cl_ntoh16(p_pi->master_sm_base_lid),
            (jint) cl_ntoh32(p_pi->capability_mask),
            (jint) cl_ntoh32(p_pi->capability_mask2),
            (jint) cl_ntoh16(p_pi->diag_code),
            (jint) cl_ntoh16(p_pi->m_key_lease_period),
            (jint) cl_ntoh16(p_pi->m_key_violations),
            (jint) cl_ntoh16(p_pi->p_key_violations),
            (jint) cl_ntoh16(p_pi->q_key_violations),
            (jint) cl_ntoh16(p_pi->max_credit_hint),
            (jint) p_pi->link_rt_latency,

            (jlong) cl_ntoh64(p_pi->m_key),
            (jlong) cl_ntoh64(p_pi->subnet_prefix));

        xInfoObject = (*pJEnv)->NewObject(pJEnv, MLX_INFO_Class.jpcClass,
            MLX_INFO_Class.constructorMethod->methodID,
            (jshort) p_epi->state_change_enable,
            (jshort) p_epi->link_speed_supported,
            (jshort) p_epi->link_speed_enabled,
            (jshort) p_epi->link_speed_active);

        if ((pInfoObject == NULL) || (xInfoObject == NULL))
        {
          /* holly crap batman, throw an error and get out */
          J_LOG(gData, OSM_LOG_ERROR, "Couldn't create port info objects for holding the port information\n");
        }
        else
        {

          /*  create a SBN port object */
          currentObject = (*pJEnv)->NewObject(pJEnv, SBN_Class.jpcClass,
              SBN_Class.constructorMethod->methodID,
              (jshort) p_ptPort->port_num,
              (jshort) p_ptPort->linked_port_num,
              (jlong) cl_ntoh64(p_ptPort->node_guid),
              (jlong) cl_ntoh64(p_ptPort->port_guid),
              (jlong) cl_ntoh64(p_ptPort->linked_node_guid),
              (jlong) cl_ntoh64(p_ptPort->linked_port_guid), pInfoObject, xInfoObject);

          if (currentObject == NULL)
          {
            J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a PT Port Object for populating the array: %d\n", n_ndex);
          }
          else
          {
            (*pJEnv)->SetObjectArrayElement(pJEnv, sbnArray, (jsize) n_ndex, currentObject);
            (*pJEnv)->DeleteLocalRef(pJEnv, currentObject);
          }
          (*pJEnv)->DeleteLocalRef(pJEnv, pInfoObject);
          (*pJEnv)->DeleteLocalRef(pJEnv, xInfoObject);
        }
      }
  }
  else
    J_LOG(gData, OSM_LOG_ERROR, "Could not create objectArray for SBN_Ports\n");

  // done with both arrays, so construct the container object
  currentObject = (*pJEnv)->NewObject(pJEnv, OSM_Class.jpcClass, OSM_Class.constructorMethod->methodID, pfmArray, sbnArray);
  if (currentObject == NULL)
  {
    J_LOG(gData, OSM_LOG_ERROR, "Couldn't create a OSM Port Object for holding the arrays\n");
  }

  // fully constructed, or failed, either way, clean up
  (*pJEnv)->DeleteLocalRef(pJEnv, pfmArray);
  (*pJEnv)->DeleteLocalRef(pJEnv, sbnArray);

  return (void *) &currentObject;

}

void jp_dump_port_info(IN ib_net64_t node_guid,IN ib_net64_t port_guid, IN uint8_t port_num)
{
  osm_opensm_t * p_osm = gData->p_osm;
  cl_qmap_t *p_port_guid_tbl;
  ib_port_info_t * p_pi;
  ib_net64_t guid;
  osm_log_level_t log_level = OSM_LOG_INFO;
  osm_log_t * p_log = &(gData->jpilog);

  /* use the port_guid as key in the port_guid_tbl to find the desired port info */
  guid = port_guid;

  CL_PLOCK_ACQUIRE(p_osm->sm.p_lock);
  p_port_guid_tbl = &(p_osm->sm.p_subn->port_guid_tbl);


  osm_port_t * p_port = (osm_port_t *)cl_qmap_get(p_port_guid_tbl, guid);
  int count = cl_qmap_count(p_port_guid_tbl);

  J_LOG(gData, OSM_LOG_INFO, "Port Map Size is (%d)\n",count );
  if((count > 0) && (p_port != NULL))
  {
    /* found a port, so extract the port info */
    p_pi = &(p_port->p_physp->port_info);

    if (osm_log_is_active(p_log, log_level))
    {
        osm_log(p_log, log_level,
            "(jni_Port) PortInfo dump:\n"
            "\t\t\t\tport number.............%u\n"
            "\t\t\t\tnode_guid...............0x%016" PRIx64 "\n"
            "\t\t\t\tport_guid...............0x%016" PRIx64 "\n"
            "\t\t\t\tm_key...................0x%016" PRIx64 "\n"
            "\t\t\t\tsubnet_prefix...........0x%016" PRIx64 "\n"
            "\t\t\t\tbase_lid................%u\n"
            "\t\t\t\tbase_lid................%u\n"
            "\t\t\t\tmaster_sm_base_lid......%u\n"
            "\t\t\t\tcapability_mask.........0x%X\n"
            "\t\t\t\tcapability_mask2........0x%X\n"
            "\t\t\t\tdiag_code...............0x%X\n"
            "\t\t\t\tm_key_lease_period......0x%X\n"
            "\t\t\t\tlocal_port_num..........%u\n"
            "\t\t\t\tlink_width_enabled......0x%X\n"
            "\t\t\t\tlink_width_supported....0x%X\n"
            "\t\t\t\tlink_width_active.......0x%X\n"
            "\t\t\t\tlink_speed_supported....0x%X\n"
            "\t\t\t\tport_state..............%s\n"
            "\t\t\t\tstate_info2.............0x%X\n"
            "\t\t\t\tm_key_protect_bits......0x%X\n"
            "\t\t\t\tlmc.....................0x%X\n"
            "\t\t\t\tlink_speed..............0x%X\n"
            "\t\t\t\tlink_speed_ext..........0x%X\n"
            "\t\t\t\tlink_speed_ext_enabled..0x%X\n"
            "\t\t\t\tmtu_smsl................0x%X\n"
            "\t\t\t\tvl_cap_init_type........0x%X\n"
            "\t\t\t\tvl_high_limit...........0x%X\n"
            "\t\t\t\tvl_arb_high_cap.........0x%X\n"
            "\t\t\t\tvl_arb_low_cap..........0x%X\n"
            "\t\t\t\tinit_rep_mtu_cap........0x%X\n"
            "\t\t\t\tvl_stall_life...........0x%X\n"
            "\t\t\t\tvl_enforce..............0x%X\n"
            "\t\t\t\tm_key_violations........0x%X\n"
            "\t\t\t\tp_key_violations........0x%X\n"
            "\t\t\t\tq_key_violations........0x%X\n"
            "\t\t\t\tguid_cap................0x%X\n"
            "\t\t\t\tclient_reregister.......0x%X\n"
            "\t\t\t\tmcast_pkey_trap_suppr...0x%X\n"
            "\t\t\t\tsubnet_timeout..........0x%X\n"
            "\t\t\t\tresp_time_value.........0x%X\n"
            "\t\t\t\terror_threshold.........0x%X\n"
            "\t\t\t\tmax_credit_hint.........0x%X\n"
            "\t\t\t\tlink_round_trip_latency.0x%X\n",
            port_num, cl_ntoh64(node_guid), cl_ntoh64(port_guid),
            cl_ntoh64(p_pi->m_key), cl_ntoh64(p_pi->subnet_prefix),
            cl_ntoh16(p_pi->base_lid),
            p_pi->base_lid,
            cl_ntoh16(p_pi->master_sm_base_lid),
            cl_ntoh32(p_pi->capability_mask),
            cl_ntoh32(p_pi->capability_mask2),
            cl_ntoh16(p_pi->diag_code),
            cl_ntoh16(p_pi->m_key_lease_period),
            p_pi->local_port_num, p_pi->link_width_enabled,
            p_pi->link_width_supported, p_pi->link_width_active,
            ib_port_info_get_link_speed_sup(p_pi),
            ib_get_port_state_str(ib_port_info_get_port_state
                          (p_pi)), p_pi->state_info2,
            ib_port_info_get_mpb(p_pi), ib_port_info_get_lmc(p_pi),
            p_pi->link_speed, p_pi->link_speed_ext, p_pi->link_speed_ext_enabled,
            p_pi->mtu_smsl, p_pi->vl_cap,
            p_pi->vl_high_limit, p_pi->vl_arb_high_cap,
            p_pi->vl_arb_low_cap, p_pi->mtu_cap,
            p_pi->vl_stall_life, p_pi->vl_enforce,
            cl_ntoh16(p_pi->m_key_violations),
            cl_ntoh16(p_pi->p_key_violations),
            cl_ntoh16(p_pi->q_key_violations), p_pi->guid_cap,
            ib_port_info_get_client_rereg(p_pi),
            ib_port_info_get_mcast_pkey_trap_suppress(p_pi),
            ib_port_info_get_timeout(p_pi), p_pi->resp_time_value,
            p_pi->error_threshold, cl_ntoh16(p_pi->max_credit_hint),
            cl_ntoh32(p_pi->link_rt_latency));

        /*  show the capabilities mask */
//        if (p_pi->capability_mask)
//        {
//            dbg_get_capabilities_str(buf, BUF_SIZE, "\t\t\t\t",
//                         p_pi);
//            osm_log(p_log, log_level, "%s", buf);
//        }

  /* show the remote port, if it exists */
        osm_physp_t * pp_port = p_port->p_physp->p_remote_physp;

        if(pp_port != NULL)
        {
          /* found a remote port, so extract the port identity */
          node_guid = pp_port->p_node->node_info.node_guid;

          osm_log(p_log, log_level,
              "Remote (linked) PortInfo dump:\n"
              "\t\t\t\tport number.............%u\n"
              "\t\t\t\tnode_guid...............0x%016" PRIx64 "\n"
              "\t\t\t\tport_guid...............0x%016" PRIx64 "\n",
              pp_port->port_num,
              cl_ntoh64(node_guid ), cl_ntoh64(pp_port->port_guid));
        }
    }
  }
  else
  {
    J_LOG(gData, OSM_LOG_INFO, "ERROR: Port Not Found 0x%" PRIx64 "\n", guid);
  }

  CL_PLOCK_RELEASE(p_osm->sm.p_lock);
}

void jp_dump_port_counters(uint64_t guid, uint8_t port)
{
  osm_opensm_t * p_osm = gData->p_osm;
  osm_perfmgr_t * p_perfmgr = &(p_osm->perfmgr);
  perfmgr_db_t * db = p_perfmgr->db;

  db_port_t *p_port = NULL;
  db_node_t *p_node = NULL;
  perfmgr_db_data_cnt_reading_t counters;
  perfmgr_db_err_reading_t errors;

  cl_plock_acquire(&db->lock);

  cl_map_item_t *rc = cl_qmap_get(&db->pc_data, cl_ntoh64(guid));
  const cl_map_item_t *end = cl_qmap_end(&db->pc_data);
  int count = cl_qmap_count(&db->pc_data);

  J_LOG(gData, OSM_LOG_INFO, "Node Map Size is (%d)\n",count );
  if (rc != end) p_node = (db_node_t *) rc;

  if (!p_node)
  {
    J_LOG(gData, OSM_LOG_INFO, "Returned node is NULL, could not find guid\n");
    cl_plock_release(&db->lock);
  }
  else if (port >= p_node->num_ports || (!p_node->esp0 && port == 0))
  {
    J_LOG(gData, OSM_LOG_INFO, "Could not find port in port_guid_table\n");
    cl_plock_release(&db->lock);
  }
  else
  {
    p_port = &p_node->ports[port];

    /* I have the port, get a copy of the absolute values (most current) */
    counters = p_port->dc_total;
    errors = p_port->err_total;

    cl_plock_release(&db->lock);

    /* lock released, so can do time intensive operations */

    /* print out the counters and errors */
    J_LOG(gData, OSM_LOG_INFO, "xd %" PRIu64 "\n",counters.xmit_data);
    J_LOG(gData, OSM_LOG_INFO, "rd %" PRIu64 "\n",counters.rcv_data);
    J_LOG(gData, OSM_LOG_INFO, "xp %" PRIu64 "\n",counters.xmit_pkts);
    J_LOG(gData, OSM_LOG_INFO, "rp %" PRIu64 "\n",counters.rcv_pkts);
    //    J_LOG(gData, OSM_LOG_INFO, "rp %" PRIu64 "\n",counters.time);
    J_LOG(gData, OSM_LOG_INFO, "ebo %" PRIu64 "\n",errors.buffer_overrun);
    J_LOG(gData, OSM_LOG_INFO, "eld %" PRIu64 "\n",errors.link_downed);
    J_LOG(gData, OSM_LOG_INFO, "eler %" PRIu64 "\n",errors.link_err_recover);
    J_LOG(gData, OSM_LOG_INFO, "eli %" PRIu64 "\n",errors.link_integrity);
    J_LOG(gData, OSM_LOG_INFO, "erc %" PRIu64 "\n",errors.rcv_constraint_err);
    J_LOG(gData, OSM_LOG_INFO, "er %" PRIu64 "\n",errors.rcv_err);
    J_LOG(gData, OSM_LOG_INFO, "err %" PRIu64 "\n",errors.rcv_rem_phys_err);
    J_LOG(gData, OSM_LOG_INFO, "ersr %" PRIu64 "\n",errors.rcv_switch_relay_err);
    J_LOG(gData, OSM_LOG_INFO, "es %" PRIu64 "\n",errors.symbol_err_cnt);
    J_LOG(gData, OSM_LOG_INFO, "evd %" PRIu64 "\n",errors.vl15_dropped);
    J_LOG(gData, OSM_LOG_INFO, "exc %" PRIu64 "\n",errors.xmit_constraint_err);
    J_LOG(gData, OSM_LOG_INFO, "exd %" PRIu64 "\n",errors.xmit_discards);
    //    J_LOG(gData, OSM_LOG_INFO, "el %" PRIu64 "\n",errors.time);
  }
}

