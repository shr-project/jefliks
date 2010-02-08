/** ui_chat.c --- 
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
#include"ui_common.h"

#include"jabber.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Chat_Inst Chat_Inst;

struct _Chat_Inst{ /* Chat Instance */
  Widget_Data *wd;
  Evas_Object *box;
  Elm_Hoversel_Item *itm;
  char *jid; // jid/res
};

struct _Widget_Data{
  Evas_Object *parent, *root, *chats, *scroll;
  Eina_List *insts; /* Chat Instances */
  Jabber_Session *jabber; /* Jabber Session */
};

static void
_chat_sel_hook(void *data, Evas_Object *obj, void *event_info){
  Chat_Inst *chat=data;
  elm_scroller_content_set(chat->wd->scroll, chat->box);
  elm_hoversel_label_set(chat->wd->chats, chat->jid);
}

static Chat_Inst *inst_fnd(Widget_Data *wd, const char *jid){
  if(!wd || !jid)return NULL;
  Eina_List *entry;
  for(entry=wd->insts; entry; entry=entry->next){
    Chat_Inst *chat=entry->data;
    if(!chat || strcmp(chat->jid, jid))continue;
    return chat;
  }
  return NULL;
}

static Chat_Inst *inst_get(Widget_Data *wd, const char *jid){
  Chat_Inst *chat=inst_fnd(wd, jid);
  
  if(!chat){
    chat=malloc(sizeof(Chat_Inst));
    
    chat->wd=wd;
    chat->jid=strdup(jid);
    
    chat->box=elm_box_add(wd->parent);
    evas_object_size_hint_weight_set(chat->box, 1.0, 0.0);
    
    chat->itm=elm_hoversel_item_add(wd->chats, jid, NULL, 0, _chat_sel_hook, chat);
    
    wd->insts=eina_list_append(wd->insts, chat);
  }
  
  return chat;
}

static void inst_free(Chat_Inst *chat){
  if(!chat)return;
  
  elm_hoversel_item_del(chat->itm);
  evas_object_del(chat->box);
  free(chat->jid);
  free(chat);
}

static void inst_del(Widget_Data *wd, const char *jid){
  Chat_Inst *chat=inst_fnd(wd, jid);
  
  inst_free(chat);
}

static void
_chat_del_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  //Evas_Object *box=elm_scroller_content_get(wd->scroll);
  const char *jid=elm_hoversel_label_get(wd->chats);
  inst_del(wd, jid);
}

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Eina_List *entry;
  
  for(entry=wd->insts; entry; entry=entry->next){
    Chat_Inst *chat=entry->data;
    inst_free(chat);
  }
  wd->insts=eina_list_free(wd->insts);
  free(wd);
}

void elm_jabber_chat_enter(Evas_Object *box, const char *jid){
  Widget_Data *wd=evas_object_data_get(box, "wd");
  
  inst_get(wd, jid);
}

void elm_jabber_chat_register(Evas_Object *box, Jabber_Session *jabber){
   Widget_Data *wd=evas_object_data_get(box, "wd");
   wd->jabber=jabber;
}

static void
_close_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;  
  evas_object_hide(wd->root);
}

Evas_Object *elm_jabber_chat_add(Evas_Object * parent){
  Widget_Data *wd;
  Evas_Object *area, *box, *buttons, *chats, *actions, *scroll;
  
  wd = malloc(sizeof(Widget_Data));
  wd->parent=parent;
  wd->jabber=NULL;
  
  /* Main box */
  box = elm_box_add(parent);
  wd->root=box;
  evas_object_data_set(box, "wd", wd);
  evas_object_event_callback_add(box, EVAS_CALLBACK_FREE, _del_hook, wd);
  evas_object_show(box);
  
  /* Messages Scroll */
  scroll = elm_scroller_add(parent);
  wd->scroll = scroll;
  elm_scroller_bounce_set(scroll, 0.0, 0.0);
  evas_object_size_hint_weight_set(scroll, 1.0, 1.0);
  evas_object_size_hint_align_set(scroll, -1.0, -1.0);
  elm_box_pack_end(box, scroll);
  evas_object_show(scroll);
  
  /* Buttons */
  buttons = elm_box_add(parent);
  elm_box_horizontal_set(buttons, 1);
  elm_box_homogenous_set(buttons, 1);
  evas_object_size_hint_weight_set(buttons, 1.0, 0.0);
  evas_object_size_hint_align_set(buttons, -1.0, 1.0);
  elm_box_pack_end(box, buttons);
  evas_object_show(buttons);
  
  /* Chats */
  chats = elm_hoversel_add(parent);
  wd->chats=chats;
  elm_hoversel_label_set(chats, _("Chats"));
  elm_hoversel_hover_parent_set(chats, box);
  evas_object_size_hint_weight_set(chats, 1.0, 1.0);
  evas_object_size_hint_align_set(chats, -1.0, 0.0);
  elm_box_pack_end(buttons, chats);
  evas_object_show(chats);
  
  /* Actions */
  actions = elm_hoversel_add(parent);
  elm_hoversel_label_set(actions, _("Actions"));
  elm_hoversel_hover_parent_set(actions, box);
  evas_object_size_hint_weight_set(actions, 1.0, 1.0);
  evas_object_size_hint_align_set(actions, -1.0, 0.0);
  
  elm_hoversel_item_add(actions, _("Roster"), NULL, 0, _close_hook, wd);
  elm_hoversel_item_add(actions, _("Close"), NULL, 0, _chat_del_hook, wd);
  
  elm_box_pack_end(buttons, actions);
  evas_object_show(actions);
}
