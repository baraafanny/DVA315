/*********************************************
* client.c
*
* Desc: lab-skeleton for the client side of an
* client-server application
* 
* Revised by Dag Nystrom & Jukka Maki-Turja
* NOTE: the server must be started BEFORE the
* client.
*********************************************/
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "wrapper.h"
#include <stdlib.h>
HANDLE ClientMailSlot;
HANDLE Mutex;
/********************************************************************\
* Function: readmessageThread                                        *
* Purpose: Läser meddelande om varför planeten togs bort.            *                                           
/********************************************************************/
DWORD WINAPI readmessageThread(void *data)
{
	for (;;)
	{
		if (ClientMailSlot)
		{
			char *message = (char*)malloc(sizeof(char) * 30);
			int bytesRead = mailslotRead(ClientMailSlot, message, 30);
			if (bytesRead != 0)
			{
				MessageBox(NULL, message, "Planet Status:", MB_OK);
			}
			else
			{
				free(message);
			}
		}
		else
		{
			printf("%d", GetLastError());
		}
	}
}

void main(void) {

	HANDLE mailSlot;
	DWORD bytesWritten;
	int loops = 2000;
	char Mailboxname[13];
	char* name3 = "Mailbox%d";
	int PID = GetCurrentProcessId();
	sprintf_s(Mailboxname, sizeof(Mailboxname), name3, PID); //Namnet på vår mailbox blir tex Mailbox13772 om PID är 13772.

	ClientMailSlot = mailslotCreate(Mailboxname);

	mailSlot = mailslotConnect("mailbox"); 

	if (mailSlot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return;
	}
	planet_type Planet;
	while(loops-- > 0) {
							
							printf("Hello Client! Type ur planetdata:\n");
							printf("Name:\n"); // Name of planet
							fgets(Planet.name, 20, stdin);

							printf("X-axis pos:\n"); // X-axis position
							char input[100];
							fgets(input, 100, stdin);
							Planet.sx = atof(input);

							printf("Y-axis pos:\n"); // Y-axis position
							fgets(input, 100, stdin);
							Planet.sy = atof(input);

							printf("X-axis velo:\n"); // X-axis velocity
							fgets(input, 100, stdin);
							Planet.vx = atof(input);

							printf("Y-axis velo:\n"); // Y-axis velocity
							fgets(input, 100, stdin);
							fflush(stdin);
							Planet.vy = atof(input);

							printf("Mass:\n"); // Planet mass
							fgets(input, 100, stdin);
							Planet.mass = atof(input);
							Planet.next = NULL; // Pointer to next planet in linked list
							
							printf("Life:\n"); // Planet life
							fgets(input, 100, stdin);
							setbuf(stdin, NULL);
							Planet.life = atof(input);
							sprintf_s(Planet.pid, sizeof(Planet.pid), "%d", PID); // String containing ID of creating process

		bytesWritten = mailslotWrite (mailSlot, &Planet, sizeof(Planet));
		if (bytesWritten!=-1)
			printf("data sent to server (bytes = %d)\n", bytesWritten);
		else
			printf("failed sending data to server\n");

		threadCreate(readmessageThread, PID);
		getchar(); // For pausing abit before making a new input.
	}

	mailslotClose (mailSlot);
	mailslotClose(ClientMailSlot);                               
	Sleep(2000);
	return;
}
