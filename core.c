/* Implementation of core functionality of the OS					          */
/* this includes the main scheduling loop							              */
/* for comments on the functions see the associated .h-file         */

/* ---------------------------------------------------------------- */
/* Include required external definitions */
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bs_types.h"
#include "globals.h"
#include "dispatcher.h"
#include "core.h"
#include "scheduler.h"
#include "processcontrol.h"
#include "simruntime.h"
//was missing from simruntime.h -> that file explicitely asks not to be editied -> so here it is.
schedulingEvent_t sim_wait4UnblockedOrNew(pid_t* pPID); 

/* ----------------------------------------------------------------	*/
/* Declarations of global variables visible only in this file 		  */
/* ----------------------------------------------------------------	*/
PCB_t process;		// the only user process used for batch and FCFS

/* ---------------------------------------------------------------- */
/*                Local Helper Funcion definition                   */
/* ---------------------------------------------------------------- */
static void  handleReleaseEventWhileBusy(schedulingEvent_t releaseEvent,    pid_t readyProcess  );
static pid_t handleSchedulingEvent      (schedulingEvent_t schedulingEvent, pid_t currentProcess);
static bool  handleReleaseEventWhileIdle(schedulingEvent_t releaseEvent,    pid_t readyProcess  );


/* ---------------------------------------------------------------- */
/*                Externally available functions                    */
/* ---------------------------------------------------------------- */

void initOS(void) { /* init the status of the OS */
	for (int i=0; i<NUM_PROCESSES+1; i++) processTable[i].valid = FALSE; 
	systemTime = 0;		  // reset system's time to zero
	initReadyList();	  // no runnable processes exist
	initBlockedList();	// no blocked  processes exist
}

void coreLoop(void) {
	stimulusCompleted     = FALSE;		  // True ifstimulus has been completed 
	pid_t currentProcess  = NO_PROCESS;	// PID of the process nextto schedule
	pid_t blockedProcess  = NO_PROCESS; // PID of the process that is blocked
	pid_t readyProcess    = NO_PROCESS; 
	schedulingEvent_t     schedulingEvent = none;	// reason for interrupting the process
	schedulingEvent_t     releaseEvent    = none;	// event that occured while process was running

	do {	// loop until stimulus is complete
    logLists(readyList, blockedList);
		currentProcess = schedule(readyList);	                                    // select and run a process
		if (currentProcess!= NO_PROCESS){		                                      // schedulable process exists, given by its PID
			systemTime=systemTime  +  SCHEDULING_DURATION; 
			logPid(currentProcess,"Process (re-)started"); 
			processTable[currentProcess].status = running;
			schedulingEvent = sim_runProcess(currentProcess, QUANTUM);              // run the processing the simulation, get info what happens next
			logPidEvent(currentProcess, schedulingEvent, "Process left CPU");
      while ((releaseEvent = sim_check4UnblockedOrNew(&readyProcess)) != none) handleReleaseEventWhileBusy(releaseEvent, readyProcess);
      // Now all processes are dealt with that caused events while the last scheduled process used the CPU
      currentProcess = handleSchedulingEvent(schedulingEvent, currentProcess);  // determine required action based on the scheduling event caused by last running process
		}
		else{	
      // no process schedulable the runtime simulation will advance the system state...
			// there are no schedulable processes, so wait for the blocked one (no multiptogramming) 
			// or either a new or an unblocked process (with multiprogramming) 
			// Now advance the system time until the next process is either unblocked or a new one appears
			logGeneric("No process ready, starting IDLE-process");
			releaseEvent      = sim_wait4UnblockedOrNew(&readyProcess); 
      stimulusCompleted = handleReleaseEventWhileIdle(releaseEvent, readyProcess);
		}
	} while (!stimulusCompleted);
}


/* ---------------------------------------------------------------- */
/*                Local Helper Funcion Implementation               */
/* ---------------------------------------------------------------- */
//Handles new/unblocked processes while CPU is running
static void handleReleaseEventWhileBusy(schedulingEvent_t releaseEvent, pid_t readyProcess) {
  switch (releaseEvent) {
    case unblocked:
      removeBlocked(readyProcess);
      addReady(readyProcess);
      logPid(readyProcess, "IO completed, process unblocked and switched to ready state");
      return;

    case started:
      addReady(readyProcess);
      logPid(readyProcess, "New process initialised and now ready");
      return;

    default:
      logGeneric("ERROR in Simulation handling unblocked/new processes while CPU busy");
      exit(-1);
  }
}

//Handles what happened to the process that just ran
//returns current process (which can modify it in the case the process was completed)
static pid_t handleSchedulingEvent(schedulingEvent_t schedulingEvent, pid_t currentProcess) {
  switch (schedulingEvent) {
    case completed: 
      logPid(currentProcess, "Process complete and terminated");
      deleteProcess(currentProcess); 
      return NO_PROCESS; 
    
    case io:	// block process for time of IO
      addBlocked(currentProcess, sim_setIOBlockTime()); 
      return currentProcess; 

    case quantumOver: // only logging needed
      logPidCompleteness(currentProcess);
      addReady(currentProcess); // add this process to the ready list
      return currentProcess; 
    
    default: break; 
  }
}



//Handles new/unblocked processes while CPU is idle
//returns whether the simulation is over (TRUE) or should continue (FALSE)
static bool handleReleaseEventWhileIdle(schedulingEvent_t releaseEvent, pid_t readyProcess) {
  switch (releaseEvent) {
    case none:	// there are neither blocked processes nor further processes in the stimulus
      logGeneric("Stimulus completed");
      return TRUE; 
    case unblocked:
      removeBlocked(readyProcess); // remove from blocked pool
      addReady(readyProcess);		   // add this process to the ready list
      logPid(readyProcess, "IO completed, process unblocked and switched to ready state");
      return FALSE;
    case started: 
      addReady(readyProcess);		// add this process to the ready list
      logPid(readyProcess, "New process initialised and now ready"); 
      return FALSE;
    default: // any other return value must be an error
      logGeneric("ERROR in Simulation handling unblocked/new processes when IDLE");
      exit(-1);		// terminate as simulation is not working correctly
  }
}
