/*
 * Copyright/Licensing information.
 */

#ifndef NGI_DEVICE_H
#define NGI_DEVICE_H

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Potentially, include other headers on which this header depends.
 */
#include "ngisensor.h"

/*
 * Type macros.
 */

#define NGI_DEVICE_TYPE		 (ngi_device_get_type ())
#define NGI_DEVICE(obj)		 (G_TYPE_CHECK_INSTANCE_CAST ((obj), NGI_DEVICE_TYPE, NgiDevice))
#define NGI_DEVICE_CLASS(klass)	 (G_TYPE_CHECK_CLASS_CAST ((klass),  NGI_DEVICE_TYPE, NgiDeviceClass))
#define NGI_IS_DEVICE(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NGI_DEVICE_TYPE))
#define NGI_IS_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  NGI_DEVICE_TYPE))
#define NGI_DEVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  NGI_DEVICE_TYPE, NgiDeviceClass))

typedef struct _NgiDevice      NgiDevice;
typedef struct _NgiDeviceClass NgiDeviceClass;

struct _NgiDevice {
  GObject parent;

  /* instance members */
};

struct _NgiDeviceClass {
  GObjectClass parent;

  /* class members */
  void (*sensor_connected)(NgiDevice *device);
  void (*sensor_disconnected)(NgiDevice *device);
  void (*data_changed) (NgiDevice *device, double data);
};


GType ngi_device_get_type (void);
GObject* ngi_device_new (GObject *interface,
			 const gchar *display_name,
			 const gchar *name,
			 GList *children);


/*
 * Method definitions.
 */
const char *ngi_device_get_name (NgiDevice *device);
const char *ngi_device_get_display_name (NgiDevice *device);

gboolean ngi_device_connected (NgiDevice *device);
gboolean ngi_device_collecting (NgiDevice *device);
void *ngi_device_get_device_handle (NgiDevice *device);
GList *ngi_device_get_children (NgiDevice *device);

void ngi_device_connect (NgiDevice *device);
void ngi_device_disconnect (NgiDevice *device);

void ngi_device_connect_device_handle (NgiDevice *device, void *device_handle);
void ngi_device_disconnect_device_handle (NgiDevice *device);

void ngi_device_check_sensor (NgiDevice *device);
void ngi_device_attach_sensor (NgiDevice *device, NgiSensor *sensor);
NgiSensor *ngi_device_detach_sensor (NgiDevice *device);

NgiSensor *ngi_device_get_sensor (NgiDevice *device);

gboolean ngi_device_start_collection (NgiDevice *device);
gboolean ngi_device_stop_collection (NgiDevice *device);

gint ngi_device_get_measurements (gpointer data);

G_END_DECLS

#endif /* NGI_DEVICE_H */
