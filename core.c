/* Implementation of core functionality of the OS					*/
/* this includes the main scheduling loop							*/
/* for comments on the functions see the associated .h-file */

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

//ALTERATION BY MIKA: 
//was missing from simruntime.h -> that file explicitely asks not to be editied -> so here it is.
schedulingEvent_t sim_wait4UnblockedOrNew(pid_t* pPID); 

/* ----------------------------------------------------------------	*/
/* Declarations of global variables visible only in this file 		  */
PCB_t process;		// the only user process used for batch and FCFS
/* ---------------------------------------------------------------- */
/*                Externally available functions                    */
/* ---------------------------------------------------------------- */

void initOS(void)
{
	systemTime = 0;				// reset the system time to zero
	/* init the status of the OS */
	for (int i = 0; i < NUM_PROCESSES + 1; i++) processTable[i].valid = FALSE; 
	initReadyList();	  // no runnable processes exist
	initBlockedList();	// no blocked  processes exist
}

void coreLoop(void)
{
	stimulusCompleted     = FALSE;		  // The stimulus has been completed 
	pid_t currentProcess  = NO_PROCESS;	// PID of the process to schedule
	pid_t blockedProcess  = NO_PROCESS; // PID of the process that is blocked
	pid_t readyProcess    = NO_PROCESS; 
	schedulingEvent_t     schedulingEvent = none;	// reason for interrupting the process
	schedulingEvent_t     releaseEvent    = none;	// event that occured while process was running

	do {	// loop until stimulus is complete
		// select and run a process
		currentProcess=schedule(readyList);	
		if (currentProcess!= NO_PROCESS)		// schedulable process exists, given by its PID
		{
			systemTime=systemTime + SCHEDULING_DURATION; 
			logPid(currentProcess, "Process (re-)started"); 
			processTable[currentProcess].status = running;
			// run the processin the simulation, get info what happens next
			schedulingEvent= sim_runProcess(currentProcess, QUANTUM);
			logPidEvent(currentProcess, schedulingEvent, "Process left CPU");
			// handle all processes that turned "ready" in the meantime (unblocked or started (in case of multiprogramming) )
			releaseEvent = sim_check4UnblockedOrNew(&readyProcess);
			while (releaseEvent != none)
			{
				/* Without multiprogramming this loop shall never be entered, but: */
				/* This must be extended for multiprogramming. */ 
				/* Add Code for handling of started or unblocked processes here. 
				/* For RR it is simply enqueing to the readylist, other schedulers may require more actions */
				addReady(readyProcess);
				
				/* Last command in the while loop is the following (must alway remain the last command in the loop) */
				releaseEvent = sim_check4UnblockedOrNew(&readyProcess);	// check for further events, must stay in!
			}
			// Now all processes are dealt with that caused events while the last scheduled process used the CPU
			// determine required action based on the scheduling event caused by last running process
			// Depending on the scheduler, extensions to the following switch-case statement are required
			switch (schedulingEvent)  {
				case completed: 
					logPid(currentProcess, "Process complete and terminated");
					deleteProcess(currentProcess); 
					currentProcess=NO_PROCESS; 
					break; 
				
        case io:	// block process for time of IO
					addBlocked(currentProcess, sim_setIOBlockTime()); 
					break; 

				case quantumOver: // only logging needed
					logPidCompleteness(
            currentProcess, 
            processTable[currentProcess].usedCPU,
						processTable[currentProcess].duration, "of the Process completed"
          );
					processTable[currentProcess].status = ready;	// update status
					addReady(currentProcess); // add this process to the ready list
					break; 
				
        default: break; 
			}
		}
		else	// no process schedulable the runtime simulation will advance the system state...
		{
			// there are no schedulable processes, so wait for the blocked one (no multiptogramming) 
			// or either a new or an unblocked process (with multiprogramming) 
			// use the directive SIM_MULTIPROGAMMING (in globals.h) to activate multiprogramming in the simulation 
			logGeneric("No process ready, starting IDLE-process");
			// There is no implementation of an IDLE-process mechanism in the initial version, add if needed

			// Now advance the system time until the next process is either unblocked or a new one appears
			releaseEvent = sim_wait4UnblockedOrNew(&readyProcess); 
			switch (releaseEvent) {
        case none:	// there are neither blocked processes nor further processes in the stimulus
          logGeneric("Stimulus completed");
          stimulusCompleted = TRUE;
          break; 
        case unblocked:
          removeBlocked(readyProcess); // remove from blocked pool
          processTable[readyProcess].status = ready;   // change status from "blocked" to "ready"
          addReady(readyProcess);		// add this process to the ready list
          logPid(readyProcess, "IO completed, process unblocked and switched to ready state");
          break;
        case started: 
          processTable[readyProcess].status = ready;   // change status from "init" to "ready"
          addReady(readyProcess);		// add this process to the ready list
          logPid(readyProcess, "New process initialised and now ready"); 
          break;
        default: 
          // any other return value must be an error
          logGeneric("ERROR in Simulation handling unblocked/new processes when IDLE");
          exit(-1);		// terminate as simulation is not working correctly
          break;
			}
      //added (multiptogramming): needs to poll again, else this logs the same event over and over!
      releaseEvent = sim_wait4UnblockedOrNew(&readyProcess); 

		}
	} while (!stimulusCompleted);
}
