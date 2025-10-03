#include <stdio.h>
#include <stdlib.h>
#include "plugin.h"
#include "../engine/engine.h"

// Estado interno del plugin
typedef struct {
    float position_x;
    float position_y;
    int counter;
} PluginState;

// Inicialización del plugin
static void plugin_init(void** state) {
    if (*state == NULL) {
        // Solo inicializa si no hay estado previo
        PluginState* plugin_state = malloc(sizeof(PluginState));
        plugin_state->position_x = 0.0f;
        plugin_state->position_y = 0.0f;
        plugin_state->counter = 0;
        *state = plugin_state;
        
        printf("Plugin inicializado\n");
    } else {
        printf("Plugin re-inicializado con estado existente\n");
    }
    
    // Muestra constantes del motor
    engine_show_constants();
}

// Actualización del plugin
static void plugin_update(void* state, float delta_time) {
    PluginState* plugin_state = (PluginState*)state;
    
    // Actualiza el estado
    plugin_state->position_x += 1.0f * delta_time;
    plugin_state->position_y += 0.5f * delta_time;
    plugin_state->counter++;
    
    // Imprime estado cada 60 actualizaciones
    if (plugin_state->counter % 60 == 0) {
        printf("Posición: (%.2f, %.2f)\n", plugin_state->position_x, plugin_state->position_y);
    }
}

// Renderizado del plugin
static void plugin_render(void* state) {
    PluginState* plugin_state = (PluginState*)state;
    
    // En una implementación real, aquí se renderizaría algo
    // Por ahora, solo imprimimos un mensaje
    if (plugin_state->counter % 120 == 0) {
        printf("Renderizando en posición (%.2f, %.2f)\n", 
               plugin_state->position_x, plugin_state->position_y);
    }
}

// Limpieza del plugin
static void plugin_cleanup(void* state) {
    if (state) {
        printf("Limpiando plugin\n");
        free(state);
    }
}

// Definición de la API del plugin
static PluginAPI plugin_api = {
    .api_version = PLUGIN_API_VERSION,
    .state = NULL,
    .init = plugin_init,
    .update = plugin_update,
    .render = plugin_render,
    .cleanup = plugin_cleanup
};

// Función de exportación que devuelve la API del plugin
PluginAPI* get_plugin_api(void) {
    return &plugin_api;
}