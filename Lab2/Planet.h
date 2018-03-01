#ifndef PLANET_H
#define PLANET_H
#include "wrapper.h"

extern planet_type *Universe;
extern HANDLE Mutex;

void createPlanet(planet_type *Planet);
void addPlanet(planet_type *Planet);
void removePlanet(planet_type *Planet);

#endif 