#ifndef ENTRY_H_ 
#define ENTRY_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



VOID NTAPI DriverUnload(IN DRIVER_OBJECT *DriverObject);

typedef struct _CONNECT_DATA {
	PDEVICE_OBJECT ClassDeviceObject;
	PVOID          ClassService;
} CONNECT_DATA, *PCONNECT_DATA;

#endif  
