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
 * osmJniPi.h
 *
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */


#ifndef _OSM_JNI_PLUGIN_H_
#define _OSM_JNI_PLUGIN_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

#include <opensm/osm_event_plugin.h>
#include <opensm/osm_version.h>
#include <opensm/osm_opensm.h>

#define OSM_JPI_CONF "libOsmJniPi.conf"    /* the default configuration file name (see configure.in) */
#define OSM_JPI_RPT_FILTER "IGNORE_PORTS"  /* the default configuration file name (see configure.in) */
#define MIN_UPDATE_PERIOD        5
#define MIN_REPORT_PERIOD        300

#define MAX_SYSTEM_PROPERTIES    7


  /** =========================================================================
   * Keep entries until the worker thread comes around to process them.
   */
  typedef struct _data_entry {
      struct _data_entry *next;
      struct _data_entry *prev;
      osm_epi_event_id_t  type;
      union {
          osm_epi_pe_event_t   pe_event;
          osm_epi_dc_event_t   dc_event;
          osm_epi_ps_event_t   ps_event;
          ib_mad_notice_attr_t trap_event;
      } data;
  } data_entry_t;



  /** =========================================================================
 * Main plugin data structure
 */
typedef struct _plugin_data {
    osm_opensm_t    *p_osm;
    osm_log_t       jpilog;
	osm_log_t       *osmlog;

    /* Monitor Admin */
    int             update_sec;
    int             report_sec;
    int             log_enabled;
    char            *log_level;
    char            *report_filter;
    char            *log_file;
    char            *spi_version;
    char            *jvm_libpath;
    char            *class_path;
    char            *system_props[MAX_SYSTEM_PROPERTIES];
    char            *jlog_props;
    char            *main_class_name;
    void            *pJvm;

    /* other crap */
    JNIEnv *env;
    JavaVM *jvm;
    jclass cls;
    jmethodID mid;
    jmethodID did;

    /* Thread */
    pthread_t        update_thread;
	int              exit_flag;
	pthread_cond_t   signal;
	pthread_mutex_t  sig_lock;
} plugin_data_t;

/** =========================================================================
 * Define an error function which indicate where this error is coming from
 * and wraps both loggers into one.  All msgs go to the plugin log if it
 * is valid and enabled, and then send only error messages to the opensm log
 * (or all messages if there is no plugin log)
 */
#define J_LOG(data, level, fmt, ...) do { \
    if(data->log_enabled == 1) \
      osm_log(&(data->jpilog), level, "%s: " fmt, __func__, ## __VA_ARGS__); \
    if(!data->log_enabled || (level & OSM_LOG_ERROR)) \
      osm_log((data->osmlog), level, "OSM_JPI: %s: " fmt, __func__, ## __VA_ARGS__); \
    } while (0)

void thread_signal(plugin_data_t *plugin_data);
int thread_wait(plugin_data_t *plugin_data, int timeout_sec);

int OJP_waitForEvent(int timeOutInMs);
int OJP_getEventId(void);


#ifdef __cplusplus
}
#endif
#endif /* _OSM_JNI_PLUGIN_H_ */

