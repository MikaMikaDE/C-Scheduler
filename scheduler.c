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
#include "processcontrol.h"


/* ----------------------------------------------------------------- */
/*                Declarations of local helper functions             */
/* ----------------------------------------------------------------- */
void logReadyAndBlockedLists(readyList_t *readyList);

/* ----------------------------------------------------------------- */
/*                Externally available functions                     */
/* ----------------------------------------------------------------- */
//returns next ready process
pid_t schedule(readyList_t *readyList) {
  logReadyAndBlockedLists(readyList);
  if (isReadyListEmpty()) return NO_PROCESS;
  
  //dequeue from readyList
  pid_t nextPid = headOfReadyList()->pid;
  removeReady(nextPid);
  return nextPid;
}
/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */



#define LOG_LIST(NAME, LIST) do {                                     \
/*Laut der Aufgabe sind printfs ohne newline verboten*/               \
/* aber in diesem Fall ergibt es Sinn, um die Listen besser zu sehen*/\
  printf("  " NAME ": [");                                \
  for (size_t i = 0; i < (LIST)->count; i++) {            \
    if (i > 0) printf(", ");                              \
    printf("%u", (unsigned)(LIST)->elems[i].pid);         \
  }                                                       \
  printf("]\n");                                          \
} while (0)


void logReadyAndBlockedLists(readyList_t *readyList) {
  printf("\n>SCHEDULING:\n");
  LOG_LIST("ReadyList", readyList  );
  LOG_LIST("BlockList", blockedList);
}
