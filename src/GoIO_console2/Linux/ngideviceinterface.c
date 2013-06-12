#include "ngideviceinterface.h"
#include "ngidevice.h"
#include <GSensorDDSMem.h>
#include <stdio.h>

enum {
  TEST_SIGNAL,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_VENDOR_ID,
  PROP_PRODUCT_ID,
  PROP_DISPLAY_NAME,
  PROP_DEVICES
};

#define NGI_DEVICE_INTERFACE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NGI_DEVICE_INTERFACE_TYPE, NgiDeviceInterfacePrivate))
typedef struct _NgiDeviceInterfacePrivate NgiDeviceInterfacePrivate;


struct _NgiDeviceInterfacePrivate {
  /* properties */
  gint vendor_id;
  gint product_id;
  gchar* display_name;
  GList* devices;
};


static void ngi_device_interface_class_init      (NgiDeviceInterfaceClass *klass);
static void ngi_device_interface_init            (GTypeInstance       *instance,
						  gpointer             g_class);
static void ngi_device_interface_set_property    (GObject             *object,
						  guint                property_id,
						  const GValue        *value,
						  GParamSpec          *pspec);
static void ngi_device_interface_get_property    (GObject             *object,
						  guint                property_id,
						  GValue              *value,
						  GParamSpec          *pspec);


static GObject* ngi_device_interface_constructor (GType      type,
						  guint      n_construct_properties,
						  GObjectConstructParam *construct_params);

static void ngi_device_interface_dispose (GObject *self);

static void ngi_device_interface_handle_test_signal(NgiDeviceInterface *interface);



static GObjectClass *parent_class = NULL;
static guint device_interface_signals[LAST_SIGNAL] = { 0 };


GType
ngi_device_interface_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (NgiDeviceInterfaceClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      (GClassInitFunc)ngi_device_interface_class_init,
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (NgiDeviceInterface),
      0,      /* n_preallocs */
      (GInstanceInitFunc)ngi_device_interface_init,
    };
    type = g_type_register_static(G_TYPE_OBJECT, "NgiDeviceInterfaceType",
				  &info, 0);
  }
  return type;
}


static void
ngi_device_interface_class_init(NgiDeviceInterfaceClass *klass)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *pspec           = NULL;

  gobject_class = G_OBJECT_CLASS(klass);


  parent_class = g_type_class_peek_parent(klass);

  gobject_class->constructor = ngi_device_interface_constructor;
  gobject_class->dispose = ngi_device_interface_dispose;
  gobject_class->set_property = ngi_device_interface_set_property;
  gobject_class->get_property = ngi_device_interface_get_property;

  klass->test_signal = ngi_device_interface_handle_test_signal;

  /* set up properties */
  pspec = g_param_spec_int("vendor-id",
			   "Ngi Device Interface Vendor ID Property",
			   "Set/Get Ngi Device Interface Vendor ID Property",
			   (gint)0,
			   (gint)G_MAXINT,
			   (gint)0,
			   G_PARAM_CONSTRUCT_ONLY|G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_VENDOR_ID, pspec);

  pspec = g_param_spec_int("product-id",
			   "Ngi Device Interface Product ID Property",
			   "Set/Get Ngi Device Interface Product ID Property",
			   (gint)0,
			   (gint)G_MAXINT,
			   (gint)0,
			   G_PARAM_CONSTRUCT_ONLY|G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_PRODUCT_ID, pspec);

  pspec = g_param_spec_string("display-name",
			   "Ngi Device Interface Display Name Property",
			   "Set/Get Ngi Device Interface Display Name Property",
			   "",
			   G_PARAM_CONSTRUCT_ONLY|G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_DISPLAY_NAME, pspec);

  pspec = g_param_spec_pointer("devices",
			       "List of device pointers",
			       "Set/Get the list of device pointers",
			       G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_DEVICES, pspec);

  /* set up signals */
  /**
   * NgiDeviceWorld::test-signal
   * @interface: the object that received the signal
   *
   * Emitted when something happens
   *
   */
  device_interface_signals[TEST_SIGNAL] =
    g_signal_new("test-signal",
		 G_OBJECT_CLASS_TYPE(gobject_class),
		 G_SIGNAL_RUN_FIRST,
		 G_STRUCT_OFFSET(NgiDeviceInterfaceClass, test_signal),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);


  g_type_class_add_private (klass, sizeof (NgiDeviceInterfacePrivate));
}


static void
ngi_device_interface_init (GTypeInstance   *instance,
			   gpointer         g_class)
{
  NgiDeviceInterface *self = (NgiDeviceInterface*)instance;
  (void)self;

  /* initialize all public and private members to reasonable default values. */
  /* If you need specific consruction properties to complete initialization,
   * delay initialization completion until the property is set.
   */
}

static GObject *
ngi_device_interface_constructor (GType                  type,
			      guint                  n_construct_properties,
			      GObjectConstructParam *construct_params)
{
  GObject *object;
  NgiDeviceInterface *interface;

  /* chain up with parent class' constructor */
  object = (*G_OBJECT_CLASS(parent_class)->constructor)(type,
							n_construct_properties,
							construct_params);

  /* here you can do stuff based on arguments which were passed to g_object_new(...) */
  interface = NGI_DEVICE_INTERFACE(object);
  /*
  GoIO_Init();
  printf ("Init GoIO\n");
  */
  g_timeout_add (1000, ngi_device_interface_search, interface);

  return object;
}

static void
ngi_device_interface_dispose (GObject *self)
{
  static gboolean first_run = TRUE;

  if (first_run)
    {
      first_run = FALSE;
      /*
      printf ("Uninit GoIO\n");
      GoIO_Uninit ();
      */
      parent_class->dispose (self);
    }
}

static void
ngi_device_interface_set_property(GObject *object,
				  guint   property_id,
				  const GValue *value,
				  GParamSpec  *pspec)
{
  NgiDeviceInterface *self = (NgiDeviceInterface*)object;
  NgiDeviceInterfacePrivate *priv = NGI_DEVICE_INTERFACE_GET_PRIVATE(self);

  switch(property_id) {
  case PROP_VENDOR_ID:
    priv->vendor_id = g_value_get_int (value);
    break;
  case PROP_PRODUCT_ID:
    priv->product_id = g_value_get_int (value);
    break;
  case PROP_DISPLAY_NAME:
    priv->display_name = g_value_dup_string (value);
    break;
  case PROP_DEVICES:
    /* jen-to-do */
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void
ngi_device_interface_get_property(GObject    *object,
				  guint      property_id,
				  GValue     *value,
				  GParamSpec *pspec)
{
  NgiDeviceInterface *self = (NgiDeviceInterface*)object;
  NgiDeviceInterfacePrivate *priv = NGI_DEVICE_INTERFACE_GET_PRIVATE(self);

  switch(property_id) {
  case PROP_VENDOR_ID:
    g_value_set_int(value, priv->vendor_id);
    break;
  case PROP_PRODUCT_ID:
    g_value_set_int(value, priv->product_id);
    break;
  case PROP_DISPLAY_NAME:
    g_value_set_string (value, priv->display_name);
    break;
  case PROP_DEVICES:
    g_value_set_pointer (value, priv->devices);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}


static void
ngi_device_interface_handle_test_signal(NgiDeviceInterface *interface)
{
  g_print("ngi_device_interface_handle_test_signal()\n");
}

GObject*
ngi_device_interface_new(gint vendor_id, gint product_id, const gchar *display_name)
{
  return g_object_new(NGI_DEVICE_INTERFACE_TYPE,
		      "vendor-id", vendor_id,
		      "product-id", product_id,
		      "display-name", display_name,
		      NULL);
}

void
ngi_device_interface_add_device (NgiDeviceInterface *interface, const gchar* name)
{
  NgiDeviceInterfacePrivate *priv = NGI_DEVICE_INTERFACE_GET_PRIVATE(interface);
  NgiDevice *device = (NgiDevice*) ngi_device_new ((GObject *)interface, name, name, NULL);
  priv->devices = g_list_append (priv->devices, device);
}

void
ngi_device_interface_remove_device (NgiDeviceInterface *interface, const gchar* remove_name)
{
  NgiDeviceInterfacePrivate *priv = NGI_DEVICE_INTERFACE_GET_PRIVATE(interface);
  GList *device_iter = NULL;

  for (device_iter = priv->devices; device_iter; device_iter = g_list_next (device_iter))
    {
      const gchar* name = ngi_device_get_name (device_iter->data);
      if (!g_ascii_strcasecmp(name, remove_name))
	{
	  priv->devices = g_list_remove (priv->devices, device_iter->data);
	  break;
	}
    }

}

gint
ngi_device_interface_search (gpointer data)
{
  NgiDeviceInterface *interface = (NgiDeviceInterface*)data;
  NgiDeviceInterfacePrivate *priv = NGI_DEVICE_INTERFACE_GET_PRIVATE(interface);
  GSList *add_device_names = NULL;
  GSList *current_device_names = NULL;
  GSList *remove_device_names = NULL;
  GSList* name_iter = NULL;
  GList* device_iter = NULL;
  gint nDevices;
  gint ix;

  nDevices = GoIO_UpdateListOfAvailableDevices(priv->vendor_id,
					       priv->product_id);
  g_printf ("Searching for devices: VendorID: %d: ProductID: %d Found: %d\n",VERNIER_DEFAULT_VENDOR_ID,priv->product_id,nDevices);

  for (ix=0; ix<nDevices; ix++)
    {
      gchar *name = g_malloc (256); /*jenhack get the 256 another way.*/
      GoIO_GetNthAvailableDeviceName(name, 256, priv->vendor_id,
				     priv->product_id, ix);
      add_device_names = g_slist_append (add_device_names, name);
      g_printf ("Found a device of name: %s\n",name);
    }

  for (device_iter = priv->devices; device_iter; device_iter = g_list_next (device_iter))
    {
      const gchar* name = ngi_device_get_name (device_iter->data);
      GSList* found = g_slist_find_custom (add_device_names,
					   name,
					   (GCompareFunc)g_ascii_strcasecmp);

      if (found)
      	{
	  NgiDevice *device = device_iter->data;
	  gboolean connected = ngi_device_connected (device);

	  g_printf("Found device, checking it.\n");
	  
	  if (connected)
	    ngi_device_check_sensor (device);

	  add_device_names = g_slist_remove (add_device_names, found->data);

	}
      else
      	{
	  remove_device_names = g_slist_append (remove_device_names, (gchar*)name);
	}
    }

  for (name_iter = add_device_names; name_iter; name_iter = g_slist_next (name_iter))
    {
      g_printf ("Trying to add a new device\n");
      ngi_device_interface_add_device (interface,name_iter->data);
    }

  for (name_iter = remove_device_names; name_iter; name_iter = g_slist_next (name_iter))
    {
      g_printf ("Trying to remove a device\n");
      ngi_device_interface_remove_device (interface, name_iter->data);
    }

  return TRUE;
}

void
ngi_device_interface_connect_device (NgiDeviceInterface *interface, NgiDevice *device)
{
  NgiDeviceInterfacePrivate *priv = NGI_DEVICE_INTERFACE_GET_PRIVATE(interface);
  const gchar *device_name = ngi_device_get_name (device);
  g_printf ("Trying to connect to device Name: %s VendorID: %d ProductID:%d\n",device_name,priv->vendor_id,priv->product_id);
  void *device_handle = GoIO_Sensor_Open(device_name,
					 priv->vendor_id,
					 priv->product_id,
					 0);
  if (device_handle)
    g_printf ("Device Connected Successfully\n");
  else
    g_printf ("Device Connected Had issues\n");
  ngi_device_connect_device_handle (device, device_handle);
}

void
ngi_device_interface_disconnect_device (NgiDeviceInterface *interface, NgiDevice *device)
{
  NgiDeviceInterfacePrivate *priv = NGI_DEVICE_INTERFACE_GET_PRIVATE(interface);
  void *device_handle = ngi_device_get_device_handle (device);
  GoIO_Sensor_Close (device_handle);
  ngi_device_disconnect_device_handle (device);
}

gint
ngi_device_interface_query_sensor_id (NgiDeviceInterface *interface, NgiDevice *device)
{
	gint sensor_id = 0;
  void *device_handle = ngi_device_get_device_handle (device);

	  if (device_handle)
	    {
	      guchar sensor_number;
	      gint err;

	      g_printf ("GOIO_sensor_DDSMEM\n");
	      err = GoIO_Sensor_DDSMem_GetSensorNumber(device_handle,
						       &sensor_number,
						       0, 0);
	      sensor_id = (gint) sensor_number;
	      g_printf ("GOIO_sensor_DDSMEM succeeded.  Got Sensor Number: %d Err:%d\n",
			sensor_id,
			err);


		}

	return sensor_id;
}

GSensorDDSRec *
ngi_device_interface_get_dds (NgiDeviceInterface *interface, NgiDevice *device)
{
  void *device_handle = ngi_device_get_device_handle (device);
  GSensorDDSRec *dds = NULL;

  if (device_handle)
    {
	  dds = g_malloc (sizeof(GSensorDDSRec));
	  if (!GoIO_Sensor_DDSMem_GetRecord (device_handle, dds))
      g_printf ("Got DDS record.......\n");
    }
    return dds;
}

gboolean
ngi_device_interface_start_collection (NgiDeviceInterface *interface, NgiDevice *device)
{
  gboolean success = FALSE;
  void *device_handle = ngi_device_get_device_handle (device);

  if (device_handle)
    {
      gint result;

      GoIO_Sensor_ClearIO(device_handle);
      result = GoIO_Sensor_SendCmdAndGetResponse(device_handle,
						 SKIP_CMD_ID_START_MEASUREMENTS, 
						 NULL, 0, NULL, NULL, SKIP_TIMEOUT_MS_DEFAULT);
      if (result == 0)
	success = TRUE;
    }
  return success;
}


gboolean
ngi_device_interface_stop_collection (NgiDeviceInterface *interface, NgiDevice *device)
{
  gboolean success = FALSE;
  void *device_handle = ngi_device_get_device_handle (device);

  if (device_handle)
    {
      gint result;

      result = GoIO_Sensor_SendCmdAndGetResponse(device_handle,
						 SKIP_CMD_ID_STOP_MEASUREMENTS, 
						 NULL, 0, NULL, NULL, SKIP_TIMEOUT_MS_DEFAULT);
      if (result == 0)
	success = TRUE;
    }
  return success;
}

GArray *
ngi_device_interface_get_measurements (NgiDeviceInterface *interface, NgiDevice *device)
{
  GArray *result = NULL;
  void *device_handle = ngi_device_get_device_handle (device);

  if (device_handle)
    { 
      gint available = GoIO_Sensor_GetNumMeasurementsAvailable(device_handle);

      if (available > 0)
	{
	  gint measurements[available];
	  available = GoIO_Sensor_ReadRawMeasurements(device_handle, 
						      measurements, 
						      available);
      
	  if (available > 0)
	    {
	      gint ix;
	      result = g_array_new (FALSE, FALSE, sizeof (gdouble));

	      for (ix=0; ix<available; ix++)
		{
		  gdouble rVolts = GoIO_Sensor_ConvertToVoltage(device_handle, 
								measurements[ix]);
		  gdouble rValue = GoIO_Sensor_CalibrateData(device_handle, 
							     rVolts);
		  g_array_append_val (result, rValue);
		}

	    }
	}
    }
  return result;
}
