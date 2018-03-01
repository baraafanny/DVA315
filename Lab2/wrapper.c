#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "wrapper.h"
#include <strsafe.h>
#include <tchar.h>

#define TIMERID			100  /* id for timer that is used by the thread that manages the window where graphics is drawn */
#define DEFAULT_STACK_SIZE	1024
#define TIME_OUT			MAILSLOT_WAIT_FOREVER 

/* ATTENTION!!! calls that require a time out, use TIME_OUT constant, specifies that calls are blocked forever */


DWORD threadCreate (LPTHREAD_START_ROUTINE threadFunc, LPVOID threadParams) {
	
	/* Creates a thread running threadFunc */
	HANDLE thread = CreateThread(NULL, 0, threadFunc, threadParams, 0, NULL);
	if (thread) 
	{
		return thread;
	}
	else
	{
		printf("No thread was created\n");
	}
}


HANDLE mailslotCreate (char *name) {

	/* Creates a mailslot with the specified name and returns the handle */
	//Namn p� sloten, Hur stort meddelande den ska ta 0 f�r vilken storlek som helst, 0 Returns immediately if no message is present, NULL f�r optional.
	char name2[100];
	char* name3 = "\\\\.\\mailslot\\%s";
	sprintf_s(name2, 100, name3, name);
	HANDLE MyMailSlot = CreateMailslot(name2, 0, 0, NULL);
	if (MyMailSlot)
	{
		return MyMailSlot;
	}
	else
	{
		printf("Error in creating mailslot!\n");
	}

}

HANDLE mailslotConnect (char * name) {
	/* Connects to an existing mailslot for writing */
	/* and returns the handle upon success     */
	//Namnet p� filen, Skriv r�ttigheter, Man kan senare skriva till filen, Optional, G�r en ny fil alltid n�r detta anropas, Vanligaste valet, Optional.
	char name2[100];
	char* name3 = "\\\\.\\mailslot\\%s";
	sprintf_s(name2, 100, name3, name);
	HANDLE Connect = CreateFile(name2, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (Connect)
	{
		return Connect;
	}
	else
	{
		printf("Error connecting to mailslot!\n");
	}
}

int mailslotWrite(HANDLE mailSlot, void *msg, int msgSize) {
	LPDWORD numberofbytes = NULL;
	/* Write a msg to a mailslot, return nr */
	/* of successful bytes written         */
	HANDLE write = WriteFile(mailSlot, msg, msgSize, &numberofbytes, NULL);
	if(write)
	{
		return numberofbytes;
	}
	else
	{
		printf("Could not write to Mailslot\n");
		return 0;
	}
}

int	mailslotRead (HANDLE mailbox, void *msg, int msgSize) {

	BOOL Result;
	DWORD NumberofBytes;

	Result = ReadFile(mailbox, msg, msgSize, &NumberofBytes, NULL);
	if (Result)
	{
		return NumberofBytes;
	}
	else
	{
		return 0;
	}

}

int mailslotClose(HANDLE mailSlot){
	CloseHandle(mailSlot);

	/* close a mailslot, returning whatever the service call returns */
}


/******************** Wrappers for window management, used for lab 2 and 3 ***********************/
/******************** DONT CHANGE!!! JUST FYI ******************************************************/


HWND windowCreate (HINSTANCE hPI, HINSTANCE hI, int ncs, char *title, WNDPROC callbackFunc, int bgcolor) {

  HWND hWnd;
  WNDCLASS wc; 

  /* initialize and create the presentation window        */
  /* NOTE: The only important thing to you is that we     */
  /*       associate the function 'MainWndProc' with this */
  /*       window class. This function will be called by  */
  /*       windows when something happens to the window.  */
  if( !hPI) {
	 wc.lpszClassName = "GenericAppClass";
	 wc.lpfnWndProc = callbackFunc;          /* (this function is called when the window receives an event) */
	 wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	 wc.hInstance = hI;
	 wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	 wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	 wc.hbrBackground = (HBRUSH) bgcolor;
	 wc.lpszMenuName = "GenericAppMenu";

	 wc.cbClsExtra = 0;
	 wc.cbWndExtra = 0;

	 RegisterClass( &wc );
  }

  /* NOTE: This creates a window instance. Don't bother about the    */
  /*       parameters to this function. It is sufficient to know     */
  /*       that this function creates a window in which we can draw. */
  hWnd = CreateWindow( "GenericAppClass",
				 title,
				 WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
				 0,
				 0,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 NULL,
				 NULL,
				 hI,
				 NULL
				 );

  /* NOTE: This makes our window visible. */
  ShowWindow( hWnd, ncs );
  /* (window creation complete) */

  return hWnd;
}

void windowRefreshTimer (HWND hWnd, int updateFreq) {

  if(SetTimer(hWnd, TIMERID, updateFreq, NULL) == 0) {
	 /* NOTE: Example of how to use MessageBoxes, see the online help for details. */
	 MessageBox(NULL, "Failed setting timer", "Error!!", MB_OK);
	 exit (1);
  }
}


/******************** Wrappers for window management, used for lab  3 ***********************/
/*****  Lab 3: Check in MSDN GetOpenFileName and GetSaveFileName  *********/
/**************  what the parameters mean, and what you must call this function with *********/


HANDLE OpenFileDialog(char* string, DWORD accessMode, DWORD howToCreate)
{

	OPENFILENAME opf;
	char szFileName[_MAX_PATH]="";

	opf.Flags				= OFN_SHOWHELP | OFN_OVERWRITEPROMPT; 
	opf.lpstrDefExt			= "dat";
	opf.lpstrCustomFilter	= NULL;
	opf.lStructSize			= sizeof(OPENFILENAME);
	opf.hwndOwner			= NULL;
	opf.lpstrFilter			= NULL;
	opf.lpstrFile			= szFileName;
	opf.nMaxFile			= _MAX_PATH;
	opf.nMaxFileTitle		= _MAX_FNAME;
	opf.lpstrInitialDir		= NULL;
	opf.lpstrTitle			= string;
	opf.lpstrFileTitle		= NULL ; 
	
	if(accessMode == GENERIC_READ)
		GetOpenFileName(&opf);
	else
		GetSaveFileName(&opf);

	return CreateFile(szFileName, 
		accessMode, 
		0, 
		NULL, 
		howToCreate, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);


}

