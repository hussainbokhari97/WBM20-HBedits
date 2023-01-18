/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_RainPotET.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDPotETID = MFUnset;

enum { MDhelp, MDinput, MDHamon, MDJensen, MDPsTaylor, MDPstd, MDPMday, MDPMdn, MDSWGday, MDSWGdn, MDTurc };

int MDCore_RainPotETDef () {
	int optID = MDinput;
	const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, "Hamon", "Jensen", "PsTaylor", "Pstd", "PMday", "PMdn", "SWGday", "SWGdn", "Turc", (char *) NULL };

	if (_MDPotETID != MFUnset) return (_MDPotETID);

	MFDefEntering ("Rainfed Potential Evapotranspiration");
	if ((optStr = MFOptionGet (MDVarCore_RainPotEvapotrans)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		default:
		case MDhelp:  MFOptionMessage (MDVarCore_RainPotEvapotrans, optStr, options); return (CMfailed);
		case MDinput: _MDPotETID = MFVarGetID (MDVarCore_RainPotEvapotrans, "mm", MFInput, MFFlux, false); break;
		case MDHamon:    _MDPotETID = MDCore_RainPotETHamonDef ();    break;
		case MDJensen:   _MDPotETID = MDCore_RainPotETJensenDef ();   break;
		case MDPsTaylor: _MDPotETID = MDCore_RainPotETPsTaylorDef (); break;
		case MDPstd:     _MDPotETID = MDCore_RainPotETPstdDef ();     break;
		case MDPMday:    _MDPotETID = MDCore_RainPotETPMdayDef ();    break;
		case MDPMdn:     _MDPotETID = MDCore_RainPotETPMdnDef ();     break;
		case MDSWGday:   _MDPotETID = MDCore_RainPotETSWGdayDef ();   break;
		case MDSWGdn:    _MDPotETID = MDCore_RainPotETSWGdnDef ();    break;
		case MDTurc:     _MDPotETID = MDCore_RainPotETTurcDef ();     break;
	}
	MFDefLeaving ("Rainfed Potential Evapotranspiration");
	return (_MDPotETID);
}
