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
#include<stdarg.h>
#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<pthread.h>
#include<iksemel.h>

#ifndef _
#define _(...) __VA_ARGS__
#endif

#define ERROR_BUFFER 2048


#define set_error(...) set_error_ex(sess, __VA_ARGS__);
#define set_state(state) set_state_ex(sess, state);

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
  Jabber_Option option;
  int features;
  char authorized;
  int counter;
  /* coonnection thread */
  pthread_t thread;
  /* out packet filter */
  iksfilter *filter;
  /* callbacks */
  Jabber_Callback_Object error_cb;
  Jabber_Callback_Object state_cb;
  Jabber_Callback_Object roster_cb;
  Jabber_Callback_Object chat_cb;
  char state;
  /* presence */
  int priority;
  Jabber_Show show;
  char *show_desc;
};


static void
set_error_ex(Jabber_Session *sess, const char *format, ...){
  va_list args;
  char errbuf[ERROR_BUFFER];
  
  va_start(args, format);
  vsnprintf(errbuf, ERROR_BUFFER, format, args);
  va_end(args);
  
  if(sess->error_cb.func){
    sess->error_cb.func((void*)sess->error_cb.data, sess, errbuf);
  }else{
    fprintf(stderr, "%s\n", errbuf);
  }
}

static void
set_state_ex(Jabber_Session *sess, Jabber_State state){
  if(state!=sess->state){
    sess->state=state;
    if(state==JABBER_DISCONNECTED){
      jabber_disconnect(sess);
    }
    if(sess->state_cb.func){
      sess->state_cb.func((void*)sess->state_cb.data, sess, (void*)state);
    }
  }
}

/*
int jabber_send_message(Jabber_Session *sess, iks *x){
  return iks_send(sess->prs, x)==IKS_OK;
}
*/

static void
get_roster(Jabber_Session *sess){
  iks *x;
  x = iks_make_iq (IKS_TYPE_GET, IKS_NS_ROSTER);
  iks_insert_attrib(x, "id", "roster");
  iks_send(sess->prs, x);
  iks_delete (x);
}

static int
on_result (Jabber_Session *sess, ikspak *pak){
  get_roster(sess);
  
  return IKS_FILTER_EAT;
}

static void
set_presence(Jabber_Session *sess, char *to, Jabber_Show show, char *desc){
  int sw=0;
  
  switch(show){
  case JABBER_ONLINE: sw=IKS_SHOW_AVAILABLE; break;
  case JABBER_AVAILABLE: sw=IKS_SHOW_AVAILABLE; break;
  case JABBER_OFFLINE: sw=IKS_SHOW_UNAVAILABLE; break;
  case JABBER_UNAVAILABLE: sw=IKS_SHOW_UNAVAILABLE; break;
  case JABBER_CHAT: sw=IKS_SHOW_CHAT; break;
  case JABBER_AWAY: sw=IKS_SHOW_AWAY; break;
  case JABBER_XA: sw=IKS_SHOW_XA; break;
  case JABBER_DND: sw=IKS_SHOW_DND; break;
  default: break;
  }
  
  iks *presence = iks_make_pres(sw, desc?desc:"");
  char buf[10];
  
  if(to){
    iks_insert_attrib(presence, "to", to);
  }
  iks_insert_attrib(presence, "from", sess->acc->full);
  snprintf(buf, sizeof(buf), "%d", sess->priority);
  iks_insert_cdata(iks_insert(presence, "priority"), buf, strlen(buf));
  iks_send(sess->prs, presence);
  
  iks_delete(presence);
}

/* Roster Handling {{{ */

static int
on_roster (Jabber_Session *sess, ikspak *pak) {
  if(sess->roster_cb.func){
    sess->roster_cb.func((void*)sess->roster_cb.data, sess, (void*)pak);
  }
  
  set_state(JABBER_CONNECTED);
  set_presence(sess, NULL, sess->show, sess->show_desc);
  
  return IKS_FILTER_EAT;
}

static int
on_presence (Jabber_Session *sess, ikspak *pak) {
  if(sess->roster_cb.func){
    sess->roster_cb.func((void*)sess->roster_cb.data, sess, (void*)pak);
  }
  return IKS_FILTER_EAT;
}

static int
on_vcard (Jabber_Session *sess, ikspak *pak) {
  if(sess->roster_cb.func){
    sess->roster_cb.func((void*)sess->roster_cb.data, sess, (void*)pak);
  }
  return IKS_FILTER_EAT;
}

/* Roster Handling }}} */

int jabber_chat_send(Jabber_Session *sess, const char *jid, const char *body){
  if(!sess || !jid || !body || sess->state!=JABBER_CONNECTED)return 0;
  iks *msg = iks_make_msg(IKS_TYPE_CHAT, jid, body);
  iks_send(sess->prs, msg);
  iks_delete(msg);
  return 1;
}

static int
on_chat (Jabber_Session *sess, ikspak *pak) {
  if(sess->chat_cb.func){
    sess->chat_cb.func((void*)sess->chat_cb.data, sess, (void*)pak);
  }
  return IKS_FILTER_EAT;
}

int jabber_status_set(Jabber_Session *sess, Jabber_Show show, const char *desc){
  if(show!=JABBER_UNDEFINED){
    if(sess->show_desc)free(sess->show_desc);
    sess->show_desc=NULL;
    if(desc)sess->show_desc=strdup(desc);
    
    sess->show=show;
    if(sess->state==JABBER_CONNECTED) set_presence(sess, NULL, sess->show, sess->show_desc);
    return 1;
  }
  return 0;
}

char jabber_hastls(){
  return iks_has_tls();
}

int jabber_vcard_req(Jabber_Session *sess, const char *jid){
  if(!sess || !jid || sess->state!=JABBER_CONNECTED)return 0;
  iks *x=iks_make_iq(IKS_TYPE_GET, IKS_NS_VCARD);
  iks_insert_attrib(x, "id", "vc2");
  iks_insert_attrib(x, "to", jid);
  iks_insert_attrib(x, "from", sess->acc->full);
  iks_send(sess->prs, x);
  iks_delete(x);
  return 1;
}

int jabber_iks_send(Jabber_Session *sess, const void *data){
  iks *x=(iks*)data;
  if(sess->state!=JABBER_CONNECTED)return 0;
  iks_send(sess->prs, x);
  return 1;
}

/* connection time outs if nothing comes for this much seconds */
static const int opt_timeout = 30;

static int
on_stream (Jabber_Session *sess, int type, iks *node) {
  sess->counter = opt_timeout;
  
  //printf(">>> on_stream: type=%d, IKS_NODE_START=%d IKS_NODE_NORMAL=%d, IKS_NODE_STOP=%d, IKS_NODE_ERROR=%d <<<\n", type, IKS_NODE_START, IKS_NODE_NORMAL, IKS_NODE_STOP, IKS_NODE_ERROR);
  
  switch (type) {
  case IKS_NODE_START:
    if ((sess->option&JABBER_USETLS) && !iks_is_secure (sess->prs)) {
      iks_start_tls (sess->prs);
      break;
    }
    if (!(sess->option&JABBER_SASL)) {
      iks *x;
      char *sid = NULL;
      
      if (!(sess->option&JABBER_PLAIN)) sid = iks_find_attrib (node, "id");
      x = iks_make_auth (sess->acc, sess->passwd, sid);
      iks_insert_attrib (x, "id", "auth");
      iks_send (sess->prs, x);
      iks_delete (x);
    }
    break;

  case IKS_NODE_NORMAL:
    if (strcmp ("stream:features", iks_name (node)) == 0) {
      sess->features = iks_stream_features (node);
      if (sess->option & JABBER_SASL) {
	if ((sess->option & JABBER_USETLS) && !iks_is_secure (sess->prs)) break;
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
      /*
      if (sess->state==JABBER_CONNECTED){
	if (node) iks_delete (node);
	return IKS_HOOK;
      }
      */
    }
    break;
    
  case IKS_NODE_STOP:
    set_state(JABBER_DISCONNECTED);
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
  set_state(JABBER_DISCONNECTED);
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
  iks_filter_add_rule (sess->filter, (iksFilterHook *)on_presence, sess,
		       IKS_RULE_TYPE, IKS_PAK_PRESENCE,
		       IKS_RULE_DONE);
  iks_filter_add_rule (sess->filter, (iksFilterHook *)on_chat, sess,
		       IKS_RULE_TYPE, IKS_PAK_MESSAGE,
		       IKS_RULE_SUBTYPE, IKS_TYPE_CHAT,
		       IKS_RULE_DONE);
  iks_filter_add_rule (sess->filter, (iksFilterHook *)on_vcard, sess,
		       IKS_RULE_TYPE, IKS_PAK_IQ,
		       IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
		       IKS_RULE_ID, "vc2",
		       IKS_RULE_DONE);
}

Jabber_Session *jabber_new(){
  Jabber_Session *sess;
  
  sess = iks_malloc(sizeof(Jabber_Session));
  memset(sess, 0, sizeof(Jabber_Session));
  
  sess->prs = iks_stream_new(IKS_NS_CLIENT, sess, (iksStreamHook *)on_stream);
  setup_filter(sess);
  
  return sess;
}

void jabber_del(Jabber_Session *sess){
  jabber_disconnect(sess);
  
  if(sess->show_desc)free(sess->show_desc);
  if(sess->filter)iks_filter_delete(sess->filter);
  if(sess->prs)iks_parser_delete(sess->prs);
  
  iks_free(sess);
}

#ifndef default_res
#define default_res NAME
#endif

#ifndef default_port
#define default_port 5222
#endif

#ifndef default_tlsport
#define default_tlsport 5222
#endif

int jabber_config(Jabber_Session *sess, const char *jidres, const char *passwd, const char *server, int port, Jabber_Option opts){
  sess->option = opts;
  
  if(opts&JABBER_LOG)iks_set_log_hook(sess->prs, (iksLogHook *)on_log);
  else iks_set_log_hook(sess->prs, NULL);
  
  sess->acc = iks_id_new(iks_parser_stack(sess->prs), jidres);
  if(!sess->acc->resource){ /* user gave no resource name, use the default */
    char *tmp;
    tmp = iks_malloc(strlen(sess->acc->user)+1+strlen(sess->acc->server)+1+strlen(default_res)+1);
    sprintf (tmp, "%s@%s/%s", sess->acc->user, sess->acc->server, default_res);
    sess->acc = iks_id_new(iks_parser_stack(sess->prs), tmp);
    iks_free(tmp);
  }
  if(passwd){
    free(sess->passwd);
    sess->passwd=strdup(passwd);
  }
  
  if(server){
    free(sess->server);
    sess->server=strdup(server);
  }
  if(port)sess->port=port;
  
  // Set port automatically
  if(!sess->port)sess->port=(opts & JABBER_USETLS)?default_tlsport:default_port;
  // Try get server from JID
  if(!sess->server || !strlen(sess->server)){
    free(sess->server);
    sess->server=strdup(sess->acc->server);
  }
  
  sess->priority=4;
  
  printf(">>> Jabber Configured..\n");
  
  return 1;
}

#define REG_CB(name)							\
  void jabber_ ## name ## _callback_set(Jabber_Session *sess,		\
					Jabber_Callback func,		\
					const void *data){		\
    sess->name ## _cb.func=func;					\
    sess->name ## _cb.data=data;					\
  }

REG_CB(error);
REG_CB(state);
REG_CB(roster);
REG_CB(chat);

#undef REG_CB

static void *
_connect_thread(void *arg){
  Jabber_Session *sess=arg;
  int e;
  
  e = iks_connect_tcp(sess->prs, sess->server, sess->port);
  switch (e) {
  case IKS_OK:
    break;
  case IKS_NET_NODNS:
    set_error(_("Hostname lookup failed"));
    set_state(JABBER_DISCONNECTED);
    return NULL;
  case IKS_NET_NOCONN:
    set_error(_("Connection failed"));
    set_state(JABBER_DISCONNECTED);
    return NULL;
  default:
    set_state(JABBER_DISCONNECTED);
    set_error(_("IO Error"));
    return NULL;
  }
  
  sess->counter = opt_timeout;
  while (1) {
    if(sess->state==JABBER_DISCONNECTED){
      iks_disconnect(sess->prs);
      return NULL;
    }
    e = iks_recv (sess->prs, 1);
    //printf(">>> iks_recv code = %d; IKS_OK = %d, IKS_HOOK = %d, IKS_BADXML = %d, IKS_NET_TLSFAIL = %d \n", e, IKS_OK, IKS_HOOK, IKS_BADXML, IKS_NET_TLSFAIL);
    if (IKS_HOOK == e){
      continue;
    }
    if (IKS_NODE_ERROR == e){
      set_error(_("Node Error"), e);
      continue;
    }
    if (IKS_NET_TLSFAIL == e){
      set_error(_("TLS handshake failed"));
      set_state(JABBER_DISCONNECTED);
      return NULL;
    }
    if (IKS_OK != e){
      set_error(_("IO Error %d"), e);
      set_state(JABBER_DISCONNECTED);
      return NULL;
    }else if(sess->state==JABBER_CONNECTED){
      continue;
    }
    sess->counter--;
    if (sess->counter == 0){
      set_error(_("Network timeout"));
      set_state(JABBER_DISCONNECTED);
      return NULL;
    }
  }
  
  return NULL;
}

int jabber_connect(Jabber_Session *sess){
  set_state(JABBER_CONNECTING);
  
  if(pthread_create(&sess->thread, NULL, _connect_thread, sess)){
    return 0;
  }
  
  return 1;
}

int jabber_disconnect(Jabber_Session *sess){
  if(sess->state!=JABBER_DISCONNECTED){
    set_state(JABBER_DISCONNECTED);
    sess->authorized=0;
    return 1;
  }
  return 0;
}

Jabber_State jabber_state(Jabber_Session *sess){
  return sess->state;
}
