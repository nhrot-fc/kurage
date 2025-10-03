#ifndef PLUGIN_H
#define PLUGIN_H

#include <stddef.h>

#define PLUGIN_API_VERSION 1

typedef struct {
  int api_version;
  void *state;
  void (*init)(void **state);
  void (*update)(void *state, float delta_time);
  void (*render)(void *state);
  void (*cleanup)(void *state);
} PluginAPI;

typedef PluginAPI *(*GetPluginAPIFn)(void);
extern PluginAPI *get_plugin_api(void);
extern int load_plugin(const char *plugin_path);
extern int reload_plugin(void);
extern void unload_plugin(void);

#endif // PLUGIN_H
