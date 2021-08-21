/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDReservoir_Uptake.c

bfekete@gc.cuny.edu

******************************************************************************/

#include <MF.h>
#include <MD.h>
 
// Output
static int _MDOutResUptakeID  = MFUnset;

int MDReservoir_UptakeDef () {
	int optID = MFnone;
	const char *optStr;

	if (_MDOutResUptakeID != MFUnset) return (_MDOutResUptakeID);

	MFDefEntering ("Reservoirs");
	if ((optStr = MFOptionGet (MDVarReservoir_Uptake)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDVarReservoir_Uptake, optStr, MFsourceOptions); return (CMfailed);
		case MFhelp: MFOptionMessage (MDVarReservoir_Uptake, optStr, MFsourceOptions);
		case MFnone: break;
		case MFinput:
			if ((_MDOutResUptakeID = MFVarGetID (MDVarReservoir_Uptake, "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed)
				return (CMfailed);
			break;
		case MFcalculate: break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResUptakeID); 
}
