/** ui_common.h --- 
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

#ifndef __UI_COMMON_H__
#define __UI_COMMON_H__

#ifndef _
#define _(...) __VA_ARGS__
#endif

#ifndef CONFIG_PATH
#define CONFIG_PATH "." NAME
#endif

#ifndef THEME_PATH
#define THEME_PATH "" NAME ".edj"
//#define THEME_PATH "/usr/share" "/" NAME "/" "default.edj"
#endif

#ifndef PHOTOS_PATH
#define PHOTOS_PATH CONFIG_PATH "/" "photos"
#endif

#ifndef EET_CONF_FILE
#define EET_CONF_FILE CONFIG_PATH "/" "config.eet"
#endif

#include"main.h"

#ifdef TEST_WIDGET_MODE
#define TEST_WIDGET(name, code, post)					\
  static void test_win_del(void *data, Evas_Object *obj,		\
			   void *event_info){ elm_exit(); }		\
  int main(int argc, char **argv) {					\
    Evas_Object *wn, *bg, *tw;						\
    elm_init(argc, argv);						\
    wn = elm_win_add(NULL, "main", ELM_WIN_BASIC);			\
    elm_win_title_set(wn, "Test Widget: " #name);			\
    evas_object_smart_callback_add(wn, "delete-request",		\
				   test_win_del, NULL);			\
    bg = elm_bg_add(wn);						\
    elm_win_resize_object_add(wn, bg);					\
    evas_object_size_hint_weight_set(bg, 1.0, 1.0);			\
    evas_object_show(bg);						\
    tw=elm_ ## name ## _add(wn);					\
    evas_object_size_hint_weight_set(tw, 1.0, 1.0);			\
    evas_object_size_hint_align_set(tw, -1.0, -1.0);			\
    elm_win_resize_object_add(wn, tw);					\
    evas_object_show(tw);						\
    code;								\
    evas_object_show(wn);						\
    elm_run();								\
    post;								\
    elm_shutdown();							\
    return 0;								\
  }
#else
#define TEST_WIDGET(name, code)
#endif

#endif
