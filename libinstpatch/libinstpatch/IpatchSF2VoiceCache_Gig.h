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
 */
/**
 * SECTION: IpatchSF2VoiceCache_Gig
 * @short_description: Voice cache converters for GigaSampler object types
 * @see_also: #IpatchSF2VoiceCache
 * @stability: Stable
 */
#ifndef __IPATCH_SF2_VOICE_CACHE_GIG_H__
#define __IPATCH_SF2_VOICE_CACHE_GIG_H__

#include <glib.h>
#include <glib-object.h>
#include <libinstpatch/IpatchConverter.h>
#include <libinstpatch/IpatchConverterSF2VoiceCache.h>

typedef IpatchConverterSF2VoiceCache IpatchConverterGigInstToSF2VoiceCache;
typedef IpatchConverterSF2VoiceCacheClass IpatchConverterGigInstToSF2VoiceCacheClass;
typedef IpatchConverterSF2VoiceCache IpatchConverterGigSampleToSF2VoiceCache;
typedef IpatchConverterSF2VoiceCacheClass IpatchConverterGigSampleToSF2VoiceCacheClass;

#define IPATCH_TYPE_CONVERTER_GIG_INST_TO_SF2_VOICE_CACHE \
  (ipatch_converter_gig_inst_to_sf2_voice_cache_get_type ())
#define IPATCH_TYPE_CONVERTER_GIG_SAMPLE_TO_SF2_VOICE_CACHE \
  (ipatch_converter_gig_sample_to_sf2_voice_cache_get_type ())

GType ipatch_converter_gig_inst_to_sf2_voice_cache_get_type (void);
GType ipatch_converter_gig_sample_to_sf2_voice_cache_get_type (void);

#endif