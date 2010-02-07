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
};

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  free(wd);
}


/* Roster Item {{{ */

typedef void (*GenlistItemSelectFunc) (void *data, Evas_Object *obj, void *event_info);

typedef enum _Roster_Item_Type Roster_Item_Type;
enum _Roster_Item_Type{
  ROSTER_ITEM_ROOT = 0,
  ROSTER_ITEM_GRP,
  ROSTER_ITEM_JID,
  ROSTER_ITEM_RES
};

typedef struct _Roster_Item_Grp Roster_Item_Grp;
typedef struct _Roster_Item_Jid Roster_Item_Jid;
typedef struct _Roster_Item_Res Roster_Item_Res;

struct _Roster_Item_Grp{
  Roster_Item_Type type;
  Widget_Data *wd;
  char *grp;
};

struct _Roster_Item_Jid{
  Roster_Item_Type type;
  Widget_Data *wd;
  char *jid;
  Jabber_Subscript sub;
  Roster_Item_Res *res;
  Roster_Item_Grp *par;
};

struct _Roster_Item_Res{
  Roster_Item_Type type;
  Widget_Data *wd;
  char *res;
  int pri;
  char show;
  char *desc;
  Roster_Item_Jid *par;
};

/* Class: Item_Grp {{{ */

static void _item_grp_del(Roster_Item_Grp *item, Evas_Object *obj) {
  free(item->grp);
  free(item);
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
  free(item->jid);
  free(item);
}

static char *_item_jid_label_get(const Roster_Item_Jid *item, Evas_Object *obj, const char *part) {
  char buf[256];
  
  if(!strcmp(part, "elm.text")){
    snprintf(buf, sizeof(buf), "%s (%s-%s)", item->jid, item->sub&JABBER_SUBSCRIPT_TO?"<":" ", item->sub&JABBER_SUBSCRIPT_FROM?">":" ");
  }
  if(!strcmp(part, "elm.text.sub")){
    if(item->res){
      int start=strlen(buf)+1;
      snprintf(buf+start, sizeof(buf)-start, "%s [%d]", item->res->res, item->res->pri);
    }else{
      buf[0]='\0';
    }
  }
  
  return strdup(buf);
}

static Evas_Object *_item_jid_icon_get(const Roster_Item_Jid *item, Evas_Object *obj, const char *part) {
  /*
  char buf[PATH_MAX];
  Evas_Object *ic = elm_icon_add(obj);
  snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
  elm_icon_file_set(ic, buf, NULL);
  evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
  return ic;
  */
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
  
  if(sub){
    if(!strcmp(sub, "to"))item->sub=JABBER_SUBSCRIPT_TO;
    if(!strcmp(sub, "from"))item->sub=JABBER_SUBSCRIPT_FROM;
    if(!strcmp(sub, "both"))item->sub=JABBER_SUBSCRIPT_BOTH;
  }
  
  if(it){
    elm_genlist_item_update(it);
  }else{
    it=elm_genlist_item_append(wd->list, &_item_jid_class, item, NULL, ELM_GENLIST_ITEM_SUBITEMS,
			       (GenlistItemSelectFunc)_item_jid_sel, item);
  }
  
  return it;
}

/* Class: Item_Jid }}} */

/* Class: Item_Res {{{ */

static void _item_res_del(Roster_Item_Res *item, Evas_Object *obj) {
  free(item->res);
  free(item->desc);
  free(item);
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

static Elm_Genlist_Item *item_res_set(Widget_Data *wd, const char* jid, const char *res, int pri, Jabber_Show show, const char *desc){
  Elm_Genlist_Item *pr=item_jid_get(wd, jid);
  Elm_Genlist_Item *it=item_res_get(wd, jid, res);
  Roster_Item_Res *item;
  
  if(!pr)return NULL;
  
  if(it){
    item=(Roster_Item_Res *)elm_genlist_item_data_get(it);
  }else{
    item=malloc(sizeof(Roster_Item_Res));
    
    item->type=ROSTER_ITEM_RES;
    item->wd=wd;
    item->res=strdup(res);
    item->par=(Roster_Item_Jid *)elm_genlist_item_data_get(pr);
    
    item->desc=NULL;
  }
  
  item->pri=pri;
  item->show=show;
  free(item->desc);
  item->desc=strdup(desc);
  
  if(it){
    elm_genlist_item_update(it);
  }else{
    it=elm_genlist_item_append(wd->list, &_item_res_class, item, pr, ELM_GENLIST_ITEM_NONE,
			       (GenlistItemSelectFunc)_item_res_sel, item);
  }
  
  return it;
}

/* Class: Item_Res }}} */

/* Roster Item }}} */

Evas_Object *elm_jabber_roster_add(Evas_Object *parent){
  Widget_Data *wd;
  
  wd = malloc(sizeof(Widget_Data));
  
  wd->list = elm_genlist_add(parent);
  evas_object_event_callback_add(wd->list, EVAS_CALLBACK_FREE, _del_hook, wd);
  evas_object_data_set(wd->list, "wd", wd);
  
  return wd->list;
}

#include<iksemel.h>

static void roster_reload(Widget_Data *wd, ikspak *pak){
  iks *entry;
  
  elm_genlist_clear(wd->list);
  
  for(entry = iks_first_tag(iks_first_tag(pak->x)); entry; entry = iks_next_tag(entry)){
    char *tag=iks_name(entry);
    if(tag && !strcmp(tag, "item")){
      char *jid=iks_find_attrib(entry, "jid"), *sub=iks_find_attrib(entry, "subscription");
      if(jid){
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
