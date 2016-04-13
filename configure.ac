dnl Process this file with autoconf to produce a configure script.

AC_INIT(opensm-jni-plugin-libs, 2.0.1, meier3@llnl.gov)
AC_META
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([META])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE($PROJECT, $VERSION)

dnl Checks for programs
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_PROG_LIBTOOL

dnl set up a #define for the code to find the config file.
CONF_TMP1="`eval echo ${sysconfdir}`"
CONF_TMP2="`echo $CONF_TMP1 | sed 's/^NONE/$ac_default_prefix/'`"
CONF_FILE_PREFIX="`eval echo $CONF_TMP2`/opensm-plugin"
AC_DEFINE_UNQUOTED(CONF_FILE_PREFIX, ["$CONF_FILE_PREFIX"], [Config Directory])

OPENSM_PI_CONFIG_DIR="`eval echo $CONF_TMP2`/opensm-plugin"
AC_SUBST(OPENSM_PI_CONFIG_DIR)

dnl Check for a different default OpenSm config file
OPENSM_PI_CONFIG_FILE=libOsmJniPi.conf
AC_DEFINE_UNQUOTED(OPENSM_PI_CONFIG_FILE, ["libOsmJniPi.conf"], [Configuration File])
AC_MSG_RESULT(${OPENSM_PI_CONFIG_FILE})
AC_DEFINE_UNQUOTED(HAVE_DEFAULT_OPENSM_PI_CONFIG_FILE,
	["$OPENSM_PI_CONFIG_DIR/$OPENSM_PI_CONFIG_FILE"],
	[Define a default OpenSM Plugin config file])
AC_SUBST(OPENSM_PI_CONFIG_FILE)

dnl OpenSM headers are in $includedir/infiniband
INC_TMP1="`eval echo ${includedir}/infiniband`"
INC_TMP2="`echo $INC_TMP1 | sed 's/^NONE/$ac_default_prefix/'`"
CFLAGS="$CFLAGS -I`eval echo $INC_TMP2`"
CPPFLAGS="$CPPFLAGS -I`eval echo $INC_TMP2`"

dnl Need, so assume we are using the PerfMgr
ENABLE_OSM_PERF_MGR=1

dnl Check for opensm compatibility header
AC_CHECK_HEADER([opensm/osm_event_plugin.h], , 
	AC_MSG_ERROR([opensm/osm_event_plugin.h not found.  libOsmJniPi requires opensm with event plugin support.]))

dnl Checks for libraries
AC_CHECK_LIB(pthread, pthread_mutex_init, [],
	AC_MSG_ERROR([pthread_mutex_init() not found.  libOsmJniPi requires libpthread.]))

dnl the library version
libOsmJniPi_api_version=2:0:1
AC_SUBST(libOsmJniPi_api_version)

dnl We use --version-script with ld if possible
AC_CACHE_CHECK(whether ld accepts --version-script, ac_cv_version_script,
    if test -n "`$LD --help < /dev/null 2>/dev/null | grep version-script`"; then
        ac_cv_version_script=yes
    else
        ac_cv_version_script=no
    fi)

AM_CONDITIONAL(HAVE_LD_VERSION_SCRIPT, test "$ac_cv_version_script" = "yes")

# we have to revive the env CFLAGS as some how they are being overwritten...
# see http://sources.redhat.com/automake/automake.html#Flag-Variables-Ordering
# for why they should NEVER be modified by the configure to allow for user
# overrides.
CFLAGS=$ac_env_CFLAGS_value

AC_CONFIG_FILES([osmJniPi_version.h \
		Makefile \
		opensm-jni-plugin.spec \
		])
AC_OUTPUT
