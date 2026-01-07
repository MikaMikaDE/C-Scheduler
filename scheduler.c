/* ----------------------------------------------------------------- */
/* Implementation of the scheduler */
/* for comments on the functions see the associated .h-file */
/* ----------------------------------------------------------------- */


/* ----------------------------------------------------------------- */
/* Include required external definitions */
/* ----------------------------------------------------------------- */
#include <math.h>
#include "bs_types.h"
#include "globals.h"
#include "scheduler.h"


/* ----------------------------------------------------------------- */
/*                Declarations of local helper functions             */
/* ----------------------------------------------------------------- */
void logStuff(readyList_t *readyList);

/* ----------------------------------------------------------------- */
/*                Externally available functions                     */
/* ----------------------------------------------------------------- */
//returns next ready process
pid_t schedule(readyList_t *readyList) {
  logStuff(readyList);

  if (!readyList || readyList->count == 0) return NO_PROCESS;

  pid_t pid = readyList->elems[0].pid;

  // pop front (shift left)
  for (size_t i = 1; i < readyList->count; i++) {
    readyList->elems[i - 1] = readyList->elems[i];
  }
  readyList->count--;

  return pid;
}
/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */

#define LOG_LIST(NAME, LIST) do {                         \
  printf("  " NAME ": [");                                \
  for (size_t i = 0; i < (LIST)->count; i++) {            \
    if (i > 0) printf(", ");                              \
    printf("%u", (unsigned)(LIST)->elems[i].pid);         \
  }                                                       \
  if ((LIST)->count > 0) printf(", ");                    \
  printf("x]\n");                                         \
} while (0)


void logStuff(readyList_t *readyList) {
  printf("\n>SCHEDULING:\n");
  LOG_LIST("ReadyList", readyList  );
  LOG_LIST("BlockList", blockedList);
}
