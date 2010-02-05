/** jabber.h --- 
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

#ifndef __JABBER_H__
#define __JABBER_H__

#include<iksemel.h>

typedef struct _Jabber_Session Jabber_Session;

typedef enum _Jabber_Options Jabber_Options;
enum _Jabber_Options {
  JABBER_USETLS = (1<<0),
  JABBER_SASL = (1<<1),
  JABBER_PLAIN = (1<<2),
  JABBER_LOG = (1<<3)
};

typedef void(*Jabber_Callback)(void *data, Jabber_Session *sess, const void* event_info);

Jabber_Session *jabber_new();
void jabber_del(Jabber_Session *sess);
int jabber_config(Jabber_Session *sess, const char *jidres, const char *passwd, const char *server, int port, Jabber_Options opts);
void jabber_error_callback_set(Jabber_Session *sess, Jabber_Callback func, const void *data);
int jabber_connect(Jabber_Session *sess);
int jabber_disconnect(Jabber_Session *sess);

#endif
