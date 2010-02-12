#include<Elementary.h>
#include"jabber.h"

#include"ui_common.h"
#include"ui_config.h"


#ifndef default_jidres
#define default_jidres "username@server.domain/" NAME
#endif

#ifndef default_passwd
#define default_passwd "pass"
#endif

#ifndef default_server
#define default_server "server.domain:port"
#endif

#ifndef default_server_enable
#define default_server_enable 0
#endif

#ifndef default_usetls
#define default_usetls 1
#endif

#ifndef default_plain
#define default_plain 1
#endif

#ifndef default_sasl
#define default_sasl 0
#endif

#ifndef default_anon
#define default_anon 0
#endif


typedef struct _Widget_Data Widget_Data;
struct _Widget_Data{
  Evas_Object *frame, *jidres, *passwd, *server, *server_enable, *usetls, *plain, *sasl, *anon, *save;
};

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  free(wd);
}

static void
_server_enable_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  char st;
  
  st=elm_check_state_get(wd->server_enable);
  //elm_object_disabled_set(wd->server, !st);
  elm_entry_editable_set(wd->server, st);
}

static void
_sasl_enable_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  char st;
  
  st=elm_check_state_get(wd->sasl);
  elm_object_disabled_set(wd->anon, !st);
}

static void
_close_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  evas_object_smart_callback_call(wd->frame, "config,close", event_info);
}

static void
_save_hook(void *data, Evas_Object *obj, void *event_info){
  Widget_Data *wd=data;
  Eet_File *ef;
  char st;
  const char *val;
  
  if(!wd){
    // notice
    return;
  }
  
  ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_WRITE);
  if(!ef){
    // notice
    return;
  }
  
#define SAVE_OPT(name)					\
  val=elm_entry_entry_get(wd->name);			\
  eet_write(ef, #name, val, strlen(val)+1, 0);
  
  SAVE_OPT(jidres);
  SAVE_OPT(passwd);
  SAVE_OPT(server);
#undef SAVE_OPT
  
#define SAVE_OPT(name)				\
  st=elm_check_state_get(wd->name);		\
  val=&st;					\
  eet_write(ef, #name, val, 1, 0);
  
  SAVE_OPT(server_enable);
  SAVE_OPT(usetls);
  SAVE_OPT(plain);
  SAVE_OPT(sasl);
  SAVE_OPT(anon);
#undef SAVE_OPT
  
  evas_object_smart_callback_call(wd->frame, "config,changed", event_info);
  
  eet_close(ef);
}

Evas_Object *elm_jabber_config_add(Evas_Object *parent){
  Widget_Data *wd;
  Evas_Object *frame, *box, *buttons, *close, *scroll, *tab;
  
  wd = malloc(sizeof(Widget_Data));
  
  /* frame box */
  frame = elm_box_add(parent);
  wd->frame=frame;
  evas_object_size_hint_weight_set(frame, 1.0, 1.0);
  evas_object_size_hint_align_set(frame, -1.0, -1.0);
  evas_object_event_callback_add(frame, EVAS_CALLBACK_FREE, _del_hook, wd);
  evas_object_show(frame);
  
  /* scroll in frame box */
  scroll = elm_scroller_add(frame);
  elm_scroller_bounce_set(scroll, 0.0, 0.0);
  evas_object_size_hint_weight_set(scroll, 1.0, 1.0);
  evas_object_size_hint_align_set(scroll, -1.0, -1.0);
  elm_box_pack_end(frame, scroll);
  evas_object_show(scroll);
  
  /* box with options */
  box = elm_box_add(scroll);
  /*
  evas_object_size_hint_weight_set(box, 1.0, 0.0);
  evas_object_size_hint_align_set(box, -1.0, -1.0);
  */
  elm_scroller_content_set(scroll, box);
  evas_object_show(box);
  
#if 1
#define WRAP_FIELD(title, widget) {					\
    Evas_Object *field;							\
    field = elm_frame_add(box);						\
    elm_frame_label_set(field, title);					\
    evas_object_size_hint_weight_set(field, 1.0, 0.0);			\
    evas_object_size_hint_align_set(field, -1.0, 0.0);			\
    elm_box_pack_end(box, field);					\
    elm_frame_content_set(field, widget);				\
    evas_object_size_hint_weight_set(widget, 1.0, 0.0);			\
    evas_object_size_hint_align_set(widget, -1.0, 0.0);			\
    evas_object_show(widget);						\
    evas_object_show(field);						\
  }
#endif
#if 0
#define WRAP_FIELD(title, widget) {					\
    Evas_Object *hbox, *label;						\
    hbox = elm_box_add(box);						\
    elm_box_horizontal_set(hbox, 1);					\
    elm_box_homogenous_set(hbox, 1);					\
    evas_object_size_hint_weight_set(hbox, 1.0, 0.0);			\
    evas_object_size_hint_align_set(hbox, -1.0, 0.0);			\
    elm_box_pack_end(box, hbox);					\
    label = elm_label_add(box);						\
    elm_label_label_set(label, title);					\
    evas_object_size_hint_weight_set(label, 1.0, 0.0);			\
    evas_object_size_hint_align_set(label, -1.0, 0.0);			\
    elm_box_pack_end(hbox, label);					\
    evas_object_show(label);						\
    evas_object_size_hint_weight_set(widget, 1.0, 0.0);			\
    evas_object_size_hint_align_set(widget, -1.0, 0.0);			\
    elm_box_pack_end(hbox, widget);					\
    evas_object_show(widget);						\
    evas_object_show(hbox);						\
  }
#endif
#if 0
#define WRAP_FIELD(title, widget)		\
  elm_box_pack_end(box, widget);
#endif
  
  wd->jidres = elm_entry_add(box);
  elm_entry_single_line_set(wd->jidres, EINA_TRUE);
  WRAP_FIELD(_("JID/Resource"), wd->jidres);
  
  wd->passwd = elm_entry_add(box);
  elm_entry_single_line_set(wd->passwd, EINA_TRUE);
  elm_entry_password_set(wd->passwd, EINA_TRUE);
  WRAP_FIELD(_("Password"), wd->passwd);
  
#define WRAP_FIELD2(title, widget1, widget2) {		\
    Evas_Object *ibox;					\
    ibox=elm_box_add(box);				\
    elm_box_horizontal_set(ibox, 1);			\
    /*elm_box_homogenous_set(ibox, 1);*/		\
    evas_object_size_hint_weight_set(ibox, 1.0, 1.0);	\
    elm_box_pack_start(ibox, widget1);			\
    elm_box_pack_end(ibox, widget2);			\
    evas_object_show(ibox);				\
    WRAP_FIELD(title, ibox);				\
  }
  
  wd->server_enable = elm_check_add(box);
  elm_check_label_set(wd->server_enable, "");
  evas_object_size_hint_weight_set(wd->server_enable, 0.0, 0.0);
  evas_object_size_hint_align_set(wd->server_enable, 0.0, 0.0);
  evas_object_show(wd->server_enable);
  
  wd->server = elm_entry_add(box);
  elm_entry_single_line_set(wd->server, EINA_TRUE);
  evas_object_size_hint_weight_set(wd->server, 1.0, 1.0);
  evas_object_size_hint_align_set(wd->server, -1.0, -1.0);
  //evas_object_size_hint_padding_set(wd->server, 0.0, 0.0, 1.0, 0.0);
  evas_object_show(wd->server);
  evas_object_smart_callback_add(wd->server_enable, "changed", _server_enable_hook, wd);
  WRAP_FIELD2(_("Server"), wd->server_enable, wd->server);
  
  /* Table for Checkboxes */
  tab = elm_table_add(box);
  elm_table_homogenous_set(tab, 1);
  evas_object_size_hint_weight_set(tab, 1.0, 1.0);
  evas_object_size_hint_align_set(tab, -1.0, -1.0);
  elm_box_pack_end(box, tab);
  evas_object_show(tab);
  
  /* Use TLS */
  wd->usetls = elm_check_add(box);
  elm_check_label_set(wd->usetls, _("Use TLS"));
  evas_object_size_hint_align_set(wd->usetls, -1.0, 0.0);
  elm_table_pack(tab, wd->usetls, 0, 0, 1, 1);
  evas_object_show(wd->usetls);
  
  /* Use PLAIN */
  wd->plain = elm_check_add(box);
  elm_check_label_set(wd->plain, _("Use PLAIN"));
  evas_object_size_hint_align_set(wd->plain, -1.0, 0.0);
  elm_table_pack(tab, wd->plain, 1, 0, 1, 1);
  evas_object_show(wd->plain);
  
  /* Use SASL */
  wd->sasl = elm_check_add(box);
  elm_check_label_set(wd->sasl, _("Use SASL"));
  evas_object_size_hint_align_set(wd->sasl, -1.0, 0.0);
  elm_table_pack(tab, wd->sasl, 0, 1, 1, 1);
  evas_object_show(wd->sasl);
  
  /* SASL Anonymous */
  wd->anon = elm_check_add(box);
  elm_check_label_set(wd->anon, _("Anonymous"));
  evas_object_size_hint_align_set(wd->anon, -1.0, 0.0);
  elm_table_pack(tab, wd->anon, 1, 1, 1, 1);
  evas_object_show(wd->anon);
  evas_object_smart_callback_add(wd->sasl, "changed", _sasl_enable_hook, wd);
  
  
  /* buttons */
  buttons = elm_box_add(frame);
  elm_box_horizontal_set(buttons, 1);
  elm_box_homogenous_set(buttons, 1);
  evas_object_size_hint_weight_set(buttons, 1.0, 0.0);
  evas_object_size_hint_align_set(buttons, -1.0, 1.0);
  elm_box_pack_end(frame, buttons);
  evas_object_show(buttons);
  
  wd->save = elm_button_add(buttons);
  evas_object_size_hint_weight_set(wd->save, 1.0, 1.0);
  evas_object_size_hint_align_set(wd->save, -1.0, 0.0);
  elm_button_label_set(wd->save, _("Save"));
  elm_box_pack_end(buttons, wd->save);
  evas_object_show(wd->save);
  evas_object_smart_callback_add(wd->save, "clicked", _save_hook, wd);
  
  close = elm_button_add(buttons);
  evas_object_size_hint_weight_set(close, 1.0, 1.0);
  evas_object_size_hint_align_set(close, -1.0, 0.0);
  elm_button_label_set(close, _("Close"));
  elm_box_pack_end(buttons, close);
  evas_object_show(close);
  evas_object_smart_callback_add(close, "clicked", _close_hook, wd);
  
  {
    Eet_File *ef;
    int size;
    char *val=NULL;
    ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_READ);
    
#define LOAD_OPT(name)						\
    val=ef?eet_read(ef, #name, &size):NULL;			\
    elm_entry_entry_set(wd->name, val?val:default_ ## name);	\
    if(val)free(val);
    
    LOAD_OPT(jidres);
    LOAD_OPT(passwd);
    LOAD_OPT(server);
#undef LOAD_OPT
    
#define LOAD_OPT(name)						\
    val=ef?eet_read(ef, #name, &size):NULL;			\
    elm_check_state_set(wd->name, val?*val:default_ ## name);	\
    if(val)free(val);
    
    LOAD_OPT(server_enable);
    LOAD_OPT(usetls);
    LOAD_OPT(plain);
    LOAD_OPT(sasl);
    LOAD_OPT(anon);
#undef LOAD_OPT
    
    _server_enable_hook(wd, NULL, NULL);
    _sasl_enable_hook(wd, NULL, NULL);
    
    if(!jabber_hastls()){
      elm_check_state_set(wd->usetls, 0);
      elm_object_disabled_set(wd->usetls, 1);
    }
    
    if(ef)eet_close(ef);
  }
  
  return frame;
}

int elm_jabber_config_load(Jabber_Session *sess){
  char *jidres=NULL, *passwd=NULL, *server=NULL;
  int port=0;
  //char usetls=default_usetls, sasl=default_sasl, plain=default_plain, anon=default_anon;
  Jabber_Option opts=0;
  
  Eet_File *ef;
  int size;
  char se=0;
  char *val=NULL, *tmp=NULL;
  
  ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_READ);
  
  if(!ef)return 0;
  
  // JID / Resource
  val=eet_read(ef, "jidres", &size);
  if(val){
    if(strchr(val, '@')){
      jidres=val;
    }else{
      free(val);
    }
  }
  // Password
  val=eet_read(ef, "passwd", &size);
  if(val){
    passwd=val;
  }
#define READ_OPT(name, cons)				\
  val=eet_read(ef, #name, &size);			\
  if(val){						\
    if(*val)opts|=JABBER_ ## cons;			\
    free(val);						\
  }else{						\
    if(default_ ## name)opts|=JABBER_ ## cons;		\
  }
  
  READ_OPT(usetls, USETLS);
  READ_OPT(plain, PLAIN);
  READ_OPT(sasl, SASL);
  READ_OPT(anon, ANON);
#undef READ_OPT
  
  // For debugging
#ifdef DEVEL_MODE
  opts|=JABBER_LOG;
#endif

  // Server / Port
  val=eet_read(ef, "server_enable", &size);
  se=*val;
  free(val);
  if(se){
    val=eet_read(ef, "server", &size);
    if(val){
      tmp=strchr(val, ':');
      if(tmp){
	*tmp='\0';
	port=atoi(tmp+1);
      }
      if(port<0||port>65535)port=0;
      if(strlen(val)>0)server=strdup(val);
      free(val);
    }
  }
  
  eet_close(ef);
  
  {
    Jabber_State state=jabber_state(sess);
    jabber_disconnect(sess);
    jabber_config(sess, jidres, passwd, server, port, opts);
    if(state!=JABBER_DISCONNECTED)jabber_connect(sess);
  }
  
  if(jidres)free(jidres);
  if(passwd)free(passwd);
  if(server)free(server);
  
  return 1;
}

#ifdef TEST_JABBER_CONFIG
#define TEST_JABBER_POST {						\
    char *jidres, *passwd, *server;					\
    int port;								\
    char usetls;							\
    									\
    if(!elm_jabber_config_opt(&jidres, &passwd,	&server,		\
			      &port, &usetls)) {			\
      printf("jidres:%s passwd:%s server:%s port:%d usetls:%d\n",	\
	     jidres, passwd?passwd:"", server, port, usetls);		\
    }else{								\
      printf("error configuring!\n");					\
    }									\
  }
TEST_WIDGET(jabber_config, , TEST_JABBER_POST)
#endif
