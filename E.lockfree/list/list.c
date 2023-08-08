#include "list.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct List_t {
  void *data;
  atomic_flag owner;
  List _Atomic next;
} List_t;

List list_init() {
  List container = (List)malloc(sizeof(*container));
  container->data = NULL;
  container->next = NULL;
}

bool white_test() {
  return false;
}

int main(int argc, char *argv[]) {
  white_test();
}