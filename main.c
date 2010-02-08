#include<Elementary.h>

#include"ui_main.h"


/* this is your elementary main function - it MUSt be called IMMEDIATELY
 * after elm_init() and MUSt be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */

static void
main_win_del(void *data, Evas_Object *obj, void *event_info)
{
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
  Evas_Object *wn, *bg, *fm;
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
  
  /* Main */
  fm = elm_jabber_main(wn);
  elm_win_resize_object_add(wn, fm);
  evas_object_size_hint_weight_set(fm, 1.0, 1.0);
  evas_object_show(fm);
  
  /*
  pc = elm_pager_add(wn);
  elm_win_resize_object_add(wn, pc);
  evas_object_show(pc);
  */
  
  //evas_object_hide(cf);
  
  //elm_pager_content_promote(pc, rl);
  
  evas_object_size_hint_weight_set(wn, 1.0, 1.0);
  evas_object_resize(wn, 480, 640);
  evas_object_show(wn);
  
  elm_run(); /* and run the program now  and handle all events etc. */
  /* if the mainloop that elm_run() runs exist - we exit the app */
  elm_shutdown(); /* clean up and shut down */
  eet_shutdown();
  /* exit code */
  return 0;
}
#endif
