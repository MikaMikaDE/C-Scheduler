#include "bs_types.h"
#include "globals.h"
#include "dispatcher.h"
#include "core.h"

/* ----------------------------------------------------------------	*/
/* Declare global variables according to definition in globals.h 	 */
unsigned systemTime = 0; 	// the current system time (up time)

int main(int argc, char *argv[])
{	// starting point, all processing is done in called functions
	sim_initSim();		// initialise simulation run-time environment
	initOS();		      // initialise operating system
	logGeneric("Starting stimulus-based simulation");
	coreLoop();		    // start scheduling loop
	logGeneric("Simulation complete, shutting down");
	sim_closeSim();		// shut down simulation envoronment
	fflush(stdout);		// make sure the output on the console is complete 
  readyList_free(readyList);
  blockedList_free(blockedList);
	return 1; 
}
