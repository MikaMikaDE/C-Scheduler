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

//appends a number (and comma) to a str
void append_num(char *str, size_t *pos, size_t value);

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

//this looks really weird because we are avoiding printf() without newlines
//ideally we would just for(i) printf(elems[i]) 
//but the assignment disallows this
void logLists(readyList_t *ready, blockedList_t *block) {
  size_t pos, i;
  char   readyStr[100];
  char   blockStr[100];
  for(i=0,pos=0; i<ready->count; i++) append_num(readyStr, &pos, ready->elems[i].pid); 
  readyStr[pos==0 ?0 :pos-1]='\0'; //trailing comma
  for(i=0,pos=0; i<block->count; i++) append_num(blockStr, &pos, block->elems[i].pid); 
  blockStr[pos==0 ?0 :pos-1]='\0'; //trailing comma
  printf("%6u : ReadyLst: [%s]\n%6u : BlockLst: [%s]\n", systemTime, readyStr, systemTime, blockStr);
}
		

/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */

void append_num(char *str, size_t *pos, size_t value) {
  char   tmp[32];     
  size_t len = 0;
  if (value == 0) {
    str[(*pos)++] = '0';
    str[(*pos)++] = ',';
    return;
  }
  while (value > 0) { // get digits in reverse
    tmp[len++] = (char)('0' + (value % 10));
    value /= 10;
  }
  while (len > 0) { // write digits in correct order
    str[(*pos)++] = tmp[--len];
  }
  str[(*pos)++] = ',';
}
