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

static Elm_Genlist_Item_Class itcl;
typedef struct {
  Widget_Data *wd;
  char *jid;
  Jabber_Subscript sub;
} Roster_Item;

static char *it_label_get(const void *data, Evas_Object *obj, const char *part) {
  const Roster_Item *item=data;
  char buf[256];
  
  snprintf(buf, sizeof(buf), "%s (%s-%s)", item->jid, item->sub&JABBER_SUBSCRIPT_TO?"<":" ", item->sub&JABBER_SUBSCRIPT_FROM?">":" ");
  return strdup(buf);
}

static Evas_Object *it_icon_get(const void *data, Evas_Object *obj, const char *part) {
  /*
    const Roster_Item *item=data;
  char buf[PATH_MAX];
  Evas_Object *ic = elm_icon_add(obj);
  snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
  elm_icon_file_set(ic, buf, NULL);
  evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
  return ic;
  */
  return NULL;
}

static Eina_Bool it_state_get(const void *data, Evas_Object *obj, const char *part) {
  //const Roster_Item *item=data;
  return EINA_FALSE;
}

static void it_del(const void *data, Evas_Object *obj) {
  Roster_Item *item=(Roster_Item*)data;
  free(item->jid);
  free(item);
}

static void it_sel(void *data, Evas_Object *obj, void *event_info) {
  //Roster_Item *item=data;
  printf("sel item data [%p] on genlist obj [%p], item pointer [%p]\n", data, obj, event_info);
}


Evas_Object *elm_jabber_roster_add(Evas_Object *parent){
  Widget_Data *wd;
  
  wd = malloc(sizeof(Widget_Data));
  
  wd->list = elm_genlist_add(parent);
  evas_object_event_callback_add(wd->list, EVAS_CALLBACK_FREE, _del_hook, wd);
  evas_object_data_set(wd->list, "wd", wd);
  
  itcl.item_style     = "default";
  itcl.func.label_get = it_label_get;
  itcl.func.icon_get  = it_icon_get;
  itcl.func.state_get = it_state_get;
  itcl.func.del       = it_del;
  
  return wd->list;
}

#include<iksemel.h>

static void roster_reload(Widget_Data *wd, iks *data){
  iks *entry;
  
  elm_genlist_clear(wd->list);
  
  for(entry = iks_first_tag(data); entry; entry = iks_next_tag(entry)){
    char *tag=iks_name(entry);
    if(tag && !strcmp(tag, "item")){
      char *jid=iks_find_attrib(entry, "jid"), *sub=iks_find_attrib(entry, "subscription");
      if(jid){
	Roster_Item *item=malloc(sizeof(Roster_Item));
	item->wd=wd;
	item->jid=strdup(jid);
	
	if(sub){
	  if(!strcmp(sub, "to"))item->sub=JABBER_SUBSCRIPT_TO;
	  if(!strcmp(sub, "from"))item->sub=JABBER_SUBSCRIPT_FROM;
	  if(!strcmp(sub, "both"))item->sub=JABBER_SUBSCRIPT_BOTH;
	}else{
	  item->sub=0;
	}
	
	elm_genlist_item_append(wd->list, &itcl, item, NULL, ELM_GENLIST_ITEM_NONE, it_sel, item);
      }
      //iks_free(jid); iks_free(sub);
    }
    //iks_free(tag);
  }
}

static void
_roster_event_hook(Widget_Data *wd, Jabber_Session *sess, iks *node){
  char *tag=iks_name(node);
  
  if(!strcmp(tag, "presence")); // presence
  else if(!strcmp(tag, "query")){
    roster_reload(wd, node);
    /*
    char *xmlns=iks_find_attrib(node, "xmlns");
    if(xmlns){
      if(!strcmp(xmlns, "jabber:iq:roster")) roster_reload(wd, node);
    }
    */
    
    //iks_free(id);
  }
  //iks_free(tag);
}

int elm_jabber_roster_register(Evas_Object *roster, Jabber_Session *sess){
  Widget_Data *wd=evas_object_data_get(roster, "wd");
  wd->jabber=sess;
  jabber_roster_callback_set(wd->jabber, (Jabber_Callback)_roster_event_hook, wd);
  return 1;
}
