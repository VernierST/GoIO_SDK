/*
 * Copyright/Licensing information.
 */

#ifndef NGI_SENSOR_WORLD_H
#define NGI_SENSOR_WORLD_H

/*#include <gtk/gtk.h>*/
#include <glib-object.h> 

G_BEGIN_DECLS

/*
 * Potentially, include other headers on which this header depends.
 */
#include "ngisensor.h"

/*
 * Type macros.
 */

#define NGI_SENSOR_WORLD_TYPE		 (ngi_sensor_world_get_type ())
#define NGI_SENSOR_WORLD(obj)		 (G_TYPE_CHECK_INSTANCE_CAST ((obj), NGI_SENSOR_WORLD_TYPE, NgiSensorWorld))
#define NGI_SENSOR_WORLD_CLASS(klass)	 (G_TYPE_CHECK_CLASS_CAST ((klass),  NGI_SENSOR_WORLD_TYPE, NgiSensorWorldClass))
#define NGI_IS_SENSOR_WORLD(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NGI_SENSOR_WORLD_TYPE))
#define NGI_IS_SENSOR_WORLD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  NGI_SENSOR_WORLD_TYPE))
#define NGI_SENSOR_WORLD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  NGI_SENSOR_WORLD_TYPE, NgiSensorWorldClass))

typedef struct _NgiSensorWorld      NgiSensorWorld;
typedef struct _NgiSensorWorldClass NgiSensorWorldClass;

struct _NgiSensorWorld {
  GObject parent;

  /* instance members */
};

struct _NgiSensorWorldClass {
  GObjectClass parent;

  /* class members */
  void (* sensor_connected) (NgiSensorWorld *sensor_world,
			     NgiSensor *sensor);
};


GType ngi_sensor_world_get_type(void);
NgiSensorWorld *ngi_sensor_world_get (void);

/*
 * Method definitions.
 */

GList* ngi_sensor_world_supported_device_interfaces (NgiSensorWorld *sensor_world);
GList* ngi_sensor_world_supported_sensor_interfaces (NgiSensorWorld *sensor_world);
GList* ngi_sensor_world_sensors (NgiSensorWorld *sensor_world);

gboolean ngi_sensor_world_start_collection (NgiSensorWorld *sensor_world, gint device_id);
gboolean ngi_sensor_world_stop_collection  (NgiSensorWorld *sensor_world, gint device_id);
/*
NgiSensor *ngi_sensor_world_get_connected_sensor (NgiSensorWorld *sensor_world, 
						  const gchar *key);
NgiSensor *ngi_sensor_world_add_connected_sensor (NgiSensorWorld *sensor_world,
						  const gchar *key);
*/
/* TESTING */
void ngi_sensor_world_generate_signal(NgiSensorWorld *sensor_world);

G_END_DECLS

#endif /* NGI_SENSOR_WORLD_H */
