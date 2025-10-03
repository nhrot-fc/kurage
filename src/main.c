#include "plugin/plugin.h"
#include <stdio.h>
#include <unistd.h>

#define FRAME_TIME (1.0f / 60.0f)

int main() {
  printf("Iniciando Kurage con sistema de hot reloading...\n");

  // Carga inicial del plugin
  if (!load_plugin("build/bin/libplugin.so")) {
    fprintf(stderr, "Error: No se pudo cargar el plugin inicial\n");
    return 1;
  }

  // Bucle principal
  int running = 1;
  int frame_count = 0;

  while (running &&
         frame_count < 600) { // Ejecuta por 600 frames (10 segundos a 60fps)
    // Comprueba si hay que recargar el plugin
    reload_plugin();

    // Obtiene el API del plugin
    PluginAPI *api = get_plugin_api();
    if (!api) {
      fprintf(stderr, "Error: API del plugin no disponible\n");
      break;
    }

    // Actualiza el plugin
    if (api->update) {
      api->update(api->state, FRAME_TIME);
    }

    // Renderiza el plugin
    if (api->render) {
      api->render(api->state);
    }

    // Simula el tiempo de un frame
    usleep((unsigned int)(FRAME_TIME * 1000000));

    frame_count++;
  }

  // Descarga el plugin al finalizar
  unload_plugin();

  printf("Programa finalizado\n");
  return 0;
}