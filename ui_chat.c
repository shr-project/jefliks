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
  Evas_Object *parent, *root, *chats, *scroll, *empty, *input;
  Eina_List *insts; /* Chat Instances */
  Chat_Inst *cinst; /* Current Chat Instance */
  Jabber_Session *jabber; /* Jabber Session */
};

static void
_chat_sel_hook(void *data, Evas_Object *obj, void *event_info){
  Chat_Inst *chat=data;
  evas_object_hide(chat->wd->empty);
  if(chat->wd->cinst) evas_object_hide(chat->wd->cinst->box);
  elm_scroller_content_set(chat->wd->scroll, chat->box);
  elm_hoversel_label_set(chat->wd->chats, chat->jid);
  evas_object_show(chat->box);
  chat->wd->cinst=chat;
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
  
  if(jid && !chat){
    chat=malloc(sizeof(Chat_Inst));
    
    chat->wd=wd;
    chat->jid=strdup(jid);
    
    chat->box=elm_box_add(wd->parent);
    evas_object_size_hint_weight_set(chat->box, 1.0, 0.0);
    evas_object_hide(chat->box);
    
    chat->itm=elm_hoversel_item_add(wd->chats, jid, NULL, 0, _chat_sel_hook, chat);
    
    wd->insts=eina_list_append(wd->insts, chat);
  }
  
  return chat;
}

static void inst_add(Chat_Inst *chat, const char* text, char dir /* 0 - in, 1 - out */){
  Evas_Object *msg=elm_bubble_add(chat->wd->parent);
  elm_bubble_label_set(msg, dir?"you":chat->jid);
  elm_bubble_info_set(msg, text);
  
  elm_box_pack_end(chat->box, msg);
  evas_object_show(msg);
}

static void inst_free(Chat_Inst *chat){
  if(!chat)return;
  
  { /* switch to other chat */
    const Eina_List *entry=elm_hoversel_items_get(chat->wd->chats);
    if(entry->next){
      for(; entry; entry=entry->next){
	const Elm_Hoversel_Item *another=entry->data;
	if(chat->itm==another)continue;
	_chat_sel_hook(chat, chat->box, NULL);
	break;
      }
    }else{
      chat->wd->cinst=NULL;
      elm_scroller_content_set(chat->wd->scroll, chat->wd->empty);
      evas_object_show(chat->wd->empty);
    }
  }
  
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

static void
_send_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Eina_Bool st=evas_object_visible_get(wd->input);
  if(st){
    Chat_Inst *chat=wd->cinst;
    if(chat){ // send message
      const char* txt=elm_entry_entry_get(wd->input);
      inst_add(chat, txt, 1);
      jabber_chat_send(wd->jabber, chat->jid, txt);
    }
    evas_object_hide(wd->input);
  }else{
    evas_object_show(wd->input);
  }
}

void elm_jabber_chat_enter(Evas_Object *box, const char *jid){
  Widget_Data *wd=evas_object_data_get(box, "wd");
  
  inst_get(wd, jid);
}

static void
_close_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;  
  evas_object_hide(wd->root);
}

Evas_Object *elm_jabber_chat_add(Evas_Object * parent){
  Widget_Data *wd;
  Evas_Object *box, *chats, *scroll, *input, *buttons, *actions, *send, *empty, *empty_label;
  
  wd = malloc(sizeof(Widget_Data));
  memset(wd, 0, sizeof(Widget_Data));
  wd->parent=parent;
  
  /* Main box */
  box = elm_box_add(parent);
  wd->root=box;
  evas_object_data_set(box, "wd", wd);
  evas_object_event_callback_add(box, EVAS_CALLBACK_FREE, _del_hook, wd);
  evas_object_show(box);
  
  /* Chats */
  chats = elm_hoversel_add(parent);
  wd->chats=chats;
  elm_hoversel_label_set(chats, _("Chats"));
  elm_hoversel_hover_parent_set(chats, box);
  evas_object_size_hint_weight_set(chats, 1.0, 0.0);
  evas_object_size_hint_align_set(chats, -1.0, 0.0);
  elm_box_pack_end(box, chats);
  evas_object_show(chats);
  
  /* Empty Label */
  empty_label = elm_label_add(parent);
  elm_label_label_set(empty_label, _("No opened chats here."));
  evas_object_show(empty_label);
  
  /* Empty Frame */
  empty = elm_frame_add(parent);
  wd->empty = empty;
  elm_frame_label_set(empty, "Empty");
  evas_object_size_hint_weight_set(empty, 1.0, 1.0);
  evas_object_size_hint_align_set(empty, -1.0, -1.0);
  elm_frame_content_set(empty, empty_label);
  evas_object_show(empty);
  
  /* Messages Scroll */
  scroll = elm_scroller_add(parent);
  wd->scroll = scroll;
  /*elm_scroller_bounce_set(scroll, 0.0, 0.0);*/
  evas_object_size_hint_weight_set(scroll, 1.0, 1.0);
  evas_object_size_hint_align_set(scroll, -1.0, -1.0);
  elm_scroller_content_set(scroll, empty);
  elm_box_pack_end(box, scroll);
  evas_object_show(scroll);
  
  /* Input Entry */
  input = elm_entry_add(parent);
  wd->input = input;
  /*elm_entry_single_line_set(input, EINA_TRUE);*/
  evas_object_size_hint_weight_set(input, 1.0, 0.0);
  evas_object_size_hint_align_set(input, -1.0, 0.0);
  elm_box_pack_end(box, input);
  evas_object_hide(input);
  
  /* Buttons */
  buttons = elm_box_add(parent);
  elm_box_horizontal_set(buttons, 1);
  elm_box_homogenous_set(buttons, 1);
  evas_object_size_hint_weight_set(buttons, 1.0, 0.0);
  evas_object_size_hint_align_set(buttons, -1.0, 1.0);
  elm_box_pack_end(box, buttons);
  evas_object_show(buttons);
  
  /* Actions */
  actions = elm_hoversel_add(parent);
  elm_hoversel_label_set(actions, _("Actions"));
  elm_hoversel_hover_parent_set(actions, box);
  evas_object_size_hint_weight_set(actions, 1.0, 0.0);
  evas_object_size_hint_align_set(actions, -1.0, 0.0);
  
  elm_hoversel_item_add(actions, _("Roster"), NULL, 0, _close_hook, wd);
  elm_hoversel_item_add(actions, _("Close"), NULL, 0, _chat_del_hook, wd);
  
  elm_box_pack_end(buttons, actions);
  evas_object_show(actions);
  
  /* Send Button */
  send = elm_button_add(parent);
  elm_button_label_set(send, _("Send"));
  elm_button_autorepeat_set(send, 0);
  evas_object_smart_callback_add(send, "clicked", _send_hook, wd);
  evas_object_size_hint_weight_set(send, 1.0, 0.0);
  evas_object_size_hint_align_set(send, -1.0, 0.0);
  elm_box_pack_end(buttons, send);
  evas_object_show(send);
  
  return box;
}

#include<iksemel.h>

void _chat_hook(Widget_Data *wd, Jabber_Session *sess, ikspak *pak){
  Chat_Inst *chat=inst_get(wd, pak->from->full);
  iks* msg=iks_find(pak->x, "body");
  inst_add(chat, iks_cdata(msg), 0);
}

void elm_jabber_chat_register(Evas_Object *box, Jabber_Session *jabber){
   Widget_Data *wd=evas_object_data_get(box, "wd");
   wd->jabber=jabber;
   jabber_chat_callback_set(jabber, (Jabber_Callback)_chat_hook, wd);
}
