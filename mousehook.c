#include "shared.h"
#include "mousehook.h"
#include "hacks.h"
#include <stdio.h>


#define Tag		'MOUSE'
#define MOUSECLASS_0			0x1
#define MOUSECLASS_1			0x2
#define MOUSECLASS_2			0x4
#define ALLOC_SIZE				0x1000

MouseServiceDpc MouseDpcRoutine = NULL;
mouinput MouseInputRoutine=NULL;
PMOUSE_INPUT_DATA mouIrp=NULL;
char MOU_DATA[5];
ULONG mouId=0;

MOUSE_INPUT_DATA mdata;
MOUSE_INPUT_DATA middlemousedata;

static ULONG	g_Mouseclsnum;
static LONG	g_StartCount = 0;
static ULONG	g_nop = 0;
static PVOID g_node=0;

static PDEVICE_OBJECT	g_MouseDeviceObject = NULL;
static PDEVICE_OBJECT	g_IOMouseDeviceObject = NULL;

static PDEVICE_OBJECT	g_TopOfStack;
static IRPMJREAD		g_OldReadFunction;
static IRPMJREAD		g_OldInternalDeviceFunction;
ULONGLONG *g_mouse_rootine = NULL;


static int mouse_inited = 0;

NTSTATUS Mouse_Create(IN PDRIVER_OBJECT driverObject)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING uniKbdDeviceName;
	PDEVICE_OBJECT devicePtr;
	MouseAddDevice MouseAddDevicePtr;
	ULONGLONG node=0;

	memset((void*)&mdata,0,sizeof(mdata));
	memset((void*)&middlemousedata, 0, sizeof(middlemousedata));
	memset((void*)MOU_DATA,0,sizeof(MOU_DATA));

	RtlInitUnicodeString(&uniKbdDeviceName,L"\\Device\\IOMouse");
	status = IoCreateDevice(driverObject,0,&uniKbdDeviceName,FILE_DEVICE_UNKNOWN,FILE_DEVICE_SECURE_OPEN,FALSE,&g_IOMouseDeviceObject);
	if(!NT_SUCCESS(status)) {
		return status;
	}
	g_IOMouseDeviceObject->Flags|=DO_BUFFERED_IO; 
	g_IOMouseDeviceObject->Flags&=~DO_DEVICE_INITIALIZING;


	status = Mouse_Hook(driverObject);
	if(!NT_SUCCESS(status)) {
		g_IOMouseDeviceObject = NULL;
		return status;
	}

	FindDevNodeRecurse(g_MouseDeviceObject,&node);
	if(g_IOMouseDeviceObject->DeviceObjectExtension) {
		g_node = g_IOMouseDeviceObject->DeviceObjectExtension->DeviceNode;
		g_IOMouseDeviceObject->DeviceObjectExtension->DeviceNode=(PVOID)node;
	}

	if(g_MouseDeviceObject->DriverObject) {

		if(g_MouseDeviceObject->DriverObject->DriverExtension) {
			MouseAddDevicePtr=(MouseAddDevice)g_MouseDeviceObject->DriverObject->DriverExtension->AddDevice;

			if(MouseAddDevicePtr) {
				MouseAddDevicePtr(g_MouseDeviceObject->DriverObject,g_IOMouseDeviceObject);
			}
		}
	}
	DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Mouse_Create ----> status[%x].\n", status);
	mouse_inited = 1;

	return status;

}
NTSTATUS Mouse_Close(IN PDRIVER_OBJECT driverObject)
{
	NTSTATUS status = STATUS_SUCCESS;
	
	if( mouse_inited == 0 ) return status;

	Mouse_UnHook(driverObject);

	if(g_IOMouseDeviceObject) {
		if(g_IOMouseDeviceObject->DeviceObjectExtension) g_IOMouseDeviceObject->DeviceObjectExtension->DeviceNode = g_node;
		IoDeleteDevice(g_IOMouseDeviceObject);
		g_IOMouseDeviceObject = NULL;
	}

	return status;

}

NTSTATUS Mouse_Hook(IN PDRIVER_OBJECT driverObject)
{
	ULONG i;
	int found = 1;
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack;
	UNICODE_STRING uniMouseDeviceName;
	PFILE_OBJECT mouseFileObject;
	PDEVICE_OBJECT MouseDeviceObject;
	ULONG counter;
	HANDLE hDir;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING uniOa;
	PVOID pBuffer;
	PVOID pContext;
	ULONG RetLen;
	PDIRECTORY_BASIC_INFORMATION pDirBasicInfo;
	UNICODE_STRING uniMouseDrv;
	char* mouseclsNum;
	char arMouseCls[0x10];
	WCHAR tmpNameBuffer[512];

	DbgPrint("Running Mouse_Create.");


	RtlInitUnicodeString(&uniOa, L"\\Device");

	InitializeObjectAttributes(	&oa,&uniOa,	OBJ_CASE_INSENSITIVE,NULL,NULL);

	status = ZwOpenDirectoryObject(	&hDir,DIRECTORY_ALL_ACCESS,&oa);
	if(!NT_SUCCESS(status)) return status;

	pBuffer = ExAllocatePool(PagedPool, ALLOC_SIZE);
	pContext = ExAllocatePool(PagedPool, ALLOC_SIZE);
// 		pBuffer = ExAllocatePoolWithTag(PagedPool, ALLOC_SIZE, Tag);
// 		pContext = ExAllocatePoolWithTag(PagedPool, ALLOC_SIZE, Tag);
	memset(pBuffer, 0, ALLOC_SIZE);
	memset(pContext, 0, ALLOC_SIZE);
	memset(arMouseCls, 0, 0x10);
	counter = 0;
	g_Mouseclsnum = 0;

	while(TRUE)	{
		status = ZwQueryDirectoryObject(hDir,pBuffer,ALLOC_SIZE,TRUE,FALSE,pContext,&RetLen);
		if(!NT_SUCCESS(status)) break;

		pDirBasicInfo =	(PDIRECTORY_BASIC_INFORMATION)pBuffer;
		pDirBasicInfo->ObjectName.Length -= 2;

		RtlInitUnicodeString(&uniMouseDrv, L"PointerClass");
		if(RtlCompareUnicodeString(	&pDirBasicInfo->ObjectName, &uniMouseDrv,FALSE) == 0){
			mouId = (ULONG)(*(char *)(pDirBasicInfo->ObjectName.Buffer+pDirBasicInfo->ObjectName.Length));
			mouId -= 0x30;
			pDirBasicInfo->ObjectName.Length += 2;
			RtlInitUnicodeString(&uniMouseDeviceName, pDirBasicInfo->ObjectName.Buffer);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "IDNAME: \n", uniMouseDeviceName);
			found = 1;
			break;
		}
		pDirBasicInfo->ObjectName.Length += 2;
	}
	ExFreePool(pBuffer); 
	ExFreePool(pContext);
	ZwClose(hDir);

	if (found == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Device finder failed.\n", mouId);
		return status;
	}

	if (found == 1) {
		DbgPrint("Device finder sucess!\n");
	}
	DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Mouse_Hook ----> mouId[%d].\n", mouId);
				
	swprintf(tmpNameBuffer, L"\\Device\\%s", uniMouseDeviceName.Buffer);
	RtlInitUnicodeString(&uniMouseDeviceName, tmpNameBuffer);

	status = My_IoGetDeviceObjectPointer(&uniMouseDeviceName, FILE_ALL_ACCESS, &mouseFileObject,&MouseDeviceObject);

	if(NT_SUCCESS(status)) {
		g_MouseDeviceObject = MouseDeviceObject;
		ObDereferenceObject(mouseFileObject);
		g_OldReadFunction = g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ];
		g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ] = Mouse_HookProc;
		g_OldInternalDeviceFunction = g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL];
		g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = Mouse_IO_InternalIoctl;

		DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Mouse_Create ----> Mouse_HookProc success.\n");
	}
	return status;
}
NTSTATUS Mouse_UnHook(IN PDRIVER_OBJECT driverObject)
{
	ULONG i;
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack;
	UNICODE_STRING uniMouseDeviceName;
	PFILE_OBJECT mouseFileObject;
	PDEVICE_OBJECT MouseDeviceObject;
	ULONG counter;
	HANDLE hDir;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING uniOa;
	PVOID pBuffer;
	PVOID pContext;
	ULONG RetLen;
	PDIRECTORY_BASIC_INFORMATION pDirBasicInfo;
	UNICODE_STRING uniMouseDrv;
	char* mouseclsNum;
	char arMouseCls[0x10];

	if(g_mouse_rootine) *g_mouse_rootine = (ULONGLONG)MouseInputRoutine;
	if(g_OldReadFunction) {
		g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ] = g_OldReadFunction;
		g_OldReadFunction = NULL;
	}
	if(g_OldInternalDeviceFunction) {
		g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = g_OldInternalDeviceFunction;
		g_OldInternalDeviceFunction = NULL;
	}

	return status;
}

NTSTATUS MouseApc(void *a1, void *a2, void *a3, void *a4, void *a5)
{
	if(mouIrp->ButtonFlags&MOUSE_LEFT_BUTTON_DOWN)
	{
		MOU_DATA[0]=1;
	}
	else if(mouIrp->ButtonFlags&MOUSE_LEFT_BUTTON_UP)
	{
		MOU_DATA[0]=0;
	}
	else if(mouIrp->ButtonFlags&MOUSE_RIGHT_BUTTON_DOWN)
	{
		MOU_DATA[1]=1;
	}
	else if(mouIrp->ButtonFlags&MOUSE_RIGHT_BUTTON_UP)
	{
		MOU_DATA[1]=0;
	}
	else if(mouIrp->ButtonFlags&MOUSE_MIDDLE_BUTTON_DOWN)
	{
		MOU_DATA[2]=1;
	}
	else if(mouIrp->ButtonFlags&MOUSE_MIDDLE_BUTTON_UP)
	{
		MOU_DATA[2]=0;
	}
	else if(mouIrp->ButtonFlags&MOUSE_BUTTON_4_DOWN)
	{
		MOU_DATA[3]=1;
	}
	else if(mouIrp->ButtonFlags&MOUSE_BUTTON_4_UP)
	{
		MOU_DATA[3]=0;
	}
	else if(mouIrp->ButtonFlags&MOUSE_BUTTON_5_DOWN)
	{
		MOU_DATA[4]=1;
	}
	else if(mouIrp->ButtonFlags&MOUSE_BUTTON_5_UP)
	{
		MOU_DATA[4]=0;
	}

	return MouseInputRoutine(a1,a2,a3,a4,a5);
}

NTSTATUS Mouse_HookProc( IN PDEVICE_OBJECT DeviceObject,	IN PIRP Irp	)
{
	NTSTATUS status;

	ULONGLONG *routine;

//	DbgPrint("Mouse_HookProc: success");

	routine=(ULONGLONG*)Irp;

	routine+=0xb;

	if(!MouseInputRoutine)
	{
		MouseInputRoutine=(mouinput)*routine;
	}

	*routine=(ULONGLONG)MouseApc;
	g_mouse_rootine = routine;

	mouIrp=(PMOUSE_INPUT_DATA)Irp->UserBuffer;

	status = g_OldReadFunction(DeviceObject, Irp);
	return status;
}
NTSTATUS Mouse_IO_InternalIoctl(PDEVICE_OBJECT device, PIRP irp)
{
	PIO_STACK_LOCATION ios;
	PCONNECT_DATA cd;
	NTSTATUS status;

	ios=IoGetCurrentIrpStackLocation(irp);

	if(ios->Parameters.DeviceIoControl.IoControlCode==MOUCLASS_CONNECT_REQUEST)
	{
		cd=ios->Parameters.DeviceIoControl.Type3InputBuffer;

		MouseDpcRoutine=(MouseServiceDpc)cd->ClassService;
		DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Mouse_IO_InternalIoctl ----> MouseDpcRoutine[%x]\n", MouseDpcRoutine);
	}

	//status = g_OldInternalDeviceFunction(device, irp);

	return STATUS_SUCCESS;
}


int GetMouseState(int key)
{
	if(MOU_DATA[key]) return 1;

	return 0;
}
void SynthesizeMouse(PMOUSE_INPUT_DATA a1)
{
	KIRQL irql;
	char *endptr;
	ULONG fill=1;

	endptr=(char*)a1;

	endptr+=sizeof(MOUSE_INPUT_DATA);

	a1->UnitId=(USHORT)mouId;

	KeRaiseIrql(DISPATCH_LEVEL,&irql);

	if(MouseDpcRoutine) MouseDpcRoutine(g_MouseDeviceObject,a1,(PMOUSE_INPUT_DATA)endptr,&fill);

	KeLowerIrql(irql);
}

void mouse_click()
{
	mdata.ButtonFlags |= MOUSE_RIGHT_BUTTON_DOWN;
	SynthesizeMouse(&mdata);

	Sleep(50);

	mdata.ButtonFlags &= ~MOUSE_RIGHT_BUTTON_DOWN;
	mdata.ButtonFlags |= MOUSE_RIGHT_BUTTON_UP;
	SynthesizeMouse(&mdata);
}

void reload_click()
{
	middlemousedata.ButtonFlags |= MOUSE_MIDDLE_BUTTON_DOWN;
	SynthesizeMouse(&middlemousedata);

	Sleep(50);

	middlemousedata.ButtonFlags &= MOUSE_MIDDLE_BUTTON_DOWN;
	middlemousedata.ButtonFlags |= MOUSE_MIDDLE_BUTTON_UP;
	SynthesizeMouse(&middlemousedata);
}
