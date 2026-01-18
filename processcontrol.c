/* Functions for control and administration of processes including  */
/* creation, destruction, blocked-control...						            */
/* for comments on the functions see the associated .h-file 		    */
/* ----------------------------------------------------------------	*/
/* Include required external definitions */
#include <math.h>
#include "bs_types.h"
#include "globals.h"
#include "dispatcher.h"
#include "processcontrol.h"
#include "simruntime.h"

/* ----------------------------------------------------------------	      */
/* Declare global variables according to definition in globals.h	        */
PCB_t         processTable[NUM_PROCESSES+1];// the process table
readyList_t   *readyList;	                  // queue of runnable processes
blockedList_t *blockedList;	                // pointer to blocked process
/* ----------------------------------------------------------------	      */
/* Declarations of global variables visible only in this file 		        */
blockedListElement_t blockedOne; // the only process that can be blocked 
/*this must be extended to a suitable data structure for multiprogramming */
readyListElement_t   readyOne;	 // the only process ready in the batch system 
/*this must be extended to a suitable data structure for multiprogramming */



/* ---------------------------------------------------------------- */
/* Functions for administration of processes in general 			      */
/* ---------------------------------------------------------------- */



/* Typically the PID is the index into the process table, thus the pool  	  */
/* for PIDs is limited by the size of the process table. If the end of the	*/
/* process table is reached, new PIDs are assigned by re-using unused 		  */
/* entries in the process table. 											                      */
/* If no new PID can be returned (i.e. the process table is full, zero is 	*/
/* returned, which is not a valid PID. 										                  */
pid_t getNextPid() {
	static unsigned pidCounter = 0;
	// determine next available pid. 
	for (int i = 0; i < NUM_PROCESSES; i++) {
		pidCounter++; 
		if (pidCounter > NUM_PROCESSES     ) pidCounter = 1; 
		if (!processTable[pidCounter].valid) return pidCounter;
	}
	/* If the loop is completed there was no free entry in the process table */
	return 0;	/* not a valid PID indicating an error  */
}


/* Voids the PCB of the process with the given pid, this includes setting*/
/* the valid-flag to invalid and setting other values to invalid values. */
/* retuns FALSE on error and TRUE on success								             */
Boolean deleteProcess(pid_t pid) {
	if (pid == NO_PROCESS) return FALSE;
	else {	/* PCB correctly passed, now delete it */
		processTable[pid].valid    = FALSE;
		processTable[pid].pid      = 0;
		processTable[pid].ppid     = 0;
		processTable[pid].ownerID  = 0;
		processTable[pid].start    = 0;
		processTable[pid].duration = 0;
		processTable[pid].usedCPU  = 0;
		processTable[pid].type     = os;
		processTable[pid].status   = ended;
		return TRUE;
	}
}

/* ---------------------------------------------------------------- */
/* Functions for administration of blocked processes 				        */
/* ---------------------------------------------------------------- */
/* Initialise the blocked process control data structure					            */
/* (no blocked processes exist at initialisation time of the os)			        */
/* CAUTION: For simulation purposes the blocked list must comply with:			  */
/*		- each entry has the information of the release time (IOready)			    */
/*		  included for each process												                      */
/*		- The blocked list is sorted by not decreasing release times (IOready)	*/
/*			(i.e. first process to become unblocked is always head of the list	  */
/* xxxx This function is a stub with reduced functionality, it must be   xxxx */
/* xxxx extended to enable full functionality of the operating system    xxxx */
/* xxxx A global variable is used to store blocked process in batch      xxxx */
/* xxxx processing. A blocked list needs to be implemented 		           xxxx */
Boolean initBlockedList(void) {
  blockedList = malloc(sizeof(*blockedList));
	blockedList->count = 0;
	return TRUE;
}





/* CAUTION: For simulation purposes the blocked list must comply with:			*/
/*		- each entry has the information of the release time (IOready)			  */
/*		  included for each process	(handled by the simulation)					      */
/*        calculated as the systemTime + blockDuration						          */
/*		- The blocked list is sorted by not decreasing release times (IOready)*/
/*			(i.e. first process to become unblocked is always head of the list	*/

/* xxxx This function is a stub with reduced functionality, it must be xxxx */
/* xxxx extended to enable full functionality of the operating system  xxxx */
/* xxxx A global variable is used to store blocked process in batch    xxxx */
/* xxxx processing. A blocked list needs to be implemented 		         xxxx */
/* retuns FALSE on error and TRUE on success								                */
Boolean addBlocked(pid_t pid, unsigned blockDuration) {
  if (pid == NO_PROCESS) return FALSE;
  //list is full
  if (blockedList->count >= NUM_PROCESSES) return FALSE;
  size_t i = blockedList->count;
  const unsigned IOready = systemTime + blockDuration;

  //sort array 
  while(i>0 && blockedList->elems[i-1].IOready > IOready) {
    blockedList->elems[i] = blockedList->elems[i-1];
    i--;
  }
  //add new blocked element
  blockedList->elems[i].pid     = pid;
  blockedList->elems[i].IOready = IOready;
  
  // set state=blocked
  processTable[pid].status  = blocked;

  //update size of list
  blockedList->count++;
  return TRUE;
}


/* predicate returning TRUE if any blocked process exists, FALSE if no		  */
/* blocked processes exist													                        */
Boolean isBlockedListEmpty(void) {
	return (blockedList->count == 0); //blockedList stores its size
}


/* Removes the given PID from the blocked-list								              */		
/* retuns FALSE on error and TRUE on success								                */
/* xxxx This function is a stub with reduced functionality, it must be xxxx */
/* xxxx extended to enable full functionality of the operating system  xxxx */
/* xxxx A global variable is used to store blocked process in batch    xxxx */
/* xxxx processing. A blocked list needs to be implemented 		         xxxx */
Boolean removeBlocked(pid_t pid) {
  if (isBlockedListEmpty()) return FALSE; //elem didnt exist
  blockedListElement_t *current;
  size_t count = blockedList->count;
  size_t idx   = 0;

  // find index
  while (idx<count && blockedList->elems[idx].pid != pid) idx++;
  if (idx == count) return FALSE; // not found

  // shift left to fill the gap
  for (size_t i=idx+1; i<count; i++) {
    blockedList->elems[i-1] = blockedList->elems[i];
  }

  blockedList->elems[count - 1].pid = NO_PROCESS; // forget the blocked process
  blockedList->elems[count - 1].IOready = 0;
  blockedList->count--;

	return TRUE;
}


/* returns a pointer to the first element of the blocked list	*/
/* MUST be implemented for simulation purposes					      */			
blockedListElement_t *headOfBlockedList() {
	if (isBlockedListEmpty()) return NO_PROCESS; // empty blocked list has no first element
  return &(blockedList->elems[0]); //list is always sorted on insert, so head is just 1st el
}


/* ----------------------------------------------------------------         */
/* Functions for administration of ready processes 				                  */
/* ----------------------------------------------------------------         */
/* CAUTION: For simulation purposes the ready list must comply with:        */
/*		- interface as given in the .h-file							                      */
/*		- The ready list is sorted by not decreasing start-times			        */
/*			(i.e. first process to become ready is always head of the list	    */
/*      															                                      */
/* Initialise the ready process control data structure						          */
/* (no ready processes exist at initialisation time of the os)				      */
/* retuns FALSE on error and TRUE on success								                */
/* xxxx This function is a stub with reduced functionality, it must be xxxx */
/* xxxx extended to enable full functionality of the operating system  xxxx */
/* xxxx A global variable is used to store the ready process in        xxxx */
/* xxxx batch processing. A ready list needs to be implemented 		     xxxx */
Boolean initReadyList(void) {
  readyList = malloc(sizeof(*readyList));
	readyList->count = 0;
	return TRUE;
}

/* retuns FALSE on error and TRUE on success								                */
/* CAUTION: For simulation purposes the ready list must comply with:        */
/*		- interface as given in the .h-file							                      */
/*		- no process is added as "ready" before its start time has elapsed	  */
/* xxxx This function is a stub with reduced functionality, it must be xxxx */
/* xxxx extended to enable full functionality of the operating system  xxxx */
/* xxxx A global variable is used to store the only ready process in   xxxx */
/* xxxx batch processing. A blocked list needs to be implemented 	     xxxx */
// add this process to the ready list

Boolean addReady(pid_t pid) {
  int i;
  size_t count = readyList->count;
  if (pid   == NO_PROCESS     ) return FALSE; //not a processs
  if (count >= NUM_PROCESSES  ) return FALSE; //list is full
  if (!processTable[pid].valid) return FALSE; //invalid procsees

  for (i=0; i<count; i++) {
    if (readyList->elems[i].pid == pid) return TRUE; //is already in list!
  }

  processTable[pid].status = ready; // set state = ready
  readyList->elems[i].pid  = pid;   //add new ready element
  readyList->count++;               //update size of list

	return TRUE;
}

/* predicate returning TRUE if any ready process exists, FALSE if no		    */
/* ready processes exist													                          */
Boolean isReadyListEmpty(void) {
	return (readyList->count == 0);
}

/* retuns FALSE on error and TRUE on success								                */
/* CAUTION: For simulation purposes the ready list must comply with:        */
/*		- interface as given in the .h-file							                      */
/*		- no process is added as "ready" before its start time has elapsed	  */
/* xxxx This function is a stub with reduced functionality, it must be xxxx */
/* xxxx extended to enable full functionality of the operating system  xxxx */
/* xxxx A global variable is used to store ready process in batch      xxxx */
/* xxxx processing. A ready list needs to be implemented 		           xxxx */
Boolean removeReady(pid_t pid) {
  if (isReadyListEmpty()) return FALSE; //elem didnt exist
  
  readyListElement_t *current;
  size_t count = readyList->count;
  size_t idx   = 0;

  // find index
  while (idx < count && readyList->elems[idx].pid != pid) idx++;
  if (idx == count) return FALSE; // not found

  // shift left to fill the gap
  for (size_t i=idx+1; i<count; i++) {
    readyList->elems[i-1] = readyList->elems[i];
  }

  readyList->elems[count - 1].pid = NO_PROCESS;
  readyList->count--;

	return TRUE;
}






/* returns a pointer to the first element of the ready list				          */
/* MUST be implemented for simulation purposes								              */
/* CAUTION: For simulation purposes the ready list must comply with:        */
/*		- interface as given in the .h-file							                      */
/*		- no process is added as "ready" before its start time has elapsed	  */
readyListElement_t* headOfReadyList() {
	if (isReadyListEmpty()) return NO_PROCESS; // empty ready list has no first element
	else return &readyList->elems[0];
}



