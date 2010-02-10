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

typedef enum _Jabber_Option Jabber_Option;
enum _Jabber_Option {
  JABBER_USETLS = (1<<0),
  JABBER_PLAIN = (1<<1),
  JABBER_SASL = (1<<2),
  JABBER_ANON = (1<<3),
  JABBER_LOG = (1<<4)
};

typedef enum _Jabber_State Jabber_State;
enum _Jabber_State {
  JABBER_DISCONNECTED = 0x0,
  JABBER_CONNECTING = 0x1,
  JABBER_CONNECTED = 0x2,
};

typedef enum _Jabber_Show Jabber_Show;
enum _Jabber_Show {
  JABBER_OFFLINE = 0x0, /* Disconnected */
  JABBER_ONLINE = 0x1,  /* Connected. Status: managed automatically */
  /* Jabber Show */
  JABBER_CHAT =0x2,     /* Connected. Status: chat */
  JABBER_AWAY = 0x3,    /* Connected. Status: away */
  JABBER_XA = 0x4,      /* Connected. Status: extended away */
  JABBER_DND = 0x5,     /* Connected. Status: do not disturb */
  JABBER_UNAVAILABLE = 0x6,
  JABBER_AVAILABLE = 0x7,
  JABBER_UNDEFINED = 0x9
};

typedef void(*Jabber_Callback)(void *data, Jabber_Session *sess, const void *event_info);

char jabber_hastls();
Jabber_Session *jabber_new();
void jabber_del(Jabber_Session *sess);
int jabber_config(Jabber_Session *sess, const char *jidres, const char *passwd, const char *server, int port, Jabber_Option opts);
void jabber_state_callback_set(Jabber_Session *sess, Jabber_Callback func, const void *data);
void jabber_error_callback_set(Jabber_Session *sess, Jabber_Callback func, const void *data);
int jabber_connect(Jabber_Session *sess);
int jabber_disconnect(Jabber_Session *sess);
Jabber_State jabber_state(Jabber_Session *sess);
int jabber_status_set(Jabber_Session *sess, Jabber_Show show, const char *desc);
int jabber_chat_send(Jabber_Session *sess, const char *to, const char *body);
int jabber_iks_send(Jabber_Session *sess, const void *x);

/* Roster handling */

typedef enum _Jabber_Subscript Jabber_Subscript;
enum _Jabber_Subscript {
  JABBER_SUBSCRIPT_NONE = 0,
  JABBER_SUBSCRIPT_TO = 1,
  JABBER_SUBSCRIPT_FROM = 2,
  JABBER_SUBSCRIPT_BOTH = 3
};

void jabber_roster_callback_set(Jabber_Session *sess, Jabber_Callback func, const void *data);

int jabber_vcard_req(Jabber_Session *sess, const char *jid);

void jabber_chat_callback_set(Jabber_Session *sess, Jabber_Callback func, const void *data);

#endif
