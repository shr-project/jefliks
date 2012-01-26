/** ui_main.c --- 
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

#include<Elementary.h>
#include"jabber.h"

#include"ui_common.h"
#include"ui_main.h"

#include"ui_config.h"
#include"ui_about.h"
#include"ui_roster.h"
#include"ui_chat.h"


#ifndef default_status
#define default_status JABBER_OFFLINE
#endif


typedef struct _Widget_Data Widget_Data;
struct _Widget_Data{
  Evas_Object *parent, *root, *main, *status, *roster, *chat;
  Jabber_Show selected_status;
  Jabber_Session *jabber;
  Elm_Jabber_Option option;
  char need_reconnect:1;
};

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  jabber_del(wd->jabber);
  free(wd);
}

static void
_exit_hook(void *data, Evas_Object *obj, void *event_info){
  elm_exit();
}

static void
_roster_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  DEBUG("Show Main Window!");
  elm_pager_content_promote(wd->root, wd->main);
  evas_object_hide(wd->chat);
  evas_object_show(wd->main);
}

static void
_all_chats_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  elm_pager_content_promote(wd->root, wd->chat);
  evas_object_hide(wd->main);
  evas_object_show(wd->chat);
}

static void
_chat_with_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  elm_jabber_chat_enter(wd->chat, elm_jabber_roster_selected(wd->roster));
  _all_chats_hook(data, obj, event_info);
}

static void
_config_changed(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  wd->need_reconnect=1;
  evas_object_smart_callback_call(wd->roster, "config,changed", NULL);
}

static void
_config_close(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  elm_pager_content_pop(wd->root);
  evas_object_del(obj);
  if(wd->need_reconnect){
    elm_jabber_config_load(wd->jabber);
    wd->need_reconnect=0;
  }
}

static void
_config_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Evas_Object *config = elm_jabber_config_add(wd->parent);
  elm_pager_content_push(wd->root, config);
  evas_object_show(config);
  evas_object_smart_callback_add(config, "config,changed", _config_changed, wd);
  evas_object_smart_callback_add(config, "config,close", _config_close, wd);
}

static void
_about_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Evas_Object *about = elm_jabber_about_add(wd->parent);
  evas_object_size_hint_weight_set(about, 1.0, 1.0);
  elm_win_resize_object_add(wd->parent, about);
  evas_object_show(about);
}

static struct {
  Jabber_Show status;
  char *title;
  const char *icon;
} status_list[] = {
  { JABBER_ONLINE, NULL, "status/available" },
  { JABBER_CHAT, NULL, "status/chat" },
  { JABBER_AWAY, NULL, "status/away" },
  { JABBER_XA, NULL, "status/xa" }, // Extended Away
  { JABBER_DND, NULL, "status/dnd" }, // Don't Disturb
  { JABBER_OFFLINE, NULL, "status/unavailable" },
  { 0, NULL, NULL }
};

static void status_init(){
  if(status_list[0].title)return;
  
  status_list[0].title=_("Online");
  status_list[1].title=_("Chat");
  status_list[2].title=_("Away");
  status_list[3].title=_("XA");
  status_list[4].title=_("DND");
  status_list[5].title=_("Offline");
}

static Jabber_Show status_by_title(const char *title){
  int i;
  for(i=0; status_list[i].title; i++){
    if(!strcmp(status_list[i].title, title)){
      return status_list[i].status;
    }
  }
  return JABBER_UNDEFINED;
}

static const char *title_by_status(Jabber_Show status){
  int i;
  for(i=0; status_list[i].title; i++){
    if(status==status_list[i].status){
      return status_list[i].title;
    }
  }
  return NULL;
}

/*
static const char *icon_by_status(Jabber_Show status){
  int i;
  for(i=0; status_list[i].icon; i++){
    if(status==status_list[i].status){
      return status_list[i].icon;
    }
  }
  return NULL;
}
*/

static void
_status_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Elm_Object_Item* item=event_info;
  
  wd->selected_status=status_by_title(elm_object_item_text_get(item));
  
  if(wd->selected_status==JABBER_OFFLINE){
    if(jabber_state(wd->jabber)!=JABBER_DISCONNECTED){
      jabber_disconnect(wd->jabber);
    }
    elm_jabber_roster_clear(wd->roster);
  }else{
    jabber_status_set(wd->jabber, wd->selected_status, _("I'm Jefliks! (Experimental Jabber Client for Handheld devices, based on Enlightenment) http://sourceforge.net/projects/jefliks/"));
    if(jabber_state(wd->jabber)==JABBER_CONNECTED){
      elm_object_text_set(wd->status, title_by_status(wd->selected_status));
    }else{
      jabber_connect(wd->jabber);
    }
  }
}

typedef struct _Async_State Async_State;
struct _Async_State {
  Widget_Data *wd;
  Jabber_State st;
};

static void
_state_change_job(void *data){
  Async_State *as=data;
  Widget_Data *wd=as->wd;
  Jabber_State state=as->st;
  const char *title=_("----");
  
  switch(state){
  case JABBER_DISCONNECTED:
    title=title_by_status(JABBER_OFFLINE);
    break;
  case JABBER_CONNECTING:
    title=_("....");
    break;
  case JABBER_CONNECTED:
    title=title_by_status(wd->selected_status);
    break;
  }
  elm_object_text_set(wd->status, title);
  
  free(as);
}

static void
_state_change_async(Widget_Data *wd, Jabber_Session *sess, Jabber_State state){
  Async_State *as=malloc(sizeof(Async_State));
  as->wd=wd;
  as->st=state;
  ecore_job_add(_state_change_job, as);
}

/*
static void
_status_load(Jabber_Show *status){
  Eet_File *ef;
  char *val;
  int size;
  
  ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_READ);
  *status=default_status;
  
  if(ef){
    val=eet_read(ef, "last_status", &size);
    if(val){
      *status=*val;
      free(val);
    }
    eet_close(ef);
  }
}

static void
_status_save(Jabber_Show status){
  Eet_File *ef;
  char st;
  char *val;
  
  ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_WRITE);
  if(ef){
    st=status;
    val=&st;
    eet_write(ef, "last_status", val, 1, 0);
    
    eet_close(ef);
  }
}
*/

typedef struct _Async_Notify Async_Notify;
struct _Async_Notify {
  Widget_Data *wd;
  char *msg;
};

static void
_error_notify_close(void *data, Evas_Object *obj, void *event_info){
  Evas_Object *notify = data;
  evas_object_hide(notify);
}

static void
_error_notify_job(void *data){
  Async_Notify *an=data;
  Widget_Data *wd=an->wd;
  const char *message=an->msg;
  Evas_Object *notify, *box, *text, *close;
  
  DEBUG("ERROR: %s", message);
  
  notify = elm_notify_add(wd->parent);
  evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(wd->parent, notify);
  
  box = elm_box_add(wd->parent);
  elm_notify_content_set(notify, box);
  elm_box_horizontal_set(box, 1);
  evas_object_show(box);
  
  text = elm_label_add(wd->parent);
  elm_object_text_set(text, message);
  elm_box_pack_end(box, text);
  evas_object_show(text);
  
  close = elm_button_add(wd->parent);
  elm_object_text_set(close, _("Close"));
  evas_object_smart_callback_add(close, "clicked", _error_notify_close, notify);
  elm_box_pack_end(box, close);
  evas_object_show(close);
  
  evas_object_show(notify);
  
  free(an->msg);
  free(an);
}

static void
_error_notify_async(Widget_Data *wd, Jabber_Session *sess, const char *message){
  Async_Notify *an=malloc(sizeof(Async_Notify));
  an->wd=wd;
  an->msg=strdup(message);
  ecore_job_add(_error_notify_job, an);
}

Evas_Object *elm_jabber_main(Evas_Object *parent){
  Widget_Data *wd;
  Evas_Object *pager, *box, *buttons, *status, *actions, *talks, *roster, *chat;
  
  status_init();
  
  wd = malloc(sizeof(Widget_Data));
  wd->jabber=jabber_new();
  wd->need_reconnect=0;
  jabber_error_callback_set(wd->jabber, (Jabber_Callback)_error_notify_async, wd);
  jabber_state_callback_set(wd->jabber, (Jabber_Callback)_state_change_async, wd);
  elm_jabber_config_load(wd->jabber);
  wd->parent=parent;
  
  /* Main Pager */
  pager = elm_pager_add(parent);
  wd->root=pager;
  evas_object_event_callback_add(pager, EVAS_CALLBACK_FREE, _del_hook, wd);
  evas_object_show(pager);
  
  /* Main box */
  box = elm_box_add(parent);
  wd->main=box;
  //evas_object_size_hint_weight_set(box, 1.0, 1.0);
  //evas_object_size_hint_align_set(box, -1.0, -1.0);
  evas_object_show(box);
  
  /* Roster */
  roster = elm_jabber_roster_add(parent);
  wd->roster=roster;
  elm_jabber_roster_register(roster, wd->jabber);
  evas_object_size_hint_weight_set(roster, 1.0, 1.0);
  evas_object_size_hint_align_set(roster, -1.0, -1.0);
  elm_box_pack_end(box, roster);
  evas_object_smart_callback_call(wd->roster, "config,changed", NULL);
  evas_object_show(roster);
  
  /* Buttons */
  buttons = elm_box_add(parent);
  elm_box_horizontal_set(buttons, 1);
  elm_box_homogeneous_set(buttons, 1);
  evas_object_size_hint_weight_set(buttons, 1.0, 0.0);
  evas_object_size_hint_align_set(buttons, -1.0, 1.0);
#ifdef BUTTONS_RESCALE
  elm_object_scale_set(buttons, BUTTONS_RESCALE);
#endif
  elm_box_pack_end(box, buttons);
  evas_object_show(buttons);
  
  /* Status */
  status = elm_hoversel_add(parent);
  wd->status=status;
  elm_object_text_set(status, _("Status"));
  elm_hoversel_hover_parent_set(status, box);
  evas_object_size_hint_weight_set(status, 1.0, 1.0);
  evas_object_size_hint_align_set(status, -1.0, 0.0);
  
  {
    int i;
    for(i=0; status_list[i].title; i++){
      elm_hoversel_item_add(status, status_list[i].title, NULL, 0, _status_hook, wd);
    }
  }
  
  elm_box_pack_end(buttons, status);
  evas_object_show(status);
  
  /* Talks */
  talks = elm_hoversel_add(parent);
  elm_object_text_set(talks, _("Talks"));
  elm_hoversel_hover_parent_set(talks, box);
  evas_object_size_hint_weight_set(talks, 1.0, 1.0);
  evas_object_size_hint_align_set(talks, -1.0, 0.0);
  
  elm_hoversel_item_add(talks, _("Chat With"), NULL, 0, _chat_with_hook, wd);
  elm_hoversel_item_add(talks, _("All Chats"), NULL, 0, _all_chats_hook, wd);
  
  elm_box_pack_end(buttons, talks);
  evas_object_show(talks);
  
  /* Actions */
  actions = elm_hoversel_add(parent);
  elm_object_text_set(actions, _("Actions"));
  elm_hoversel_hover_parent_set(actions, box);
  evas_object_size_hint_weight_set(actions, 1.0, 1.0);
  evas_object_size_hint_align_set(actions, -1.0, 0.0);
  
  elm_hoversel_item_add(actions, _("Settings"), NULL, 0, _config_hook, wd);
  elm_hoversel_item_add(actions, _("About"), NULL, 0, _about_hook, wd);
  elm_hoversel_item_add(actions, _("Exit"), NULL, 0, _exit_hook, NULL);
  
  elm_box_pack_end(buttons, actions);
  evas_object_show(actions);
  
  /* Chat Widget */
  chat = elm_jabber_chat_add(parent);
  wd->chat=chat;
  elm_jabber_chat_register(chat, wd->jabber);
  evas_object_size_hint_weight_set(chat, 1.0, 1.0);
  evas_object_smart_callback_add(chat, "goto,roster", _roster_hook, wd);
  evas_object_show(chat);
  
  /* Show Main Box */
  elm_pager_content_push(pager, chat);
  elm_pager_content_push(pager, box);
  
  return pager;
}
