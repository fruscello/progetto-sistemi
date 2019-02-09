
#ifndef HIGHHANDLER_H
#define HIGHHANDLER_H
#define READTERMINAL 11
#define WRITETERMINAL 12
#define VSEMVIRT 13
#define PSEMVIRT 14
#define DELAY 15
#define DISK_PUT 16
#define DISK_GET 17
#define WRITEPRINTER 18
#define GETTOD 19
#define TERMINATE 20
#include <types.h>
#include <arch.h>
void schedule(state_t *old);
void highSysHandler();

#endif // MAIN_H
