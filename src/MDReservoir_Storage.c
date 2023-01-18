/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDReservoir_Storage.c

bfekete@gc.cuny.edu

******************************************************************************/

#include <MF.h>
#include <MD.h>
 
// Output
static int _MDOutResStorageID = MFUnset;

int MDReservoir_StorageDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResStorageID != MFUnset) return (_MDOutResStorageID);

	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
 	switch (optID) {
		default:
		case MFhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions); return (CMfailed);
		case MFoff: break; 
		case MFon:
			MFDefEntering ("Reservoir release");
			_MDOutResStorageID = MDReservoir_OperationDef () == CMfailed ? CMfailed : MFVarGetID (MDVarReservoir_Storage, "km3", MFOutput, MFState, MFInitial);
			MFDefLeaving ("Reservoir release");
			break;
	}
	return (_MDOutResStorageID); 
}
