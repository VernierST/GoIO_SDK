#include "ngidevice.h"
#include "ngideviceinterface.h"
#include "ngisensor.h"
#include <stdio.h>

enum {
  SENSOR_CONNECTED,
  SENSOR_DISCONNECTED,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_INTERFACE,
  PROP_DISPLAY_NAME,
  PROP_NAME,
  PROP_CHILDREN
};

#define NGI_DEVICE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NGI_DEVICE_TYPE, NgiDevicePrivate))
typedef struct _NgiDevicePrivate NgiDevicePrivate;


struct _NgiDevicePrivate {
  /* properties */
  NgiDeviceInterface *interface;
  gchar* display_name;
  gchar* name;
  GList* children;
  NgiSensor *sensor;
  gboolean collecting;

  void* device_handle;
};


static void ngi_device_class_init      (NgiDeviceClass *klass);
static void ngi_device_init            (GTypeInstance       *instance,
					gpointer             g_class);
static void ngi_device_set_property    (GObject             *object,
					guint                property_id,
					const GValue        *value,
					GParamSpec          *pspec);
static void ngi_device_get_property    (GObject             *object,
					guint                property_id,
					GValue              *value,
					GParamSpec          *pspec);


static GObject* ngi_device_constructor (GType      type,
					guint      n_construct_properties,
					GObjectConstructParam *construct_params);

static void ngi_device_dispose (GObject *self);

static GObjectClass *parent_class = NULL;
static guint device_signals[LAST_SIGNAL] = { 0 };


GType
ngi_device_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (NgiDeviceClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      (GClassInitFunc)ngi_device_class_init,
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (NgiDevice),
      0,      /* n_preallocs */
      (GInstanceInitFunc)ngi_device_init,
    };
    type = g_type_register_static(G_TYPE_OBJECT, "NgiDeviceType",
				  &info, 0);
  }
  return type;
}


static void
ngi_device_class_init(NgiDeviceClass *klass)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *pspec           = NULL;

  gobject_class = G_OBJECT_CLASS(klass);


  parent_class = g_type_class_peek_parent(klass);

  gobject_class->constructor = ngi_device_constructor;
  gobject_class->dispose = ngi_device_dispose;
  gobject_class->set_property = ngi_device_set_property;
  gobject_class->get_property = ngi_device_get_property;

  klass->sensor_connected = NULL;
  klass->sensor_disconnected = NULL;

  /* set up properties */
  pspec = g_param_spec_string("display-name",
			   "Ngi Device Display Name Property",
			   "Set/Get Ngi Device Display Name Property",
			   "",
			   G_PARAM_CONSTRUCT_ONLY|G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_DISPLAY_NAME, pspec);
  pspec = g_param_spec_string("name",
			   "Ngi Device Name Property",
			   "Set/Get Ngi Device Name Property",
			   "",
			   G_PARAM_CONSTRUCT_ONLY|G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_NAME, pspec);
  pspec = g_param_spec_pointer("children",
			   "Ngi Device Children Property",
			   "Set/Get Ngi Device Children Property",
			   G_PARAM_CONSTRUCT_ONLY|G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_CHILDREN, pspec);
  pspec = g_param_spec_pointer("interface",
			   "Ngi Device Interface Property",
			   "Set/Get Ngi Device Interface Property",
			   G_PARAM_CONSTRUCT_ONLY|G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_INTERFACE, pspec);


  /* set up signals */
  /**
   * NgiDevice::sensor-connected
   * @interface: the object that received the signal
   *
   * Emitted when something happens
   *
   */
  device_signals[SENSOR_CONNECTED] =
    g_signal_new("sensor-connected",
		 G_OBJECT_CLASS_TYPE(gobject_class),
		 G_SIGNAL_RUN_FIRST,
		 G_STRUCT_OFFSET(NgiDeviceClass, sensor_connected),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * NgiDevice::sensor-disconnected
   * @interface: the object that received the signal
   *
   * Emitted when something happens
   *
   */
  device_signals[SENSOR_DISCONNECTED] =
    g_signal_new("sensor-disconnected",
		 G_OBJECT_CLASS_TYPE(gobject_class),
		 G_SIGNAL_RUN_FIRST,
		 G_STRUCT_OFFSET(NgiDeviceClass, sensor_disconnected),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  g_type_class_add_private (klass, sizeof (NgiDevicePrivate));
}


static void
ngi_device_init (GTypeInstance   *instance,
		 gpointer         g_class)
{
  NgiDevice *self = (NgiDevice*)instance;
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(self);
  (void)self;

  /* initialize all public and private members to reasonable default values. */
  /* If you need specific consruction properties to complete initialization,
   * delay initialization completion until the property is set.
   */
  priv->device_handle = NULL;
  priv->collecting = FALSE;
  priv->sensor = NULL;
}

static GObject *
ngi_device_constructor (GType                  type,
			guint                  n_construct_properties,
			GObjectConstructParam *construct_params)
{
  GObject *object;
  NgiDevice *self;

  /* chain up with parent class' constructor */
  object = (*G_OBJECT_CLASS(parent_class)->constructor)(type,
							n_construct_properties,
							construct_params);

  /* here you can do stuff based on arguments which were passed to g_object_new(...) */
  self = NGI_DEVICE(object);

  return object;
}

static void
ngi_device_dispose (GObject *self)
{
  static gboolean first_run = TRUE;

  if (first_run)
    {
      first_run = FALSE;

      parent_class->dispose (self);
    }
}

static void
ngi_device_set_property(GObject *object,
			guint   property_id,
			const GValue *value,
			GParamSpec  *pspec)
{
  NgiDevice *self = (NgiDevice*)object;
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(self);

  switch(property_id) {
  case PROP_DISPLAY_NAME:
    priv->display_name = g_value_dup_string (value);
    break;
  case PROP_NAME:
    priv->name = g_value_dup_string (value);
    break;
  case PROP_INTERFACE:
    priv->interface = g_value_get_pointer (value);
    break;
  case PROP_CHILDREN:
    priv->children = g_value_get_pointer (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void
ngi_device_get_property(GObject    *object,
			guint      property_id,
			GValue     *value,
			GParamSpec *pspec)
{
  NgiDevice *self = (NgiDevice*)object;
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(self);

  switch(property_id) {
  case PROP_DISPLAY_NAME:
    g_value_set_string (value, priv->display_name);
    break;
  case PROP_NAME:
    g_value_set_string (value, priv->name);
    break;
  case PROP_INTERFACE:
    g_value_set_pointer (value, priv->interface);
    break;
  case PROP_CHILDREN:
    g_value_set_pointer (value, priv->children);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

GObject*
ngi_device_new(GObject *interface,
	       const gchar *display_name,
	       const gchar *name,
	       GList *children)
{
  return g_object_new(NGI_DEVICE_TYPE,
		      "interface", interface,
		      "display-name", display_name,
		      "name", name,
		      "children", children,
		      NULL);
}

void
ngi_device_connect (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  g_printf ("trying to connect the device\n");
  ngi_device_interface_connect_device (priv->interface, device);

  ngi_device_check_sensor (device);
}

void
ngi_device_disconnect (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  ngi_device_interface_disconnect_device (priv->interface, device);
}

const gchar*
ngi_device_get_display_name (NgiDevice    *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  return priv->display_name;
}

const gchar*
ngi_device_get_name (NgiDevice    *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  return priv->name;
}

void
ngi_device_connect_device_handle (NgiDevice *device,
				  void *device_handle)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  priv->device_handle = device_handle;
}

void
ngi_device_disconnect_device_handle (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  priv->device_handle = NULL;
}

gboolean
ngi_device_connected(NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);

  if (priv->device_handle)
    {
      return TRUE;
    }
  return FALSE;
}

gboolean
ngi_device_collecting (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  return priv->collecting;
}

void *
ngi_device_get_device_handle (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  return priv->device_handle;
}

GList *
ngi_device_get_children (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  return priv->children;
}

NgiSensor *
ngi_device_detach_sensor (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  NgiSensor *sensor = priv->sensor;
  priv->sensor = NULL;

   g_signal_emit (device, device_signals[SENSOR_DISCONNECTED], 0);
  return sensor;
}

void
ngi_device_attach_sensor (NgiDevice *device, NgiSensor *sensor)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);

  if (priv->sensor)
  	ngi_device_detach_sensor (device);

  priv->sensor = sensor;

  /*emit a signal so the sensor world knows a sensor is connected*/
   g_signal_emit (device, device_signals[SENSOR_CONNECTED], 0);
}

void
ngi_device_check_sensor (NgiDevice *device)
{
  if (ngi_device_connected (device) && !ngi_device_collecting (device))
    {
      NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
      if (!priv->children)
	{
	  gint sensor_id = ngi_device_interface_query_sensor_id (priv->interface, device);
	  if (priv->sensor)
	    {
	      /*do nothing for now.*/
	    }
	  else
	    {
	      /*add a sensor*/
	      NgiSensor *sensor = (NgiSensor *)ngi_sensor_new ();
	      GSensorDDSRec *dds = ngi_device_interface_get_dds (priv->interface, device);
	      ngi_sensor_set_dds (sensor, dds);
	      ngi_device_attach_sensor (device, sensor);
	    }
	}
      else
	{
	  /*Pass it along, eventually.*/
	}
    }
}


NgiSensor *
ngi_device_get_sensor (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  return priv->sensor;
}

gboolean 
ngi_device_start_collection (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  gboolean success = FALSE;

  success = ngi_device_interface_start_collection (priv->interface, device);
  priv->collecting = success;

  g_timeout_add (10, ngi_device_get_measurements, device);

  return success;
}

gboolean 
ngi_device_stop_collection (NgiDevice *device)
{
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  gboolean success = FALSE;

  success = ngi_device_interface_stop_collection (priv->interface, device);
  priv->collecting = !success;

  return success;
}

gint
ngi_device_get_measurements (gpointer data)
{
  NgiDevice *device = (NgiDevice *)data;
  NgiDevicePrivate *priv = NGI_DEVICE_GET_PRIVATE(device);
  gboolean still_continue = TRUE;

  if (priv->collecting)
    {
      GArray *measurements = ngi_device_interface_get_measurements (priv->interface, device);
      if (measurements && measurements->len > 0)
	{
	  gint ix;
	  for (ix=0; ix<measurements->len; ix++)
	    {
	      gdouble data = g_array_index (measurements, gdouble, ix);
	      ngi_sensor_add_measurement (priv->sensor, data);
	      g_signal_emit_by_name (priv->sensor, "data-changed", data);
	    }

	  g_array_free (measurements, TRUE);
	}
    }
  else
    {
      /* stop this timer */
      still_continue = FALSE;
    }

  return still_continue;
}
