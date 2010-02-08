/** elm_genlist_ext.c --- 
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
#include"elm_genlist_ext.h"

#ifndef ELM_PRIV_H
typedef struct _Widget_Data Widget_Data;
typedef struct _Item_Block Item_Block;
typedef struct _Pan Pan;

struct _Widget_Data
{
  Evas_Object *obj, *scr, *pan_smart;
  Eina_Inlist *items, *blocks;
  Pan *pan;
  Evas_Coord pan_x, pan_y, minw, minh;
  Ecore_Job *calc_job, *update_job;
  Ecore_Idler *queue_idler;
  Eina_List *queue, *selected;
  Elm_Genlist_Item *show_item;
  Elm_List_Mode mode;
  Eina_Bool on_hold : 1;
  Eina_Bool multi : 1;
  Eina_Bool always_select : 1;
  Eina_Bool longpressed : 1;
  Eina_Bool wasselected : 1;
  Eina_Bool no_select : 1;
  Eina_Bool bring_in : 1;
  Eina_Bool compress : 1;
  Eina_Bool homogeneous : 1;
  int item_width;
  int item_height;
  int max_items_per_block;
};

struct _Item_Block
{
  EINA_INLIST;
  int count;
  int num;
  Widget_Data *wd;
  Eina_List *items;
  Evas_Coord x, y, w, h, minw, minh;
  Eina_Bool realized : 1;
  Eina_Bool changed : 1;
  Eina_Bool updateme : 1;
  Eina_Bool showme : 1;
};

struct _Elm_Genlist_Item
{
  EINA_INLIST;
  Widget_Data *wd;
  Item_Block *block;
  Eina_List *items;
  Evas_Coord x, y, w, h, minw, minh;
  const Elm_Genlist_Item_Class *itc;
  const void *data;
  Elm_Genlist_Item *parent;
  Elm_Genlist_Item_Flags flags;
  struct{
    void (*func) (void *data, Evas_Object *obj, void *event_info);
    const void *data;
  } func;
  
  Evas_Object *base, *spacer;
  Eina_List *labels, *icons, *states, *icon_objs;
  Ecore_Timer *long_timer;
  Evas_Coord dx, dy;
  
  Elm_Genlist_Item *rel;
  int relcount;
  Eina_Bool before : 1;
  
  Eina_Bool realized : 1;
  Eina_Bool selected : 1;
  Eina_Bool hilighted : 1;
  Eina_Bool expanded : 1;
  Eina_Bool disabled : 1;
  Eina_Bool display_only : 1;
  Eina_Bool mincalcd : 1;
  Eina_Bool queued : 1;
  Eina_Bool showme : 1;
  Eina_Bool delete_me : 1;
  Eina_Bool down : 1;
  Eina_Bool dragging : 1;
  Eina_Bool updateme : 1;
};
#endif

EAPI Elm_Genlist_Item *
elm_genlist_item_first_subitem_get(const Elm_Genlist_Item *it){
  if(!it || !it->items) return NULL;
  return it->items->data;
}

EAPI Elm_Genlist_Item *
elm_genlist_subitem_next_get(const Elm_Genlist_Item *it){
  if(!it || !it->parent || !it->parent->items) return NULL;
  Eina_List *cur=eina_list_data_find_list(it->parent->items, it);
  if(!cur || !cur->next) return NULL;
  return cur->next->data;
}

EAPI Elm_Genlist_Item *
elm_genlist_subitem_prev_get(const Elm_Genlist_Item *it){
  if(!it || !it->parent || !it->parent->items) return NULL;
  Eina_List *cur=eina_list_data_find_list(it->parent->items, it);
  if(!cur || !cur->prev) return NULL;
  return cur->prev->data;
}

static void
_exp_end(void *data, Evas_Object *obj, void *event_info) {
  Elm_Genlist_Item *it = event_info;
  Elm_Genlist_Item *ch;
  for(ch=elm_genlist_item_first_subitem_get(it); ch; ch=elm_genlist_subitem_next_get(ch)){
    ch->realized=1;
    evas_object_show((Evas_Object*)elm_genlist_item_object_get(ch));
  }
}
static void
_con_end(void *data, Evas_Object *obj, void *event_info) {
  Elm_Genlist_Item *it = event_info;
  Elm_Genlist_Item *ch;
  for(ch=elm_genlist_item_first_subitem_get(it); ch; ch=elm_genlist_subitem_next_get(ch)){
    ch->realized=0;
    evas_object_hide((Evas_Object*)elm_genlist_item_object_get(ch));
  }
}

static void
_exp_req(void *data, Evas_Object *obj, void *event_info) {
  Elm_Genlist_Item *it = event_info;
  elm_genlist_item_expanded_set(it, 1);
}
static void
_con_req(void *data, Evas_Object *obj, void *event_info) {
  Elm_Genlist_Item *it = event_info;
  elm_genlist_item_expanded_set(it, 0);
}

EAPI void
elm_genlist_autoexpcon_mode_set(Evas_Object *obj, Elm_List_Mode mode){
  if(mode){
    evas_object_smart_callback_add(obj, "expand,request", _exp_req, NULL);
    evas_object_smart_callback_add(obj, "contract,request", _con_req, NULL);
    
    evas_object_smart_callback_add(obj, "expanded", _exp_end, NULL);
    evas_object_smart_callback_add(obj, "contracted", _con_end, NULL);
  }else{
    evas_object_smart_callback_del(obj, "expand,request", _exp_req);
    evas_object_smart_callback_del(obj, "contract,request", _con_req);
    
    evas_object_smart_callback_del(obj, "expanded", _exp_end);
    evas_object_smart_callback_del(obj, "contracted", _con_end);
  }
}
