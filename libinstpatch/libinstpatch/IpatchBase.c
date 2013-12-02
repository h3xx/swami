/*
 * libInstPatch
 * Copyright (C) 1999-2010 Joshua "Element" Green <jgreen@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1
 * of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA or on the web at http://www.gnu.org.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include "IpatchBase.h"
#include "IpatchConverter.h"
#include "IpatchParamProp.h"
#include "util.h"
#include "ipatch_priv.h"

enum {
  PROP_0,
  PROP_CHANGED,
  PROP_SAVED,
  PROP_FILENAME,
  PROP_FILE
};


static void ipatch_base_finalize (GObject *gobject);
static void ipatch_base_set_property (GObject *object, guint property_id,
				      const GValue *value, GParamSpec *pspec);
static void ipatch_base_get_property (GObject *object, guint property_id,
				      GValue *value, GParamSpec *pspec);
static void ipatch_base_real_set_file (IpatchBase *base, IpatchFile *file);
static void ipatch_base_real_set_file_name (IpatchBase *base,
					    const char *file_name);

/* private var used by IpatchItem, for fast "changed" property notifies */
GParamSpec *ipatch_base_pspec_changed;

/* cached parameter specs to speed up prop notifies */
static GParamSpec *file_pspec;
static GParamSpec *file_name_pspec;

G_DEFINE_ABSTRACT_TYPE (IpatchBase, ipatch_base, IPATCH_TYPE_CONTAINER);


static void
ipatch_base_class_init (IpatchBaseClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  IpatchItemClass *item_class = IPATCH_ITEM_CLASS (klass);

  item_class->item_set_property = ipatch_base_set_property;
  obj_class->get_property = ipatch_base_get_property;
  obj_class->finalize = ipatch_base_finalize;

  ipatch_base_pspec_changed =
    g_param_spec_boolean ("changed", "Changed", "Changed Flag",
			  TRUE, G_PARAM_READWRITE | IPATCH_PARAM_NO_SAVE_CHANGE
			  | IPATCH_PARAM_NO_SAVE);
  g_object_class_install_property (obj_class, PROP_CHANGED,
				   ipatch_base_pspec_changed);

  g_object_class_install_property (obj_class, PROP_SAVED,
		    g_param_spec_boolean ("saved", "Saved", "Been Saved Flag",
					  FALSE, G_PARAM_READWRITE
					  | IPATCH_PARAM_NO_SAVE_CHANGE
					  | IPATCH_PARAM_NO_SAVE));
  file_name_pspec = g_param_spec_string ("file-name", "File Name",
					 "File Name", "untitled",
					 G_PARAM_READWRITE
                                         | IPATCH_PARAM_NO_SAVE_CHANGE);
  g_object_class_install_property (obj_class, PROP_FILENAME, file_name_pspec);

  file_pspec = g_param_spec_object ("file", "File", "File Object",
				    IPATCH_TYPE_FILE,
				    G_PARAM_READWRITE | IPATCH_PARAM_NO_SAVE
				    | IPATCH_PARAM_HIDE
                                    | IPATCH_PARAM_NO_SAVE_CHANGE);
  g_object_class_install_property (obj_class, PROP_FILE, file_pspec);
}

static void
ipatch_base_init (IpatchBase *base)
{
}

/* function called when a patch is being destroyed */
static void
ipatch_base_finalize (GObject *gobject)
{
  IpatchBase *base = IPATCH_BASE (gobject);

  IPATCH_ITEM_WLOCK (base);

  if (base->file) ipatch_file_unref_from_object (base->file, gobject);  // -- unref file from object
  base->file = NULL;

  IPATCH_ITEM_WUNLOCK (base);

  if (G_OBJECT_CLASS (ipatch_base_parent_class)->finalize)
    G_OBJECT_CLASS (ipatch_base_parent_class)->finalize (gobject);
}

static void
ipatch_base_set_property (GObject *object, guint property_id,
			  const GValue *value, GParamSpec *pspec)
{
  IpatchBase *base = IPATCH_BASE (object);

  switch (property_id)
    {
    case PROP_CHANGED:
      if (g_value_get_boolean (value))
	ipatch_item_set_flags (IPATCH_ITEM (base), IPATCH_BASE_CHANGED);
      else ipatch_item_clear_flags (IPATCH_ITEM (base), IPATCH_BASE_CHANGED);
      break;
    case PROP_SAVED:
      if (g_value_get_boolean (value))
	ipatch_item_set_flags (IPATCH_ITEM (base), IPATCH_BASE_SAVED);
      else ipatch_item_clear_flags (IPATCH_ITEM (base), IPATCH_BASE_SAVED);
      break;
    case PROP_FILENAME:
      ipatch_base_real_set_file_name (base, g_value_get_string (value));
      break;
    case PROP_FILE:
      ipatch_base_real_set_file (base, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
ipatch_base_get_property (GObject *object, guint property_id,
			  GValue *value, GParamSpec *pspec)
{
  IpatchBase *base;

  g_return_if_fail (IPATCH_IS_BASE (object));
  base = IPATCH_BASE (object);

  switch (property_id)
    {
    case PROP_CHANGED:
      g_value_set_boolean (value,
			   ipatch_item_get_flags (IPATCH_ITEM (base))
			   & IPATCH_BASE_CHANGED);
      break;
    case PROP_SAVED:
      g_value_set_boolean (value,
			   ipatch_item_get_flags (IPATCH_ITEM (base))
			   & IPATCH_BASE_SAVED);
      break;
    case PROP_FILENAME:
      g_value_take_string (value, ipatch_base_get_file_name (base));
      break;
    case PROP_FILE:
      g_value_take_object (value, ipatch_base_get_file (base));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

/**
 * ipatch_base_set_file:
 * @base: Patch base object to set file object of
 * @file: File object
 *
 * Sets the file object associated with a patch.
 */
void
ipatch_base_set_file (IpatchBase *base, IpatchFile *file)
{
  GValue value = { 0 }, oldval = { 0 };

  g_return_if_fail (IPATCH_IS_BASE (base));
  g_return_if_fail (IPATCH_IS_FILE (file));

  g_value_init (&value, IPATCH_TYPE_FILE);
  g_value_set_object (&value, file);

  ipatch_item_get_property_fast ((IpatchItem *)base, file_pspec, &oldval);
  ipatch_base_real_set_file (base, file);
  ipatch_item_prop_notify ((IpatchItem *)base, file_pspec, &value, &oldval);

  g_value_unset (&value);
  g_value_unset (&oldval);
}

static void
ipatch_base_real_set_file (IpatchBase *base, IpatchFile *file)
{
  IpatchFile *oldfile;

  ipatch_file_ref_from_object (file, (GObject *)base);                          // ++ ref new file from object

  IPATCH_ITEM_WLOCK (base);
  oldfile = base->file;
  base->file = file;
  IPATCH_ITEM_WUNLOCK (base);

  if (oldfile) ipatch_file_unref_from_object (oldfile, (GObject *)base);        // -- remove reference to old file
}

/**
 * ipatch_base_get_file:
 * @base: Patch base object to get file object from
 *
 * Get the file object associated with a patch base object. Caller owns a
 * reference to the returned file object and it should be unrefed when
 * done with it.
 *
 * Returns: File object or %NULL if not set. Remember to unref it when done
 *   with it.
 */
IpatchFile *
ipatch_base_get_file (IpatchBase *base)
{
  IpatchFile *file;

  g_return_val_if_fail (IPATCH_IS_BASE (base), NULL);

  IPATCH_ITEM_RLOCK (base);
  file = base->file;
  if (file) g_object_ref (file);
  IPATCH_ITEM_RLOCK (base);

  return (file);
}

/**
 * ipatch_base_set_file_name:
 * @base: Patch base object to set file name of
 * @file_name: Path and name to set filename to
 *
 * Sets the file name of the file object in a patch base object. File object
 * should have been set before calling this function (otherwise request is
 * silently ignored). A convenience function as one could get the file object
 * and set it directly.
 */
void
ipatch_base_set_file_name (IpatchBase *base, const char *file_name)
{
  GValue value = { 0 }, oldval = { 0 };

  g_return_if_fail (IPATCH_IS_BASE (base));

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, file_name);

  ipatch_item_get_property_fast ((IpatchItem *)base, file_name_pspec, &oldval);
  ipatch_base_real_set_file_name (base, file_name);
  ipatch_item_prop_notify ((IpatchItem *)base, file_name_pspec, &value, &oldval);

  g_value_unset (&value);
  g_value_unset (&oldval);
}

/* the real set file name routine, user routine does a notify */
static void
ipatch_base_real_set_file_name (IpatchBase *base, const char *file_name)
{
  IPATCH_ITEM_RLOCK (base);
  if (!base->file)		/* silently fail */
    {
      IPATCH_ITEM_RUNLOCK (base);
      return;
    }
  ipatch_file_set_name (base->file, file_name);
  IPATCH_ITEM_WUNLOCK (base);
}

/**
 * ipatch_base_get_file_name:
 * @base: Base patch item to get file name from.
 *
 * Get the file name of the file object in a base patch item. A convenience
 * function.
 *
 * Returns: New allocated file name or %NULL if not set or file object
 *   not set. String should be freed when finished with it.
 */
char *
ipatch_base_get_file_name (IpatchBase *base)
{
  char *file_name = NULL;

  g_return_val_if_fail (IPATCH_IS_BASE (base), NULL);

  IPATCH_ITEM_RLOCK (base);
  if (base->file)
    file_name = ipatch_file_get_name (base->file);
  IPATCH_ITEM_RUNLOCK (base);

  return (file_name);
}

/**
 * ipatch_base_find_unused_midi_locale:
 * @base: Patch base object
 * @bank: MIDI bank number (input and output parameter)
 * @program: MIDI program number (input and output parameter)
 * @exclude: Child of @base with MIDI locale to exclude or %NULL
 * @percussion: Set to %TRUE to find a free percussion MIDI locale
 *
 * Finds an unused MIDI locale (bank:program number pair) in a patch
 * base object. The way in which MIDI bank and program numbers are
 * used is implementation dependent. Percussion instruments often
 * affect the bank parameter (for example SoundFont uses bank 128 for
 * percussion presets).  On input the @bank and @program parameters
 * set the initial locale to start from (set to 0:0 to find the first
 * free value). If the @percussion parameter is set it may affect
 * @bank, if its not set, bank will not be modified (e.g., if bank is
 * a percussion value it will be used). The exclude parameter can be
 * set to a child item of @base to exclude from the list of "used"
 * locales (useful when making an item unique that is already parented
 * to @base).  On return @bank and @program will be set to an unused
 * MIDI locale based on the input criteria.
 */
void
ipatch_base_find_unused_midi_locale (IpatchBase *base, int *bank,
				     int *program, const IpatchItem *exclude,
				     gboolean percussion)
{
  IpatchBaseClass *klass;

  g_return_if_fail (IPATCH_IS_BASE (base));
  g_return_if_fail (bank != NULL);
  g_return_if_fail (program != NULL);

  *bank = 0;
  *program = 0;

  klass = IPATCH_BASE_GET_CLASS (base);
  if (klass && klass->find_unused_locale)
    klass->find_unused_locale (base, bank, program, exclude, percussion);
}

/**
 * ipatch_base_find_item_by_midi_locale:
 * @base: Patch base object
 * @bank: MIDI bank number of item to search for
 * @program: MIDI program number of item to search for
 *
 * Find a child object in a base patch object which matches the given MIDI
 * locale (@bank and @program numbers).
 *
 * Returns: The item or %NULL if not found.  The caller owns a reference to the
 *   returned object, and is responsible for unref'ing when finished.
 */
IpatchItem *
ipatch_base_find_item_by_midi_locale (IpatchBase *base, int bank, int program)
{
  IpatchBaseClass *klass;

  g_return_val_if_fail (IPATCH_IS_BASE (base), NULL);

  klass = IPATCH_BASE_GET_CLASS (base);
  if (klass && klass->find_item_by_locale)
    return (klass->find_item_by_locale (base, bank, program));
  else return (NULL);
}

/**
 * ipatch_base_save:
 * @base: Base item to save
 * @filename: New file name to save to or %NULL to use current one
 * @err: Location to store error info or %NULL
 *
 * Save a patch item to a file.  This function handles saving over the existing
 * file and migrates sample stores as needed.  It is an error to try to save
 * over an open file that is not owned by @base though.
 * A new file object is created and assigned to the @base object.
 *
 * Returns: %TRUE on success, %FALSE otherwise (in which case @err may be set)
 */
gboolean
ipatch_base_save (IpatchBase *base, const char *filename, GError **err)
{
  IpatchConverterInfo *info;
  IpatchFile *lookup_file, *newfile = NULL, *oldfile = NULL;
  char *tmp_fname = NULL, *base_fname, *abs_fname = NULL, *bak_fname = NULL;
  IpatchConverter *converter;
  gboolean replacing = FALSE;
  GError *local_err = NULL;
  int tmpfd;

  g_return_val_if_fail (IPATCH_IS_BASE (base), FALSE);
  g_return_val_if_fail (!err || !*err, FALSE);

  g_object_get (base, "file", &oldfile, NULL);          // ++ ref old file (if any)

  /* Check if file name specified would overwrite another open file */
  if (filename)
  {
    abs_fname = ipatch_util_abs_filename (filename);    // ++ allocate absolute filename
    lookup_file = ipatch_file_pool_lookup (abs_fname);  // ++ ref file matching filename
    if (lookup_file) g_object_unref (lookup_file);      // -- unref file (we only need the pointer value)

    if (lookup_file && lookup_file != oldfile)
    {
      g_set_error (err, IPATCH_ERROR, IPATCH_ERROR_BUSY,
                   _("Refusing to save over other open file '%s'"), abs_fname);
      goto error;
    }
  }

  g_object_get (base, "file-name", &base_fname, NULL);       // ++ allocate patch file name

  /* if no filename specified try to use current one */
  if (!abs_fname)
  {
    if (!base_fname)
    {
      g_set_error (err, IPATCH_ERROR, IPATCH_ERROR_INVALID,
                   _("File name not supplied and none assigned"));
      goto error;
    }

    abs_fname = base_fname;
  }

  /* Check if request is to overwrite the existing file (requested file name and base
   * file name are equal and file actually exists). */
  replacing = strcmp (abs_fname, base_fname) == 0 && g_file_test (abs_fname, G_FILE_TEST_EXISTS);

  g_free (base_fname);       // -- free base filename

  /* Find a converter from base object to file */
  info = ipatch_lookup_converter_info (0, G_OBJECT_TYPE (base), IPATCH_TYPE_FILE);

  if (!info)
  {
    g_set_error (err, IPATCH_ERROR, IPATCH_ERROR_UNSUPPORTED,
                 _("Saving object of type '%s' to file '%s' not supported"),
                 g_type_name (G_OBJECT_TYPE (base)), abs_fname);
    goto error;
  }

  tmp_fname = g_build_filename (abs_fname, "_tmpXXXXXX", NULL);       // ++ alloc temporary file name

  // open temporary file in same directory as destination
  if ((tmpfd = g_mkstemp (tmp_fname)) == -1)
  {
    g_set_error (err, G_FILE_ERROR, g_file_error_from_errno (errno),
                 _("Unable to open temp file '%s' for writing: %s"),
                 tmp_fname, g_strerror (errno));
    goto error;
  }

  newfile = IPATCH_FILE (g_object_new (info->dest_type, "name", tmp_fname, NULL));       /* ++ ref new file */
  ipatch_file_assign_fd (newfile, tmpfd, TRUE);         /* Assign file descriptor and set close on finalize */

  // ++ Create new converter and set create-stores property if file is being replaced
  converter = IPATCH_CONVERTER (g_object_new (info->conv_type, "create-stores", replacing, NULL));

  ipatch_converter_add_input (converter, G_OBJECT (base));
  ipatch_converter_add_output (converter, G_OBJECT (newfile));

  /* attempt to save patch file */
  if (!ipatch_converter_convert (converter, err))
  {
    g_object_unref (converter);                 // -- unref converter
    goto error;
  }

  g_object_unref (converter);                   // -- unref converter

  ipatch_file_assign_fd (newfile, -1, FALSE);      // Unset file descriptor of file (now using file name), closes file descriptor

  // Sample migration is called whether or not replacing, so samples can be migrated out of SWAP
  if (!ipatch_migrate_file_sample_data (oldfile, newfile, IPATCH_SAMPLE_DATA_MIGRATE_REMOVE_NEW_IF_UNUSED
                                        | IPATCH_SAMPLE_DATA_MIGRATE_REPLACE, err))
    goto error;

  if (!replacing && !ipatch_file_rename (newfile, abs_fname, err))
    goto error;

  g_free (bak_fname);           // -- unref backup file name
  g_object_unref (newfile);	// -- unref creators reference
  g_free (tmp_fname);            // -- free temp file name
  g_free (abs_fname);        // -- free file name
  if (oldfile) g_object_unref (oldfile);        // -- unref old file

  return (TRUE);

error:

  if (newfile)
  {
    if (!ipatch_file_unlink (newfile, &local_err))   // -- Delete new file
    {
      g_warning (_("Failed to remove file after save failure: %s"),
                 ipatch_gerror_message (local_err));
      g_clear_error (&local_err);
    }

    g_object_unref (newfile);   // -- unref creators reference
  }

  if (bak_fname && !ipatch_file_rename (oldfile, abs_fname, &local_err))
  {
    g_critical (_("Failed to restore backup '%s' after save failure: %s"),
                bak_fname, ipatch_gerror_message (local_err));
    g_clear_error (&local_err);
  }

  g_free (bak_fname);           // -- unref backup file name

  g_free (tmp_fname);           // -- free temp file name
  g_free (abs_fname);           // -- free file name
  if (oldfile) g_object_unref (oldfile);        // -- unref old file

  return (FALSE);
}

