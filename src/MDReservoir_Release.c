/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDReservoir_Release.c

bfekete@gc.cuny.edu

******************************************************************************/

#include <MF.h>
#include <MD.h>
 
// Output
static int _MDOutResReleaseID           = MFUnset;
static int _MDOutResExtractableReleaseID = MFUnset;

int MDReservoir_ReleaseDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoir release");
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions); return (CMfailed);
		case MFhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions);
		case MFoff: break;
		case MFon:
			if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
			break;
	}
	MFDefLeaving ("Reservoir release");
	return (_MDOutResReleaseID); 
}

int MDReservoir_ExtractableReleaseDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResExtractableReleaseID != MFUnset) return (_MDOutResExtractableReleaseID);

	MFDefEntering ("Extractable release");
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions); return (CMfailed);
		case MFhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions);
		case MFoff: break;
		case MFon:
			if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
			break;
	}
	MFDefLeaving ("Extractable release");
	_MDOutResExtractableReleaseID = _MDOutResReleaseID != MFUnset ? MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFInput, MFFlux,  MFBoundary) : MFUnset;
	return (_MDOutResExtractableReleaseID);
}
