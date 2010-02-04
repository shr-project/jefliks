#include<Elementary.h>
#include"ui_common.h"
#include"ui_config.h"

#ifndef EET_CONF_FILE
#define EET_CONF_FILE "." NAME ".eet"
#endif

#ifndef default_jidres
#define default_jidres "username@server.domain/" NAME
#endif

#ifndef default_res
#define default_res NAME
#endif

#ifndef default_port
#define default_port 5222
#endif

#ifndef default_tlsport
#define default_tlsport 5223
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


//#include<stdlib.h>
//#include<string.h>

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data{
  Evas_Object *jidres, *passwd, *server, *server_enable, *usetls, *save;
};

static void
_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
  Widget_Data *wd=data;
  free(wd);
}

static void
_server_enable_hook(void *data, Evas_Object *obj, void *event_info)
{
  Widget_Data *wd=data;
  char st;
  
  st=elm_check_state_get(wd->server_enable);
  elm_object_disabled_set(wd->server, !st);
}

static void
_close_hook(void *data, Evas_Object *obj, void *event_info)
{
  Evas_Object *frame=data;
  evas_object_del(frame);
}

static void
_save_hook(void *data, Evas_Object *obj, void *event_info)
{
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
  
#define GET_VAL(name)					\
  val=elm_entry_entry_get(wd->name);			\
  eet_write(ef, #name, val, strlen(val)+1, 0);
  
  GET_VAL(jidres);
  GET_VAL(passwd);
  GET_VAL(server);
#undef GET_VAL
  
  st=elm_check_state_get(wd->server_enable);
  val=&st;
  eet_write(ef, "server_enable", val, 1, 0);
  
  st=elm_check_state_get(wd->usetls);
  val=&st;
  eet_write(ef, "usetls", val, 1, 0);
  
  eet_close(ef);
}

Evas_Object *elm_jabber_config_add(Evas_Object *parent){
  Widget_Data *wd;
  Evas_Object *frame, *fbox, *box, *buttons, *close, *scroll;
  
  wd = malloc(sizeof(Widget_Data));
  
  frame = elm_frame_add(parent);
  elm_frame_label_set(frame, _("Connection settings"));
  evas_object_event_callback_add(frame, EVAS_CALLBACK_FREE, _del_hook, wd);
  
  box = elm_box_add(parent);
  evas_object_size_hint_weight_set(box, 1.0, 0.0);
  evas_object_show(box);
  
#if 1
#define WRAP_FIELD(title, widget) {					\
    Evas_Object *field;							\
    field = elm_frame_add(parent);					\
    elm_frame_label_set(field, title);					\
    evas_object_size_hint_weight_set(field, 1.0, 0.0);			\
    evas_object_size_hint_align_set(field, -1.0, 0.0);			\
    elm_box_pack_end(box, field);					\
    elm_frame_content_set(field, widget);				\
    evas_object_show(widget);						\
    evas_object_show(field);						\
  }
#endif
#if 0
#define WRAP_FIELD(title, widget) {					\
    Evas_Object *hbox, *label;						\
    hbox = elm_box_add(parent);						\
    elm_box_horizontal_set(hbox, 1);					\
    elm_box_homogenous_set(hbox, 1);					\
    evas_object_size_hint_weight_set(hbox, 1.0, 0.0);			\
    evas_object_size_hint_align_set(hbox, -1.0, 0.0);			\
    elm_box_pack_end(box, hbox);					\
    label = elm_label_add(parent);					\
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
  
  wd->jidres = elm_entry_add(parent);
  elm_entry_single_line_set(wd->jidres, EINA_TRUE);
  WRAP_FIELD(_("JID/Resource"), wd->jidres);
  
  wd->passwd = elm_entry_add(parent);
  elm_entry_single_line_set(wd->passwd, EINA_TRUE);
  elm_entry_password_set(wd->passwd, EINA_TRUE);
  WRAP_FIELD(_("Password"), wd->passwd);
  
#define WRAP_FIELD2(title, widget1, widget2) {		\
    Evas_Object *ibox;					\
    ibox=elm_box_add(parent);				\
    elm_box_horizontal_set(ibox, 1);			\
    /*elm_box_homogenous_set(ibox, 1);*/		\
    evas_object_size_hint_weight_set(ibox, 1.0, 1.0);	\
    elm_box_pack_start(ibox, widget1);			\
    elm_box_pack_end(ibox, widget2);			\
    evas_object_show(ibox);				\
    WRAP_FIELD(title, ibox);				\
  }
  
  wd->server_enable = elm_check_add(parent);
  elm_check_label_set(wd->server_enable, "");
  evas_object_size_hint_weight_set(wd->server_enable, 0.0, 0.0);
  evas_object_size_hint_align_set(wd->server_enable, 0.0, 0.0);
  evas_object_show(wd->server_enable);
  
  wd->server = elm_entry_add(parent);
  elm_entry_single_line_set(wd->server, EINA_TRUE);
  evas_object_size_hint_weight_set(wd->server, 1.0, 1.0);
  evas_object_size_hint_align_set(wd->server, -1.0, -1.0);
  //evas_object_size_hint_padding_set(wd->server, 0.0, 0.0, 1.0, 0.0);
  evas_object_show(wd->server);
  evas_object_smart_callback_add(wd->server_enable, "changed", _server_enable_hook, wd);
  WRAP_FIELD2(_("Server"), wd->server_enable, wd->server);
  
  wd->usetls = elm_check_add(parent);
  elm_check_label_set(wd->usetls, _("Use TLS"));
  elm_box_pack_end(box, wd->usetls);
  evas_object_show(wd->usetls);
  
  
  fbox = elm_box_add(parent);
  evas_object_size_hint_weight_set(fbox, 1.0, 1.0);
  elm_frame_content_set(frame, fbox);
  evas_object_show(fbox);
  
  scroll = elm_scroller_add(parent);
  elm_scroller_bounce_set(scroll, 0.0, 0.0);
  evas_object_size_hint_weight_set(scroll, 1.0, 1.0);
  evas_object_size_hint_align_set(scroll, -1.0, -1.0);
  elm_scroller_content_set(scroll, box);
  elm_box_pack_end(fbox, scroll);
  evas_object_show(scroll);
  
  
  buttons = elm_box_add(parent);
  elm_box_horizontal_set(buttons, 1);
  elm_box_homogenous_set(buttons, 1);
  evas_object_size_hint_weight_set(buttons, 1.0, 0.0);
  evas_object_size_hint_align_set(buttons, -1.0, 1.0);
  elm_box_pack_end(fbox, buttons);
  evas_object_show(buttons);
  
  wd->save = elm_button_add(parent);
  evas_object_size_hint_weight_set(wd->save, 1.0, 1.0);
  evas_object_size_hint_align_set(wd->save, -1.0, 0.0);
  elm_button_label_set(wd->save, _("Save"));
  elm_box_pack_end(buttons, wd->save);
  evas_object_show(wd->save);
  evas_object_smart_callback_add(wd->save, "clicked", _save_hook, wd);
  
  close = elm_button_add(parent);
  evas_object_size_hint_weight_set(close, 1.0, 1.0);
  evas_object_size_hint_align_set(close, -1.0, 0.0);
  elm_button_label_set(close, _("Close"));
  elm_box_pack_end(buttons, close);
  evas_object_show(close);
  evas_object_smart_callback_add(close, "clicked", _close_hook, frame);
  
  {
    Eet_File *ef;
    int size;
    char *val=NULL;
    ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_READ);
    
#define SET_VAL(name)						\
    if(ef)val=eet_read(ef, #name, &size);			\
    elm_entry_entry_set(wd->name, val?val:default_ ## name);
    
    SET_VAL(jidres);
    SET_VAL(passwd);
    SET_VAL(server);
#undef SET_VAL
    
    val=eet_read(ef, "usetls", &size);
    elm_check_state_set(wd->usetls, val?*val:default_usetls);
    
    val=eet_read(ef, "server_enable", &size);
    {
      char st=val?*val:default_server_enable;
      elm_check_state_set(wd->server_enable, st);
      elm_object_disabled_set(wd->server, !st);
    }
    
    if(ef)eet_close(ef);
  }
  
  return frame;
}

int elm_jabber_config_opt(char **jid, char **res, char **passwd, char **server, int *port, char *usetls){
  Eet_File *ef;
  int size;
  char se=0;
  char *val=NULL, *tmp=NULL;
  ef = eet_open(EET_CONF_FILE, EET_FILE_MODE_READ);
  
  if(ef){
    // JID / Resource
    *jid=NULL; *res=NULL;
    val=eet_read(ef, "jidres", &size);
    if(val){
      tmp=strchr(val, '/');
      if(tmp){
	*tmp='\0';
	*res=strdup(tmp+1);
      }else{
	*res=strdup(default_res);
      }
      *jid=strdup(val);
      free(val);
    }
    // Password
    *passwd=NULL;
    val=eet_read(ef, "passwd", &size);
    if(val){
      *passwd=strdup(val);
      free(val);
    }
    // Use TLS
    *usetls=0;
    val=eet_read(ef, "usetls", &size);
    if(val){
      *usetls=val[0];
      free(val);
    }
    // Server / Port
    *server=NULL; *port=0;
    val=eet_read(ef, "server_enable", &size);
    se=val[0];
    free(val);
    if(se){
      val=eet_read(ef, "passwd", &size);
      if(val){
	tmp=strchr(val, ':');
	if(tmp){
	  *tmp='\0';
	  *port=atoi(tmp+1);
	}
	if(*port<0||*port>65535)*port=0;
	if(strlen(val)>0)*server=strdup(val);
	free(val);
      }
    }
    // Set port automatically
    if(!*port)*port=*usetls?default_tlsport:default_port;
    // Try get server from JID
    if(!*server){
      val=strdup(*jid);
      tmp=strchr(val, '@');
      if(tmp && strlen(tmp+1)>0){
	*server=strdup(tmp+1);
      }
      free(val);
    }
    
    eet_close(ef);
  }
  
  if(!*jid || !*res || !*server || !*port) return -1;
  return 0;
}

#ifdef TEST_JABBER_CONFIG
#define TEST_JABBER_POST {						\
    char *jid, *res, *passwd, *server;					\
    int port;								\
    char usetls;							\
    									\
    if(!elm_jabber_config_opt(&jid, &res,	&passwd,		\
			      &server, &port, &usetls)) {		\
      printf("jid:%s res:%s passwd:%s server:%s port:%d usetls:%d\n",	\
	     jid,res, passwd?passwd:"", server, port, usetls);		\
    }else{								\
      printf("error configuring!\n");					\
    }									\
  }
TEST_WIDGET(jabber_config, , TEST_JABBER_POST)
#endif
