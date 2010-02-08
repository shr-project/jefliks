/** ui_roster.c --- 
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
#include"ui_roster.h"

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data{
  Evas_Object *list;
  Jabber_Session *jabber;
  Eina_List *jids;
};

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  free(wd);
}

/* Show {{{ */
static struct {
  Jabber_Show show;
  const char *icon;
} _show_icons_[] = {
  { JABBER_UNAVAILABLE, "status/unavailable" },
  { JABBER_AVAILABLE, "status/available" },
  { JABBER_CHAT, "status/chat" },
  { JABBER_AWAY, "status/away" },
  { JABBER_XA, "status/xa" },
  { JABBER_DND, "status/dnd" },
  { JABBER_UNDEFINED, NULL }
};
static const char *icon_by_show(Jabber_Show show){
  int i;
  for(i=0; _show_icons_[i].icon; i++){
    if(show==_show_icons_[i].show) return _show_icons_[i].icon;
  }
  return NULL;
}
/* Show }}} */


/* Roster Item {{{ */

typedef void (*GenlistItemSelectFunc) (void *data, Evas_Object *obj, void *event_info);

typedef enum _Roster_Item_Type Roster_Item_Type;
enum _Roster_Item_Type{
  ROSTER_ITEM_GRP = 1,
  ROSTER_ITEM_JID,
  ROSTER_ITEM_RES
};

typedef struct _Roster_Item Roster_Item;
typedef struct _Roster_Item_Grp Roster_Item_Grp;
typedef struct _Roster_Item_Jid Roster_Item_Jid;
typedef struct _Roster_Item_Res Roster_Item_Res;

struct _Roster_Item{
  // Common fields
  Roster_Item_Type type; // type of entry
  Widget_Data *wd; // Widget Data
  Elm_Genlist_Item *it; // Item (if visible)
};

struct _Roster_Item_Grp{
  Roster_Item_Type type;
  Widget_Data *wd;
  Elm_Genlist_Item *it;
  // Specific fields
  char exp;
  char *grp;
  Eina_List *jid;
};

struct _Roster_Item_Jid{
  Roster_Item_Type type;
  Widget_Data *wd;
  Elm_Genlist_Item *it;
  // Specific fields
  char exp;
  char *jid;
  Jabber_Subscript sub;
  Roster_Item_Grp *par;
  Eina_List *res;
};

struct _Roster_Item_Res{
  Roster_Item_Type type;
  Widget_Data *wd;
  Elm_Genlist_Item *it;
  // Specific fields
  char *res;
  int pri;
  char show;
  char *desc;
  Roster_Item_Jid *par;
};

/* Class: Item_Grp {{{ */

static void _item_grp_del(Roster_Item_Grp *item, Evas_Object *obj) {
  //free(item->grp);
  //free(item);
}

static char *_item_grp_label_get(const Roster_Item_Grp *item, Evas_Object *obj, const char *part) {
  char buf[256];
  
  snprintf(buf, sizeof(buf), "%s", item->grp);
  
  return strdup(buf);
}

static Evas_Object *_item_grp_icon_get(const Roster_Item_Grp *item, Evas_Object *obj, const char *part) {
  return NULL;
}

static Eina_Bool _item_grp_state_get(const Roster_Item_Grp *item, Evas_Object *obj, const char *part) {
  return EINA_FALSE;
}

static void _item_grp_sel(const Roster_Item_Grp *item, Evas_Object *obj, void *event_info) {
  printf("Selected Group %s\n", item->grp);
}

static const Elm_Genlist_Item_Class _item_grp_class={
  .item_style  = "default",
  .func = {
    .label_get = (GenlistItemLabelGetFunc)_item_grp_label_get,
    .icon_get  = (GenlistItemIconGetFunc)_item_grp_icon_get,
    .state_get = (GenlistItemStateGetFunc)_item_grp_state_get,
    .del       = (GenlistItemDelFunc)_item_grp_del
  }
};

/* Class: Item_Grp }}} */

/* Class: Item_Jid {{{ */

static void _item_jid_del(Roster_Item_Jid *item, Evas_Object *obj) {
  //free(item->jid);
  //free(item);
}

static char *_item_jid_label_get(const Roster_Item_Jid *item, Evas_Object *obj, const char *part) {
  char buf[256];
  
  if(!strcmp(part, "elm.text")){
    snprintf(buf, sizeof(buf), "%s (%s-%s)", item->jid, item->sub&JABBER_SUBSCRIPT_TO?"<":" ", item->sub&JABBER_SUBSCRIPT_FROM?">":" ");
  }
  if(!strcmp(part, "elm.text.sub")){
    if(item->res && item->res->data){
      const Roster_Item_Res *res_item=item->res->data;
      snprintf(buf, sizeof(buf), "%s [%d]", res_item->res, res_item->pri);
    }else{
      buf[0]='\0';
    }
  }
  
  return strdup(buf);
}

static Evas_Object *_item_jid_icon_get(const Roster_Item_Jid *item, Evas_Object *obj, const char *part) {
  if(!strcmp(part, "elm.swallow.icon")){
    Jabber_Show show=JABBER_UNAVAILABLE;
    if(item->res && item->res->data){
      const Roster_Item_Res *res_item=item->res->data;
      show=res_item->show;
    }
    const char *name=icon_by_show(show);
    
    if(name){
      Evas_Object *icon = elm_icon_add(obj);
      elm_icon_file_set(icon, "./" NAME ".edj", name);
      evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
      return icon;
    }
  }
  return NULL;
}

static Eina_Bool _item_jid_state_get(const Roster_Item_Jid *item, Evas_Object *obj, const char *part) {
  return EINA_FALSE;
}

static void _item_jid_sel(const Roster_Item_Jid *item, Evas_Object *obj, void *event_info) {
  printf("Selected Item %s\n", item->jid);
}

static const Elm_Genlist_Item_Class _item_jid_class={
  .item_style  = "double_label",
  .func = {
    .label_get = (GenlistItemLabelGetFunc)_item_jid_label_get,
    .icon_get  = (GenlistItemIconGetFunc)_item_jid_icon_get,
    .state_get = (GenlistItemStateGetFunc)_item_jid_state_get,
    .del       = (GenlistItemDelFunc)_item_jid_del
  }
};

/*
static Elm_Genlist_Item *item_jid_get(Widget_Data *wd, const char *jid){
  Elm_Genlist_Item *it;
  
  for(it=elm_genlist_first_item_get(wd->list); it; it=elm_genlist_item_next_get(it)){
    const Roster_Item_Jid *item=elm_genlist_item_data_get(it);
    if(item->type!=ROSTER_ITEM_JID)continue;
    if(!strcmp(item->jid, jid))return it;
  }
  
  return NULL;
}

static Elm_Genlist_Item *item_jid_set(Widget_Data *wd, const char *jid, const char *sub){
  Elm_Genlist_Item *it=item_jid_get(wd, jid);
  Roster_Item_Jid *item;
  
  if(it){
    item=(Roster_Item_Jid *)elm_genlist_item_data_get(it);
  }else{
    item=malloc(sizeof(Roster_Item_Jid));
    
    item->type=ROSTER_ITEM_JID;
    item->wd=wd;
    item->jid=strdup(jid);
    item->res=NULL;
    item->sub=0;
  }
  
  if(it){
    elm_genlist_item_update(it);
  }else{
    it=elm_genlist_item_append(wd->list, &_item_jid_class, item, NULL, ELM_GENLIST_ITEM_SUBITEMS,
			       (GenlistItemSelectFunc)_item_jid_sel, item);
  }
  
  return it;
}
*/

/* Class: Item_Jid }}} */

/* Class: Item_Res {{{ */

static void _item_res_del(Roster_Item_Res *item, Evas_Object *obj) {
  //free(item->res);
  //free(item->desc);
  //free(item);
}

static char *_item_res_label_get(const Roster_Item_Res *item, Evas_Object *obj, const char *part) {
  char buf[256];
  
  if(!strcmp(part, "elm.text")){
    snprintf(buf, sizeof(buf), "%s [%d]", item->res, item->pri);
  }
  if(!strcmp(part, "elm.text.sub")){
    snprintf(buf, sizeof(buf), "%s", item->desc);
  }
  
  return strdup(buf);
}

static Evas_Object *_item_res_icon_get(const Roster_Item_Res *item, Evas_Object *obj, const char *part) {
  if(!strcmp(part, "elm.swallow.icon")){
    const char *name=icon_by_show(item->show);
    if(name){
      Evas_Object *icon = elm_icon_add(obj);
      elm_icon_file_set(icon, "./" NAME ".edj", name);
      evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
      return icon;
    }
  }
  return NULL;
}

static Eina_Bool _item_res_state_get(const Roster_Item_Res *item, Evas_Object *obj, const char *part) {
  return EINA_FALSE;
}

static void _item_res_sel(const Roster_Item_Res *item, Evas_Object *obj, void *event_info) {
  printf("Selected Resource %s with pri:%d desc:%s\n", item->res, item->pri, item->desc);
}

static const Elm_Genlist_Item_Class _item_res_class={
  .item_style  = "double_label",
  .func = {
    .label_get = (GenlistItemLabelGetFunc)_item_res_label_get,
    .icon_get  = (GenlistItemIconGetFunc)_item_res_icon_get,
    .state_get = (GenlistItemStateGetFunc)_item_res_state_get,
    .del       = (GenlistItemDelFunc)_item_res_del
  }
};

/*
static Elm_Genlist_Item *item_res_get(Widget_Data *wd, const char *jid, const char *res){
  Elm_Genlist_Item *it;//=item_jid_get(wd, jid);
  
  //if(!it)return NULL;
  for(it=elm_genlist_first_item_get(wd->list); it; it=elm_genlist_item_next_get(it)){
    const Roster_Item_Res *item=elm_genlist_item_data_get(it);
    if(item->type!=ROSTER_ITEM_RES)continue;
    if(item->par && item->par->jid && !strcmp(item->par->jid, jid)){
      return it;
    }
  }
  
  return NULL;
}

static void items_print(Widget_Data *wd){
  Elm_Genlist_Item *it;
  
  printf("ENTRY {\n");
  for(it=elm_genlist_first_item_get(wd->list); it; it=elm_genlist_item_next_get(it)){
    const Roster_Item_Jid *item=elm_genlist_item_data_get(it);
    if(item->type==ROSTER_ITEM_RES)printf(" >> RES slf:0x%x par:0x%x <<\n", it, elm_genlist_item_parent_get(it));
    if(item->type==ROSTER_ITEM_JID)printf(" >> JID slf:0x%x par:0x%x <<\n", it, elm_genlist_item_parent_get(it));
  }
  printf("}\n");
}

static Elm_Genlist_Item *item_res_set(Widget_Data *wd, const char* jid, const char *res, int pri, Jabber_Show show, const char *desc){
  Elm_Genlist_Item *pr=item_jid_get(wd, jid);
  if(!pr)return NULL;
  
  Roster_Item_Jid *pr_item=(Roster_Item_Jid *)elm_genlist_item_data_get(pr);
  if(!pr_item)return NULL;
  
  Elm_Genlist_Item *it=item_res_get(wd, jid, res);
  Roster_Item_Res *item;
  
  if(it){
    printf(">> Resource Upd\n");
    item=(Roster_Item_Res *)elm_genlist_item_data_get(it);
  }else{
    printf(">> Resource Add\n");
    item=malloc(sizeof(Roster_Item_Res));
    
    item->type=ROSTER_ITEM_RES;
    item->wd=wd;
    item->res=strdup(res);
    item->par=pr_item;
    
    item->desc=NULL;
  }
  
  item->pri=pri;
  item->show=show;
  free(item->desc);
  item->desc=strdup(desc);
  
  if(!pr_item->res || pr_item->res->pri < item->pri){ // update jid entry
    pr_item->res=item;
  }
  elm_genlist_item_update(pr);
  
  // show resource
  if(it){
    elm_genlist_item_update(it);
  }else{
    it=elm_genlist_item_append(wd->list, &_item_res_class, item, pr, ELM_GENLIST_ITEM_NONE,
			       (GenlistItemSelectFunc)_item_res_sel, item);
    if(!elm_genlist_item_expanded_get(pr)){
      evas_object_hide((Evas_Object*)elm_genlist_item_object_get(it));
    }
  }
  
  items_print(wd);
  
  return it;
}
*/

/* Class: Item_Res }}} */

static Roster_Item_Jid *item_jid_fnd(Widget_Data *wd, const char* jid){
  if(!wd || !jid) return NULL;
  Eina_List *entry;
  for(entry=wd->jids; entry; entry=entry->next){
    Roster_Item_Jid *item=entry->data;
    if(item && item->type==ROSTER_ITEM_JID && !strcmp(item->jid, jid)) return item;
  }
  return NULL;
}

static Roster_Item_Res *item_res_fnd(Roster_Item_Jid *jid_item, const char *res){
  if(!jid_item || !res) return NULL;
  Eina_List *entry;
  for(entry=jid_item->res; entry; entry=entry->next){
    Roster_Item_Res *item=entry->data;
    if(item && item->type==ROSTER_ITEM_RES && !strcmp(item->res, res)) return item;
  }
  return NULL;
}

static Roster_Item_Res *item_res_add(Roster_Item_Jid *jid_item, const char *res){
  if(!jid_item || !res) return NULL;
  Roster_Item_Res *item=malloc(sizeof(Roster_Item_Res));
  
  item->type=ROSTER_ITEM_RES;
  item->wd=NULL;
  item->it=NULL;
  item->desc=NULL;
  
  item->res=strdup(res);
  item->par=jid_item;
  
  jid_item->res=eina_list_append(jid_item->res, item);
  
  return item;
}

static void item_res_del(Roster_Item_Jid *jid_item, Roster_Item_Res *res_item){
  if(!jid_item || !res_item) return;
  jid_item->res=eina_list_remove(jid_item->res, res_item); // remove from list
  /* free data */
  free(res_item->res);
  free(res_item->desc);
  free(res_item);
}

static void item_res_clr(Roster_Item_Jid *jid_item){
  if(!jid_item || !jid_item->res) return;
  Eina_List *entry;
  for(entry=jid_item->res; entry; entry=jid_item->res){
    Roster_Item_Res *res_item=entry->data;
    item_res_del(jid_item, res_item);
  }
  jid_item->res=NULL;
}

static int item_res_cmp(const Roster_Item_Res *a, const Roster_Item_Res *b){
  int s=b->pri-a->pri;
  return s<0?-1:(s>0?1:0);
}

static void item_res_srt(Roster_Item_Jid *jid_item){
  jid_item->res=eina_list_sort(jid_item->res, eina_list_count(jid_item->res), (Eina_Compare_Cb)item_res_cmp);
}

static void item_res_upd(Roster_Item_Res *res_item){
  if(!res_item->it) return;
  elm_genlist_item_update(res_item->it);
}

static void item_jid_upd(Roster_Item_Jid *jid_item){
  if(!jid_item->it) return;
  if(jid_item->res) {
    elm_genlist_item_update(jid_item->it);
    
    if(jid_item->exp){
      Eina_List *entry;
      elm_genlist_item_subitems_clear(jid_item->it);
      
      for(entry=jid_item->res; entry; entry=entry->next){
	Roster_Item_Res *res_item=entry->data;
	if(!res_item || res_item->type!=ROSTER_ITEM_RES) continue;
	res_item->it=elm_genlist_item_append(jid_item->wd->list, &_item_res_class, res_item, jid_item->it,
					     ELM_GENLIST_ITEM_NONE, (GenlistItemSelectFunc)_item_res_sel, res_item);
      }
    }
  }
}

static void item_res_set(Widget_Data *wd, const char* jid, const char *res, int pri, Jabber_Show show, const char *desc){
  Roster_Item_Jid *jid_item=item_jid_fnd(wd, jid);
  if(!jid_item) return;
  Roster_Item_Res *res_item=item_res_fnd(jid_item, res);
  // if unavailable delete resource end exit
  if(show==JABBER_UNAVAILABLE){
    if(res_item) item_res_del(jid_item, res_item);
  }else{
    // create if not exists
    if(!res_item) res_item=item_res_add(jid_item, res);
    if(!res_item) return;
    // update data
    res_item->pri=pri;
    res_item->show=show;
    free(res_item->desc);
    res_item->desc=strdup(desc);
  }
  item_res_srt(jid_item);
  item_jid_upd(jid_item);
}

static Roster_Item_Jid *item_jid_add(Widget_Data *wd, const char *jid){
  if(!wd || !jid) return NULL;
  Roster_Item_Jid *item=malloc(sizeof(Roster_Item_Jid));
  
  item->type=ROSTER_ITEM_JID;
  item->wd=NULL;
  item->it=NULL;
  item->res=NULL;
  item->exp=0;
  
  item->jid=strdup(jid);
  item->par=NULL;
  
  wd->jids=eina_list_append(wd->jids, item);
  
  return item;
}

static void item_jid_del(Widget_Data *wd, Roster_Item_Jid *jid_item){
  if(!wd || !jid_item) return;
  wd->jids=eina_list_remove(wd->jids, jid_item); // remove from list
  /* free data */
  item_res_clr(jid_item);
  free(jid_item->jid);
  free(jid_item);
}

static void item_jid_clr(Widget_Data *wd){
  if(!wd)return;
  Eina_List *entry;
  for(entry=wd->jids; entry; entry=wd->jids){
    Roster_Item_Jid *jid_item=entry->data;
    item_jid_del(wd, jid_item);
  }
  wd->jids=NULL;
}

static void list_upd(Widget_Data *wd){
  if(!wd) return;
  elm_genlist_clear(wd->list);
  Eina_List *entry;
  for(entry=wd->jids; entry; entry=entry->next){
    Roster_Item_Jid *jid_item=entry->data;
    jid_item->it=elm_genlist_item_append(wd->list, &_item_jid_class, jid_item, NULL,
					 ELM_GENLIST_ITEM_SUBITEMS,
					 (GenlistItemSelectFunc)_item_jid_sel, jid_item);
  }
}

static void item_jid_set(Widget_Data *wd, const char *jid, Jabber_Subscript sub){
  Roster_Item_Jid *jid_item=item_jid_fnd(wd, jid);
  if(!jid_item) jid_item=item_jid_add(wd, jid);
  
  jid_item->wd=wd;
  jid_item->sub=sub;
  
  list_upd(wd);  
}

static Eina_Bool item_jid_exp_req(const Roster_Item_Jid *item){
  return item->res!=NULL;
}

static Eina_Bool item_jid_con_req(const Roster_Item_Jid *item){
  return 0;
}

static void item_jid_exp(Roster_Item_Jid *item){
  item->exp=1;
  item_jid_upd(item);
}

static void item_jid_con(Roster_Item_Jid *item){
  item->exp=0;
  item_jid_upd(item);
}

static Eina_Bool item_grp_exp_req(const Roster_Item_Grp *item){
  return item->jid!=NULL;
}

static Eina_Bool item_grp_con_req(const Roster_Item_Grp *item){
  return 0;
}

static void item_grp_exp(Roster_Item_Grp *item){
  item->exp=1;
}

static void item_grp_con(Roster_Item_Grp *item){
  item->exp=0;
}


static void
_exp_end(void *data, Evas_Object *obj, void *event_info) {
  Elm_Genlist_Item *it = event_info;
  Roster_Item *item = (Roster_Item *)elm_genlist_item_data_get(it);
  switch(item->type){
  case ROSTER_ITEM_GRP: item_grp_exp((Roster_Item_Grp *)item); break;
  case ROSTER_ITEM_JID: item_jid_exp((Roster_Item_Jid *)item); break;
  case ROSTER_ITEM_RES: break;
  }
}
static void
_con_end(void *data, Evas_Object *obj, void *event_info) {
  Elm_Genlist_Item *it = event_info;
  Roster_Item *item = (Roster_Item *)elm_genlist_item_data_get(it);
  switch(item->type){
  case ROSTER_ITEM_GRP: item_grp_con((Roster_Item_Grp *)item); break;
  case ROSTER_ITEM_JID: item_jid_con((Roster_Item_Jid *)item); break;
  case ROSTER_ITEM_RES: break;
  }
  elm_genlist_item_subitems_clear(it);
}

static void
_exp_req(void *data, Evas_Object *obj, void *event_info) {
  Elm_Genlist_Item *it = event_info;
  Roster_Item *item = (Roster_Item *)elm_genlist_item_data_get(it);
  Eina_Bool st=1;
  switch(item->type){
  case ROSTER_ITEM_GRP: st=item_grp_exp_req((Roster_Item_Grp *)item); break;
  case ROSTER_ITEM_JID: st=item_jid_exp_req((Roster_Item_Jid *)item); break;
  case ROSTER_ITEM_RES: break;
  }
  elm_genlist_item_expanded_set(it, st);
}
static void
_con_req(void *data, Evas_Object *obj, void *event_info) {
  Elm_Genlist_Item *it = event_info;
  Roster_Item *item = (Roster_Item *)elm_genlist_item_data_get(it);
  Eina_Bool st=0;
  switch(item->type){
  case ROSTER_ITEM_GRP: st=item_grp_con_req((Roster_Item_Grp *)item); break;
  case ROSTER_ITEM_JID: st=item_jid_con_req((Roster_Item_Jid *)item); break;
  case ROSTER_ITEM_RES: break;
  }
  elm_genlist_item_expanded_set(it, st);
}

/* Roster Item }}} */


Evas_Object *elm_jabber_roster_add(Evas_Object *parent){
  Widget_Data *wd;
  
  wd = malloc(sizeof(Widget_Data));
  
  wd->jids=NULL;
  
  wd->list = elm_genlist_add(parent);
  evas_object_event_callback_add(wd->list, EVAS_CALLBACK_FREE, _del_hook, wd);
  elm_genlist_compress_mode_set(wd->list, 1);
  evas_object_data_set(wd->list, "wd", wd);
  
  evas_object_smart_callback_add(wd->list, "expand,request", _exp_req, NULL);
  evas_object_smart_callback_add(wd->list, "contract,request", _con_req, NULL);
  
  evas_object_smart_callback_add(wd->list, "expanded", _exp_end, NULL);
  evas_object_smart_callback_add(wd->list, "contracted", _con_end, NULL);
  
  return wd->list;
}

void elm_jabber_roster_clear(Evas_Object *roster){
  Widget_Data *wd=evas_object_data_get(roster, "wd");
  elm_genlist_clear(wd->list);
  item_jid_clr(wd);
}

#include<iksemel.h>

static void roster_reload(Widget_Data *wd, ikspak *pak){
  iks *entry;
  
  item_jid_clr(wd);
  
  for(entry = iks_first_tag(iks_first_tag(pak->x)); entry; entry = iks_next_tag(entry)){
    char *tag=iks_name(entry);
    if(tag && !strcmp(tag, "item")){
      char *jid=iks_find_attrib(entry, "jid"), *subscr=iks_find_attrib(entry, "subscription");
      if(jid){
	Jabber_Subscript sub=JABBER_SUBSCRIPT_NONE;
	if(subscr){
	  if(!strcmp(subscr, "to"))sub=JABBER_SUBSCRIPT_TO;
	  if(!strcmp(subscr, "from"))sub=JABBER_SUBSCRIPT_FROM;
	  if(!strcmp(subscr, "both"))sub=JABBER_SUBSCRIPT_BOTH;
	}
	item_jid_set(wd, jid, sub);
      }
    }
  }
}

static void roster_status(Widget_Data *wd, ikspak *pak){
  Jabber_Show show=JABBER_UNDEFINED;
  int pri;
  char *tmp;
  
  switch(pak->show){
#define C(name) case IKS_SHOW_ ## name: show=JABBER_ ## name; break;
    C(UNAVAILABLE);
    C(AVAILABLE);
    C(CHAT);
    C(AWAY);
    C(XA);
    C(DND);
#undef C
  }
  
  /* get priority */
  tmp = iks_find_cdata (pak->x, "priority");
  pri = tmp?atoi(tmp):0;
  
  /* get status */
  tmp = iks_find_cdata (pak->x, "status");
  if(!tmp)tmp="";
  
  item_res_set(wd, pak->from->partial, pak->from->resource, pri, show, tmp);
}

static void
_roster_event_hook(Widget_Data *wd, Jabber_Session *sess, ikspak *pak){
  if(pak->type==IKS_PAK_PRESENCE){
    if(pak->subtype==IKS_TYPE_AVAILABLE || pak->subtype==IKS_TYPE_UNAVAILABLE){
      roster_status(wd, pak);
    }
  }
  if(pak->type==IKS_PAK_IQ){
    if(pak->subtype==IKS_TYPE_RESULT){
      if(!strcmp(pak->id, "roster")){
	roster_reload(wd, pak);
      }
    }
  }
}

int elm_jabber_roster_register(Evas_Object *roster, Jabber_Session *sess){
  Widget_Data *wd=evas_object_data_get(roster, "wd");
  wd->jabber=sess;
  jabber_roster_callback_set(wd->jabber, (Jabber_Callback)_roster_event_hook, wd);
  return 1;
}
