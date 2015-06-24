OsmJpiInterface
=========================
by Tim Meier, [meier3@llnl.gov](mailto:meier3@llnl.gov)

**libOsmJniPi.so** is the Java Native Interface between `opensm` and `OMS`

Released under the GNU LGPL, `LLNL-CODE-673346`.  See the `LICENSE`
file for details.

Overview
-------------------------

This native C library is an "osm_event_plugin" package.  It is designed to be
dynamically loaded by opensm, via the settings in the config file.

This package implements the event plugin interface ( see **opensm/osm_event_plugin.h** ),
and populates a region of shared memory with fabric information.  This shared
memory region is in fact the Java Native Interface, for use by the OsmJpiServer
package.

This package interfaces directly with opensm through the event plugin interface,
and also through the main data pointer.  This package loads a Java Virtual
Machine, and starts a "main" routine in the OsmJpiServer package for handling
connection requests from OMS Clients and listeners.

Together, these packages provide a mechanism for OMS Clients and Listeners to
connect to the subnet manager and obtain information about the fabric.

**NOTE:** The plugins version is contained in osmJniPi_version.h.in which in turn is obtained
from the META file and a perl script (first line in the spec file) that combines the
opensm version with this packages version & release.  