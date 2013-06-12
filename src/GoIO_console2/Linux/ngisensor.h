/*
 * Copyright/Licensing information.
 */

#ifndef NGI_SENSOR_H
#define NGI_SENSOR_H

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Potentially, include other headers on which this header depends.
 */
#include <GSensorDDSMem.h>

/*
 * Type macros.
 */

#define NGI_SENSOR_TYPE		 (ngi_sensor_get_type ())
#define NGI_SENSOR(obj)		 (G_TYPE_CHECK_INSTANCE_CAST ((obj), NGI_SENSOR_TYPE, NgiSensor))
#define NGI_SENSOR_CLASS(klass)	 (G_TYPE_CHECK_CLASS_CAST ((klass),  NGI_SENSOR_TYPE, NgiSensorClass))
#define NGI_IS_SENSOR(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NGI_SENSOR_TYPE))
#define NGI_IS_SENSOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  NGI_SENSOR_TYPE))
#define NGI_SENSOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  NGI_SENSOR_TYPE, NgiSensorClass))

typedef struct _NgiSensor      NgiSensor;
typedef struct _NgiSensorClass NgiSensorClass;

struct _NgiSensor {
  GObject parent;

  /* instance members */
};

struct _NgiSensorClass {
  GObjectClass parent;

  /* class members */
  void (*data_changed)(NgiSensor *sensor, double data);
};


GType ngi_sensor_get_type (void);
GObject* ngi_sensor_new ();


/*
 * Method definitions.
 */
void ngi_sensor_set_dds (NgiSensor *sensor, GSensorDDSRec *dds);
GSensorDDSRec *ngi_sensor_get_dds (NgiSensor *sensor);
void ngi_sensor_set_connected (NgiSensor *sensor, GObject *device);
GObject *ngi_sensor_get_connected (NgiSensor *sensor);

const gchar *ngi_sensor_get_display_name (NgiSensor *sensor);
const gchar *ngi_sensor_get_display_units (NgiSensor *sensor);
const gchar *ngi_sensor_get_readout (NgiSensor *device);

void ngi_sensor_add_measurement (NgiSensor *sensor, gdouble data);

G_END_DECLS

#endif /* NGI_SENSOR_H */
