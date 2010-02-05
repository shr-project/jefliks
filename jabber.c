/** jabber.c --- 
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

#include"jabber.h"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<iksemel.h>

#ifndef _
#define _(...) __VA_ARGS__
#endif

#define ERROR_BUFFER 2048

#define set_error(...) if(sess->error.func){			\
    char *error = iks_malloc(ERROR_BUFFER);			\
    sprintf(error, __VA_ARGS__);				\
    sess->error.func((void*)sess->error.data, sess, error);	\
    iks_free(error);						\
  }

typedef struct _Jabber_Callback_Object Jabber_Callback_Object;
struct _Jabber_Callback_Object {
  Jabber_Callback func;
  const void *data;
};

/* stuff we keep per session */
struct _Jabber_Session {
  iksparser *prs;
  iksid *acc;
  char *server;
  int port;
  char *passwd;
  Jabber_Options options;
  int features;
  int authorized;
  int counter;
  int job_done;
  /* precious roster we'll deal with */
  iks *roster;
  /* out packet filter */
  iksfilter *filter;
  /* error message */
  Jabber_Callback_Object error;
};

/* connection time outs if nothing comes for this much seconds */
static const int opt_timeout = 30;


static int
on_result (Jabber_Session *sess, ikspak *pak)
{
  iks *x;
  
  //if (sess->set_roster == 0) {
  x = iks_make_iq (IKS_TYPE_GET, IKS_NS_ROSTER);
  iks_insert_attrib (x, "id", "roster");
  iks_send (sess->prs, x);
  iks_delete (x);
  /*} else {
    iks_insert_attrib (my_roster, "type", "set");
    iks_send (sess->prs, my_roster);
    }*/
  return IKS_FILTER_EAT;
}

static int
on_stream (Jabber_Session *sess, int type, iks *node) {
  sess->counter = opt_timeout;
  
  switch (type) {
  case IKS_NODE_START:
    if (sess->options & JABBER_USETLS && !iks_is_secure (sess->prs)) {
      iks_start_tls (sess->prs);
      break;
    }
    if (!(sess->options & JABBER_SASL)) {
      iks *x;
      char *sid = NULL;

      if (!(sess->options & JABBER_PLAIN)) sid = iks_find_attrib (node, "id");
      x = iks_make_auth (sess->acc, sess->passwd, sid);
      iks_insert_attrib (x, "id", "auth");
      iks_send (sess->prs, x);
      iks_delete (x);
    }
    break;

  case IKS_NODE_NORMAL:
    if (strcmp ("stream:features", iks_name (node)) == 0) {
      sess->features = iks_stream_features (node);
      if (sess->options & JABBER_SASL) {
	if (sess->options & JABBER_USETLS && !iks_is_secure (sess->prs)) break;
	if (sess->authorized) {
	  iks *t;
	  if (sess->features & IKS_STREAM_BIND) {
	    t = iks_make_resource_bind (sess->acc);
	    iks_send (sess->prs, t);
	    iks_delete (t);
	  }
	  if (sess->features & IKS_STREAM_SESSION) {
	    t = iks_make_session ();
	    iks_insert_attrib (t, "id", "auth");
	    iks_send (sess->prs, t);
	    iks_delete (t);
	  }
	} else {
	  if (sess->features & IKS_STREAM_SASL_MD5)
	    iks_start_sasl (sess->prs, IKS_SASL_DIGEST_MD5, sess->acc->user, sess->passwd);
	  else if (sess->features & IKS_STREAM_SASL_PLAIN)
	    iks_start_sasl (sess->prs, IKS_SASL_PLAIN, sess->acc->user, sess->passwd);
	}
      }
    } else if (strcmp ("failure", iks_name (node)) == 0) {
      set_error(_("SASL authentication failed"));
    } else if (strcmp ("success", iks_name (node)) == 0) {
      sess->authorized = 1;
      iks_send_header (sess->prs, sess->acc->server);
    } else {
      ikspak *pak;
      
      pak = iks_packet (node);
      iks_filter_packet (sess->filter, pak);
      if (sess->job_done == 1) return IKS_HOOK;
    }
    break;

  case IKS_NODE_STOP:
    set_error(_("Server disconnected"));

  case IKS_NODE_ERROR:
    set_error(_("Stream error"));
  }
  
  if (node) iks_delete (node);
  return IKS_OK;
}

static int
on_error (Jabber_Session *sess, ikspak *pak) {
  set_error(_("Authorization failed"));
  return IKS_FILTER_EAT;
}

static int
on_roster (Jabber_Session *sess, ikspak *pak) {
  sess->roster = pak->x;
  sess->job_done = 1;
  return IKS_FILTER_EAT;
}

static void
on_log (Jabber_Session *sess, const char *data, size_t size, int is_incoming) {
  if (iks_is_secure (sess->prs)) fprintf (stderr, "Sec");
  if (is_incoming) fprintf (stderr, "RECV"); else fprintf (stderr, "SEND");
  fprintf (stderr, "[%s]\n", data);
}

static void
setup_filter (Jabber_Session *sess) {
  if (sess->filter) iks_filter_delete(sess->filter);
  sess->filter = iks_filter_new();
  iks_filter_add_rule (sess->filter, (iksFilterHook *)on_result, sess,
		       IKS_RULE_TYPE, IKS_PAK_IQ,
		       IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
		       IKS_RULE_ID, "auth",
		       IKS_RULE_DONE);
  iks_filter_add_rule (sess->filter, (iksFilterHook *)on_error, sess,
		       IKS_RULE_TYPE, IKS_PAK_IQ,
		       IKS_RULE_SUBTYPE, IKS_TYPE_ERROR,
		       IKS_RULE_ID, "auth",
		       IKS_RULE_DONE);
  iks_filter_add_rule (sess->filter, (iksFilterHook *)on_roster, sess,
		       IKS_RULE_TYPE, IKS_PAK_IQ,
		       IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
		       IKS_RULE_ID, "roster",
		       IKS_RULE_DONE);
}

Jabber_Session *jabber_new(){
  Jabber_Session *sess;
  
  sess = iks_malloc(sizeof(Jabber_Session));
  
  sess->prs = iks_stream_new(IKS_NS_CLIENT, &sess, (iksStreamHook *)on_stream);
  sess->acc = NULL;
}

static void
session_clean(Jabber_Session *sess){
  if(sess->acc){
    iks_free(sess->acc);
    sess->acc = NULL;
  }
}

void jabber_del(Jabber_Session *sess){
  //session_clean(sess);
  
  if(sess->prs)iks_parser_delete(sess->prs);
  
  iks_free(sess);
}

int jabber_config(Jabber_Session *sess, const char *jidres, const char *passwd, const char *server, int port, Jabber_Options opts){
  sess->options = opts;
  
  if(opts&JABBER_LOG)iks_set_log_hook(sess->prs, (iksLogHook *)on_log);
  else iks_set_log_hook(sess->prs, NULL);
  
  sess->acc = iks_id_new(iks_parser_stack(sess->prs), jidres);

  if(server)sess->server=strdup(server);
  if(port)sess->port=port;
  if(passwd)sess->passwd=strdup(passwd);
  
  setup_filter(sess);
}

void jabber_error_callback_set(Jabber_Session *sess, Jabber_Callback func, const void *data){
  sess->error.func=func;
  sess->error.data=data;
}

int jabber_connect(Jabber_Session *sess){
  int e;
  
  e = iks_connect_tcp(sess->prs, sess->server, sess->port);
  switch (e) {
  case IKS_OK:
    break;
  case IKS_NET_NODNS:
    set_error(_("Hostname lookup failed"));
    return 0;
  case IKS_NET_NOCONN:
    set_error(_("Connection failed"));
    return 0;
  default:
    set_error(_("IO Error"));
    return 0;
  }
  
  sess->counter = opt_timeout;
  while (1) {
    e = iks_recv (sess->prs, 1);
    if (IKS_HOOK == e) return 1;
    if (IKS_NET_TLSFAIL == e) set_error(_("TLS handshake failed"));
    if (IKS_OK != e) set_error(_("IO Error"));
    sess->counter--;
    if (sess->counter == 0) set_error(_("Network timeout"));
  }
  
  return 0;
}

int jabber_disconnect(Jabber_Session *sess){
  iks_disconnect(sess->prs);
  return 1;
}
