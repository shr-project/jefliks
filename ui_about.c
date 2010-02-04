/** ui_about.c --- 
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
#include"ui_about.h"

static void
_close_hook(void *data, Evas_Object *obj, void *event_info)
{
  Evas_Object *about=data;
  evas_object_del(about);
}

Evas_Object *elm_jabber_about_add(Evas_Object *parent){
  Evas_Object *about, *vbox, *hbox, *close, *logo, *text;
  
  /* Logo Image */
  logo = elm_image_add(parent);
  //elm_image_no_scale_set(logo, 1);
  elm_image_scale_set(logo, 0.4, 1.0);
  
  evas_object_size_hint_weight_set(logo, 0.1, 0.1);
  evas_object_size_hint_align_set(logo, -1.0, -1.0);
  
  if(!elm_image_file_set(logo, "logo.png", NULL))printf("Logo not found!\n");
  evas_object_show(logo);
  
  /* About Text */
  text = elm_anchorview_add(parent);
  elm_anchorview_text_set(text, NAME " - <a href=efl>EFL</a>-based <a href=iksemel>Iksemel</a>-powered Jabber Client" "<br><br>" "Authors:" "<br>" "  Phoenix Kayo <a href=1>kayo.k11.4@gmail.com</a>" "<br><br><br>" "Have fun!");
  evas_object_size_hint_weight_set(text, 1.0, 1.0);
  evas_object_size_hint_align_set(text, -1.0, -1.0);
  evas_object_show(text);
  
  /* Horizontal SubSet */
  /*
  hbox = elm_box_add(parent);
  elm_box_horizontal_set(hbox, 1);
  elm_box_pack_end(hbox, logo);
  elm_box_pack_end(hbox, text);
  evas_object_show(hbox);
  */
  
  /* Close Button */
  close = elm_button_add(parent);
  elm_button_label_set(close, _("Close"));
  evas_object_show(close);
  
  /* Vertical SubSet */
  vbox = elm_box_add(parent);
  /*elm_box_pack_end(vbox, hbox);*/
  elm_box_pack_end(vbox, logo);
  elm_box_pack_end(vbox, text);
  elm_box_pack_end(vbox, close);
  evas_object_show(vbox);
  
  /* Main Inwin */
  about = elm_win_inwin_add(parent);
  elm_win_inwin_activate(about);
  elm_win_inwin_content_set(about, vbox);
  
  /* Close Handler */
  evas_object_smart_callback_add(close, "clicked", _close_hook, about);
  
  return about;
}
