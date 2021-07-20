/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDRouting_DischLevel3.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

enum { MDhelp, MDmuskingum, MDaccumulate, MDcascade };

static int _MDDischLevel3ID = MFUnset;

int MDRouting_DischargeInChannelDef() {
	int optID = MDmuskingum;
	const char *optStr, *optName = MDOptConfig_Routing;
	const char *options []    = { MFhelpStr, "muskingum", "accumulate", "cascade", (char *) NULL };

	if (_MDDischLevel3ID != MFUnset) return (_MDDischLevel3ID);

	MFDefEntering ("Discharge - In channel");
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		case MDhelp:       MFOptionMessage (optName, optStr, options);
		case MDmuskingum:  _MDDischLevel3ID = MDRouting_DischargeInChannelMuskingumDef();  break;
		case MDaccumulate: _MDDischLevel3ID = MDRouting_DischargeInChannelAccumulateDef(); break;
		case MDcascade:    _MDDischLevel3ID = MDRouting_DischargeInChannelCascadeDef();    break;
		default: MFOptionMessage (optName, optStr, options); return (CMfailed);
	}
	if (_MDDischLevel3ID == CMfailed) return (CMfailed);
	MFDefLeaving ("Discharge - In channel");
	return (_MDDischLevel3ID);
}
