/* Implementation of the log functions */
/* for comments on the functions see the associated .h-file */
/* please extend if needed to suit the needs of logging added functionality */
/* ---------------------------------------------------------------- */
/* Include required external definitions */
#include <math.h>
#include "bs_types.h"
#include "globals.h"
#include "log.h"


/* ---------------------------------------------------------------- */
/*                Declarations of local helper functions            */

/* ---------------------------------------------------------------- */
/* Declarations of global variables visible only in this file 		  */
// array with strings associated to scheduling events for log outputs
char eventString[6][12] = {"none", "started", "completed", "io", "quantumOver", "unblocked" };

/* ---------------------------------------------------------------- */
/*                Externally available functions					*/
/* ---------------------------------------------------------------- */

void logGeneric(char* message) {
	printf("%6u : %s\n", systemTime, message); 
}
	
void logPid(unsigned pid, char * message) {
	printf("%6u : PID %3u : %s\n", systemTime, pid, message); 
}
		
void logPidEvent(unsigned pid, schedulingEvent_t reason, char * message) {
	printf("%6u : PID %3u : Event: %s | %s\n", systemTime, pid, eventString[reason], message); 
}
		
void logPidCompleteness(unsigned pid) {
  unsigned done   = processTable[pid].usedCPU ;
  unsigned length = processTable[pid].duration;
  char*   message = "of the Process completed";
	printf("%6u : PID %3u : completeness: %u/%u | %s\n", systemTime, pid, done, length, message); 
}
		

/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */



