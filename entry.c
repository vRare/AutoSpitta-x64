#include "mousehook.h"
#include "keyboardhook.h"
#include "entry.h" 
#include "hacks.h"

#define IOCTL_PDC CTL_CODE( FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS )

NTSTATUS Function_IRP_MJ_CREATE(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS Function_IRP_MJ_CLOSE(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS Function_IRP_UNSUPPORTED(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	DbgPrint("An unsupported majour function was requested. Returning STATUS_SUCCESS");
	return NtStatus;
}

NTSTATUS Function_IRP_DEVICE_CONTROL(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	PIO_STACK_LOCATION			pIoStackLocation;
	NTSTATUS					status = STATUS_SUCCESS;
	PVOID						pBuf = Irp->AssociatedIrp.SystemBuffer;
	ULONG						bytesIO = 0;
	ULONG						outputBufferLength;

	pIoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	outputBufferLength = pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

	do {

		if (pIoStackLocation == NULL) {
			status = STATUS_INTERNAL_ERROR;
			break;
		}

		pBuf = Irp->AssociatedIrp.SystemBuffer;
		if (pBuf == NULL) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		switch (pIoStackLocation->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_PDC:
		{
			bytesIO = 0;
			if ((pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (unsigned long)sizeof(PASSDATA)) { // 32x PTR to 64x PTR
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			// FUNCTION THAT COPIES MEMORY
			status = shotbot_thread((PASSDATACTRL)pBuf);
			break;
		}

		default:
		{
			status = STATUS_INVALID_PARAMETER;
		}
		}

	} while (FALSE);

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = bytesIO;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS AutoSpitta_InternalIoctl(PDEVICE_OBJECT device, PIRP irp)
{
	PIO_STACK_LOCATION ios;
	PCONNECT_DATA cd;
	NTSTATUS status;

	ios = IoGetCurrentIrpStackLocation(irp);

	if (ios->Parameters.DeviceIoControl.IoControlCode == MOUCLASS_CONNECT_REQUEST)
	{
		cd = ios->Parameters.DeviceIoControl.Type3InputBuffer;

		MouseDpcRoutine = (MouseServiceDpc)cd->ClassService;
	//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Mouse_IO_InternalIoctl ----> MouseDpcRoutine[%x]\n", MouseDpcRoutine);
	}

	else if (ios->Parameters.DeviceIoControl.IoControlCode == KBDCLASS_CONNECT_REQUEST)
	{
		cd = ios->Parameters.DeviceIoControl.Type3InputBuffer;

		KeyboardDpcRoutine = (KeyboardServiceDpc)cd->ClassService;
	//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Keyboard_IO_InternalIoctl ----> KeyboardDpcRoutine[%x]\n", KeyboardDpcRoutine);
	}

	else
	{
		Function_IRP_UNSUPPORTED(device, irp);

	}
	
	return STATUS_SUCCESS;

}

const WCHAR deviceNameBuffer[] = L"\\Device\\AutoSpitta";
const WCHAR deviceSymLinkBuffer[] = L"\\DosDevices\\AutoSpitta";
PDEVICE_OBJECT g_MyDevice;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT driverObject, IN PUNICODE_STRING regPath)
{
	CLIENT_ID nthread;
	HANDLE thread;

	NTSTATUS ntStatus = 0;
	UNICODE_STRING deviceNameUnicodeString, deviceSymLinkUnicodeString;

	// Normalize name and symbolic link.
	RtlInitUnicodeString(&deviceNameUnicodeString,
		deviceNameBuffer);
	RtlInitUnicodeString(&deviceSymLinkUnicodeString,
		deviceSymLinkBuffer);
	 
	// Create the device.
	ntStatus = IoCreateDevice(driverObject,
		0, // For driver extension
		&deviceNameUnicodeString,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_UNKNOWN,
		FALSE,
		&g_MyDevice);

	// Create the symbolic link
	ntStatus = IoCreateSymbolicLink(&deviceSymLinkUnicodeString,
		&deviceNameUnicodeString);

	driverObject->DriverUnload = DriverUnload;

	unsigned int i;
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		driverObject->MajorFunction[i] = Function_IRP_UNSUPPORTED;

    driverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = AutoSpitta_InternalIoctl;
	driverObject->MajorFunction[IRP_MJ_CREATE] = Function_IRP_MJ_CREATE;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = Function_IRP_MJ_CLOSE;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Function_IRP_DEVICE_CONTROL;
	

	DbgPrint("AutoSpitta V1 loaded./n");

	Mouse_Create(driverObject);
	Keyboard_Create(driverObject);


	//PsCreateSystemThread(&thread, STANDARD_RIGHTS_ALL, NULL, NULL, &nthread, (PKSTART_ROUTINE)SystemRoutine, NULL);

	//ZwClose(thread);


	return STATUS_SUCCESS;
}

VOID NTAPI DriverUnload(IN DRIVER_OBJECT *DriverObject) { 
	UNICODE_STRING symLink;
	RtlInitUnicodeString(&symLink, deviceSymLinkBuffer);
	IoDeleteSymbolicLink(&symLink);
	Mouse_Close(DriverObject);
	Keyboard_Close(DriverObject);
	IoDeleteDevice(DriverObject->DeviceObject);

	DbgPrint("AutoSpitta V1 unloaded./n");
}
