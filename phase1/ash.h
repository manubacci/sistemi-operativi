#ifndef ACTIVE_SEMAPHORE_HASH
#define ACTIVE_SEMAPHORE_HASH

#include "pandos_types.h"
#include "types.h"
#include "hashtable.h"
#include "pcb.h"

int insertBlocked (int* semAdd, pcb_t* p);
pcb_t* removeBlocked (int* semAdd);
pcb_t* outBlocked (pcb_t* p);
pcb_t* headBlocked (int* semAdd);

void initASH ();

#endif