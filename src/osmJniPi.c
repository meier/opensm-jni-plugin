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
 * osmJniPi.c
 *
 *  Created on: Jun 23, 2015
 *      Author: meier3
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <jni.h>

#include "osmJniPi.h"
#include "osmJniPi_version.h"
#include "jni_NativeUtils.h"

#include "jni_PeerClass.h"

#include "jni_SharedResources.h"
#include "jni_Synchronization.h"

/* OpenSM headers to get data from */
#include <opensm/osm_port.h>

plugin_data_t *       gData; /* the global plugin data (see construct...) */

static int numSysProps = 0;

static void
dump_plugin_list(plugin_data_t *data)
{
  /* show all installed plugins */
  cl_qlist_t plugin_list = data->p_osm->plugin_list;
  osm_epi_plugin_t *pi;
  int n = 0;

  cl_list_item_t *itor;

  itor = cl_qlist_head(&plugin_list);

  for (n = 0; n < plugin_list.count; n++)
  {
    pi = (osm_epi_plugin_t *) itor;
    J_LOG(data, OSM_LOG_INFO, "Open Subnet Manager: version (%s)\n", pi->impl->osm_version);
    J_LOG(data, OSM_LOG_INFO, "Plugin: %d) name (%s)\n", ++n, pi->plugin_name);
    itor = cl_qlist_next(itor);
  }
}

static void
dump_plugin_data(plugin_data_t *data)
{
  int j = 0;
  dump_plugin_list(data);
  J_LOG(data, OSM_LOG_INFO, "OSM Plugin Version: (%s)\n", data->spi_version);
  J_LOG(data, OSM_LOG_INFO, "Log File: %s\n", data->log_file);
  J_LOG(data, OSM_LOG_INFO, "Log Flags: %s\n", data->log_level);
  J_LOG(data, OSM_LOG_INFO, "Update Interval (secs): %d\n", data->update_sec);
  J_LOG(data, OSM_LOG_INFO, "Report Interval (secs): %d\n", data->report_sec);
  J_LOG(data, OSM_LOG_INFO, "Event Filter: (%s)\n", data->report_filter);
  J_LOG(data, OSM_LOG_INFO, "JVM library location: (%s)\n", data->jvm_libpath);
  J_LOG(data, OSM_LOG_INFO, "ClassPath: (%s)\n", data->class_path);
  J_LOG(data, OSM_LOG_INFO, "LogConfig File: (%s)\n", data->jlog_props);
  J_LOG(data, OSM_LOG_INFO, "PrimaryClassName (main): (%s)\n", data->main_class_name);
  for(j=0; j<numSysProps; j++)
      J_LOG(data, OSM_LOG_INFO, "SystemParameter %d: (%s)\n", (j+1), data->system_props[j]);

}

inline void thread_signal(plugin_data_t *plugin_data)
{
        pthread_mutex_lock(&(plugin_data->sig_lock));
        pthread_cond_signal(&(plugin_data->signal));
        pthread_mutex_unlock(&(plugin_data->sig_lock));
}

inline int thread_wait(plugin_data_t *plugin_data, int timeout_sec)
{
  struct timespec ts;
  int val = EINVAL;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += timeout_sec;
  pthread_mutex_lock(&(plugin_data->sig_lock));
  val = pthread_cond_timedwait(&(plugin_data->signal), &(plugin_data->sig_lock), &ts);
  pthread_mutex_unlock(&(plugin_data->sig_lock));
  return val;
}

/******************************************************************************
*** Function: OJP_waitForEvent
***
*** Summary_Description_Of_What_The_Function_Does.
*** <p>
***
***   Parameters:
***
***   Returns:
***
******************************************************************************/
int OJP_waitForEvent(int timeOutInMs)
{
    int eventType = -2;
    int secs = timeOutInMs/1000;
    eventType = thread_wait(gData, secs);
//    int state = 1;
//
//      // only wait if running
//    if(state == 1)
//    {
//
//      eventType = JSZ_waitForOsmEvent(timeOutInMs);
//
//      // get a local copy of the dynamic stuff that is in shared memory, the JNI will use local versions
//      }
//      else
//      {
//        usleep((long)timeOutInMs * (long)1000);
//        eventType = -1;
//      }
    return eventType;
}

/** =========================================================================
 * This worker thread keeps the native interface's cache up to date by
 * periodically refreshing it.  The period is specified in the config file.
 *
 * There are two types of periodic updates performed here.
 *  1.  Normal Update: smaller set of most important information collected
 *      often.  Typically this would be the more dynamic data.
 *  2.  Report Update: less frequent updates, typically for relatively
 *      static attributes.
 */
static void *
periodic_update_thread(void *pd)
{
  plugin_data_t *plugin_data = (plugin_data_t *) pd;
  int sleep_secs = (MIN_UPDATE_PERIOD > plugin_data->update_sec) ? MIN_UPDATE_PERIOD:plugin_data->update_sec;
  int report_secs = (MIN_REPORT_PERIOD > plugin_data->report_sec) ? MIN_REPORT_PERIOD:plugin_data->report_sec;
  int report_loops = report_secs/sleep_secs;
  int loop = 0;

  plugin_data->exit_flag = 0;
  thread_signal(plugin_data);

  J_LOG(plugin_data, OSM_LOG_INFO, "starting periodic update thread (every %d seconds)\n", sleep_secs);
  while (1)
  {
    sleep(sleep_secs);
    loop++;
    if (loop > report_loops)
    {
      /* do some sort of mandatory periodic reporting here */
//      dump_plugin_list(plugin_data);
        J_LOG(plugin_data, OSM_LOG_INFO, "Native Report loop running\n");

        // put periodic diagnostics here
 //        jsr_printPortGuids();
//        jsn_printMultiCastGroups();
//        jsn_printPartitions();
//        jsn_printSwitches();

      /* TODO increment heartbeat counter */
      loop = 0;
    }

    /* do normal periodic updates here (get critical data) */
    jsr_UpdateSharedResources(sleep_secs, report_secs);

    if (plugin_data->exit_flag)
    {
      J_LOG(plugin_data, OSM_LOG_ERROR, "periodic update thread exiting\n");
      // free resources??
      return (NULL);
    }
  }
}

/** =========================================================================
 */
static inline int
construct_plugin_data(plugin_data_t *data)
{
	FILE *fp = NULL;
	char  line[1024];
	char *last = NULL;
	char *key = NULL;
	char *val = NULL;

    int status;
    uint8_t b_verbose = OSM_LOG_DEFAULT_LEVEL;
    uint32_t log_max_size = 1000;
    boolean_t force_log_flush = FALSE;
    boolean_t accum_log_file = TRUE;

    data->spi_version = (char *)jsr_getPluginVersion();
    data->log_enabled = 0; /* the plugin log has not been initialized yet */

	if (pthread_mutex_init(&(data->sig_lock), NULL)
	    || pthread_cond_init(&(data->signal), NULL))
	  {
		return (0);
	}
	/* set the default event filter */
	data->report_filter = strdup(OSM_JPI_RPT_FILTER);

	/* Read config file here... */
	snprintf(line, 1023, "%s/%s", CONF_FILE_PREFIX, OPENSM_PI_CONFIG_FILE);
        J_LOG(data, OSM_LOG_INFO, "Attempting to read Plugin config file: %s\n", line);
	if (!(fp = fopen(line, "r")))
	{
		J_LOG(data, OSM_LOG_ERROR, "Failed to read %s\n", line);
		return (0);
	}

	while (fgets(line, 1023, fp) != NULL)
	  {
		/* get the first token */
		key = strtok_r(line, " \t\n", &last);
		if (!key)
			continue;
		val = strtok_r(NULL, " \t\n", &last);

        if (!strcmp("JVM_LIBRARY_PATH", key) && val)
        {
            data->jvm_libpath = strdup(val);
        }

        if (!strcmp("JVM_CLASS_PATH", key) && val)
        {
            data->class_path = strdup(val);
        }

        if (!strcmp("EVENT_FILTER", key) && val)
        {
            data->report_filter = strdup(val);
        }

        if (!strcmp("JVM_SYS_PROPS", key) && val)
        {
            /* get a number of system properties, up to max */
            while (val && (MAX_SYSTEM_PROPERTIES > numSysProps))
              {
                data->system_props[numSysProps++] = strdup(val);
                val = strtok_r(NULL, " \t\n", &last);
              }
         }

        if (!strcmp("JVM_LOGCONFIG_FILE", key) && val)
        {
            data->jlog_props = strdup(val);
        }

        if (!strcmp("JAVA_MAIN_CLASS", key) && val)
        {
            data->main_class_name = strdup(val);
        }

        if (!strcmp("UPDATE_PERIOD", key) && val)
        {
          data->update_sec = abs((int)(strtol(val, NULL, 0)));
        }

        if (!strcmp("REPORT_PERIOD", key) && val)
        {
          data->report_sec = abs((int)(strtol(val, NULL, 0)));
        }

        if (!strcmp("LOG_MAX_SIZE", key) && val)
        {
          log_max_size = strtoul(val, NULL, 0);
        }

		if (!strcmp("LOG_FLAGS", key) && val)
		{
			data->log_level = strdup(val);
			b_verbose = strtoul(val, NULL, 0);
		}

		if (!strcmp("LOG_FILE", key) && val)
		{
			data->log_file = strdup(val);
		}

        if (!strcmp("FORCE_LOG_FLUSH", key) && val)
        {
          if (strcmp("TRUE", val))
            force_log_flush = FALSE;
          else force_log_flush = TRUE;
        }

        if (!strcmp("ACCUM_LOG_FILE", key) && val)
        {
          if (strcmp("TRUE", val))
            accum_log_file = FALSE;
          else accum_log_file = TRUE;
        }

	}
	fclose(fp);
	dump_plugin_data(data);

	/* create the plugin log file, using the osm_log */
	J_LOG(data, OSM_LOG_INFO, "Attempting to create Plugin Log: %s\n", data->log_file);
	osm_log_construct(&data->jpilog);

    status = osm_log_init_v2(&data->jpilog, force_log_flush,
                 b_verbose, data->log_file,
                 log_max_size, accum_log_file);
    if (status != IB_SUCCESS)
    {
      J_LOG(data, OSM_LOG_ERROR, "FAILED to create Plugin Log: %s  %d\n", data->log_file, status);
    }
    else
    {
      data->log_enabled = 1;
      gData = data;
    }

    /* write a version number to the log file, showing startup */
    J_LOG(data, OSM_LOG_INFO, "Attempting to write to the Plugin Log: %s\n", data->log_file);

    dump_plugin_data(data);
	return (1);
}

static void
free_plugin_data(plugin_data_t *data)
{
    free(data);
}

static int destroy_JVM(JNIEnv *env, JavaVM *jvm)
{
  if ((*env)->ExceptionOccurred(env))
  {
      (*env)->ExceptionDescribe(env);
  }
  (*jvm)->DestroyJavaVM(jvm);
  return 0;
}

static int create_JVM(plugin_data_t *pData)
{
  JNIEnv *env;
  JavaVM *jvm;
  jint res;
  jclass cls;
  jmethodID mid;
  jmethodID did;

  jint (*createJVM) (JavaVM **, void **, void *);

/* must be JNI_VERSION_1_4 or higher, not supporting earlier versions */
  JavaVMInitArgs vm_args;
  JavaVMOption options[MAX_SYSTEM_PROPERTIES + 2];
  options[0].optionString = pData->class_path;
  options[1].optionString = pData->jlog_props;
  vm_args.version = JNI_VERSION_1_4;
  vm_args.options = options;
  vm_args.nOptions = 2 + numSysProps;
  vm_args.ignoreUnrecognized = JNI_TRUE;

  int j = 0;

  for(j = 0; j < numSysProps; j++)
    options[j+2].optionString = pData->system_props[j];

  /* Load the JVM library */
  pData->pJvm = dlopen(pData->jvm_libpath, RTLD_LAZY);
    if (!pData->pJvm)
    {
      J_LOG(pData, OSM_LOG_ERROR, "Failed to open the JVM library \"%s\" : \"%s\"\n", pData->jvm_libpath, dlerror());
      return 1;
    }

    /* Find the create function */
    createJVM =  dlsym(pData->pJvm, "JNI_CreateJavaVM");
    if (!createJVM)
    {
      J_LOG(pData, OSM_LOG_ERROR, "Failed to find \"%s\" symbol in \"%s\" : \"%s\"\n", "JNI_CreateJavaVM", pData->jvm_libpath, dlerror());
      return 2;
    }

    J_LOG(pData, OSM_LOG_ERROR, "ATTN: Starting the JVM with program arguments and options ... now.\n");
    J_LOG(pData, OSM_LOG_ERROR, "ATTN: if no immediate success, suspect java option errors in the CONF file.\n");

  /* Create the Java VM FIXME */
  res = (*createJVM)(&jvm, (void**)&env, &vm_args);  /* causes a deref warning, not sure how to fix */
  J_LOG(pData, OSM_LOG_ERROR, "ATTN: Successfully started the JVM and program!\n");

  if (res < 0)
  {
    J_LOG(pData, OSM_LOG_ERROR, "Can't create Java VM (err: %d)\n", res);
    J_LOG(pData, OSM_LOG_ERROR, "check for illegal options string in the conf file\n");

    fprintf(stderr, "Can't create Java VM (err: %d)\n", res);
    fprintf(stderr, "check for illegal options string in the conf file\n");
      exit(1);
  }
  pData->jvm = jvm;
  pData->env = env;

  /* check the version from the environment */
  J_LOG(pData, OSM_LOG_ERROR, "Java Native Interface Version (%x)\n", (*env)->GetVersion(env));

  /* initialize the initial "program" class that starts it all */
  cls = (*env)->FindClass(env, pData->main_class_name);
  if (cls == NULL)
  {
    J_LOG(pData, OSM_LOG_ERROR, "Can't find Class (%s) at (%s)\n", pData->main_class_name, pData->class_path);
      destroy_JVM(env, jvm);
  }

  if(jnu_registerAllNatives(env, pData) != 0)
  {
    J_LOG(pData, OSM_LOG_ERROR, "Can't register the native methods\n");
  }

  mid = (*env)->GetStaticMethodID(env, cls, "main", "([Ljava/lang/String;)V");
  if (mid == NULL)
  {
    J_LOG(pData, OSM_LOG_ERROR, "Can't find main method in Class\n");
    destroy_JVM(env, jvm);
  }

  did = (*env)->GetStaticMethodID(env, cls, "destroy", "()V");
  if (did == NULL)
  {
    J_LOG(pData, OSM_LOG_ERROR, "Can't find destroy method in Class\n");
    destroy_JVM(env, jvm);
  }

  pData->cls = cls;  // main, for startup
  pData->did = did;  // destroy, for shutdown

//  jpc_printJniReferences();

  (*env)->CallStaticVoidMethod(env, cls, mid, NULL);
  return 0;
}

/** =========================================================================
 */
static void *
create(struct osm_opensm *osm)
{
	struct timespec  ts;
	plugin_data_t   *plugin_data = NULL;
	pthread_attr_t   th_attr;
	int              rc = 0;

	/* create the main data structure that will be used by this plugin */
	/* ( it gets returned as the first arg in report() & delete() ) */

	if (!(plugin_data = malloc(sizeof(*plugin_data))))
		return (NULL);

    plugin_data->p_osm = osm;
    plugin_data->osmlog = &(osm->log);

	if (!construct_plugin_data(plugin_data))
	  {
		free(plugin_data);
		return (NULL);
	}

        // initialize the Shared Resources
        if(jsr_initialize() != 1)
        {
          J_LOG(plugin_data, OSM_LOG_ERROR, "****** Could not initialize the Shared Resources\n");
        }

	  // Open the Synchronization Objects
	  if(JSZ_createMasterSyncObjects(1) != 1)
	  {
	    J_LOG(plugin_data, OSM_LOG_ERROR, "****** Could not open the Synchronization objects\n");
	  }

	if (pthread_attr_init(&(th_attr)))
	  {
		free_plugin_data(plugin_data);
		return (NULL);
	}
	pthread_mutex_lock(&(plugin_data->sig_lock));
	    J_LOG(plugin_data, OSM_LOG_INFO, "Creating the periodic_update thread...\n");
	if (pthread_create(&(plugin_data->update_thread), &(th_attr), periodic_update_thread, (void *)plugin_data))
	  {
		J_LOG(plugin_data, OSM_LOG_INFO, "Failed to create periodic_update thread\n");
		pthread_attr_destroy(&(th_attr));
		free_plugin_data(plugin_data);
		return (NULL);
	}
	pthread_attr_destroy(&(th_attr));

        J_LOG(plugin_data, OSM_LOG_INFO, "Starting the periodic_update thread...\n");
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 2; /* give 2 sec to start up */
	rc = pthread_cond_timedwait(&(plugin_data->signal), &(plugin_data->sig_lock), &ts);
	pthread_mutex_unlock(&(plugin_data->sig_lock));
	if (rc == ETIMEDOUT)
	  {
		J_LOG(plugin_data, OSM_LOG_ERROR, "periodic_update thread failed to initialize\n");
		pthread_join(plugin_data->update_thread, NULL);
		free_plugin_data(plugin_data);
		return (NULL);
	}

    J_LOG(plugin_data, OSM_LOG_INFO, "creating JVM\n");

    create_JVM(plugin_data);

    J_LOG(plugin_data, OSM_LOG_INFO, "JVM created\n");

	return ((void *)plugin_data);
}

/** =========================================================================
 */
static void
destroyVM(plugin_data_t *pData)
{
    J_LOG(pData, OSM_LOG_INFO, "destroy called, cleaning up Java...\n");

    (*(pData->env))->CallStaticVoidMethod(pData->env, pData->cls, pData->did, NULL);
}

/** =========================================================================
 */
static void
delete(void *data)
{
    plugin_data_t *plugin_data = (plugin_data_t *)data;
    destroyVM(plugin_data);

    J_LOG(plugin_data, OSM_LOG_INFO, "destroy called, cleaning up...\n");

    jsr_destroy();
    JSZ_releaseMasterSyncObjects();

    /* create a termination event to send to the Virtual Machine */


    /* signal all the threads we are quitting */
    plugin_data->exit_flag = 1;
    thread_signal(plugin_data);
    J_LOG(plugin_data, OSM_LOG_INFO, "Stopping periodic_update thread...\n");
    pthread_join(plugin_data->update_thread, NULL);
    sleep(1);  /* give the thread some time to notice and respond */

    /* destroy the monitors' log */
    osm_log_destroy(&plugin_data->jpilog);
    free_plugin_data(plugin_data);
}

/** =========================================================================
 */

/** =========================================================================
 * Compare the "event_id" against the report filter setting to see if this
 * event should be ignored.  Return true if ignored, zero otherwise.
 *
 * The current list of valid filters;
 *  IGNORE_NONE              - allow all
 *  IGNORE_ALL               - turn reporting off
 *  IGNORE_PORT_COUNTERS     - filter out data counter events
 *  IGNORE_PORT_ERRORS       - filter out error counter events
 *  IGNORE_PORTS             - filter out all counter events from the perf manager (default)
 *
 *
 */
static int event_is_filtered(osm_epi_event_id_t event_id)
{
  int filtered = 0;

  if (!strcmp("IGNORE_NONE", gData->report_filter))
    return 0;

  if (!strcmp("IGNORE_ALL", gData->report_filter))
    return 1;

  switch (event_id)
  {
  case OSM_EVENT_ID_PORT_ERRORS:
    if (!strcmp("IGNORE_PORTS", gData->report_filter) || !strcmp("IGNORE_PORT_ERRORS", gData->report_filter))
      {
        filtered = 1;
      }
      break;

  case OSM_EVENT_ID_PORT_DATA_COUNTERS:
    if (!strcmp("IGNORE_PORTS", gData->report_filter) || !strcmp("IGNORE_PORT_COUNTERS", gData->report_filter))
      {
        filtered = 1;
      }
      break;

  case OSM_EVENT_ID_PORT_SELECT:
  case OSM_EVENT_ID_TRAP:
  case OSM_EVENT_ID_SUBNET_UP:
  case OSM_EVENT_ID_HEAVY_SWEEP_START:
  case OSM_EVENT_ID_HEAVY_SWEEP_DONE:
  case OSM_EVENT_ID_UCAST_ROUTING_DONE:
  case OSM_EVENT_ID_STATE_CHANGE:
  case OSM_EVENT_ID_SA_DB_DUMPED:
  case OSM_EVENT_ID_LFT_CHANGE:
  case OSM_EVENT_ID_MAX:
  default:
      break;
  }

  return filtered;
}

  /** =========================================================================
   * This is a callback method used by OpenSM to signal some sort of event.  It
   * should return as soon as possible.
   *
   * The event type is all I am really interested in here, since the data is
   * globally available.  I reserve the option of using it for optimization purposes.
   *
   */
  static void report(void *data, osm_epi_event_id_t event_id, void *event_data)
  {
	static long report_count = 0;
	static jsr_OsmEvent currentEvent;

	/* apply the event filter BEFORE all else (even logging), so no extra work is performed */
	if(event_is_filtered(event_id))
	  return;

	report_count++;
	if(report_count%100 == 0)
	    J_LOG(gData, OSM_LOG_INFO, "The Event handler (report) has been called %ld times\n", report_count);
	/* event Ids can be found in osm_event_plugin.h */
    switch (event_id)
    {
    case OSM_EVENT_ID_PORT_ERRORS:
        currentEvent.EventId = event_id;
        currentEvent.PortDescription.next = NULL;
        currentEvent.PortDescription.node_guid = ((osm_epi_pe_event_t *) event_data)->port_id.node_guid;
        currentEvent.PortDescription.port_num = ((osm_epi_pe_event_t *) event_data)->port_id.port_num;
        sstrncpy(currentEvent.PortDescription.print_desc, ((osm_epi_pe_event_t *) event_data)->port_id.node_name, IB_NODE_DESCRIPTION_SIZE);
        break;
    case OSM_EVENT_ID_PORT_DATA_COUNTERS:
        currentEvent.EventId = event_id;
        currentEvent.PortDescription.next = NULL;
        currentEvent.PortDescription.node_guid = ((osm_epi_dc_event_t *) event_data)->port_id.node_guid;
        currentEvent.PortDescription.port_num = ((osm_epi_dc_event_t *) event_data)->port_id.port_num;
        sstrncpy(currentEvent.PortDescription.print_desc, ((osm_epi_dc_event_t *) event_data)->port_id.node_name, IB_NODE_DESCRIPTION_SIZE);
        break;
    case OSM_EVENT_ID_PORT_SELECT:
        currentEvent.EventId = event_id;
        currentEvent.PortDescription.next = NULL;
        currentEvent.PortDescription.node_guid = ((osm_epi_ps_event_t *) event_data)->port_id.node_guid;
        currentEvent.PortDescription.port_num = ((osm_epi_ps_event_t *) event_data)->port_id.port_num;
        sstrncpy(currentEvent.PortDescription.print_desc, ((osm_epi_ps_event_t *) event_data)->port_id.node_name, IB_NODE_DESCRIPTION_SIZE);
        break;
    case OSM_EVENT_ID_TRAP:
        currentEvent.EventId = event_id;
        currentEvent.trapType = ib_notice_get_type((ib_mad_notice_attr_t *) event_data);
        currentEvent.trapLID = cl_ntoh16(((ib_mad_notice_attr_t *) event_data)->issuer_lid);
        currentEvent.trapNum = 0;
        if (ib_notice_is_generic(((ib_mad_notice_attr_t *) event_data)))
          currentEvent.trapNum = cl_ntoh16(((ib_mad_notice_attr_t *) event_data)->g_or_v.generic.trap_num);
        break;
    case OSM_EVENT_ID_LFT_CHANGE:
        currentEvent.EventId = event_id;
        osm_epi_lft_change_event_t *lft_change = (osm_epi_lft_change_event_t *) event_data;

        currentEvent.PortDescription.next = NULL;
        currentEvent.PortDescription.node_guid = osm_node_get_node_guid(lft_change->p_sw->p_node);
        currentEvent.PortDescription.port_num = 0;
        sstrncpy(currentEvent.PortDescription.print_desc, "Linear Forwarding Table Change", IB_NODE_DESCRIPTION_SIZE);
        break;
    case OSM_EVENT_ID_SUBNET_UP:
    case OSM_EVENT_ID_HEAVY_SWEEP_START:
    case OSM_EVENT_ID_HEAVY_SWEEP_DONE:

      /* this one means that routing is finished.  which likely means NEW routing, so we need to get
       * the new routing information whenever we detect this;
       *
       *        osm_opensm_report_event(sm->p_subn->p_osm,
                                OSM_EVENT_ID_UCAST_ROUTING_DONE,
                                (void *) UCAST_ROUTING_HEAVY_SWEEP);

       *
       */
    case OSM_EVENT_ID_UCAST_ROUTING_DONE:
    case OSM_EVENT_ID_STATE_CHANGE:
    case OSM_EVENT_ID_SA_DB_DUMPED:
       currentEvent.EventId = event_id;
        break;
    case OSM_EVENT_ID_MAX:
    default:
        J_LOG(gData, OSM_LOG_ERROR, "Unknown event reported to the plugin (%d)\n", event_id);
        return;
        break;
    }

    jsr_signalNextEvent(&currentEvent );
}

#if OSM_EVENT_PLUGIN_INTERFACE_VER != 2
#error OpenSM plugin interface version missmatch
#endif

/** =========================================================================
 * Define the object symbol for loading
 */
osm_event_plugin_t osm_event_plugin = {
      osm_version:OSM_VERSION,
      create:create,
      delete:delete,
      report:report
};
