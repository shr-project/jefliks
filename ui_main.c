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

#include"ui_common.h"
#include"ui_main.h"
#include"ui_config.h"
#include"ui_about.h"

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data{
  Evas_Object *parent, *box, *status;
};

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  Widget_Data *wd=data;
  free(wd);
}

static void
_exit_hook(void *data, Evas_Object *obj, void *event_info)
{
  elm_exit();
}

static void
_dialog_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  Widget_Data *wd=data;
  evas_object_show(wd->box);
}

static void
_config_hook(void *data, Evas_Object *obj, void *event_info)
{
  Widget_Data *wd=data;  
  Evas_Object *settings = elm_jabber_config_add(wd->parent);
  evas_object_size_hint_weight_set(settings, 1.0, 1.0);
  elm_win_resize_object_add(wd->parent, settings);
  evas_object_hide(wd->box);
  evas_object_show(settings);
  evas_object_event_callback_add(settings, EVAS_CALLBACK_FREE, _dialog_del, wd);
}

static void
_about_hook(void *data, Evas_Object *obj, void *event_info)
{
  Widget_Data *wd=data;  
  Evas_Object *about = elm_jabber_about_add(wd->parent);
  evas_object_size_hint_weight_set(about, 1.0, 1.0);
  elm_win_resize_object_add(wd->parent, about);
  //evas_object_hide(wd->box);
  evas_object_show(about);
  //evas_object_event_callback_add(about, EVAS_CALLBACK_FREE, _dialog_del, wd);
}

Evas_Object *elm_jabber_main(Evas_Object *parent){
  Widget_Data *wd;
  Evas_Object *box, *buttons, *status, *actions, *rl;
  
  wd = malloc(sizeof(Widget_Data));
  wd->parent=parent;
  
  /* Main box */
  box = elm_box_add(parent);
  wd->box=box;
  //evas_object_size_hint_weight_set(box, 1.0, 1.0);
  //evas_object_size_hint_align_set(box, -1.0, -1.0);
  evas_object_event_callback_add(box, EVAS_CALLBACK_FREE, _del_hook, wd);
  evas_object_show(box);
  
  /* Roster */
  
  rl = elm_anchorview_add(parent);
  evas_object_size_hint_weight_set(rl, 1.0, 1.0);
  evas_object_size_hint_align_set(rl, -1.0, -1.0);
  elm_anchorview_text_set(rl, "It's startup <a href=1>kayo@neko.im</a>");
  //elm_win_resize_object_add(wn, rl);
  elm_box_pack_end(box, rl);
  evas_object_show(rl);
  
  
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
  
  elm_hoversel_item_add(status, _("Offline"), NULL, 0, NULL, NULL);
  elm_hoversel_item_add(status, _("Online"), NULL, 0, NULL, NULL);
  elm_hoversel_item_add(status, _("Away"), NULL, 0, NULL, NULL);
  elm_hoversel_item_add(status, _("Extended Away"), NULL, 0, NULL, NULL);
  elm_hoversel_item_add(status, _("Do not disturb"), NULL, 0, NULL, NULL);
  
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
