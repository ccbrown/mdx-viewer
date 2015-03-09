#include "World.h"

#include "Utility.h"

#include <OpenGL/gl.h>
#include <stdlib.h>
#include <stdio.h>

bool gWorldIsInitialized = false;

World* gMainWorld = NULL;

bool gWorldShouldDrawModel = false;

void WorldInit() {
	if (!gWorldIsInitialized) {
		gMainWorld = new World();
		gMainWorld->camera.turnUp(0.5 * M_PI);
		gWorldIsInitialized = true;
	}
}

void WorldDestroy() {
	delete gMainWorld;
}

World* CurrentWorld() {
	return gMainWorld;
}

World::World() {
	gWorldShouldDrawModel = false;

	// raven lord
	
	if (!ravenLord.openCreatureDisplayId(21492)) {
		printf("Error opening raven lord model.\n");
		return;
	}
	ravenLord.setAnimation(2);
	ravenLord.setPosition(Vector3f(0.0, 0.0, 11.0));

//	// platform
//
//	if (!platform.open("World\\Expansion02\\doodads\\nexus\\nexus_raid_floating_platform_model.mdx")) {
//		printf("Error opening platform model.\n");
//		return;
//	}
//	platform.setPosition(Vector3f(0.0, 1.0, 0.0));
//	
//	// crystals
//	
//	float crystalScale  = 3.0;
//	float crystalHeight = 11.0;
//	
//	if (!crystalNE.open("World\\Dungeon\\dragonsoul\\hagaracrystal\\dragonsoul_hagaracrystal_01.mdx")) {
//		printf("Error opening crystal model.\n");
//		return;
//	}
//	crystalNE.setPosition(Vector3f(30.0, 30.0, crystalHeight));
//	crystalNE.setScale(Vector3f(crystalScale));
//
//	if (!crystalNW.open("World\\Dungeon\\dragonsoul\\hagaracrystal\\dragonsoul_hagaracrystal_01.mdx")) {
//		printf("Error opening crystal model.\n");
//		return;
//	}
//	crystalNW.setPosition(Vector3f(-30.0, 30.0, crystalHeight));
//	crystalNW.setScale(Vector3f(crystalScale));
//
//	if (!crystalSE.open("World\\Dungeon\\dragonsoul\\hagaracrystal\\dragonsoul_hagaracrystal_01.mdx")) {
//		printf("Error opening crystal model.\n");
//		return;
//	}
//	crystalSE.setPosition(Vector3f(30.0, -30.0, crystalHeight));
//	crystalSE.setScale(Vector3f(crystalScale));
//
//	if (!crystalSW.open("World\\Dungeon\\dragonsoul\\hagaracrystal\\dragonsoul_hagaracrystal_01.mdx")) {
//		printf("Error opening crystal model.\n");
//		return;
//	}
//	crystalSW.setPosition(Vector3f(-30.0, -30.0, crystalHeight));
//	crystalSW.setScale(Vector3f(crystalScale));
//
//	// lightning rods
//	
//	if (!lightningRodN.open("World\\Dungeon\\dragonsoul\\lightningrod\\dragonsoul_hagaralightningrod_01.mdx")) {
//		printf("Error opening lightning rod model.\n");
//		return;
//	}
//	lightningRodN.setPosition(Vector3f(0.0, 46.0, 11.0));
//	lightningRodN.setAnimation(2);
//
//	if (!lightningRodE.open("World\\Dungeon\\dragonsoul\\lightningrod\\dragonsoul_hagaralightningrod_01.mdx")) {
//		printf("Error opening lightning rod model.\n");
//		return;
//	}
//	lightningRodE.setPosition(Vector3f(46.0, 0.0, 11.0));
//	lightningRodE.setAnimation(2);
//
//	if (!lightningRodS.open("World\\Dungeon\\dragonsoul\\lightningrod\\dragonsoul_hagaralightningrod_01.mdx")) {
//		printf("Error opening lightning rod model.\n");
//		return;
//	}
//	lightningRodS.setPosition(Vector3f(0.0, -46.0, 11.0));
//	lightningRodS.setAnimation(2);
//
//	if (!lightningRodW.open("World\\Dungeon\\dragonsoul\\lightningrod\\dragonsoul_hagaralightningrod_01.mdx")) {
//		printf("Error opening lightning rod model.\n");
//		return;
//	}
//	lightningRodW.setPosition(Vector3f(-46.0, 0.0, 11.0));
//	lightningRodW.setAnimation(2);
//
//	if (!lightningRodNE.open("World\\Dungeon\\dragonsoul\\lightningrod\\dragonsoul_hagaralightningrod_01.mdx")) {
//		printf("Error opening lightning rod model.\n");
//		return;
//	}
//	lightningRodNE.setPosition(Vector3f(21.0, 21.0, 11.0));
//	lightningRodNE.setAnimation(2);
//
//	if (!lightningRodNW.open("World\\Dungeon\\dragonsoul\\lightningrod\\dragonsoul_hagaralightningrod_01.mdx")) {
//		printf("Error opening lightning rod model.\n");
//		return;
//	}
//	lightningRodNW.setPosition(Vector3f(-21.0, 21.0, 11.0));
//	lightningRodNW.setAnimation(2);
//
//	if (!lightningRodSE.open("World\\Dungeon\\dragonsoul\\lightningrod\\dragonsoul_hagaralightningrod_01.mdx")) {
//		printf("Error opening lightning rod model.\n");
//		return;
//	}
//	lightningRodSE.setPosition(Vector3f(21.0, -21.0, 11.0));
//	lightningRodSE.setAnimation(2);
//
//	if (!lightningRodSW.open("World\\Dungeon\\dragonsoul\\lightningrod\\dragonsoul_hagaralightningrod_01.mdx")) {
//		printf("Error opening lightning rod model.\n");
//		return;
//	}
//	lightningRodSW.setPosition(Vector3f(-21.0, -21.0, 11.0));
//	lightningRodSW.setAnimation(2);
//	
//	if (!markerCyan.open("Spells\\Raid_ui_fx_cyan.mdx")) {
//		printf("Error opening raid marker model.\n");
//		return;
//	}
//	markerCyan.setAnimation(1);
//	markerCyan.setPosition(Vector3f(-21.0, -21.0, 11.0));
//
//	if (!markerGreen.open("Spells\\Raid_ui_fx_green.mdx")) {
//		printf("Error opening raid marker model.\n");
//		return;
//	}
//	markerGreen.setAnimation(1);
//	markerGreen.setPosition(Vector3f(21.0, -21.0, 11.0));
//
//	if (!markerPurple.open("Spells\\Raid_ui_fx_purple.mdx")) {
//		printf("Error opening raid marker model.\n");
//		return;
//	}
//	markerPurple.setAnimation(1);
//	markerPurple.setPosition(Vector3f(-21.0, 21.0, 11.0));
//
//	if (!markerRed.open("Spells\\Raid_ui_fx_red.mdx")) {
//		printf("Error opening raid marker model.\n");
//		return;
//	}
//	markerRed.setAnimation(1);
//	markerRed.setPosition(Vector3f(21.0, 21.0, 11.0));
//
//	if (!markerYellow.open("Spells\\Raid_ui_fx_yellow.mdx")) {
//		printf("Error opening raid marker model.\n");
//		return;
//	}
//	markerYellow.setAnimation(1);
//	markerYellow.setPosition(Vector3f(0.0, -46.0, 11.0));
	
	gWorldShouldDrawModel = true;
}

World::~World() {
}

void World::render() {
	if (gWorldShouldDrawModel) {
		unsigned long int time = TimeUSecs();

		ravenLord.draw(time);

//		platform.draw(time);
//		
//		crystalNE.draw(time);
//		crystalNW.draw(time);
//		crystalSE.draw(time);
//		crystalSW.draw(time);
//		
//		lightningRodN.draw(time);
//		lightningRodE.draw(time);
//		lightningRodS.draw(time);
//		lightningRodW.draw(time);
//
//		lightningRodNE.draw(time);
//		lightningRodNW.draw(time);
//		lightningRodSE.draw(time);
//		lightningRodSW.draw(time);
//		
//		markerCyan.draw(time);
//		markerGreen.draw(time);
//		markerPurple.draw(time);
//		markerRed.draw(time);
//		markerYellow.draw(time);
	}
}