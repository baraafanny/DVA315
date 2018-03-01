#include "wrapper.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
HANDLE Mutex;
HANDLE MyMailSlot;
BOOL loop = TRUE;
char* pmail = "MinMailSlot";

typedef struct test 
{
	char message[100];
} MyStructMessage;

DWORD WINAPI ThreadFunc(void *data)
{

	DWORD WaitResult = WaitForSingleObject(
		Mutex,    // handle to mutex
		INFINITE);  // no time-out interval
	MyStructMessage Input;
	while (loop == TRUE)
	{
		switch (WaitResult)
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			__try
			{
				HANDLE MyMailSlot2 = mailslotConnect(pmail);
				if (MyMailSlot2)
				{

					printf("Write Message here:\n");
					fgets(Input.message, 50, stdin);
					int structlength = sizeof(Input);
					if (Input.message != NULL)
					{
						int numberofbytes = mailslotWrite(MyMailSlot2, &Input.message, structlength);
						printf("%d number of bytes where written.\n", numberofbytes);
					}
					if (strcmp(Input.message, "END\n") == 0)
					{
						loop = FALSE;
					}
				}
				else
				{
					printf("Error connecting to Mailslot");
				}
				mailslotClose(MyMailSlot2);

			}
			__finally {
				// Release ownership of the mutex object
				if (!ReleaseMutex(Mutex))
				{
					printf("Error when releasing ownership of mutex\n");
				}
			}
			break;
		case WAIT_ABANDONED:
			return FALSE;
		}

		WaitResult = WaitForSingleObject(
			Mutex,    // handle to mutex
			INFINITE);  // no time-out interval
	}
	return TRUE;
}
DWORD WINAPI ThreadFunc2(void *data)
{
	char *inputstr = NULL;
	int msgSize = 0, NumberofMessage = 0;
	Sleep(1000);
	BOOL result;
	
	while (loop == TRUE)
	{
		DWORD WaitResult = WaitForSingleObject(
			Mutex,    // handle to mutex
			INFINITE);  // no time-out interval
		switch (WaitResult)
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			__try {
					result = GetMailslotInfo(MyMailSlot, // mailslot handle 
					(LPDWORD)NULL,               // no maximum message size 
					&msgSize,                   // size of next message 
					&NumberofMessage,                    // number of messages 
					(LPDWORD)NULL);              // no read time-out 
					if (result)
					{
						inputstr = (MyStructMessage*)malloc(msgSize);
						int numberofbytes = mailslotRead(MyMailSlot, inputstr, msgSize);
						if (strcmp(inputstr, "END") == 0)
						{
							loop = FALSE;
						}
						printf("%d number of bytes where read.\n", numberofbytes);
						printf("%s\n", inputstr);
					}
					else
					{
						printf("Error get mailslotinfo");
						printf("%d", GetLastError());
					}
				
			}
			__finally {
				// Release ownership of the mutex object
				if (!ReleaseMutex(Mutex))
				{
					printf("Error when releasing ownership of mutex\n");
				}
			}
			break;
		case WAIT_ABANDONED:
			return FALSE;
		}
	}
	return TRUE;

}
int main()
{
	Mutex = CreateMutex(NULL, FALSE, NULL);

	if (!Mutex)
	{
		printf("Error, %d", GetLastError());
	}
	else
	{
		MyMailSlot = mailslotCreate(pmail);
		
		HANDLE thread1 = threadCreate(ThreadFunc, NULL);
		HANDLE thread2 = threadCreate(ThreadFunc2, NULL);
		
		WaitForSingleObject(thread1, INFINITE);
		WaitForSingleObject(thread2, INFINITE);
		CloseHandle(thread1);
		CloseHandle(thread2);
		getchar();
	}
	return 0;
}
