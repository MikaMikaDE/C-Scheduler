/* ---------------------------------------------------------------------------- */
// Include-file defining elementary data types used by the operating system
/* ---------------------------------------------------------------------------- */
#ifndef __BS_TYPES__
#define __BS_TYPES__


/* ---------------------------------------------------------------------------- */
// process id data type
/* ---------------------------------------------------------------------------- */
//ALTERATION BY MIKA: 
//sys/types.h to compile and test on linux systems
#if defined(__linux__) 
  #include <sys/types.h>
#else 
  typedef unsigned pid_t;
#endif
/* ---------------------------------------------------------------------------- */

#define LIST_SIZE 20

/* ---------------------------------------------------------------------------- */
// enums
/* ---------------------------------------------------------------------------- */
typedef enum { FALSE=0, TRUE                                            } Boolean; 
typedef enum { os,   interactive, batch, background, foreground         } ProcessType_t; 
typedef enum { init, running,     ready, blocked, ended                 } Status_t; 
typedef enum { none, started, completed, io,     quantumOver, unblocked } schedulingEvent_t;



/* ---------------------------------------------------------------------------- */
// Simulation Environment data type
// the information contained in this struct is not available to the OS
/* ---------------------------------------------------------------------------- */
typedef struct sim_info_struct {
	unsigned IOready;	  // simulation time when IO is complete, may be in the future
} sim_info_t;


/* ---------------------------------------------------------------------------- */
// Process List (ready state)
/* ---------------------------------------------------------------------------- */
typedef struct readyListElement_struct {
	pid_t	pid;
} readyListElement_t;

typedef struct readyList_struct { 
  readyListElement_t elems[LIST_SIZE];
  size_t             count;
} readyList_t; // queue of readyListElement_t




/* ---------------------------------------------------------------------------- */
// Block List (ready state) data type
// list of processes in blocked state
/* ---------------------------------------------------------------------------- */
typedef struct blockedListElement_struct {
	pid_t		 pid;
	unsigned IOready;
} blockedListElement_t;

typedef struct {
  blockedListElement_t elems[LIST_SIZE];
  size_t               count;
} blockedList_t; // queue of blockedListElement_t







/* ---------------------------------------------------------------------------- */
// Process Control Block data type
/* ---------------------------------------------------------------------------- */
typedef struct {
  Boolean       valid   ; 		
  pid_t         pid     ; 
  pid_t         ppid    ; 
  unsigned      ownerID ;
  unsigned      start   ; 
  unsigned      duration; 
  unsigned      usedCPU ; 
  ProcessType_t type    ; 
  Status_t      status  ;
  sim_info_t    simInfo ;
} PCB_t; 




#endif  /* __BS_TYPES__ */ 
