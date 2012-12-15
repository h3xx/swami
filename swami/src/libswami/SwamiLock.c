/*
 * SwamiLock.c - Base Swami multi-thread locked object
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
#include <glib.h>
#include <glib-object.h>

#include "SwamiLock.h"


/* --- private function prototypes --- */

static void swami_lock_init (SwamiLock *lock);


/* --- functions --- */


GType
swami_lock_get_type (void)
{
  static GType item_type = 0;

  if (!item_type) {
    static const GTypeInfo item_info = {
      sizeof (SwamiLockClass), NULL, NULL,
      (GClassInitFunc) NULL, NULL, NULL,
      sizeof (SwamiLock), 0,
      (GInstanceInitFunc) swami_lock_init,
    };

    item_type = g_type_register_static (G_TYPE_OBJECT, "SwamiLock",
					&item_info, G_TYPE_FLAG_ABSTRACT);
  }

  return (item_type);
}

static void
swami_lock_init (SwamiLock *lock)
{
  g_static_rec_mutex_init (&lock->mutex);
}

/**
 * swami_lock_set_atomic:
 * @lock: SwamiLock derived object to set properties of
 * @first_property_name: Name of first property
 * @Varargs: Variable list of arguments that should start with the value to
 * set @first_property_name to, followed by property name/value pairs. List is
 * terminated with a %NULL property name.
 *
 * Sets properties on a Swami lock item atomically (i.e. item is
 * multi-thread locked while all properties are set). This avoids
 * critical parameter sync problems when multiple threads are
 * accessing the same item. See g_object_set() for more information on
 * setting properties. This function is rarely needed, only useful for cases
 * where multiple properties depend on each other.
 */
void
swami_lock_set_atomic (gpointer lock, const char *first_property_name, ...)
{
  va_list args;

  g_return_if_fail (SWAMI_IS_LOCK (lock));

  va_start (args, first_property_name);

  SWAMI_LOCK_WRITE (lock);
  g_object_set_valist (G_OBJECT (lock), first_property_name, args);
  SWAMI_UNLOCK_WRITE (lock);

  va_end (args);
}

/**
 * swami_lock_get_atomic:
 * @lock: SwamiLock derived object to get properties from
 * @first_property_name: Name of first property
 * @Varargs: Variable list of arguments that should start with a
 * pointer to store the value from @first_property_name, followed by
 * property name/value pointer pairs. List is terminated with a %NULL
 * property name.
 *
 * Gets properties from a Swami lock item atomically (i.e. item is
 * multi-thread locked while all properties are retrieved). This
 * avoids critical parameter sync problems when multiple threads are
 * accessing the same item. See g_object_get() for more information on
 * getting properties. This function is rarely needed, only useful when
 * multiple properties depend on each other.
 */
void
swami_lock_get_atomic (gpointer lock, const char *first_property_name, ...)
{
  va_list args;

  g_return_if_fail (SWAMI_IS_LOCK (lock));

  va_start (args, first_property_name);

  SWAMI_LOCK_WRITE (lock);
  g_object_get_valist (G_OBJECT (lock), first_property_name, args);
  SWAMI_UNLOCK_WRITE (lock);

  va_end (args);
}