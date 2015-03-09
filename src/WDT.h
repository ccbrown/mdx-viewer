#ifndef WDT_H
#define WDT_H

#include "Buffer.h"
#include "ADT.h"

#include <stdint.h>
#include <list>

typedef enum {
	WDTMapTileFlagHasADT = 0x01,
	WDTMapTileFlagLoaded = 0x02,
} WDTMapTileFlag;

struct WDTMapTile {
	uint32_t flags;
	uint32_t unused;
};

class WDT {

	public:
	
		WDT();
		~WDT();
				
		bool open(const char* name);

		void animate(long long unsigned int time);
		
		void draw();
		
		void close();

	private:
		
		Buffer<char> _name;

		std::list<ADT*> _adts;
};

#endif