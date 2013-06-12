#include "ngisensorworld.h"
#include "GoIO_DLL_interface.h"
#include <stdio.h>
#include "ngideviceinterface.h"

enum {
  SENSOR_CONNECTED_SIGNAL,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_INTERFACES
};

#define NGI_SENSOR_WORLD_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NGI_SENSOR_WORLD_TYPE, NgiSensorWorldPrivate))
typedef struct _NgiSensorWorldPrivate NgiSensorWorldPrivate;


struct _NgiSensorWorldPrivate {
  /* properties */
  GList *interfaces;
  GHashTable *connected;
};


static void ngi_sensor_world_class_init      (NgiSensorWorldClass *klass);
static void ngi_sensor_world_init            (GTypeInstance       *instance,
					      gpointer             g_class);
static void ngi_sensor_world_set_property    (GObject             *object,
					      guint                property_id,
					      const GValue        *value,
					      GParamSpec          *pspec);
static void ngi_sensor_world_get_property    (GObject             *object,
					      guint                property_id,
					      GValue              *value,
					      GParamSpec          *pspec);


static GObject* ngi_sensor_world_constructor (GType      type,
					      guint      n_construct_properties,
					      GObjectConstructParam *construct_params);

static void ngi_sensor_world_dispose (GObject *self);

static void ngi_sensor_world_handle_test_signal(NgiSensorWorld *sensor_world);



static GObjectClass *parent_class = NULL;
static guint world_signals[LAST_SIGNAL] = { 0 };


GType
ngi_sensor_world_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (NgiSensorWorldClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      (GClassInitFunc)ngi_sensor_world_class_init,
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (NgiSensorWorld),
      0,      /* n_preallocs */
      (GInstanceInitFunc)ngi_sensor_world_init,
    };
    type = g_type_register_static(G_TYPE_OBJECT, "NgiSensorWorldType",
				  &info, 0);
  }
  return type;
}


static void
ngi_sensor_world_class_init(NgiSensorWorldClass *klass)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *pspec           = NULL;

  gobject_class = G_OBJECT_CLASS(klass);


  parent_class = g_type_class_peek_parent(klass);

  gobject_class->constructor = ngi_sensor_world_constructor;
  gobject_class->dispose = ngi_sensor_world_dispose;
  gobject_class->set_property = ngi_sensor_world_set_property;
  gobject_class->get_property = ngi_sensor_world_get_property;

  klass->sensor_connected = NULL;/*ngi_sensor_world_handle_test_signal;*/

  /* set up properties */
  pspec = g_param_spec_pointer("interfaces",
			      "Ngi Sensor World List of supported interfaces",
			      "Set/Get Ngi Sensor World interfaces property",
			      G_PARAM_CONSTRUCT | G_PARAM_READWRITE); /* G_PARAM_CONSTRUCT_ONLY */
  g_object_class_install_property(gobject_class, PROP_INTERFACES, pspec);

  /* set up signals */
  /**
   * NgiSensorWorld::sensor-connected
   * @sensor_world: the object that received the signal
   *
   * Emitted when something happens
   *
   */
  world_signals[SENSOR_CONNECTED_SIGNAL] =
    g_signal_new("sensor-connected",
		 G_OBJECT_CLASS_TYPE(gobject_class),
		 G_SIGNAL_RUN_FIRST,
		 G_STRUCT_OFFSET(NgiSensorWorldClass, sensor_connected),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 
		 1,
		 NGI_SENSOR_TYPE);


  g_type_class_add_private (klass, sizeof (NgiSensorWorldPrivate));
}


static void
ngi_sensor_world_init (GTypeInstance   *instance,
		       gpointer         g_class)
{
  NgiSensorWorld *self = (NgiSensorWorld*)instance;
  NgiSensorWorldPrivate *priv = NGI_SENSOR_WORLD_GET_PRIVATE(self);
  (void)self;

  /* initialize all public and private members to reasonable default values. */
  /* If you need specific consruction properties to complete initialization,
   * delay initialization completion until the property is set. 
   */
}

static GObject *
ngi_sensor_world_constructor (GType                  type,
			      guint                  n_construct_properties,
			      GObjectConstructParam *construct_params)
{
  GObject *object;
  NgiSensorWorld *sensor_world;

  /* chain up with parent class' constructor */
  object = (*G_OBJECT_CLASS(parent_class)->constructor)(type,
							n_construct_properties,
							construct_params);

  /* here you can do stuff based on arguments which were passed to g_object_new(...) */
  sensor_world = NGI_SENSOR_WORLD(object);

  GoIO_Init();
  printf ("Init GoIO\n");
  
  return object;
}

static void
ngi_sensor_world_dispose (GObject *self)
{
  static gboolean first_run = TRUE;
  printf ("Uninit GoIO\n");

  if (first_run)
    {
      first_run = FALSE;
      GoIO_Uninit ();

      parent_class->dispose (self);
    }
}

static void
ngi_sensor_world_set_property(GObject *object,
			      guint   property_id,
			      const GValue *value,
			      GParamSpec  *pspec)
{
  NgiSensorWorld *self = (NgiSensorWorld*)object;
  NgiSensorWorldPrivate *priv = NGI_SENSOR_WORLD_GET_PRIVATE(self);

  switch(property_id) {
  case PROP_INTERFACES:
    priv->interfaces = g_value_get_pointer (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void
ngi_sensor_world_get_property(GObject    *object,
			      guint      property_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  NgiSensorWorld *self = (NgiSensorWorld*)object;
  NgiSensorWorldPrivate *priv = NGI_SENSOR_WORLD_GET_PRIVATE(self);

  switch(property_id) {
  case PROP_INTERFACES:
    g_value_set_pointer (value, priv->interfaces);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}


static void
ngi_sensor_world_handle_test_signal(NgiSensorWorld *sensor_world)
{
  g_print("ngi_sensor_world_handle_test_signal()\n");
}

/**
 * ngi_sensor_world_get:
 * @returns: the singleton #NgiSensorWorld object
 * 
 * Returns the globale #NgiSensorWorld singleton.
 *
 **/
NgiSensorWorld *
ngi_sensor_world_get (void)
{
  static NgiSensorWorld *sensor_world = NULL;

  if (!sensor_world)
    sensor_world = g_object_new (NGI_SENSOR_WORLD_TYPE, NULL);

  return sensor_world;
}

/**
 * ngi_sensor_world_supported_device_interfaces:
 * @sensor_world: a #NGISensorWorld
 * 
 * Returns a list of device interfaces currently supported.
 *
 * Return value: #GList
 **/
GList*
ngi_sensor_world_supported_device_interfaces (NgiSensorWorld *sensor_world)
{  
  NgiSensorWorldPrivate *priv = NGI_SENSOR_WORLD_GET_PRIVATE(sensor_world);

  if (!priv->interfaces)
    { 
      gpointer interface = (gpointer) ngi_device_interface_new (VERNIER_DEFAULT_VENDOR_ID,
								USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID, 
								"Go! Temp");
      priv->interfaces = g_list_append (priv->interfaces, interface); 
      interface = (gpointer) ngi_device_interface_new (VERNIER_DEFAULT_VENDOR_ID,
						       SKIP_DEFAULT_PRODUCT_ID, 
						       "Go! Link");
      priv->interfaces = g_list_append (priv->interfaces, interface); 
      interface = (gpointer) ngi_device_interface_new (VERNIER_DEFAULT_VENDOR_ID,
						       CYCLOPS_DEFAULT_PRODUCT_ID, 
						       "Go! Motion");
      priv->interfaces = g_list_append (priv->interfaces, interface); 

    }
  return g_list_copy (priv->interfaces);
}

gint
ngi_sensor_world_connect_to_device(NgiSensorWorld *sensor_world,
				   const gchar *device_name)
{

  g_print("ngi_sensor_world_connect_to_device()\n");
  return 0;
}

gboolean
ngi_sensor_world_start_collection(NgiSensorWorld *sensor_world,
				  gint device_id)
{
  g_print("ngi_sensor_world_start_collection()\n");
  return FALSE;
}

gboolean
ngi_sensor_world_stop_collection(NgiSensorWorld *sensor_world,
				 gint device_id)
{
  g_print("ngi_sensor_world_stop_collection()\n");
  return FALSE;
}

gboolean
ngi_sensor_world_disconnect_device(NgiSensorWorld *sensor_world,
				   gint device_id)
{
  g_print("ngi_sensor_world_disconnect_device()\n");
  return FALSE;
}
/*
NgiSensor * 
ngi_sensor_world_get_connected_sensor (NgiSensorWorld *sensor_world, const gchar *key)
{
  NgiSensorWorldPrivate *priv = NGI_SENSOR_WORLD_GET_PRIVATE(sensor_world);
  return (NgiSensor *)g_hash_table_lookup (priv->connected_sensors, key);
}

NgiSensor * 
ngi_sensor_world_add_connected_sensor (NgiSensorWorld *sensor_world, const gchar *key)
{
  NgiSensorWorldPrivate *priv = NGI_SENSOR_WORLD_GET_PRIVATE(sensor_world);
  NgiSensor *sensor = (NgiSensor *)ngi_sensor_new ();

  if (sensor)
    {
      g_hash_table_insert (priv->connected_sensors, g_strdup (key), (gpointer) sensor);
      g_signal_emit (sensor_world, world_signals[SENSOR_CONNECTED_SIGNAL], 1, (gpointer)sensor);
    }
  return sensor;
}
*/
