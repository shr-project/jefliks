/** elm_genlist_ext.h --- 
 *
 * Copyright (C) 2010 PhoeniX11 Kayo
 *
 * Author: PhoeniX11 Kayo <phoenix11@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __ELM_GENLIST_EXT__
#define __ELM_GENLIST_EXT__

EAPI Elm_Genlist_Item *
elm_genlist_item_first_subitem_get(const Elm_Genlist_Item *it);
EAPI Elm_Genlist_Item *
elm_genlist_subitem_next_get(const Elm_Genlist_Item *it);
EAPI Elm_Genlist_Item *
elm_genlist_subitem_prev_get(const Elm_Genlist_Item *it);

EAPI void
elm_genlist_autoexpcon_mode_set(Evas_Object *obj, Elm_List_Mode mode);

#endif
