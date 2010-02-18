/** config.h --- 
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef enum _Elm_Jabber_Option Elm_Jabber_Option;
enum _Elm_Jabber_Option {
  ELM_JABBER_HIDE_UNAVAILABLE = (1 << 0),
  ELM_JABBER_HIDE_SERVER_PART = (1 << 1),
  ELM_JABBER_HIDE_USER_PHOTOS = (1 << 2),
  ELM_JABBER_NONE = 0
};

Evas_Object *elm_jabber_config_add(Evas_Object *parent);
int elm_jabber_config_load(Jabber_Session *sess);
Elm_Jabber_Option elm_jabber_config_option();

#endif
