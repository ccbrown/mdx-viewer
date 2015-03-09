#ifndef WORLD_H
#define WORLD_H

#include "Camera.h"
#include "WDT.h"
#include "MDX.h"
#include "WMO.h"

void WorldInit();
void WorldDestroy();

class World {
	public:
		World();
		~World();
		
		void render();
		
		Camera camera;

	private:
		MDX ravenLord;

		MDX platform;

		MDX crystalNE;
		MDX crystalNW;
		MDX crystalSE;
		MDX crystalSW;

		MDX lightningRodN;
		MDX lightningRodE;
		MDX lightningRodS;
		MDX lightningRodW;

		MDX lightningRodNE;
		MDX lightningRodSE;
		MDX lightningRodNW;
		MDX lightningRodSW;
		
		MDX markerCyan;
		MDX markerGreen;
		MDX markerPurple;
		MDX markerRed;
		MDX markerYellow;
};

World* CurrentWorld();

#endif