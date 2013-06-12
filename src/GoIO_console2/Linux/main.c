#include <stdio.h>
#include <gtk/gtk.h>
#include <gtkextra/gtksheet.h>
#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtkextra/gtkplot.h>
#include <gtkextra/gtkplot3d.h>
#include <gtkextra/gtkplotdata.h>
#include <gtkextra/gtkplotcanvas.h>
#include <gtkextra/gtkplotps.h>
#include <gtkextra/gtkplotprint.h>
#include <gtkextra/gtkplotcanvastext.h>
#include <gtkextra/gtkplotcanvasline.h>
#include <gtkextra/gtkplotcanvasellipse.h>
#include <gtkextra/gtkplotcanvasrectangle.h>
#include <gtkextra/gtkplotcanvasplot.h>
#include <gtkextra/gtkplotcanvaspixmap.h>
#include "ngisensorworld.h"
#include "ngideviceinterface.h"


static  GtkWidget *plotwindow;
static  GtkWidget *active_plot;
static  GtkWidget *start_button;
static  GtkWidget *stop_button;
static  GtkWidget *readout;
static  GtkWidget *readout_unit;
static  GtkPlotData *dataset;
static  GtkWidget *canvas;
static  GtkPlotCanvasChild *child;
static  GArray *sensor_data = NULL;

static void     
quit_action ()
{
  gtk_main_quit();
}



gdouble 
function(GtkPlot *plot, 
	 GtkPlotData *data, 
	 gdouble x, 
	 gboolean *err)
{
  gdouble y=0;
  /*
  *err = FALSE;
  y = (.5+.3*sin(3.*x)*sin(50.*x));
  */
  if (sensor_data && sensor_data->len > x)
    y = (gdouble) g_array_index (sensor_data, gdouble, (gint)x);
  return y;
}

void 
build_example1(GtkWidget *plot)
{
  GdkColor color;
  GtkPlotAxis *axis;
  
  dataset = gtk_plot_add_function(GTK_PLOT(plot), (GtkPlotFunc)function);
  
  gtk_widget_show(GTK_WIDGET(dataset));
 
  gdk_color_parse("red", &color);
  gdk_color_alloc(gdk_colormap_get_system(), &color); 
  
  gtk_plot_data_set_symbol(dataset,
			   GTK_PLOT_SYMBOL_DIAMOND,
			   GTK_PLOT_SYMBOL_EMPTY,
			   10, 2, &color, &color);
  gtk_plot_data_set_line_attributes(dataset,
				    GTK_PLOT_LINE_SOLID,
				    0, 0, 1, &color);

}


static void
goio_console_stop_collection (GtkWidget *button, GObject *device)
{
  printf ("Stop collecting on %s\n",ngi_device_get_name(NGI_DEVICE(device)));
  ngi_device_stop_collection (NGI_DEVICE (device));

  gtk_widget_set_sensitive (start_button, TRUE);
  gtk_widget_set_sensitive (stop_button, FALSE);

  g_signal_handlers_disconnect_by_func (G_OBJECT (stop_button), 
		    G_CALLBACK (goio_console_stop_collection),
		    device);

}
static void
goio_console_update_readout (GObject *sensor, gdouble data)
{
  gchar latest[256];
  if (!sensor_data)
    sensor_data = g_array_new (FALSE,FALSE,sizeof(gdouble));

  if (sensor_data)
    g_array_append_val (sensor_data, data);

  g_sprintf (latest, "%f", data);
  gtk_label_set_label (GTK_LABEL (readout), latest);
  
}

static gint
goio_console_update_graph (gpointer device)
{
  double secs =60;
  if (sensor_data)
    secs = sensor_data->len+60;
  gtk_plot_set_xrange (GTK_PLOT (active_plot), 0.0, secs);

  if (ngi_device_collecting (NGI_DEVICE (device)))
    return TRUE;
  else
    return FALSE;
}

static void
goio_console_start_collection (GtkWidget *button, GObject *device)
{
  NgiSensor *sensor = ngi_device_get_sensor (NGI_DEVICE (device));
  printf ("Start collecting on %s\n",ngi_device_get_name(NGI_DEVICE(device)));
  ngi_device_start_collection (NGI_DEVICE (device));

  gtk_widget_set_sensitive (start_button, FALSE);
  gtk_widget_set_sensitive (stop_button, TRUE);

  if (sensor_data)
    {
      g_array_free (sensor_data, TRUE);
      sensor_data = NULL;
    }

  g_signal_connect (G_OBJECT (stop_button), "clicked",
		    G_CALLBACK (goio_console_stop_collection),
		    device);
  g_signal_connect (G_OBJECT (sensor), "data-changed",
		    G_CALLBACK (goio_console_update_readout),
		    NULL);


  gtk_plot_set_yrange (GTK_PLOT (active_plot), 0.0, 60.0);
  gtk_plot_set_xrange (GTK_PLOT (active_plot), 0.0, 60.0);
  g_timeout_add (60000, goio_console_update_graph, device);

}

static void 
goio_console_device_connected (GObject *device)
{
  NgiSensor *sensor = ngi_device_get_sensor (NGI_DEVICE (device));

  if (sensor)
    {
      const gchar *units =  ngi_sensor_get_display_units (sensor);
      gtk_widget_set_sensitive (start_button, TRUE);
      gtk_widget_set_sensitive (stop_button, FALSE);

      g_signal_connect (G_OBJECT (start_button), "clicked",
			G_CALLBACK (goio_console_start_collection),
			device);

      g_printf ("Displayed units: %d\n",units);
      gtk_label_set_label (GTK_LABEL (readout_unit),  units );
      gtk_widget_set_sensitive (readout_unit, TRUE);
			   
      /* set up titles */
      gtk_plot_axis_set_title (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_LEFT),
			       ngi_sensor_get_display_name (sensor));

      gtk_plot_axis_set_title (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_BOTTOM),
			       "Time (s)");
      gtk_plot_remove_data (GTK_PLOT (active_plot), dataset);
    }
}

static void
goio_console_device_disconnected (GObject *device)
{
  gtk_widget_set_sensitive (start_button, FALSE);
  gtk_widget_set_sensitive (stop_button, FALSE);

  g_signal_handlers_disconnect_by_func (G_OBJECT (start_button), 
		    G_CALLBACK (goio_console_start_collection),
		    device);
  g_signal_handlers_disconnect_by_func (G_OBJECT (stop_button), 
		    G_CALLBACK (goio_console_stop_collection),
		    device);

  gtk_label_set_label (GTK_LABEL (readout_unit), "()"); 
  gtk_widget_set_sensitive (readout_unit, FALSE);

 /* set up titles */
  gtk_plot_axis_set_title (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_LEFT),
			   "Y");
  gtk_plot_axis_set_title (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_BOTTOM),
			   "X");
}

static void
goio_console_device_selected (GtkWidget *menu_item, GObject *device)
{
  //jenhack check your types and actually do something.

  if (ngi_device_connected (NGI_DEVICE (device)))
    {
      goio_console_device_disconnected (device);
      ngi_device_disconnect (NGI_DEVICE (device));
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), FALSE);
    }
  else
    {
      ngi_device_connect (NGI_DEVICE (device));
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
      goio_console_device_connected (device);
    }
}


static gboolean 
goio_console_populate_device_menu (GtkWidget *menu, GObject *interface)
{
  GtkWidget *sub_menu;
  GtkWidget *menu_item;
  GList *devices = NULL;
  GList *iter;
  gboolean bNone = TRUE;
  GValue gvalpointer = {G_TYPE_INVALID};
  g_value_init (&gvalpointer, G_TYPE_POINTER); 
  g_object_get_property (interface, "devices", &gvalpointer);
  devices = (GList*) g_value_get_pointer (&gvalpointer);
  
  g_return_val_if_fail (menu != NULL, FALSE);
  g_return_val_if_fail (GTK_MENU_ITEM (menu), FALSE);

    
  sub_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu));
  
  GList *current = gtk_container_get_children (GTK_CONTAINER (sub_menu));
  GList *iterc;

  for (iterc = current; iterc; iterc = g_list_next (iterc))
    {
      gtk_container_remove (GTK_CONTAINER (sub_menu),GTK_WIDGET (iterc->data));
    }

  for (iter = devices; iter; iter = g_list_next (iter))
    {
      GValue gval = {G_TYPE_INVALID};

      g_value_init (&gval, G_TYPE_STRING);  
      g_object_get_property (iter->data, "display-name", &gval);      
      
      bNone = FALSE;
      menu_item = gtk_check_menu_item_new_with_label (g_value_get_string (&gval));

      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), ngi_device_connected (NGI_DEVICE (iter->data)));
      
      gtk_menu_shell_append (GTK_MENU_SHELL (sub_menu), menu_item);
      g_signal_connect (G_OBJECT (menu_item), "activate",
			G_CALLBACK (goio_console_device_selected),
			iter->data);
      gtk_widget_show (menu_item);
      
    }

  if (bNone)
    {
      menu_item = gtk_menu_item_new_with_label ("none");
      gtk_menu_shell_append (GTK_MENU_SHELL (sub_menu), menu_item);
      gtk_widget_show (menu_item);      
    }
  
  return TRUE;
}


static gboolean 
goio_console_update_device_menu (GtkWidget *menu)
{
  GtkWidget *sub_menu;
  GtkWidget *menu_item;
  GSList *device_names = NULL;//ngi_sensor_world_get_available_device_names(NGI_SENSOR_WORLD (sensor_world));
  GSList *iter;
  
  g_return_val_if_fail (menu != NULL, FALSE);
  g_return_val_if_fail (GTK_MENU_ITEM (menu), FALSE);

  
  sub_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu));
  
  if (!device_names)
      device_names = g_slist_append(device_names, g_string_new ("None"));
  
  GList *current = gtk_container_get_children (GTK_CONTAINER (sub_menu));
  GList *iterc;

  for (iterc = current; iterc; iterc = g_list_next (iterc))
    {
      gtk_container_remove (GTK_CONTAINER (sub_menu),GTK_WIDGET (iterc->data));
    }

  for (iter = device_names; iter; iter = g_slist_next (iter))
    {
      GString *label_name = iter->data;
      menu_item = gtk_menu_item_new_with_label (label_name->str);
      gtk_menu_shell_append (GTK_MENU_SHELL (sub_menu), menu_item);
      g_signal_connect_swapped (G_OBJECT (menu_item), "activate",
				G_CALLBACK (goio_console_device_selected),
				menu_item);
      gtk_widget_show (menu_item);
      
    }
  
  return TRUE;
}

static gboolean 
delete( GtkWidget *widget,
	GtkWidget *event,
	gpointer   data )
{
  gtk_main_quit ();
  return FALSE;
}

void goio_console_add_plot ()
{
  gfloat scale = 1.;
  gint page_width, page_height;

  page_width = 400 * scale;
  page_height = 300 * scale;

  canvas = gtk_plot_canvas_new (page_width*1.2, page_height*1.2, 1.0);
  GTK_PLOT_CANVAS_SET_FLAGS (GTK_PLOT_CANVAS (canvas), GTK_PLOT_CANVAS_DND_FLAGS);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (plotwindow), canvas);
  gtk_widget_set_size_request (plotwindow, (page_width*1.25), (page_height*1.25));
  gtk_widget_show (canvas);
  
  active_plot = gtk_plot_new_with_size(NULL, .5, .25);
  gtk_widget_show (active_plot);
 
  gtk_plot_set_range (GTK_PLOT (active_plot), 0., 1., 0., 1.);

  gtk_plot_set_ticks (GTK_PLOT (active_plot), GTK_PLOT_AXIS_X, 1., 1);
  gtk_plot_set_ticks (GTK_PLOT (active_plot), GTK_PLOT_AXIS_Y, 1., 1);
  gtk_plot_axis_set_visible (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_TOP), 
			     FALSE);
  gtk_plot_axis_set_visible (gtk_plot_get_axis (GTK_PLOT(active_plot), 
						GTK_PLOT_AXIS_RIGHT), 
			    FALSE);
  gtk_plot_x0_set_visible (GTK_PLOT(active_plot), TRUE);
  gtk_plot_y0_set_visible (GTK_PLOT(active_plot), TRUE);
  gtk_plot_hide_legends (GTK_PLOT(active_plot));

  child = gtk_plot_canvas_plot_new (GTK_PLOT (active_plot));
  gtk_plot_canvas_put_child (GTK_PLOT_CANVAS (canvas), child, 0.15, 0.05, .9, .90);

  GTK_PLOT_CANVAS_PLOT (child)->flags |= GTK_PLOT_CANVAS_PLOT_SELECT_POINT;
  GTK_PLOT_CANVAS_PLOT (child)->flags |= GTK_PLOT_CANVAS_PLOT_DND_POINT;

  /* set up titles */
  gtk_plot_axis_set_title (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_LEFT),
			   "Y");
  gtk_plot_axis_set_title (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_BOTTOM),
			   "X");


  gtk_plot_axis_justify_title (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_LEFT),
			       GTK_JUSTIFY_CENTER);
  gtk_plot_axis_justify_title (gtk_plot_get_axis (GTK_PLOT (active_plot), GTK_PLOT_AXIS_BOTTOM), 
			       GTK_JUSTIFY_RIGHT);


  build_example1 (active_plot);

}

int 
main( int argc,
      char *argv[] )
{
  NgiSensorWorld *sensor_world;
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *layout;
  GtkWidget *data_graph;
  GtkWidget *label;
  GtkWidget *menu_box;
  GError    *err;
  GtkWidget *menubar;
  GtkWidget *topmenu;
  GtkWidget *submenu;
  GtkWidget *menuitem;

  gtk_init (&argc, &argv);
  gtk_rc_parse("ngi.rc");
    
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (G_OBJECT (window), "delete_event",
                    G_CALLBACK (delete), NULL);

  sensor_world = ngi_sensor_world_get ();

  
  gtk_container_set_border_width (GTK_CONTAINER (window), 1);
  
  layout = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), layout);

  menu_box = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (layout), menu_box, FALSE, FALSE, 0);

  menubar = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (menu_box), menubar, FALSE, TRUE, 0);

  topmenu = gtk_menu_item_new_with_label ("File");  
  gtk_menu_bar_append (GTK_MENU_BAR (menubar), topmenu);
  submenu = gtk_menu_new ();
  menuitem = gtk_menu_item_new_with_label ("Quit");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);

  g_signal_connect_swapped (G_OBJECT (menuitem), "activate",
			    G_CALLBACK (quit_action), NULL);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu), submenu);

  topmenu = gtk_menu_item_new_with_label ("Devices");
  gtk_menu_bar_append (GTK_MENU_BAR (menubar), topmenu);
  submenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu), submenu);

  GList *list = ngi_sensor_world_supported_device_interfaces (NGI_SENSOR_WORLD (sensor_world));
  GList *iter;


  for (iter = list; iter; iter=g_list_next (iter))
    {
      GObject *interface = G_OBJECT (iter->data);
      if (interface)
	{
	  GValue gval = {G_TYPE_INVALID};
	  g_value_init (&gval, G_TYPE_STRING);  
	  g_object_get_property (interface, "display-name", &gval);
	  menuitem = gtk_menu_item_new_with_label (g_value_get_string(&gval));
	  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);

      
	  GtkWidget *sub_menu = gtk_menu_new ();
	  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), sub_menu);
	  GtkWidget *none_item = gtk_menu_item_new_with_label ("None");
	  gtk_menu_shell_append (GTK_MENU_SHELL (sub_menu), none_item);

	  g_signal_connect (G_OBJECT (menuitem), "activate",
			    G_CALLBACK (goio_console_populate_device_menu),
			    interface);
      
	}
    }


  GtkWidget *measurments = gtk_frame_new ("Measurments");
  gtk_box_pack_start (GTK_BOX (layout), measurments, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (measurments), 5);

  gtk_widget_set_size_request (measurments, 400, 70);

  GtkWidget *hbox = gtk_hbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (measurments), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  start_button = gtk_button_new_with_label ("Start");
  gtk_box_pack_start (GTK_BOX (hbox), start_button, FALSE, TRUE, 0);
  gtk_widget_set_sensitive (start_button, FALSE);

  stop_button = gtk_button_new_with_label ("Stop");
  gtk_box_pack_start (GTK_BOX (hbox), stop_button, FALSE, TRUE, 0);
  gtk_widget_set_sensitive (stop_button, FALSE);

  GtkWidget *currentframe = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (hbox), currentframe, FALSE, TRUE, 0);
  
  GtkWidget *hlabelbox = gtk_hbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (currentframe), hlabelbox);

  readout = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (hlabelbox), readout, TRUE, TRUE, 0);
  gtk_widget_set_size_request (readout, 100, 50);

  readout_unit = gtk_label_new ("()");
  gtk_box_pack_start (GTK_BOX (hbox), readout_unit, FALSE, TRUE, 0);
  gtk_widget_set_size_request (readout_unit, 50, 50);
  gtk_widget_set_sensitive (readout_unit, FALSE);

  plotwindow=gtk_scrolled_window_new (NULL, NULL);
  gtk_container_border_width (GTK_CONTAINER (plotwindow), 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (plotwindow),
				GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
  
  gtk_box_pack_start (GTK_BOX (layout), plotwindow, TRUE, TRUE, 0);

  goio_console_add_plot ();
  
  gtk_widget_show_all (window);
  
  gtk_main ();
  g_object_unref (sensor_world);

  return 0;
}

