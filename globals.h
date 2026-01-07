/* Include-file defining global defines and constants */
/* used by the operating system */
#ifndef __GLOBALS__
#define __GLOBALS__

#define	_CRT_SECURE_NO_WARNINGS		// suppress legacy warnings 

#include "bs_types.h"
#include "core.h"
#include "log.h"
#include "scheduler.h"
#include "simruntime.h"

#define SIM_MULTIPROGAMMING TRUE  // if this directive is enabled, the simulation supports multiprogramming. 
#define NO_RANDOM_SEED            // if this directive is enabled, the random number generator is seeded with zero
#define NUM_PROCESSES 20    // Number of possible concurrent processes 
#define NO_PROCESS     0    // PID of zero is used to indicate that no matching process exists
#define QUANTUM       50    // Maximum duration one process gets the CPU, zero indicates no preemption
//
#define PROCESS_FILENAME "processes.txt" // name of the file with process definitions

/* ----------------------------------------------------------------	*/
/* Define global variables that will be visible in all sourcefiles	*/
/* ----------------------------------------------------------------	*/
extern PCB_t          processTable[NUM_PROCESSES+1]; 	// the process table
extern unsigned       systemTime; 	// the current system time (up time)
extern readyList_t    *readyList;	  // pointer to queue of runnable processes
extern blockedList_t  *blockedList;	// pointer to blocked process

#endif  /* __GLOBALS__ */ 
