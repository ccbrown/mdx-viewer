#include "WDT.h"
#include "MPQ.h"

#include <stdio.h>
#include <string.h>

WDT::WDT() {
}

WDT::~WDT() {
	close();
}

bool WDT::open(const char* name) {
	// NOTE: If this is ever compiled on a platform with a different byte order, this function will need some work.
	// NOTE: The mallocs could also use some error checking just in case.

	close();

	if (!name) {
		printf("Invalid name.\n");
		return false;
	}
		
	_name.set(name, strlen(name) + 7); // give it a little bit of extra room for tile numbers like "_30_28"

	size_t nl = strlen(name);
	if (nl < 5 || (strcmp(_name + nl - 4, ".wdt") && strcmp(_name + nl - 4, ".WDT"))) {
		printf("Invalid name.\n");
		return false;
	}
	
	MPQFile f(*_name);

	if (!f.data()) {
		return false;
	}

	char* dataptr = f.data();
	size_t dataRemaining = f.size();

	uint32_t tag;
	uint32_t tagsize;
	while (dataRemaining >= 8) {
		tag  = *(int*)dataptr;
		dataptr += 4;
		tagsize = *(int*)dataptr;
		dataptr += 4;
		
		dataRemaining -= 8;
		
		if (tagsize > dataRemaining) {
			printf("Invalid tag size.\n");
			return false;
		}
		
		switch (tag) {
			case 'MAIN': { // map tiles
				if (tagsize != sizeof(WDTMapTile) * 4096) {
					printf("Couldn't read map tiles.\n");
					return false;
				}
				WDTMapTile* t = (WDTMapTile*)dataptr;
				char* nameext = _name + strlen(*_name) - 4;
				for (int i = 0; i < 4096; ++i, ++t) {
					if (t->flags & WDTMapTileFlagHasADT) {
						sprintf(nameext, "_%02u_%02u.adt", i / 64, i % 64);
						ADT* adt = new ADT();
						if (!adt->open(*_name)) {
							delete adt;
							printf("Couldn't open ADT.\n");
						} else {
							_adts.push_back(adt);
						}
					}
				}
				break;
			}
			
			default:
				// unprogrammed tag
				//printf("Unknown WMO tag: '%c%c%c%c'\n", ((char*)&tag)[3], ((char*)&tag)[2], ((char*)&tag)[1], ((char*)&tag)[0]);
				break;
		}
		
		dataptr += tagsize;
		dataRemaining -= tagsize;
	}		
	
	return true;
}

void WDT::animate(long long unsigned int time) {
	for (std::list<ADT*>::iterator it = _adts.begin(); it != _adts.end(); ++it) {
		(*it)->animate(time);
	}
}

void WDT::draw() {
	for (std::list<ADT*>::iterator it = _adts.begin(); it != _adts.end(); ++it) {
		(*it)->draw();
	}
}

void WDT::close() {
	for (std::list<ADT*>::iterator it = _adts.begin(); it != _adts.end(); ++it) {
		delete *it;
	}
}
