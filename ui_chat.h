/** ui_chat.h --- 
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

#ifndef __UI_CHAR_H__
#define __UI_CHAR_H__

Evas_Object *elm_jabber_chat_add(Evas_Object *parent);
void elm_jabber_chat_register(Evas_Object *parent, Jabber_Session *jabber);
void elm_jabber_chat_enter(Evas_Object *parent, const char *jid);
//Evas_Object *elm_jabber_chat_leave(Evas_Object *parent);

#endif
