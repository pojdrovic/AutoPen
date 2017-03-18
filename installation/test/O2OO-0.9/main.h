// SVN: $Revision: 442 $
typedef enum { MODE_ELM327, MODE_TEST } io_mode_t;

typedef enum { CM_NONE, CM_BATCH, CM_INTERVAL } commit_mode_t;

typedef enum { DISP_SIMPLE, DISP_DASHBOARD } display_mode_t;

#define DEFAULT_SERIAL_PORT "/dev/rfcomm1"

#define GPS_TABLE "gps_location"
#define GPS_MAP_LINES "gps_osm_map_lines"
#define GPS_MAP_NODES "gps_osm_map_nodes"
#define GPS_MAP_POLY "gps_osm_map_poly"
#define SENSOR_META_DATA "table_meta_data"
#define META_DATA "global_info"
#define SENSOR_TABLE_PREFIX "sensor_"

#define SENSOR_MAX_ERROR_BEFORE_IGNORE	5
