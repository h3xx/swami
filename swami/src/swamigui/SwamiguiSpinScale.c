/*
 * SwamiguiSpinScale.c - A GtkSpinButton/GtkScale combo widget
 *
 * Swami
 * Copyright (C) 1999-2010 Joshua "Element" Green <jgreen@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA or point your web browser to http://www.gnu.org.
 */
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "SwamiguiSpinScale.h"
#include "i18n.h"
#include "util.h"

enum
{
  PROP_0,
  PROP_ADJUSTMENT,
  PROP_DIGITS,
  PROP_VALUE,
  PROP_SCALE_FIRST	/* the order of the widgets */
};


/* Local Prototypes */

static void swamigui_spin_scale_set_property (GObject *object, guint property_id,
					      const GValue *value,
					      GParamSpec *pspec);
static void swamigui_spin_scale_get_property (GObject *object, guint property_id,
					      GValue *value, GParamSpec *pspec);
static void swamigui_spin_scale_init (SwamiguiSpinScale *spin_scale);
static gboolean swamigui_spin_scale_real_set_order (SwamiguiSpinScale *spin_scale,
						    gboolean scale_first);


/* define the SwamiguiSpinScale type */
G_DEFINE_TYPE (SwamiguiSpinScale, swamigui_spin_scale, GTK_TYPE_HBOX);


static void
swamigui_spin_scale_class_init (SwamiguiSpinScaleClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->set_property = swamigui_spin_scale_set_property;
  obj_class->get_property = swamigui_spin_scale_get_property;

  g_object_class_install_property (obj_class, PROP_ADJUSTMENT,
	    g_param_spec_object ("adjustment", "Adjustment", "Adjustment",
				 GTK_TYPE_ADJUSTMENT, G_PARAM_READWRITE));
  g_object_class_install_property (obj_class, PROP_DIGITS,
	    g_param_spec_uint ("digits", "Digits", "Digits",
			       0, 20, 0, G_PARAM_READWRITE));
  g_object_class_install_property (obj_class, PROP_VALUE,
	    g_param_spec_double ("value", "Value", "Value",
				 -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				 G_PARAM_READWRITE));
  g_object_class_install_property (obj_class, PROP_SCALE_FIRST,
	    g_param_spec_boolean ("scale-first", "Scale first", "Scale first",
				  FALSE, G_PARAM_READWRITE));
}

static void
swamigui_spin_scale_set_property (GObject *object, guint property_id,
				  const GValue *value, GParamSpec *pspec)
{
  SwamiguiSpinScale *sc = SWAMIGUI_SPIN_SCALE (object);
  GtkAdjustment *adj;
  guint digits;
  double d;

  switch (property_id)
    {
    case PROP_ADJUSTMENT:
      adj = GTK_ADJUSTMENT (g_value_get_object (value));
      gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (sc->spinbtn), adj);
      gtk_range_set_adjustment (GTK_RANGE (sc->hscale), adj);
      break;
    case PROP_DIGITS:
      digits = g_value_get_uint (value);
      gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sc->spinbtn), digits);
      gtk_scale_set_digits (GTK_SCALE (sc->hscale), digits);
      break;
    case PROP_VALUE:
      d = g_value_get_double (value);
      adj = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (sc->spinbtn));
      gtk_adjustment_set_value (adj, d);
      break;
    case PROP_SCALE_FIRST:
      swamigui_spin_scale_real_set_order (sc, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
swamigui_spin_scale_get_property (GObject *object, guint property_id,
				  GValue *value, GParamSpec *pspec)
{
  SwamiguiSpinScale *sc = SWAMIGUI_SPIN_SCALE (object);

  switch (property_id)
    {
    case PROP_ADJUSTMENT:
      g_value_set_object (value, (GObject *)gtk_spin_button_get_adjustment
			  (GTK_SPIN_BUTTON (sc->spinbtn)));
      break;
    case PROP_DIGITS:
      g_value_set_uint (value, gtk_spin_button_get_digits
			(GTK_SPIN_BUTTON (sc->spinbtn)));
      break;
    case PROP_VALUE:
      g_value_set_double (value, gtk_spin_button_get_value
			(GTK_SPIN_BUTTON (sc->spinbtn)));
      break;
    case PROP_SCALE_FIRST:
      g_value_set_boolean (value, sc->scale_first);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
swamigui_spin_scale_init (SwamiguiSpinScale *spin_scale)
{
  GtkAdjustment *adj;

  spin_scale->scale_first = FALSE;

  adj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

  spin_scale->spinbtn = gtk_spin_button_new (adj, 1.0, 0);
  gtk_widget_show (spin_scale->spinbtn);
  gtk_box_pack_start (GTK_BOX (spin_scale), spin_scale->spinbtn, FALSE, FALSE, 0);

  spin_scale->hscale = gtk_hscale_new (adj);
  gtk_scale_set_draw_value (GTK_SCALE (spin_scale->hscale), FALSE);
  gtk_widget_show (spin_scale->hscale);
  gtk_box_pack_start (GTK_BOX (spin_scale), spin_scale->hscale, TRUE, TRUE, 0);
}

/**
 * swamigui_spin_scale_new:
 *
 * Create a new spin button/scale combo widget.
 *
 * Returns: New widget.
 */
GtkWidget *
swamigui_spin_scale_new (void)
{
  return (GTK_WIDGET (g_object_new (SWAMIGUI_TYPE_SPIN_SCALE, NULL)));
}

/**
 * swamigui_spin_scale_set_order:
 * @spin_scale: Spin scale widget
 * @scale_first: %TRUE if the GtkHScale should be before the GtkSpinButton,
 *   %FALSE otherwise.
 *
 * Sets the order that the horizontal scale and spin button widgets appear.
 */
void
swamigui_spin_scale_set_order (SwamiguiSpinScale *spin_scale,
			       gboolean scale_first)
{
  if (swamigui_spin_scale_real_set_order (spin_scale, scale_first))
    g_object_notify (G_OBJECT (spin_scale), "scale-first");
}

static gboolean
swamigui_spin_scale_real_set_order (SwamiguiSpinScale *spin_scale,
				    gboolean scale_first)
{
  g_return_val_if_fail (SWAMIGUI_IS_SPIN_SCALE (spin_scale), FALSE);

  scale_first = (scale_first != 0);	/* force booleanize it */

  if (spin_scale->scale_first == scale_first) return (FALSE);

  spin_scale->scale_first = scale_first;
  gtk_box_reorder_child (GTK_BOX (spin_scale), spin_scale->hscale, !scale_first);

  return (TRUE);
}