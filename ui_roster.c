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
  Evas_Object *node;
};

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  free(wd);
}

Evas_Object *elm_jabber_roster_add(Evas_Object *parent){
  Widget_Data *wd;
  Evas_Object *roster;
  
  wd = malloc(sizeof(Widget_Data));
  roster = elm_genlist_add(parent);
  evas_object_event_callback_add(roster, EVAS_CALLBACK_FREE, _del_hook, wd);
  
  return roster;
}

/*
int elm_jabber_roster_set(Evas_Object *roster, const iks *data){
  
}

int elm_jabber_roster_get(Evas_Object *roster, iks **data){
  
}
*/
