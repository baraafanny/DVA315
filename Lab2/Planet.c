#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "Planet.h"

#include "wrapper.h"
planet_type *Universe = NULL;
HANDLE Mutex;

void createPlanet(planet_type *Planet)
{
	if (Universe == NULL)
	{
		Universe = Planet;
	}
	else
	{
		addPlanet(Planet);
	}
}
void addPlanet(planet_type *Planet)
{
	planet_type *current = Universe;

	while (current != NULL)
	{
		if (current->next == NULL)
		{
			current->next = Planet;
			printf("Planet was added to List\n");
			return;
		}
		current = current->next;
	}
}
void removePlanet(planet_type *Planet)
{
	if (Universe == NULL)
	{
		printf("Nothing to remove in list.");
	}

	//Universe �r namnet p� v�r l�nkade lista av planeter
	//Vi s�tter pekaren "tmp" till att peka p� listans huvud
	planet_type* tmp = Universe;
	planet_type* prev = NULL;

	//H�r kollar vi ifall Planeten vi tagit in som argument �r huvudet f�r listan.
	if (Universe == Planet)
	{
		if (tmp->next)
		{
			//Om det finns fler planeter s� s�tter vi huvudet till den planet som den pekar p� och frig�r sedan minnet (huvudet) som "tmp" pekar p�
			Universe = tmp->next;
			free(tmp);
			Planet = NULL;
			return;
		}
		else
		{
			//Om huvudet inte har n�gon "next" s� frig�r vi minnet och s�tter den till NULL
			free(Universe);
			Universe = NULL;
			Planet = NULL;
			return;
		}
	}

	//Detta fall kommer att ske ifall planeten vi vill ta bort inte �r listans huvud
	while (tmp->next != Planet && tmp->next != NULL)
	{
		//Vi traverserar listan genom att uppdatera "tmp" pekaren, samt anv�nder prev-pekaren f�r att h�lla koll p� minnesplatsen f�r planeten innan
		//Prev kommer aldrig att anv�ndas(?)
		prev = tmp;
		tmp = tmp->next;
	}

	//I detta fall s� pekar "tmp-next" p� planeten vi vill ta bort
	if (tmp->next == Planet)
	{
		//Om det finns fler planeter i listan efter planeten vi vill ta bort s� l�t prev peka p� kommande planet
		if (tmp->next->next != NULL)
		{

			//(1)->(2)->(prev)->(tmp)->(tmpnext == planet)->(n)->(n+1)...
			planet_type* to_delete = tmp->next;
			//(1)->(2)->(prev)->(tmp)->(n)->(n+1)...   to_delete == (tmpnext == planet)
			tmp->next = tmp->next->next;

			//Frig�r minnet som to_delete pekar p�
			free(to_delete);
			to_delete = NULL;
		}
		else //I detta fallet �r Planeten vi vill ta bort den sista planeten i listan
		{
			//(1)->(2)->(prev)->(tmp)->(tmpnext == planet)->NULL
			planet_type* to_delete = tmp->next;
			//(1)->(2)->(prev)->(tmp)->NULL  to_delete == (tmpnext == planet)
			tmp->next = NULL;
			//Frig�r minnet som to_delete pekar p�
			free(to_delete);
			to_delete = NULL;
		}

		return;
	}

	//Detta fall t�cker det f�rhoppningsvis om�jliga fallet att planeten vi vill ta bort aldrig fanns med i listan (2spooky4me)
	return;

}
