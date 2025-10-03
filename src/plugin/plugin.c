#include "plugin.h"
#include <dlfcn.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *PLUGIN_TEMP_PATH = "build/bin/libplugin_temp.so";
static const char *PLUGIN_PATH = "build/bin/libplugin.so";

static void *plugin_handle = NULL;
static PluginAPI *plugin_api = NULL;
static time_t last_modified = 0;

static int has_file_changed(const char *path) {
  struct stat attr;
  if (stat(path, &attr) == 0) {
    if (attr.st_mtime > last_modified) {
      last_modified = attr.st_mtime;
      return 1;
    }
  }
  return 0;
}

int load_plugin(const char *plugin_path) {
  if (plugin_handle) {
    unload_plugin();
  }

  plugin_handle = dlopen(plugin_path, RTLD_NOW);
  if (!plugin_handle) {
    fprintf(stderr, "Error cargando el plugin: %s\n", dlerror());
    return 0;
  }

  GetPluginAPIFn get_api =
      (GetPluginAPIFn)dlsym(plugin_handle, "get_plugin_api");
  if (!get_api) {
    fprintf(stderr, "Error obteniendo función get_plugin_api: %s\n", dlerror());
    dlclose(plugin_handle);
    plugin_handle = NULL;
    return 0;
  }

  plugin_api = get_api();
  if (!plugin_api) {
    fprintf(stderr, "Error obteniendo API del plugin\n");
    dlclose(plugin_handle);
    plugin_handle = NULL;
    return 0;
  }

  if (plugin_api->api_version != PLUGIN_API_VERSION) {
    fprintf(stderr,
            "Versión del API del plugin incorrecta. Esperada: %d, Actual: %d\n",
            PLUGIN_API_VERSION, plugin_api->api_version);
    dlclose(plugin_handle);
    plugin_handle = NULL;
    plugin_api = NULL;
    return 0;
  }

  if (plugin_api->init) {
    plugin_api->init(&plugin_api->state);
  }

  struct stat attr;
  if (stat(plugin_path, &attr) == 0) {
    last_modified = attr.st_mtime;
  }

  printf("Plugin cargado exitosamente\n");
  return 1;
}

int reload_plugin(void) {
  if (!has_file_changed(PLUGIN_PATH)) {
    return 0;
  }

  printf("Detectado cambio en el plugin, recargando...\n");

  void *old_state = NULL;
  if (plugin_api) {
    old_state = plugin_api->state;
  }

  if (plugin_handle) {
    plugin_api = NULL;
    dlclose(plugin_handle);
    plugin_handle = NULL;
  }

  if (!load_plugin(PLUGIN_PATH)) {
    fprintf(stderr, "Error recargando el plugin\n");
    return 0;
  }

  if (old_state && plugin_api) {
    plugin_api->state = old_state;
  }

  printf("Plugin recargado exitosamente\n");
  return 1;
}

PluginAPI *get_plugin_api(void) { return plugin_api; }

void unload_plugin(void) {
  if (plugin_api && plugin_api->cleanup) {
    plugin_api->cleanup(plugin_api->state);
  }

  if (plugin_handle) {
    dlclose(plugin_handle);
    plugin_handle = NULL;
  }

  plugin_api = NULL;
}
