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

#include"ui_roster.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Chat_Inst Chat_Inst;

struct _Chat_Inst{ /* Chat Instance */
  Widget_Data *wd;
  Evas_Object *box /* chat object */, *que /* messages queue */, *scroll /* messages queue */, *input /* input area */, *photo;
  Elm_Hoversel_Item *itm;
  char *jid; // jid/res
  char autoscroll:1;
  char needscroll:1;
};

struct _Widget_Data{
  Evas_Object *parent, *root, *chats, *pager, *empty, *empty_label, *write_send, *end_cancel;
  Eina_List *insts; /* Chat Instances */
  Chat_Inst *cinst; /* Current Chat Instance */
  Jabber_Session *jabber; /* Jabber Session */
  char finalize;
};

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

static void inst_cin(Widget_Data *wd, Eina_Bool st){
  Chat_Inst *chat=wd->cinst;
  if(!chat)return;
  Eina_Bool sr=evas_object_visible_get(chat->input);
  if(st==sr)return; // no need switch state
  if(sr){
    evas_object_hide(chat->input);
    elm_box_unpack(chat->box, chat->input);
    elm_button_label_set(wd->write_send, _("Write"));
    elm_button_label_set(wd->end_cancel, _("End"));
  }else{
    elm_box_pack_end(chat->box, chat->input);
    evas_object_show(chat->input);
    elm_button_label_set(wd->write_send, _("Send"));
    elm_button_label_set(wd->end_cancel, _("Cancel"));
  }
}

static void inst_sel(Widget_Data *wd, Chat_Inst *chat){
  if(chat){
    elm_pager_content_promote(wd->pager, chat->box);
    elm_hoversel_label_set(wd->chats, chat->jid);
  }else{
    elm_pager_content_promote(wd->pager, wd->empty);
    elm_hoversel_label_set(wd->chats, _("Chats"));
  }
  if(wd->cinst!=chat) inst_cin(wd, 0); // hide input
  wd->cinst=chat;
}

static void
_chat_sel_hook(void *data, Evas_Object *obj, void *event_info){
  Chat_Inst *chat=data;
  Widget_Data *wd=chat->wd;
  
  inst_sel(wd, chat);
}

static void // auto scrolling to last message
_que_resize(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Chat_Inst *chat=data;
  if(chat->autoscroll){
    Evas_Coord cw, ch, rx, ry, rw, rh;
    elm_scroller_child_size_get(chat->scroll, &cw, &ch);
    elm_scroller_region_get(chat->scroll, &rx, &ry, &rw, &rh);
    elm_scroller_region_bring_in(chat->scroll, rx, ry+(ch-rh), rw, rh);
  }else{
    chat->needscroll=1;
  }
}

static void // disables autoscrolling when user start scroll messages manually
_scroll_drag_start_hook(void *data, Evas_Object *obj, void *event_info){
  Chat_Inst *chat=data;
  chat->autoscroll=0;
}

static void // enables autoscrolling when user stop scroll messages manually
_scroll_drag_stop_hook(void *data, Evas_Object *obj, void *event_info){
  Chat_Inst *chat=data;
  chat->autoscroll=1;
  if(chat->needscroll)_que_resize(data, NULL, obj, event_info);
}

static Chat_Inst *inst_get(Widget_Data *wd, const char *jid){
  Chat_Inst *chat=inst_fnd(wd, jid);
  
  if(jid && !chat){
    Evas_Object *box, *scroll, *que, *input, *photo;
    
    chat=malloc(sizeof(Chat_Inst));
    memset(chat, 0, sizeof(Chat_Inst));
    
    printf("Chat Inst Add: 0x%x\n", chat);
    
    chat->wd=wd;
    chat->jid=strdup(jid);
    chat->autoscroll=1;
    chat->needscroll=0;
    
    /* Page Box */
    box = elm_box_add(wd->parent);
    chat->box = box;
    elm_object_scale_set(chat->box, 0.8);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_pager_content_push(wd->pager, box);
    evas_object_show(box);
    
    /* Messages Box */
    que = elm_box_add(box);
    chat->que=que;
    evas_object_size_hint_weight_set(que, EVAS_HINT_EXPAND, 0.0);
    //evas_object_size_hint_align_set(que, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_align_set(que, EVAS_HINT_FILL, 1.0);
    evas_object_event_callback_add(que, EVAS_CALLBACK_RESIZE, _que_resize, chat);
    evas_object_show(que);
    
    /* Messages Scroll */
    scroll = elm_scroller_add(box);
    chat->scroll = scroll;
    elm_scroller_bounce_set(scroll, 0.0, 0.0);
    evas_object_size_hint_weight_set(scroll, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(scroll, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(scroll, "scroll,drag,start", _scroll_drag_start_hook, chat);
    evas_object_smart_callback_add(scroll, "scroll,drag,stop", _scroll_drag_stop_hook, chat);
    elm_scroller_content_set(scroll, que);
    elm_box_pack_end(box, scroll);
    evas_object_show(scroll);
    
    /* Input Entry */
    input = elm_entry_add(box);
    chat->input = input;
    /*elm_entry_single_line_set(input, EINA_TRUE);*/
    evas_object_size_hint_weight_set(input, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(input, EVAS_HINT_FILL, 1.0);
    evas_object_size_hint_padding_set(input, 0.1, 0.1, 0.0, 0.0);
    evas_object_hide(input);
    
    /* User Photo */
    photo = elm_jabber_photo_add(box, jid);
    chat->photo = photo?photo:NULL;
    
    /* Adds Item in Chat Switcher */
    chat->itm=elm_hoversel_item_add(wd->chats, jid, NULL, 0, _chat_sel_hook, chat);
    
    wd->insts=eina_list_append(wd->insts, chat);
    
    /* Show on top each newest chat */
    inst_sel(wd, chat);
  }
  
  return chat;
}

static void inst_add(Chat_Inst *chat, const char* text, char dir /* 0 - in, 1 - out */){
  if(!chat || !text)return;
  Widget_Data *wd=chat->wd;
  Evas_Object *repl, *body, *photo;
  
  body=elm_anchorblock_add(wd->parent);
  elm_anchorblock_text_set(body, text);
  /*elm_anchorblock_hover_style_set(body, "popout");*/
  /*elm_anchorblock_hover_parent_set(body, wd->parent);*/
  
  repl=elm_bubble_add(wd->parent);
  elm_bubble_label_set(repl, dir?"you":chat->jid);
  if(!dir){
    photo = elm_jabber_photo_add(repl, chat->jid);
    if(photo) elm_bubble_icon_set(repl, photo);
  }
  
  switch(1){
    char buf[64];
    time_t now;
    struct tm t;
  case 1:
#define default_timefmt "%Y-%m-%d %H:%M:%S"
    memset(&t, 0, sizeof(t));
    now=time(&now);
    if(!localtime_r(&now, &t))break;
    if(!strftime(buf, sizeof(buf), default_timefmt, &t))break;
    
    elm_bubble_info_set(repl, buf);
  }
  
  evas_object_size_hint_weight_set(repl, EVAS_HINT_EXPAND, 0.0);
  evas_object_size_hint_align_set(repl, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_bubble_content_set(repl, body);
  
  elm_box_pack_end(chat->que, repl);
  evas_object_show(repl);
}

static void inst_free(Chat_Inst *chat){
  if(!chat)return;
  
  printf("Chat Inst Del: 0x%x\n", chat);
  
  /* switch to other chat */
  Widget_Data *wd=chat->wd;
  
  wd->insts=eina_list_remove(wd->insts, chat);
  
  if(!wd->finalize){
    elm_pager_content_promote(wd->pager, chat->box);
    elm_pager_content_pop(wd->pager);
    evas_object_del(chat->box);
    
    if(wd->insts){
      Eina_List *entry;
      for(entry=wd->insts; entry; entry=entry->next){
	Chat_Inst *other=entry->data;
	if(chat==other)continue;
	inst_sel(wd, other);
	break;
      }
    }else{
      inst_sel(wd, NULL);
    }
    
    elm_hoversel_item_del(chat->itm);
  }
  
  free(chat->jid);
  free(chat);
}

static void inst_del(Widget_Data *wd, const char *jid){
  Chat_Inst *chat=inst_fnd(wd, jid);
  inst_free(chat);
}

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Eina_List *entry;
  
  wd->finalize=1;
  for(entry=wd->insts; entry; entry=entry->next){
    Chat_Inst *chat=entry->data;
    inst_free(chat);
  }
  wd->insts=eina_list_free(wd->insts);
  free(wd);
}

static void
_write_send_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Chat_Inst *chat=wd->cinst;
  if(!chat)return;
  Eina_Bool st=evas_object_visible_get(chat->input);
  if(st){
    const char* txt=elm_entry_entry_get(chat->input);
    inst_add(chat, txt, 1);
    char *txs=elm_entry_markup_to_utf8(txt);
    if(txs) jabber_chat_send(wd->jabber, chat->jid, txt);
    free(txs);
    elm_entry_entry_set(chat->input, "");
    inst_cin(wd, 0); // hide input
  }else{
    inst_cin(wd, 1); // show input
  }
}

static void
_end_cancel_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Chat_Inst *chat=wd->cinst;
  if(!chat)return;
  Eina_Bool st=evas_object_visible_get(chat->input);
  if(st){
    inst_cin(wd, 0); // hide input
  }else{
    const char *jid=elm_hoversel_label_get(wd->chats);
    inst_del(wd, jid);
  }
}

void elm_jabber_chat_enter(Evas_Object *box, const char *jid){
  Widget_Data *wd=evas_object_data_get(box, "wd");
  if(jid){
    Chat_Inst *chat=inst_get(wd, jid);
    inst_sel(wd, chat);
  }
}

static void
_close_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  evas_object_hide(wd->root);
}

Evas_Object *elm_jabber_chat_add(Evas_Object * parent){
  Widget_Data *wd;
  Evas_Object *box, *chats, *pager, *buttons, *actions, *empty, *empty_label, *write_send, *end_cancel;
  
  wd = malloc(sizeof(Widget_Data));
  memset(wd, 0, sizeof(Widget_Data));
  wd->parent=parent;
  wd->finalize=0;
  
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
  evas_object_size_hint_weight_set(chats, 0.0, 0.0);
  evas_object_size_hint_align_set(chats, -1.0, 0.0);
  elm_box_pack_end(box, chats);
  evas_object_show(chats);
  
  /* Empty Label */
  empty_label = elm_label_add(parent);
  wd->empty_label = empty_label;
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
  
  /* Chats Pager */
  pager = elm_pager_add(parent);
  wd->pager = pager;
  elm_pager_content_push(pager, empty);
  evas_object_size_hint_weight_set(pager, 1.0, 1.0);
  evas_object_size_hint_align_set(pager, -1.0, -1.0);
  elm_box_pack_end(box, pager);
  evas_object_show(pager);
  
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
  /*elm_hoversel_item_add(actions, _("End Chat"), NULL, 0, _chat_end_hook, wd);*/
  
  elm_box_pack_end(buttons, actions);
  evas_object_show(actions);
  
  /* Write/Send Button */
  write_send = elm_button_add(parent);
  wd->write_send = write_send;
  elm_button_label_set(write_send, _("Write"));
  //elm_button_autorepeat_set(write_send, 0);
  evas_object_smart_callback_add(write_send, "clicked", _write_send_hook, wd);
  evas_object_size_hint_weight_set(write_send, 1.0, 0.0);
  evas_object_size_hint_align_set(write_send, -1.0, 0.0);
  elm_box_pack_end(buttons, write_send);
  evas_object_show(write_send);
  
  /* Close/Cancel Button */
  end_cancel = elm_button_add(parent);
  wd->end_cancel = end_cancel;
  elm_button_label_set(end_cancel, _("Close"));
  //elm_button_autorepeat_set(end_cancel, 0);
  evas_object_smart_callback_add(end_cancel, "clicked", _end_cancel_hook, wd);
  evas_object_size_hint_weight_set(end_cancel, 1.0, 0.0);
  evas_object_size_hint_align_set(end_cancel, -1.0, 0.0);
  elm_box_pack_end(buttons, end_cancel);
  evas_object_show(end_cancel);
  
  return box;
}

#include<iksemel.h>

void _chat_hook(Widget_Data *wd, Jabber_Session *sess, ikspak *pak){
  Chat_Inst *chat=inst_get(wd, pak->from->full);
  const char *txt=iks_find_cdata(pak->x, "body");
  char *txr=elm_entry_utf8_to_markup(txt);
  //printf(">>pak:%s txt:[%s] txr:[%s] <<\n", iks_name(pak->x), txt, txr);
  if(txr) inst_add(chat, txr, 0);
  free(txr);
}

void elm_jabber_chat_register(Evas_Object *box, Jabber_Session *jabber){
  Widget_Data *wd=evas_object_data_get(box, "wd");
  wd->jabber=jabber;
  jabber_chat_callback_set(jabber, (Jabber_Callback)_chat_hook, wd);
}
