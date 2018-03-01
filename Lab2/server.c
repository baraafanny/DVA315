/********************************************************************\
* server.c                                                           *
*                                                                    *
* Desc: example of the server-side of an application                 *
* Revised: Dag Nystrom & Jukka Maki-Turja                     *
*                                                                    *
* Based on generic.c from Microsoft.                                 *
*                                                                    *
*  Functions:                                                        *
*     WinMain      - Application entry point                         *
*     MainWndProc  - main window procedure                           *
*                                                                    *
* NOTE: this program uses some graphic primitives provided by Win32, *
* therefore there are probably a lot of things that are unfamiliar   *
* to you. There are comments in this file that indicates where it is *
* appropriate to place your code.                                    *
* *******************************************************************/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "wrapper.h"
#include "Planet.h"

							/* the server uses a timer to periodically update the presentation window */
							/* here is the timer id and timer period defined                          */

#define UPDATE_FREQ     10	/* update frequency (in ms) for the timer */

							/* (the server uses a mailslot for incoming client requests) */



/*********************  Prototypes  ***************************/
/* NOTE: Windows has defined its own set of types. When the   */
/*       types are of importance to you we will write comments*/ 
/*       to indicate that. (Ignore them for now.)             */
/**************************************************************/

LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );
DWORD WINAPI mailThread(LPVOID);



HDC hDC;		/* Handle to Device Context, gets set 1st time in MainWndProc */
				/* we need it to access the window for printing and drawin */

/********************************************************************\
* Function: sendMessageToClient                                      *
* Purpose: Skickar meddelande om vad som hände med                   *
* planeten till clienten                                             *
/********************************************************************/
void sendMessageToClient(char *CurrentPID, int State)
{
	char Mailboxname[13];
	HANDLE ClientMailSlot;
	char* name3 = "Mailbox%s";
	sprintf_s(Mailboxname, sizeof(Mailboxname), name3, CurrentPID); //Namnet på vår mailbox blir tex Mailbox13772 om PID är 13772.

	ClientMailSlot = mailslotConnect(Mailboxname);

	if (ClientMailSlot != INVALID_HANDLE_VALUE) 
	{
		if (State == 1)//Planeten åkte utanför bounds.
		{
			char message[30] = "Ur planet went out of bounds!";
			int messagesize = sizeof(message);
			int numberofbytes = mailslotWrite(ClientMailSlot, &message, messagesize);
			printf("%d number of bytes where written.\n", numberofbytes);

		}
		else if(State == 2)//Planeten dog
		{
			char message[16] = "Ur planet died!";
			int messagesize = sizeof(message);
			int numberofbytes = mailslotWrite(ClientMailSlot, &message, messagesize);
			printf("%d number of bytes where written.\n", numberofbytes);
		}
		else
		{
			printf("Error messagetoclient func!");
		}
	}
	mailslotClose(ClientMailSlot);
}
//Algoritm för att beräkna radien/sträckan mellan två planeter
int calcRadius(int x1, int x2, int y1, int y2)
{
	return sqrt((pow((x2 - x1), 2) + pow((y2 - y1), 2)));
}
/********************************************************************\
* Function: planetThread                                             *
* Purpose: Planet beräkningar, planet info uppdatering.              *                          
/********************************************************************/
DWORD WINAPI planetThread(void *data)
{
	planet_type *CurrentPlanet = data; //Pekare till vår aktiva planet

	while (CurrentPlanet->life > 0) //Gör beräkningar för vår planet så länge som den inte dör
	{
		planet_type *Current = Universe; // Pekare för att traversera lista.
		double totalAccelerationX = 0;
		double totalAccelerationY = 0;
		float gravity = 6.67259*pow(10, -11);

		while (Current != NULL) //Loopa egenom alla planeter i vårat Universum(Listan)
		{
			if (Current != CurrentPlanet) //Gör beräkningar för alla planeter förutom våran egen
			{
				double radius = 0;
				double sqrRadius = 0;
				double acceleration1 = 0;
				double accelerationX = 0;
				double accelerationY = 0;

				//Med detta menas att vi vill ha den totala påverkan från alla andra planeter 
				//Detta läggs till på totalAccelerationX och totalAccelerationY som vi vill ha i slutet

				//Beräkningar här!
				/* Vi ska representera följande formler:
				Fs = Gravity * ((Mass1 * Mass2) / Radius^2) där Radius är avståndet från planeternas medelpunkter
				Fs kan också beskrivas som Fs = Mass1 * Acceleration1
				Detta ger oss att Mass1 * Acceleration1 = Gravity * ((Mass1 * Mass2) / Radius^2)
				Acceleration1 = Gravity * (Mass2 / Radius^2)
				*/

				DWORD WaitResult = WaitForSingleObject(
					Mutex,    // handle to mutex
					INFINITE);  // no time-out interval
				switch (WaitResult)
				{
					// The thread got ownership of the mutex
				case WAIT_OBJECT_0:
					__try
					{

						radius = calcRadius(CurrentPlanet->sx, Current->sx, CurrentPlanet->sy, Current->sy);
						if (radius == 0)
						{
							Current->vx = CurrentPlanet->vx * (1 / 2);
							Current->vy = CurrentPlanet->vy * (1 / 2);
							CurrentPlanet->vx * (-1 / 2);
							CurrentPlanet->vy * (-1 / 2);
							radius = 1;

						}
						sqrRadius = pow(radius, 2);

						acceleration1 = gravity * (Current->mass / sqrRadius);

					    accelerationX = acceleration1 * ((Current->sx - CurrentPlanet->sx) / radius);
					    accelerationY = acceleration1 * ((Current->sy - CurrentPlanet->sy) / radius);
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

				totalAccelerationX += accelerationX;
				totalAccelerationY += accelerationY;
			}
			DWORD WaitResult = WaitForSingleObject(
				Mutex,    // handle to mutex
				INFINITE);  // no time-out interval
			switch (WaitResult)
			{
				// The thread got ownership of the mutex
			case WAIT_OBJECT_0:
				__try
				{
					Current = Current->next;
				}
				__finally{
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
		//Uppdatera alla värden här tex:
		 double dt = 10;

		double newPositionX = 0;
		double newPositionY = 0;
		double newVelocityX = 0;
		double newVelocityY = 0;

		newVelocityX = CurrentPlanet->vx + (totalAccelerationX * dt);
		newVelocityY = CurrentPlanet->vy + (totalAccelerationY * dt);
		newPositionX = CurrentPlanet->sx + (newVelocityX * dt);
		newPositionY = CurrentPlanet->sy + (newVelocityY * dt);

		CurrentPlanet->vx = newVelocityX;
		CurrentPlanet->vy = newVelocityY;
		CurrentPlanet->sx = newPositionX;
		CurrentPlanet->sy = newPositionY;
		CurrentPlanet->life--;

		Sleep(10);
		if (CurrentPlanet->sx >= 1200 || CurrentPlanet->sy >= 650 || CurrentPlanet->sx <= 0 || CurrentPlanet->sy <= 0) //Ifall vår planet åker utanför bounds.
		{
			sendMessageToClient(CurrentPlanet->pid, 1);
			DWORD WaitResult = WaitForSingleObject(
				Mutex,    // handle to mutex
				INFINITE);  // no time-out interval

			switch (WaitResult)
			{
				// The thread got ownership of the mutex
			case WAIT_OBJECT_0:
				__try
				{
					removePlanet(CurrentPlanet);
					return;
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
			return;
		}
	}
	sendMessageToClient(CurrentPlanet->pid, 2);

	DWORD WaitResult = WaitForSingleObject(
		Mutex,    // handle to mutex
		INFINITE);  // no time-out interval

	switch (WaitResult)
	{
		// The thread got ownership of the mutex
	case WAIT_OBJECT_0:
		__try
		{
			removePlanet(CurrentPlanet);
			return;
		}
		__finally{
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

/********************************************************************\
*  Function: int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)    *
*                                                                    *
*   Purpose: Initializes Application                                 *
*                                                                    *
*  Comments: Register window class, create and display the main      *
*            window, and enter message loop.                         *
*                                                                    *
*                                                                    *
\********************************************************************/

							/* NOTE: This function is not too important to you, it only */
							/*       initializes a bunch of things.                     */
							/* NOTE: In windows WinMain is the start function, not main */

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	HWND hWnd;
	DWORD threadID;
	MSG msg;
	
	
							/* Create the window, 3 last parameters important */
							/* The tile of the window, the callback function */
							/* and the backgrond color */

	hWnd = windowCreate (hPrevInstance, hInstance, nCmdShow, "Himmel", MainWndProc, COLOR_WINDOW+1);

							/* start the timer for the periodic update of the window    */
							/* (this is a one-shot timer, which means that it has to be */
							/* re-set after each time-out) */
							/* NOTE: When this timer expires a message will be sent to  */
							/*       our callback function (MainWndProc).               */
  
	windowRefreshTimer (hWnd, UPDATE_FREQ);
  

							/* create a thread that can handle incoming client requests */
							/* (the thread starts executing in the function mailThread) */
							/* NOTE: See online help for details, you need to know how  */ 
							/*       this function does and what its parameters mean.   */
							/* We have no parameters to pass, hence NULL				*/
  

	threadID = threadCreate (mailThread, NULL); 
  

							/* (the message processing loop that all windows applications must have) */
							/* NOTE: just leave it as it is. */
	while( GetMessage( &msg, NULL, 0, 0 ) ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return msg.wParam;
}


/********************************************************************\
* Function: mailThread                                               *
* Purpose: Handle incoming requests from clients                     *
* NOTE: This function is important to you.                           *
/********************************************************************/
DWORD WINAPI mailThread(LPVOID arg) {

	char buffer[1024];
	DWORD bytesRead;
	static int posY = 0;
	HANDLE mailbox;
	Mutex = CreateMutex(NULL, FALSE, NULL);

	if (!Mutex)
	{
		printf("Error Mutex, %d", GetLastError());
	}

							/* create a mailslot that clients can use to pass requests through   */
							/* (the clients use the name below to get contact with the mailslot) */
							/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */

	
	mailbox = mailslotCreate ("mailbox");


	for(;;) {				
							/* (ordinary file manipulating functions are used to read from mailslots) */
							/* in this example the server receives strings from the client side and   */
							/* displays them in the presentation window                               */
							/* NOTE: binary data can also be sent and received, e.g. planet structures*/
	planet_type *Planet = (planet_type*)malloc(sizeof(planet_type));
	bytesRead = mailslotRead (mailbox, Planet, strlen(Planet)); 

	if(bytesRead!= 0) {
		DWORD WaitResult = WaitForSingleObject(
			Mutex,    // handle to mutex
			INFINITE);  // no time-out interval

		switch (WaitResult)
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			__try
			{
				createPlanet(Planet);
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

		threadCreate(planetThread, Planet);
		
							/* NOTE: It is appropriate to replace this code with something */
							/*       that match your needs here.                           */
							/* (hDC is used reference the previously created window) */							
	}
	else {
		free(Planet);
		printf("Failed reading from \"mailbox\"");
		printf("%d", GetLastError());
							/* failed reading from mailslot                              */
							/* (in this example we ignore this, and happily continue...) */
    }
  }
	CloseHandle(Mutex);
	CloseHandle(mailbox);
  return 0;
}


/********************************************************************\
* Function: LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM) *
*                                                                    *
* Purpose: Processes Application Messages (received by the window)   *
* Comments: The following messages are processed                     *
*                                                                    *
*           WM_PAINT                                                 *
*           WM_COMMAND                                               *
*           WM_DESTROY                                               *
*           WM_TIMER                                                 *
*                                                                    *
\********************************************************************/
/* NOTE: This function is called by Windows when something happens to our window */

LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
  
	PAINTSTRUCT ps;
	static int posX = 10;
	int posY;
	HANDLE context;
	static DWORD color = 0;
	planet_type *Current = Universe;
  
	switch( msg ) {
							/**************************************************************/
							/*    WM_CREATE:        (received on window creation)
							/**************************************************************/
		case WM_CREATE:       
			hDC = GetDC(hWnd);  
			break;   
							/**************************************************************/
							/*    WM_TIMER:         (received when our timer expires)
							/**************************************************************/
		case WM_TIMER:

							/* NOTE: replace code below for periodic update of the window */
							/*       e.g. draw a planet system)                           */
							/* NOTE: this is referred to as the 'graphics' thread in the lab spec. */

							/* here we draw a simple sinus curve in the window    */
							/* just to show how pixels are drawn                  */
			while (Current != NULL)
			{
				if(Current->mass > 1000000) Ellipse(hDC, Current->sx - 10, Current->sy + 10, Current->sx + 10, Current->sy - 10);
				else SetPixel(hDC, Current->sx, Current->sy, (COLORREF)Current->mass);
				Current = Current->next;
			}
							/****************************************************************\
							*     WM_PAINT: (received when the window needs to be repainted, *
							*               e.g. when maximizing the window)                 *
							\****************************************************************/

		case WM_PAINT:
							/* NOTE: The code for this message can be removed. It's just */
							/*       for showing something in the window.                */
			context = BeginPaint( hWnd, &ps ); /* (you can safely remove the following line of code) */
			EndPaint( hWnd, &ps );
			break;
							/**************************************************************\
							*     WM_DESTROY: PostQuitMessage() is called                  *
							*     (received when the user presses the "quit" button in the *
							*      window)                                                 *
							\**************************************************************/
		case WM_DESTROY:
			PostQuitMessage( 0 );
							/* NOTE: Windows will automatically release most resources this */
     						/*       process is using, e.g. memory and mailslots.           */
     						/*       (So even though we don't free the memory which has been*/     
     						/*       allocated by us, there will not be memory leaks.)      */

			ReleaseDC(hWnd, hDC); /* Some housekeeping */
			break;

							/**************************************************************\
							*     Let the default window proc handle all other messages    *
							\**************************************************************/
		default:
			return( DefWindowProc( hWnd, msg, wParam, lParam )); 
   }
   return 0;
}




