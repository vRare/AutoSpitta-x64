#include "hacks.h"
#include "mousehook.h"
#include "keyboardhook.h"
#include <ntifs.h>

int procID = 0, memaddr = 0;


NTSTATUS Sleep(ULONGLONG milliseconds)
{
	LARGE_INTEGER delay;
	ULONG *split;

	milliseconds *= 1000000;

	milliseconds /= 100;

	milliseconds = -milliseconds;

	split = (ULONG*)&milliseconds;

	delay.LowPart = *split;

	split++;

	delay.HighPart = *split;


	KeDelayExecutionThread(KernelMode, 0, &delay);

	return STATUS_SUCCESS;
}


UINT32 ReadMemory(int PID, int Addr) {

	KAPC_STATE apc;
	UINT32 Buffer = 0;
	static PEPROCESS GameProcess;
	UNREFERENCED_PARAMETER(Addr);

		if (!GameProcess)
			PsLookupProcessByProcessId((HANDLE)PID, &GameProcess);

		if (GameProcess) {
			try {
				if ((void*)Addr != NULL) {
					KeStackAttachProcess(GameProcess, &apc);
					RtlCopyMemory(&Buffer, (const void*)Addr, sizeof(UINT32));
					KeUnstackDetachProcess(&apc);
				}
			}
			except(EXCEPTION_EXECUTE_HANDLER)
			{
				DbgPrint("We hit an Exeption");
				KeUnstackDetachProcess(&apc);
				ObDereferenceObject(GameProcess);
				return NULL;
			}
		}
		return Buffer;
	}
	

NTSTATUS shotbot_thread( PASSDATACTRL pd )
{
	NTSTATUS status;

	if (pd->controlswitch == TRUE)
	{
		if (pd->address != NULL && pd->pid != NULL)
		{
			int crosshair = ReadMemory((int)pd->pid, (int)pd->address);

			if (crosshair > 0) {

				if (!GetMouseState(0)) // if mouse button isn't already down.
				{
					DbgPrint("Crosshair picked! Sending input..\n");
					mouse_click();
				}
			}

		}
	}

	return STATUS_SUCCESS;
}

//Use these scan codes for GetKeyState()
//http://msdn.microsoft.com/en-us/library/aa299374%28v=vs.60%29.aspx

//#define __F4 62
//#define __F5 63
//typedef ULONGLONG FWORD;
//NTSTATUS SystemRoutine()
//{
//	FWORD gamebase = 0;
//	int activate = 0;
//
//	while (TRUE)
//	{
//
//		if (GetKeyState(__F4))
//		{
//			// ENABLE
//			activate = 1;
//		}
//
//		if (GetKeyState(__F5))
//		{
//			// DISABLE
//			activate = 0;
//		}
//
//
//		if (/**procID != 0 &&  **/ activate != 0)
//		{
//			int crosshair = ReadMemory(4140, 0x44EA9844);
//
//
//			/**	    0 - Left mouse
//					1 - Right mouse
//					2 - Middle button
//					3 - Mouse button 4
//					4 - Mouse button 5
//					**/
//			if (crosshair > 0) {
//
//				if (!GetMouseState(0)) // if mouse button isn't already down.
//				{
//					DbgPrint("Crosshair picked! Sending input..\n");
//					mouse_click();
//					Sleep(350);
//					reload_click();
//				}
//			}
//		}
//
//		Sleep(5);
//	}
//
//	return STATUS_SUCCESS;
//}
