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

	//Universe är namnet på vår länkade lista av planeter
	//Vi sätter pekaren "tmp" till att peka på listans huvud
	planet_type* tmp = Universe;
	planet_type* prev = NULL;

	//Här kollar vi ifall Planeten vi tagit in som argument är huvudet för listan.
	if (Universe == Planet)
	{
		if (tmp->next)
		{
			//Om det finns fler planeter så sätter vi huvudet till den planet som den pekar på och frigör sedan minnet (huvudet) som "tmp" pekar på
			Universe = tmp->next;
			free(tmp);
			Planet = NULL;
			return;
		}
		else
		{
			//Om huvudet inte har någon "next" så frigör vi minnet och sätter den till NULL
			free(Universe);
			Universe = NULL;
			Planet = NULL;
			return;
		}
	}

	//Detta fall kommer att ske ifall planeten vi vill ta bort inte är listans huvud
	while (tmp->next != Planet && tmp->next != NULL)
	{
		//Vi traverserar listan genom att uppdatera "tmp" pekaren, samt använder prev-pekaren för att hålla koll på minnesplatsen för planeten innan
		//Prev kommer aldrig att användas(?)
		prev = tmp;
		tmp = tmp->next;
	}

	//I detta fall så pekar "tmp-next" på planeten vi vill ta bort
	if (tmp->next == Planet)
	{
		//Om det finns fler planeter i listan efter planeten vi vill ta bort så låt prev peka på kommande planet
		if (tmp->next->next != NULL)
		{

			//(1)->(2)->(prev)->(tmp)->(tmpnext == planet)->(n)->(n+1)...
			planet_type* to_delete = tmp->next;
			//(1)->(2)->(prev)->(tmp)->(n)->(n+1)...   to_delete == (tmpnext == planet)
			tmp->next = tmp->next->next;

			//Frigör minnet som to_delete pekar på
			free(to_delete);
			to_delete = NULL;
		}
		else //I detta fallet är Planeten vi vill ta bort den sista planeten i listan
		{
			//(1)->(2)->(prev)->(tmp)->(tmpnext == planet)->NULL
			planet_type* to_delete = tmp->next;
			//(1)->(2)->(prev)->(tmp)->NULL  to_delete == (tmpnext == planet)
			tmp->next = NULL;
			//Frigör minnet som to_delete pekar på
			free(to_delete);
			to_delete = NULL;
		}

		return;
	}

	//Detta fall täcker det förhoppningsvis omöjliga fallet att planeten vi vill ta bort aldrig fanns med i listan (2spooky4me)
	return;

}
