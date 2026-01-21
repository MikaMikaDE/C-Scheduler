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

/* ----------------------------------------------------------------- */
/*                Externally available functions                     */
/* ----------------------------------------------------------------- */
//returns next ready process
pid_t schedule(readyList_t *readyList) {
  if (isReadyListEmpty()) return NO_PROCESS;
  
  //dequeue from readyList
  pid_t nextPid = headOfReadyList()->pid;
  removeReady(nextPid);
  return nextPid;
}
/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */

