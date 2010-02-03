#include<Elementary.h>

#ifndef _
#define _(...) __VA_ARGS__
#endif

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

#define EET_CONF_FILE "." NAME ".eet"
#define DEFAULT_JID "user@server.domain"
#define DEFAULT_PASS "pass"

static void
elm_jabber_config_save(void *data, Evas_Object *obj, void *event_info){
  Evas_Object *content, *jid_ent, *pass_ent;
  Eet_File *ef;
  Eina_List *ents;
  const char *jid, *pass;
  
  content = (Evas_Object *)data;
  ents = evas_object_box_children_get(content);
  
  jid_ent = ents->data;
  pass_ent = ents->next->data;
  
  eina_list_free(ents);

  jid = elm_entry_entry_get(jid_ent);
  pass = elm_entry_entry_get(pass_ent);
  
  ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_WRITE);
  if(!ef){
    // notice
    return;
  }
  eet_write(ef, "jid", jid, strlen(jid) + 1, 0);
  eet_write(ef, "pass", jid, strlen(jid) + 1, 0);
  eet_close(ef);
}

Evas_Object *elm_jabber_config_add(Evas_Object *parent){
  Evas_Object *panel, *content, *jid_ent, *pass_ent, *save_btn;
  Eet_File *ef;
  char *jid, *pass;
  int size;
  
  panel = elm_panel_add(parent);
  elm_panel_orient_set(panel, ELM_PANEL_ORIENT_TOP);
  
  content = elm_box_add (parent);
  evas_object_resize(content, 480, 480);
  elm_panel_content_set(panel, content);
  evas_object_show(content);
  
  jid_ent = elm_entry_add(parent);
  elm_entry_single_line_set(jid_ent, EINA_TRUE);
  elm_box_pack_end(content, jid_ent);
  evas_object_show(jid_ent);
  
  pass_ent = elm_entry_add(parent);
  elm_entry_single_line_set(pass_ent, EINA_TRUE);
  elm_entry_password_set(pass_ent, EINA_TRUE);
  elm_box_pack_end(content, pass_ent);
  evas_object_show(pass_ent);
  
  save_btn = elm_button_add(parent);
  elm_button_label_set(save_btn, _("Save"));
  elm_box_pack_end(content, save_btn);
  evas_object_smart_callback_add(save_btn, "clicked", elm_jabber_config_save, (void*)content);
  evas_object_show(save_btn);
  
  ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_READ);
  if(!ef){
    ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_WRITE);
    if(!ef){
      // notice
    }else{
      eet_write(ef, "jid", DEFAULT_JID, strlen(DEFAULT_JID) + 1, 0);
      eet_write(ef, "pass", DEFAULT_PASS, strlen(DEFAULT_PASS) + 1, 0);
      eet_close(ef);
      
      ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_READ);
    }
  }
  if(ef){
    jid = eet_read(ef, "jid", &size);
    pass = eet_read(ef, "pass", &size);
    elm_entry_entry_set(jid_ent, jid);
    elm_entry_entry_set(pass_ent, pass);
  }else{
    // notice
    elm_entry_entry_set(jid_ent, DEFAULT_JID);
    elm_entry_entry_set(pass_ent, DEFAULT_PASS);
  }
  eet_close(ef);
  
  return panel;
}

#define DBGMSG(m) printf("DBG: " #m "..\n");

int
main(int argc, char **argv){
  Evas_Object *wn, *bg, *rl, *cf;
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
  
  rl = elm_anchorview_add(wn);
  elm_anchorview_text_set(rl, "It's startup <a href=1>kayo@neko.im</a>");
  elm_win_resize_object_add(wn, rl);
  evas_object_show(rl);
  
  cf = elm_jabber_config_add(wn);
  evas_object_show(cf);
  evas_object_resize(cf, 480, 480);
  
  //evas_object_resize(wn, 240, 480);
  evas_object_show(wn);
  
  elm_run(); /* and run the program now  and handle all events etc. */
  /* if the mainloop that elm_run() runs exist - we exit the app */
  elm_shutdown(); /* clean up and shut down */
  eet_shutdown();
  /* exit code */
  return 0;
}
/* all emeentary apps should use this. but it right after elm_main() */
//ELM_MAIN()
