#include<Elementary.h>

#include"ui_config.h"



/* this is your elementary main function - it MUSt be called IMMEDIATELY
 * after elm_init() and MUSt be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */

static void
main_win_del(void *data, Evas_Object *obj, void *event_info){
   /* called when my_win_main is requested to be deleted */
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

/*
static const char *main_win_theme_pretends[]={
  "./" NAME ".edj",
  "~/." NAME "/theme.edj",
  "/usr/share/" NAME "/theme.edj",
  NULL
};

static const char *
main_win_theme(){
  int i;
  for(i=0;main_win_theme_pretends[i];i++)
    if(edje_file_group_exists(main_win_theme_pretends[i], "icon"))
      return main_win_theme_pretends[i];
  return NULL;
}
*/


#define DBGMSG(m) printf("DBG: " #m "..\n");

#ifndef TEST_WIDGET_MODE
int main(int argc, char **argv){
  Evas_Object *wn, *bg, *rl, *cf, *pc, *bx, *bs;
  //const char *tf;
  
  eet_init();
  /* put ere any init specific to this app like parsing args etc. */
  elm_init(argc, argv);
  
  //tf = main_win_theme();
  
  wn = elm_win_add(NULL, "main", ELM_WIN_BASIC);
  elm_win_title_set(wn, "Elementary Jabber Client");
  //elm_win_autodel_set(wn, 1);
  evas_object_smart_callback_add(wn, "delete-request", main_win_del, NULL);
  
  /* Background */
  bg = elm_bg_add(wn);
  elm_win_resize_object_add(wn, bg);
  evas_object_size_hint_weight_set(bg, 1.0, 1.0);
  evas_object_show(bg);
  
  /* Main box */
  bx = elm_box_add(wn);
  evas_object_size_hint_weight_set(bx, 1.0, 1.0);
  evas_object_size_hint_align_set(rl, -1.0, -1.0);
  elm_win_resize_object_add(wn, bx);
  evas_object_show(bx);
  
  /* Roster */
  rl = elm_anchorview_add(wn);
  evas_object_size_hint_weight_set(rl, 1.0, 1.0);
  evas_object_size_hint_align_set(rl, -1.0, -1.0);
  elm_anchorview_text_set(rl, "It's startup <a href=1>kayo@neko.im</a>");
  //elm_win_resize_object_add(wn, rl);
  elm_box_pack_end(bx, rl);
  evas_object_show(rl);
  
  /* Buttons */
  bs = elm_box_add(wn);
  elm_box_horizontal_set(bs, 1);
  elm_box_homogenous_set(bs, 1);
  evas_object_size_hint_weight_set(bs, 1.0, 0.0);
  evas_object_size_hint_align_set(bs, -1.0, 0.0);
  elm_box_pack_end(bx, bs);
  evas_object_show(bs);
  
  /* Status */
  {
    Evas_Object *sb = elm_hoversel_add(wn);
    elm_hoversel_label_set(sb, _("Status"));
    elm_hoversel_hover_parent_set(sb, bx);
    evas_object_size_hint_weight_set(bs, 1.0, 1.0);
    evas_object_size_hint_align_set(bs, -1.0, 0.0);
    
    elm_hoversel_item_add(sb, _("Offline"), NULL, 0, NULL, NULL);
    elm_hoversel_item_add(sb, _("Online"), NULL, 0, NULL, NULL);
    elm_hoversel_item_add(sb, _("Away"), NULL, 0, NULL, NULL);
    elm_hoversel_item_add(sb, _("Extended Away"), NULL, 0, NULL, NULL);
    elm_hoversel_item_add(sb, _("Do not disturb"), NULL, 0, NULL, NULL);
    
    elm_box_pack_end(bs, sb);
    evas_object_show(sb);
  }

  /* Actions */
  {
    Evas_Object *ab = elm_hoversel_add(wn);
    elm_hoversel_label_set(ab, _("Actions"));
    elm_hoversel_hover_parent_set(ab, bx);
    evas_object_size_hint_weight_set(bs, 1.0, 1.0);
    evas_object_size_hint_align_set(bs, -1.0, 0.0);
    
    elm_hoversel_item_add(ab, _("Settings"), NULL, 0, NULL, NULL);
    elm_hoversel_item_add(ab, _("About"), NULL, 0, NULL, NULL);
    elm_hoversel_item_add(ab, _("Exit"), NULL, 0, NULL, NULL);
    
    elm_box_pack_end(bs, ab);
    evas_object_show(ab);
  }
  
  /*
  pc = elm_pager_add(wn);
  elm_win_resize_object_add(wn, pc);
  evas_object_show(pc);
  */
  
  /*
  cf = elm_jabber_config_add(wn);
  evas_object_size_hint_weight_set(cf, 1.0, 1.0);
  elm_pager_content_push(pc, cf);
  elm_win_resize_object_add(wn, cf);
  evas_object_show(cf);
  */
  //evas_object_hide(cf);
  
  //elm_pager_content_promote(pc, rl);
  
  evas_object_show(wn);
  
  elm_run(); /* and run the program now  and handle all events etc. */
  /* if the mainloop that elm_run() runs exist - we exit the app */
  elm_shutdown(); /* clean up and shut down */
  eet_shutdown();
  /* exit code */
  return 0;
}
#endif
