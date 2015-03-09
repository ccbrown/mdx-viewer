#include "DBC.h"

DBC<DBCAnimationDataRecord>* gDBCAnimationData = NULL;
DBC<DBCAnimationDataRecord>*  DBCAnimationData() { return gDBCAnimationData; }

DBC<DBCCreatureDisplayInfoRecord>* gDBCCreatureDisplayInfo = NULL;
DBC<DBCCreatureDisplayInfoRecord>*  DBCCreatureDisplayInfo() { return gDBCCreatureDisplayInfo; }

DBC<DBCCreatureModelDataRecord>* gDBCCreatureModelData = NULL;
DBC<DBCCreatureModelDataRecord>*  DBCCreatureModelData() { return gDBCCreatureModelData; }

void DBCInit() {
	gDBCAnimationData       = new DBC<DBCAnimationDataRecord>("DBFilesClient\\AnimationData.dbc");
	gDBCCreatureDisplayInfo = new DBC<DBCCreatureDisplayInfoRecord>("DBFilesClient\\CreatureDisplayInfo.dbc");
	gDBCCreatureModelData   = new DBC<DBCCreatureModelDataRecord>  ("DBFilesClient\\CreatureModelData.dbc");
}

void DBCDestroy() {
	delete gDBCAnimationData;
	delete gDBCCreatureDisplayInfo;
	delete gDBCCreatureModelData;
}
