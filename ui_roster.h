/** ui_roster.h --- 
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

#ifndef __UI_ROSTER__
#define __UI_ROSTER__

Evas_Object *elm_jabber_roster_add(Evas_Object *parent);
int elm_jabber_roster_register(Evas_Object *roster, Jabber_Session *sess);
void elm_jabber_roster_clear(Evas_Object *roster);
const char *elm_jabber_roster_selected(Evas_Object *roster);
Evas_Object *elm_jabber_photo_add(Evas_Object *obj, const char *jid);

#endif
