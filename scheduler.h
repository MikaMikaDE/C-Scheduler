/* header-file defining the interface of the scheduler */
/* the scheduler contains all functions required to decide which process	*/
/* to run next																*/
/* +++	this version only implements a batch-scheuling using FIFO without	*/
/* +++	SPOOLING. It needs to be extended to support multiprogramming		*/

#ifndef __SCHEDULER__
#define __SCHEDULER__

#include "bs_types.h"
#include "globals.h"
#include "dispatcher.h"


pid_t schedule(readyList_t *readyList);
/* the parameter readyList passes the data structure holding all runnable	*/
/* processes, of which the function schedule() selects one to run next		*/
/* returns a pointer to the next process to run								*/
/* returns NULL if no runnable process exists								*/



#endif /* __SCHEDULER__ */
