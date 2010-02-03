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

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<iksemel.h>

/* stuff we keep per session */
struct session {
  iksparser *prs;
  iksid *acc;
  char *pass;
  int features;
  int authorized;
  int counter;
  int job_done;
};

/* precious roster we'll deal with */
iks *my_roster;

/* out packet filter */
iksfilter *my_filter;

/* connection time outs if nothing comes for this much seconds */
int opt_timeout = 30;

/* connection flags */
int opt_use_tls;
int opt_use_sasl;
int opt_use_plain;
int opt_log;

void
j_error (char *msg)
{
  fprintf (stderr, "iksroster: %s\n", msg);
  exit (2);
}

int
on_result (struct session *sess, ikspak *pak)
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

int
on_stream (struct session *sess, int type, iks *node)
{
  sess->counter = opt_timeout;

  switch (type) {
  case IKS_NODE_START:
    if (opt_use_tls && !iks_is_secure (sess->prs)) {
      iks_start_tls (sess->prs);
      break;
    }
    if (!opt_use_sasl) {
      iks *x;
      char *sid = NULL;

      if (!opt_use_plain) sid = iks_find_attrib (node, "id");
      x = iks_make_auth (sess->acc, sess->pass, sid);
      iks_insert_attrib (x, "id", "auth");
      iks_send (sess->prs, x);
      iks_delete (x);
    }
    break;

  case IKS_NODE_NORMAL:
    if (strcmp ("stream:features", iks_name (node)) == 0) {
      sess->features = iks_stream_features (node);
      if (opt_use_sasl) {
	if (opt_use_tls && !iks_is_secure (sess->prs)) break;
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
	    iks_start_sasl (sess->prs, IKS_SASL_DIGEST_MD5, sess->acc->user, sess->pass);
	  else if (sess->features & IKS_STREAM_SASL_PLAIN)
	    iks_start_sasl (sess->prs, IKS_SASL_PLAIN, sess->acc->user, sess->pass);
	}
      }
    } else if (strcmp ("failure", iks_name (node)) == 0) {
      j_error ("sasl authentication failed");
    } else if (strcmp ("success", iks_name (node)) == 0) {
      sess->authorized = 1;
      iks_send_header (sess->prs, sess->acc->server);
    } else {
      ikspak *pak;

      pak = iks_packet (node);
      iks_filter_packet (my_filter, pak);
      if (sess->job_done == 1) return IKS_HOOK;
    }
    break;

  case IKS_NODE_STOP:
    j_error ("server disconnected");

  case IKS_NODE_ERROR:
    j_error ("stream error");
  }

  if (node) iks_delete (node);
  return IKS_OK;
}

int
on_error (void *user_data, ikspak *pak)
{
  j_error ("authorization failed");
  return IKS_FILTER_EAT;
}

int
on_roster (struct session *sess, ikspak *pak)
{
  my_roster = pak->x;
  sess->job_done = 1;
  return IKS_FILTER_EAT;
}

void
on_log (struct session *sess, const char *data, size_t size, int is_incoming)
{
  if (iks_is_secure (sess->prs)) fprintf (stderr, "Sec");
  if (is_incoming) fprintf (stderr, "RECV"); else fprintf (stderr, "SEND");
  fprintf (stderr, "[%s]\n", data);
}

void
j_setup_filter (struct session *sess)
{
  if (my_filter) iks_filter_delete (my_filter);
  my_filter = iks_filter_new ();
  iks_filter_add_rule (my_filter, (iksFilterHook *) on_result, sess,
		       IKS_RULE_TYPE, IKS_PAK_IQ,
		       IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
		       IKS_RULE_ID, "auth",
		       IKS_RULE_DONE);
  iks_filter_add_rule (my_filter, on_error, sess,
		       IKS_RULE_TYPE, IKS_PAK_IQ,
		       IKS_RULE_SUBTYPE, IKS_TYPE_ERROR,
		       IKS_RULE_ID, "auth",
		       IKS_RULE_DONE);
  iks_filter_add_rule (my_filter, (iksFilterHook *) on_roster, sess,
		       IKS_RULE_TYPE, IKS_PAK_IQ,
		       IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
		       IKS_RULE_ID, "roster",
		       IKS_RULE_DONE);
}

void
j_connect (char *jabber_id, char *pass)
{
  struct session sess;
  int e;
  
  memset (&sess, 0, sizeof (sess));
  sess.prs = iks_stream_new (IKS_NS_CLIENT, &sess, (iksStreamHook *) on_stream);
  if (opt_log) iks_set_log_hook (sess.prs, (iksLogHook *) on_log);
  sess.acc = iks_id_new (iks_parser_stack (sess.prs), jabber_id);
  if (NULL == sess.acc->resource) {
    /* user gave no resource name, use the default */
    char *tmp;
    tmp = iks_malloc (strlen (sess.acc->user) + strlen (sess.acc->server) + 9 + 3);
    sprintf (tmp, "%s@%s/%s", sess.acc->user, sess.acc->server, "iksroster");
    sess.acc = iks_id_new (iks_parser_stack (sess.prs), tmp);
    iks_free (tmp);
  }
  sess.pass = pass;
  
  j_setup_filter (&sess);

  e = iks_connect_tcp (sess.prs, sess.acc->server, IKS_JABBER_PORT);
  switch (e) {
  case IKS_OK:
    break;
  case IKS_NET_NODNS:
    j_error ("hostname lookup failed");
  case IKS_NET_NOCONN:
    j_error ("connection failed");
  default:
    j_error ("io error");
  }
  
  sess.counter = opt_timeout;
  while (1) {
    e = iks_recv (sess.prs, 1);
    if (IKS_HOOK == e) break;
    if (IKS_NET_TLSFAIL == e) j_error ("tls handshake failed");
    if (IKS_OK != e) j_error ("io error");
    sess.counter--;
    if (sess.counter == 0) j_error ("network timeout");
  }
  iks_parser_delete (sess.prs);
}
