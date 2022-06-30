/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDRouting_DischLevel3.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

enum { MDhelp, MDmuskingum, MDaccumulate, MDcascade };

static int _MDDischLevel3ID = MFUnset;

int MDRouting_ChannelDischargeDef () {
	int optID = MDmuskingum;
	const char *optStr;
	const char *options []    = { MFhelpStr, "muskingum", "accumulate", "cascade", (char *) NULL };

	if (_MDDischLevel3ID != MFUnset) return (_MDDischLevel3ID);

	MFDefEntering ("Discharge - In channel");
	if ((optStr = MFOptionGet (MDOptConfig_Routing)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		default:
		case MDhelp:       MFOptionMessage (MDOptConfig_Routing, optStr, options); return (CMfailed);
		case MDmuskingum:  _MDDischLevel3ID = MDRouting_ChannelDischargeMuskingumDef();  break;
		case MDaccumulate: _MDDischLevel3ID = MDRouting_ChannelDischargeAccumulateDef(); break;
		case MDcascade:    _MDDischLevel3ID = MDRouting_ChannelDischargeCascadeDef();    break;
	}
	if (_MDDischLevel3ID == CMfailed) return (CMfailed);
	MFDefLeaving ("Discharge - In channel");
	return (_MDDischLevel3ID);
}

static int _MDRouting_ChannelStorageChgID = MFUnset;

int MDRouting_ChannelStorageChgDef () { 
	if (_MDRouting_ChannelStorageChgID != MFUnset) return (_MDRouting_ChannelStorageChgID);
	if (((MDRouting_DischargeDef()) == CMfailed) ||
	    ((_MDRouting_ChannelStorageChgID  = MFVarGetID (MDVarRouting_RiverStorageChg, "m3", MFInput, MFFlux,  MFBoundary)) == CMfailed))
		return (CMfailed);
	return (_MDRouting_ChannelStorageChgID);
}