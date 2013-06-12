/*
 * Copyright/Licensing information.
 */

#ifndef NGI_DEVICE_INTERFACE_H
#define NGI_DEVICE_INTERFACE_H

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Potentially, include other headers on which this header depends.
 */
#include "GoIO_DLL_interface.h"
#include "ngidevice.h"


/*
 * Type macros.
 */

#define NGI_DEVICE_INTERFACE_TYPE		 (ngi_device_interface_get_type ())
#define NGI_DEVICE_INTERFACE(obj)		 (G_TYPE_CHECK_INSTANCE_CAST ((obj), NGI_DEVICE_INTERFACE_TYPE, NgiDeviceInterface))
#define NGI_DEVICE_INTERFACE_CLASS(klass)	 (G_TYPE_CHECK_CLASS_CAST ((klass),  NGI_DEVICE_INTERFACE_TYPE, NgiDeviceInterfaceClass))
#define NGI_IS_DEVICE_INTERFACE(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NGI_DEVICE_INTERFACE_TYPE))
#define NGI_IS_DEVICE_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  NGI_DEVICE_INTERFACE_TYPE))
#define NGI_DEVICE_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  NGI_DEVICE_INTERFACE_TYPE, NgiDeviceInterfaceClass))

typedef struct _NgiDeviceInterface      NgiDeviceInterface;
typedef struct _NgiDeviceInterfaceClass NgiDeviceInterfaceClass;

struct _NgiDeviceInterface {
  GObject parent;

  /* instance members */
};

struct _NgiDeviceInterfaceClass {
  GObjectClass parent;

  /* class members */
  void (*test_signal)(NgiDeviceInterface *interface);
};


GType ngi_device_interface_get_type(void);
GObject* ngi_device_interface_new(gint vendor_id, gint product_id, const gchar *display_name);


/*
 * Method definitions.
 */

void ngi_device_interface_add_device (NgiDeviceInterface *interface, const gchar* name);
void ngi_device_interface_remove_device (NgiDeviceInterface *interface, const gchar* name);
gint ngi_device_interface_search (gpointer data);
void ngi_device_interface_connect_device (NgiDeviceInterface *interface, NgiDevice *device);
void ngi_device_interface_disconnect_device (NgiDeviceInterface *interface, NgiDevice *device);
gint ngi_device_interface_query_sensor_id (NgiDeviceInterface *interface, NgiDevice *device);
GSensorDDSRec *ngi_device_interface_get_dds (NgiDeviceInterface *interface, NgiDevice *device);
gboolean ngi_device_interface_start_collection (NgiDeviceInterface *interface, NgiDevice *device);
gboolean ngi_device_interface_stop_collection (NgiDeviceInterface *interface, NgiDevice *device);
GArray *ngi_device_interface_get_measurements (NgiDeviceInterface *interface, NgiDevice *device);

/* TESTING */
void ngi_device_interface_generate_signal(NgiDeviceInterface *interface);

G_END_DECLS

#endif /* NGI_DEVICE_INTERFACE_H */
