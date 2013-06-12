
#include "ngisensor.h"
#include <stdio.h>

enum {
  DATA_CHANGED,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_SENSOR_ID,
  PROP_LONG_COLUMN_NAME,
  PROP_SHORT_COLUMN_NAME,
  PROP_UNIT_NAME
};

#define NGI_SENSOR_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NGI_SENSOR_TYPE, NgiSensorPrivate))
typedef struct _NgiSensorPrivate NgiSensorPrivate;


struct _NgiSensorPrivate {
  /* properties */
  GSensorDDSRec *dds;
  GObject *device;
  gdouble latest;
};


static void ngi_sensor_class_init      (NgiSensorClass *klass);
static void ngi_sensor_init            (GTypeInstance       *instance,
					gpointer             g_class);
static void ngi_sensor_set_property    (GObject             *object,
					guint                property_id,
					const GValue        *value,
					GParamSpec          *pspec);
static void ngi_sensor_get_property    (GObject             *object,
					guint                property_id,
					GValue              *value,
					GParamSpec          *pspec);


static GObject* ngi_sensor_constructor (GType      type,
					guint      n_construct_properties,
					GObjectConstructParam *construct_params);

static void ngi_sensor_dispose (GObject *self);

static void ngi_sensor_handle_data_changed (NgiSensor *sensor, gdouble data);



static GObjectClass *parent_class = NULL;
static guint sensor_signals[LAST_SIGNAL] = { 0 };


GType
ngi_sensor_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (NgiSensorClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      (GClassInitFunc)ngi_sensor_class_init,
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (NgiSensor),
      0,      /* n_preallocs */
      (GInstanceInitFunc)ngi_sensor_init,
    };
    type = g_type_register_static(G_TYPE_OBJECT, "NgiSensorType",
				  &info, 0);
  }
  return type;
}


static void
ngi_sensor_class_init(NgiSensorClass *klass)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *pspec           = NULL;

  gobject_class = G_OBJECT_CLASS(klass);


  parent_class = g_type_class_peek_parent(klass);

  gobject_class->constructor = ngi_sensor_constructor;
  gobject_class->dispose = ngi_sensor_dispose;
  gobject_class->set_property = ngi_sensor_set_property;
  gobject_class->get_property = ngi_sensor_get_property;

  klass->data_changed = ngi_sensor_handle_data_changed;

  /* set up properties */


  /* set up signals */
  /**
   * NgiSensor::data-changed
   * @interface: the object that received the signal
   *
   * Emitted when something happens
   *
   */
  sensor_signals[DATA_CHANGED] =
    g_signal_new("data-changed",
		 G_OBJECT_CLASS_TYPE(gobject_class),
		 G_SIGNAL_RUN_FIRST,
		 G_STRUCT_OFFSET(NgiSensorClass, data_changed),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__DOUBLE,
		 G_TYPE_NONE, 1,
		 G_TYPE_DOUBLE);


  g_type_class_add_private (klass, sizeof (NgiSensorPrivate));
}


static void
ngi_sensor_init (GTypeInstance   *instance,
		 gpointer         g_class)
{
  NgiSensor *self = (NgiSensor*)instance;
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(self);
  (void)self;

  /* initialize all public and private members to reasonable default values. */
  /* If you need specific consruction properties to complete initialization,
   * delay initialization completion until the property is set.
   */

  priv->device = NULL;
}

static GObject *
ngi_sensor_constructor (GType                  type,
			guint                  n_construct_properties,
			GObjectConstructParam *construct_params)
{
  GObject *object;
  NgiSensor *self;

  /* chain up with parent class' constructor */
  object = (*G_OBJECT_CLASS(parent_class)->constructor)(type,
							n_construct_properties,
							construct_params);

  /* here you can do stuff based on arguments which were passed to g_object_new(...) */
  self = NGI_SENSOR(object);

  return object;
}

static void
ngi_sensor_dispose (GObject *self)
{
  static gboolean first_run = TRUE;

  if (first_run)
    {
      first_run = FALSE;

      parent_class->dispose (self);
    }
}

static void
ngi_sensor_set_property(GObject *object,
			guint   property_id,
			const GValue *value,
			GParamSpec  *pspec)
{
  NgiSensor *self = (NgiSensor*)object;
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(self);

  switch(property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void
ngi_sensor_get_property(GObject    *object,
			guint      property_id,
			GValue     *value,
			GParamSpec *pspec)
{
  NgiSensor *self = (NgiSensor*)object;
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(self);

  switch(property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}


static void
ngi_sensor_handle_data_changed (NgiSensor *sensor, gdouble data)
{
  g_printf("ngi_sensor_handle_data_changed(%f)\n",data);
}


GObject*
ngi_sensor_new()
{
  return g_object_new(NGI_SENSOR_TYPE, NULL);
}

void
ngi_sensor_set_dds (NgiSensor *sensor, GSensorDDSRec *dds)
{
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(sensor);
  priv->dds = dds;

}

GSensorDDSRec *
ngi_sensor_get_dds (NgiSensor *sensor)
{
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(sensor);
  return priv->dds;
}

void
ngi_sensor_set_connected (NgiSensor *sensor, GObject *device)
{
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(sensor);
  priv->device = device;
}

GObject *
ngi_sensor_get_connected (NgiSensor *sensor)
{
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(sensor);
  return priv->device;
}

const gchar *
ngi_sensor_get_display_name (NgiSensor *sensor)
{
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(sensor);

  if (priv->dds)
    return priv->dds->SensorLongName;

  return "Undefined";
}

const gchar *
ngi_sensor_get_display_units (NgiSensor *sensor)
{
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(sensor);

  if (priv->dds)
    {
      gint cal_page = priv->dds->ActiveCalPage;
      const gchar *units = priv->dds->CalibrationPage[cal_page].Units;

      g_printf ("Found active cal page: %d with units: %s\n",cal_page, units);
      return units;
    }

  return "Undefined";
}
/*
const gchar *
ngi_sensor_get_readout (NgiSensor *sensor);
{
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(sensor);
  return "todo";
}
*/
void
ngi_sensor_add_measurement (NgiSensor *sensor, gdouble data)
{
  NgiSensorPrivate *priv = NGI_SENSOR_GET_PRIVATE(sensor);
  g_printf ("Collected: %f\n",data);
  priv->latest = data;
}
