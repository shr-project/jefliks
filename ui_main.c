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


#ifndef default_status
#define default_status JABBER_OFFLINE
#endif


typedef struct _Widget_Data Widget_Data;
struct _Widget_Data{
  Evas_Object *parent, *box, *status, *roster;
  Jabber_Show selected_status;
  Jabber_Session *jabber;
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
_config_del(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  
  printf(">>> Show Main Window! <<<\n");
  
  evas_object_show(wd->box);
  elm_jabber_config_load(wd->jabber);
}

static void
_config_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;  
  Evas_Object *settings = elm_jabber_config_add(wd->parent);
  evas_object_size_hint_weight_set(settings, 1.0, 1.0);
  elm_win_resize_object_add(wd->parent, settings);
  evas_object_hide(wd->box);
  evas_object_show(settings);
  evas_object_event_callback_add(settings, EVAS_CALLBACK_FREE, _config_del, wd);
}

static void
_about_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Evas_Object *about = elm_jabber_about_add(wd->parent);
  evas_object_size_hint_weight_set(about, 1.0, 1.0);
  elm_win_resize_object_add(wd->parent, about);
  evas_object_show(about);
}

struct {
  Jabber_Show status;
  const char *title;
} status_list[] = {
  { JABBER_ONLINE, _("Online") },
  { JABBER_CHAT, _("Chat") },
  { JABBER_AWAY, _("Away") },
  { JABBER_XA, _("Extended Away") },
  { JABBER_DND, _("Don't Disturb") },
  { JABBER_OFFLINE, _("Offline") },
  { 0, NULL }
};

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

static void
_status_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Elm_Hoversel_Item* item=event_info;
  
  wd->selected_status=status_by_title(elm_hoversel_item_label_get(item));
  
  if(wd->selected_status==JABBER_OFFLINE){
    if(jabber_state(wd->jabber)!=JABBER_DISCONNECTED){
      jabber_disconnect(wd->jabber);
    }
  }else{
    jabber_status_set(wd->jabber, wd->selected_status, _("I'm Jefliks! (Experimental Jabber Client for Handheld devices, based on Enlightenment) http://sourceforge.net/projects/jefliks/"));
    if(jabber_state(wd->jabber)==JABBER_CONNECTED){
      elm_hoversel_label_set(wd->status, title_by_status(wd->selected_status));
    }else{
      jabber_connect(wd->jabber);
    }
  }
}

static void
_state_change_hook(Widget_Data *wd, Jabber_Session *sess, Jabber_State state){
  const char *title=_("Undefined..");
  
  switch(state){
  case JABBER_DISCONNECTED:
    title=title_by_status(JABBER_OFFLINE);
    break;
  case JABBER_CONNECTING:
    title=_("Connecting..");
    break;
  case JABBER_CONNECTED:
    title=title_by_status(wd->selected_status);
    break;
  }
  elm_hoversel_label_set(wd->status, title);
}

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

static void
_error_notify_close(void *data, Evas_Object *obj, void *event_info){
  Evas_Object *notify = data;
  evas_object_hide(notify);
}

static void
_error_notify_hook(Widget_Data *wd, Jabber_Session *sess, const char *message){
  Evas_Object *notify, *box, *text, *close;
  
  printf("ERROR: %s\n", message);
  
  notify = elm_notify_add(wd->parent);
  evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(wd->parent, notify);
  
  box = elm_box_add(wd->parent);
  elm_notify_content_set(notify, box);
  elm_box_horizontal_set(box, 1);
  evas_object_show(box);
  
  text = elm_label_add(wd->parent);
  elm_label_label_set(text, message);
  elm_box_pack_end(box, text);
  evas_object_show(text);
  
  close = elm_button_add(wd->parent);
  elm_button_label_set(close, _("Close"));
  evas_object_smart_callback_add(close, "clicked", _error_notify_close, notify);
  elm_box_pack_end(box, close);
  evas_object_show(close);
  
  evas_object_show(notify);
}

Evas_Object *elm_jabber_main(Evas_Object *parent){
  Widget_Data *wd;
  Evas_Object *box, *buttons, *status, *actions, *roster;
  
  wd = malloc(sizeof(Widget_Data));
  wd->jabber=jabber_new();
  jabber_error_callback_set(wd->jabber, (Jabber_Callback)_error_notify_hook, wd);
  jabber_state_callback_set(wd->jabber, (Jabber_Callback)_state_change_hook, wd);
  elm_jabber_config_load(wd->jabber);
  wd->parent=parent;
  
  /* Main box */
  box = elm_box_add(parent);
  wd->box=box;
  //evas_object_size_hint_weight_set(box, 1.0, 1.0);
  //evas_object_size_hint_align_set(box, -1.0, -1.0);
  evas_object_event_callback_add(box, EVAS_CALLBACK_FREE, _del_hook, wd);
  evas_object_show(box);
  
  /* Roster */
  roster = elm_jabber_roster_add(parent);
  wd->roster=roster;
  elm_jabber_roster_register(roster, wd->jabber);
  evas_object_size_hint_weight_set(roster, 1.0, 1.0);
  evas_object_size_hint_align_set(roster, -1.0, -1.0);
  elm_box_pack_end(box, roster);
  evas_object_show(roster);
  
  /* Buttons */
  buttons = elm_box_add(parent);
  elm_box_horizontal_set(buttons, 1);
  elm_box_homogenous_set(buttons, 1);
  evas_object_size_hint_weight_set(buttons, 1.0, 0.0);
  evas_object_size_hint_align_set(buttons, -1.0, 1.0);
  elm_box_pack_end(box, buttons);
  evas_object_show(buttons);
  
  /* Status */
  status = elm_hoversel_add(parent);
  wd->status=status;
  elm_hoversel_label_set(status, _("Status"));
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
  
  /* Actions */
  actions = elm_hoversel_add(parent);
  elm_hoversel_label_set(actions, _("Actions"));
  elm_hoversel_hover_parent_set(actions, box);
  evas_object_size_hint_weight_set(actions, 1.0, 1.0);
  evas_object_size_hint_align_set(actions, -1.0, 0.0);
  
  elm_hoversel_item_add(actions, _("Settings"), NULL, 0, _config_hook, wd);
  elm_hoversel_item_add(actions, _("About"), NULL, 0, _about_hook, wd);
  elm_hoversel_item_add(actions, _("Exit"), NULL, 0, _exit_hook, NULL);
  
  elm_box_pack_end(buttons, actions);
  evas_object_show(actions);
  
  return box;
}
