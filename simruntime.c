/* Implementation of the simulation run-time environment.			*/
/* The functions in this file simulate the remainig system			*/
/* (covering hardware-related and software components)				*/
/* for comments on the functions see the associated .h-file			*/
/* !!!!!!!!!! Do not change the functions in this file !!!!!!!!!!	*/

#define	_CRT_SECURE_NO_WARNINGS		// suppress legacy warnings 

/* ---------------------------------------------------------------- */
/* Include required external definitions */
#include <math.h>
#include "bs_types.h"
#include "globals.h"
#include "scheduler.h"
#include "processcontrol.h"

//ALTERATION BY MIKA: 
//to compile and run on linux
#if defined(__linux__)
  #include <limits.h>
#endif

Boolean stimulusComplete = FALSE;		// stimulus file completely read ?
Boolean sim_complete = FALSE;		// end of OS indicator
Boolean stimulusCompleted = FALSE;		// The stimulus has been completed		// TODO kann davon ein Flag weg???
FILE* processFile;
PCB_t processCandidate;		// buffer for the next process to start
Boolean candidateAvailable = FALSE;		// Flag indicating that a process is available for start

/* ---------------------------------------------------------------- */
/*                Declarations of local helper functions			*/

FILE* openStimulusFile(FILE *file, const char * filename);
/* opens the  process info file specified by <filename>						*/
/* file has to refer to a valid FILE handle in the calling instance			*/
/* returns the file handle (which is NULL on error)							*/
/* Data in the file must be read using the function readNextProcess()		*/

Boolean readNextProcess(FILE* f, PCB_t* pProcess);
/* the file handle must refer to an already opened process info file		*/
/* the pointer pProcess must point to a valid PCB_t variable, the function  */
/* does not allocate memory for this. 										*/
/* Only components stored in the fie are updated.							*/
/* returns NULL if no process block was read, either because the file		*/
/* handle was invalid (NULL) or EOF was reached								*/
/* returns the pointer pProcess on success									*/

Boolean sim_newProcessEvent(unsigned timestamp);
/* Predicate returning TRUE, if a new process is available at the	*/
/* time given in the parameter and shall be created and run			*/
/* A return value of FALSE indicating that currently no Process is	*/
/* available for execution, but there may be processes in the future*/

pid_t sim_makeNewProcessReady(void);
/* Initialises the next procees given bay the stimulus file.		*/
/* If the process is not yet ready to be startet, the system time	*/
/* is advanced until the process becomes startable					*/


int sim_initSim(void)
/* initialise the simulation, not part of the os					*/
{
	char filename[128] = PROCESS_FILENAME;

	srand(0);
#ifndef NO_RANDOM_SEED	
	srand((unsigned)time(NULL));	// init the random number generator
#endif
	systemTime = 0;				// reset the system time to zero
	// open the file with process definitions
	processFile = openStimulusFile(processFile, filename);
	if (processFile == NULL) exit(-1); 
	logGeneric("Simulation: Process info file opened");
	stimulusComplete = FALSE;
	// read first entry to generate event-information for simulation
	candidateAvailable = readNextProcess(processFile, &processCandidate);
	if (!candidateAvailable)
	{	// if this fails there are no processes in the file
		stimulusComplete = TRUE; // file completely processed
	}
	return TRUE;
}

int sim_closeSim(void)
/* Exit from the simulation environment regularly					*/
{
	return TRUE; 
}


Boolean sim_noMoreProcesses(void)
/* returns TRUE when simulation has no more processes to run		*/
/* i.e. when the stimulus-file was complete read and the buffered   */
/* candidate process was used						 				*/			
{
	return !candidateAvailable;
}

Boolean sim_getProcessParameters(pid_t pid)
/* initialises the process handed over in pProcess with the			*/
/* parameters read from stimulus file. Only parameters given by the */
/* stimulis file are changed										*/		
/* retuns FALSE on error and TRUE on success						*/
{
	char processTypeStr[16] = ""; 	// plain text of process type 
	
//	if (sim_noMoreProcesses()) return FALSE; 
	if (candidateAvailable)	// there ist still at least one process to run
	{
		// use the candidate as new process
		processTable[pid].ownerID = processCandidate.ownerID;
		processTable[pid].start = processCandidate.start;
		processTable[pid].duration = processCandidate.duration;
		processTable[pid].type = processCandidate.type;
		switch (processTable[pid].type) {
			case os:			strcpy(processTypeStr, "os"); break; 
			case interactive:	strcpy(processTypeStr, "interactive"); break;
			case batch:			strcpy(processTypeStr, "batch"); break;
			case background:	strcpy(processTypeStr, "background"); break;
			case foreground:	strcpy(processTypeStr, "foreground"); break;
		}
		printf("%6u : Simulation: Process %u configured as: %u %u %u %s\n", systemTime, 
			pid, processTable[pid].ownerID,
			processTable[pid].start, processTable[pid].duration, processTypeStr);
		// read next entry from stimulus, make it the next candidate
		candidateAvailable = readNextProcess(processFile, &processCandidate);
		return TRUE; 
	}
	else
		return FALSE; // there is no process information available
}

unsigned int sim_setIOBlockTime(void)
/* returns the duration of a simulated IO-operation					*/ 
{
	// determine the time the process will be blocked for IO
	// using uniform probability distribution 
	return  rand() % (2 * IO_MEAN_DURATION)+1; // for simulation only, must stay in!
}



/* checks the simulation environment for processes that have become */
/* ready since the last scheduling event processed by the 			    */
/* simulation or the dispatcher.									                  */
/* Returns what happened: a process was unblocked or a new process	*/
/* was initialised. Either event type results in a process that is	*/
/* now ready and must be processed accordingly.						          */
/* Returns 'none' if nothing happened in the relevant interval		  */
/* The paramater must point to a location for the pid of the		    */
/* process that became ready. The value is changed to 0 if no event */
/* occurred and to the pid of the process causing the event			    */
/* otherwise.														                            */
/* No data structures of the OS are changed, the OS must deal with	*/ 
/* the returned informnation appropriately, e.g. update blocked-    */
/* and ready-list													                          */
/* USAGE:															                              */
/* The function must be called in multiprogramming-systems after a	*/
/* process has left the CPU and	thus the system time was advanced.	*/
// This code is only valid for the simulation of systems supporting multiprogramming.
// If activated for batch systems the simulation will fail.
schedulingEvent_t sim_check4UnblockedOrNew(pid_t* pPID) {
	pid_t             newPID = NO_PROCESS, unblockedPID = NO_PROCESS; 
	unsigned          unblockedTime      , startTime    = INT_MAX;    // timestamps of found events
	schedulingEvent_t releaseEvent                      = none; 

#ifdef SIM_MULTIPROGAMMING
	*pPID = NO_PROCESS;				// by default no process is returned
  // there are blocked processes
	if (!isBlockedListEmpty()) {
		unblockedTime = headOfBlockedList()->IOready; 
		if (unblockedTime <= systemTime) {
			releaseEvent = unblocked;
			unblockedPID = headOfBlockedList()->pid; 
		}
	}	
  // check when the next process becomes startable
	if (!sim_noMoreProcesses()) {
		if (((releaseEvent==unblocked) && sim_newProcessEvent(unblockedTime)) 
		|| ( (releaseEvent==none     ) && sim_newProcessEvent(systemTime))  )
		{	// Either the next new process is available before the found blocked process is unblocked
			// or there is no unblocked process and the new process is now available 
			releaseEvent = started;	                  // this happens first, so the event is the starting of a process
			newPID       = sim_makeNewProcessReady();	// initialise the new process
		}
	}
	switch (releaseEvent) {                      // choose the PID of the process causing the first event (if any)
	case unblocked: *pPID = unblockedPID; break; // unblocking of a process occured first
	case started  : *pPID = newPID      ; break; // the first event was the initialisiation of a new process
	case none     : *pPID = NO_PROCESS  ; break; // nothing happened...
	default:	// something irregular happened
		logGeneric("ERROR in simulation analysing events during process execution");
		exit(-1);
	}
#endif
	return releaseEvent; // return what kind of event occured
}



/* advances the system time until either a blocked process can be	*/
/* unblocked or a new process from the stimulus file is ready to	*/
/* be started. */
/* New processes from the stimulus file are initialised and stored	*/
/* in the processTable*/
/* Without multiprogramming the stimulus file is only checked for	*/
/* the next process to start if there are is no blocked process.	*/
schedulingEvent_t sim_wait4UnblockedOrNew(pid_t* pPID) {
	pid_t             newPID        = NO_PROCESS, unblockedPID = NO_PROCESS;
	unsigned          unblockedTime = INT_MAX   , startTime    = INT_MAX;   // timestamps of found events
	schedulingEvent_t releaseEvent  = none;
	*pPID = NO_PROCESS;				// by default no process is returned
	
  // check blocked list for next process to become unblocked
	if (!isBlockedListEmpty()) {		// there are blocked processes
		// check for next process to unblock, this is one candidate for the next scheduling event
		unblockedTime = headOfBlockedList()->IOready;
		releaseEvent  = unblocked;
		unblockedPID  = headOfBlockedList()->pid;
		if (unblockedTime <= systemTime) {
			logGeneric("ERROR in simulation, blocked list not correct: in idle state unblockable process found");
			exit(-1);
		}
	}
#ifndef SIM_MULTIPROGAMMING
	// only in multiprogramming both needs to be checked. 
	// In pure batch check for new processes is only allowed if no blocked process exists (thus the "else")
	else
#endif
	// check if there are further processes listed in the simulation-stimulus
	if (!sim_noMoreProcesses())
	{	// check when the next process becomes startable
		if (((releaseEvent == unblocked) && sim_newProcessEvent(unblockedTime))
			|| (releaseEvent == none) )
		{	// Either the next new process is available before the found blocked process is unblocked
			// or there is no unblocked process and there is still a new process available 
			releaseEvent = started;	// this happens first, so the event is the starting of a process
			newPID = sim_makeNewProcessReady();	// advance system time and initialise the new process
		}
	}
	// Now either a process for unblocking is found, a new one has been initialised or no more processes exist
	switch (releaseEvent)
	{
	case unblocked:
		// unblocking of a process occured first
		*pPID = unblockedPID;
		systemTime = unblockedTime;		// advance system time to the moment of unblocking
		break;
	case started:
		// the first event was the initialisiation of a new process
		*pPID = newPID;
		// system time was already updated when the new process was initialised
		break;
	case none:
		// nothing happened... so there is nothing to do --> stimulus is over, simulation complete
		*pPID = NO_PROCESS;
		break;
	default:	// something irregular happened
		logGeneric("ERROR in simulation analysing events in IDLE mode");
		exit(-1);
	}
	// return what kind of event occured
	return releaseEvent;
}


/* ---------------------------------------------------------------- */
/*                Implementation of local helper functions          */

Boolean sim_newProcessEvent(unsigned timestamp)
/* Predicate returning TRUE, if a new process is available at the	*/
/* time given in the parameter and shall be created and run			*/
/* A return value of FALSE indicating that currently no Process is	*/
/* available for execution, but there may be processes in the future*/
{
	if (!candidateAvailable) return FALSE;
	return ((signed)((processCandidate.start - timestamp)) <= 0);
}

pid_t sim_makeNewProcessReady(void)
/* Initialises the next procees given bay the stimulus file.		*/	
/* If the process is not yet ready to be startet, the system time	*/
/* is advanced until the process becomes startable					*/
{
	pid_t pid = 0;
	// check availablility of the process
	if (!sim_newProcessEvent(systemTime)) {
		logGeneric("Simulation: No process ready");
		while (!sim_newProcessEvent(systemTime)) {
			// check for availability of a new process in intervals of POLLING_INTERVAL
			systemTime = systemTime + POLLING_INTERVAL;
		}	// now there is a new process
		logGeneric("Simulation: Start time of next process in stimulus reached");
	}
	pid = getNextPid();
	if (pid == 0) {	// error handling 		
		logGeneric("++++ Fatal error: Process Table full");
		exit(-1);
	}
	processTable[pid].status = init;			// process is being initialised
	processTable[pid].usedCPU = 0;
	processTable[pid].pid = pid;
	if (!sim_getProcessParameters(pid)) {
		logGeneric("++++ Fatal error initiliasing new process");
		exit(-1);
	}
	processTable[pid].valid = TRUE;		// flag as valid pcb 
	// advance system time to reflect initialisation time 
	systemTime = systemTime + LOADING_DURATION;
	logPid(pid, "Simulation: Process loaded according to stimulus");

	return pid;
}

FILE* openStimulusFile(FILE *file, const char * filename)
{

	char linebuffer[129] = "x"; // read buffer for file-input
	// try to open file for read
	file = fopen(filename, "r");
	// test for success and error handling
	if (file == NULL)
	{
		logGeneric("Error opening file containing process information");
		return NULL;
	}
	// skip first line, only a comment
	if (!feof(file))
		fgets(linebuffer, 128, file);
	printf("Read from File: %s", linebuffer);
	candidateAvailable = TRUE; 
	return file;
}


Boolean readNextProcess(FILE* f, PCB_t* pProcess)
{
	char linebuffer[129] = "";		// read buffer for file-input
	char processTypeStr[21] = ""; 	// buffers for reading process type-string
	if (f == NULL) return FALSE;		// error: file handle not initialised
	if (feof(f)) {
		fclose(f);			// close the file on reaching EOF
		stimulusComplete = TRUE; // file completely processed
		return FALSE;		// error occured (EOF reached)
	}
	else {
		fgets(linebuffer, 128, f);
		if (strcmp(linebuffer, "") == 0) {
			logGeneric("Error reading process-info file: empty line");
			return FALSE;			// error occured: line is empty
		}
		else {
			printf("%6u : Simulation: Upcoming Process from File: %s\n", systemTime, linebuffer);
			if (sscanf(linebuffer, "%u %u %u %s", &pProcess->ownerID, &pProcess->start,
				&pProcess->duration, processTypeStr) != EOF) {
				if (strcmp(processTypeStr, "os") == 0)
					pProcess->type = os;
				else if (strcmp(processTypeStr, "interactive") == 0)
					pProcess->type = interactive;
				else if (strcmp(processTypeStr, "batch") == 0)
					pProcess->type = batch;
				else if (strcmp(processTypeStr, "background") == 0)
					pProcess->type = background;
				else if (strcmp(processTypeStr, "foreground") == 0)
					pProcess->type = foreground;
				else pProcess->type = os;
			}
		}
		pProcess->valid = TRUE; 
	}
	return TRUE;
}

