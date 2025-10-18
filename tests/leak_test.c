#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_framework.h"

int test_leak_free_allocation(void) {
  // PRECONDICIONES: Capacidad de reservar memoria dinámica para N enteros y
  // puntero inicializado en NULL.
  // PROCESO: Reservar, inicializar, validar y liberar el bloque de memoria.
  // POSTCONDICIONES: No deben existir fugas de memoria y los datos deben
  // conservarse durante el ciclo de vida del bloque.

  const size_t N = 1000;
  int *arr = malloc(sizeof(int) * N);
  if (!arr) {
    fprintf(stderr, "malloc failed\n");
    return 2;
  }

  for (size_t i = 0; i < N; ++i) {
    arr[i] = (int)i;
  }

  if (arr[0] != 0 || arr[N - 1] != (int)(N - 1)) {
    fprintf(stderr, "sanity check failed\n");
    free(arr);
    return 3;
  }

  DEBUG_PRINTF("Memory block validated: first=%d last=%d size=%zu\n", arr[0],
               arr[N - 1], N);

  free(arr);
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_leak_free_allocation", test_leak_free_allocation},
  };

  return RUN_TEST_SUITE("LEAK", tests);
}
