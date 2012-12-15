/*
 * libInstPatch
 * Copyright (C) 1999-2010 Joshua "Element" Green <jgreen@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Moderal Public License
 * as published by the Free Software Foundation; version 2.1
 * of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Moderal Public License for more details.
 *
 * You should have received a copy of the GNU Moderal Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA or on the web at http://www.gnu.org.
 */
/**
 * SECTION: IpatchSF2ModItem
 * @short_description: SoundFont modulator item interface
 * @see_also: 
 * @stability: Stable
 *
 * An interface type which is used by #IpatchSF2Preset, #IpatchSF2Inst,
 * #IpatchSF2PZone and #IpatchSF2IZone objects to add modulator realtime effect
 * functionality.
 */
#ifndef __IPATCH_SF2_MOD_ITEM_H__
#define __IPATCH_SF2_MOD_ITEM_H__

#include <glib.h>
#include <glib-object.h>

#include <libinstpatch/IpatchItem.h>
#include <libinstpatch/IpatchSF2Mod.h>

/* forward type declarations */

typedef IpatchItem IpatchSF2ModItem;
typedef struct _IpatchSF2ModItemIface IpatchSF2ModItemIface;

#define IPATCH_TYPE_SF2_MOD_ITEM   (ipatch_sf2_mod_item_get_type ())
#define IPATCH_SF2_MOD_ITEM(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), IPATCH_TYPE_SF2_MOD_ITEM, \
   IpatchSF2ModItem))
#define IPATCH_SF2_MOD_ITEM_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), IPATCH_TYPE_SF2_MOD_ITEM, \
   IpatchSF2ModItemIface))
#define IPATCH_IS_SF2_MOD_ITEM(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IPATCH_TYPE_SF2_MOD_ITEM))
#define IPATCH_SF2_MOD_ITEM_GET_IFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), IPATCH_TYPE_SF2_MOD_ITEM, \
   IpatchSF2ModItemIface))

/* modulator item interface */
struct _IpatchSF2ModItemIface
{
  GTypeInterface parent_iface;

  guint modlist_ofs;	/* offset in item instance to modulator list pointer */
  GParamSpec *mod_pspec;  /* cached value of modulator pspec for fast notifies */
};

/**
 * ipatch_sf2_mod_item_add_mod:
 * @item: Item that conforms to #IpatchSF2ModItem interface
 * @mod: Modulator to append to end of @item object's modulator list
 *
 * Just a convenience function which uses ipatch_sf2_mod_item_insert_mod() to
 * append a modulator.
 */
#define ipatch_sf2_mod_item_add(item, mod) \
  ipatch_sf2_mod_item_insert (item, mod, -1)

GType ipatch_sf2_mod_item_get_type (void);
GSList *ipatch_sf2_mod_item_get_mods (IpatchSF2ModItem *item);
void ipatch_sf2_mod_item_set_mods (IpatchSF2ModItem *item, GSList *mod_list,
				   int flags);
void ipatch_sf2_mod_item_insert (IpatchSF2ModItem *item,
				 const IpatchSF2Mod *mod, int pos);
void ipatch_sf2_mod_item_remove (IpatchSF2ModItem *item,
				 const IpatchSF2Mod *mod);
void ipatch_sf2_mod_item_change (IpatchSF2ModItem *item,
				 const IpatchSF2Mod *oldmod,
				 const IpatchSF2Mod *newmod);
guint ipatch_sf2_mod_item_count (IpatchSF2ModItem *item);

#endif