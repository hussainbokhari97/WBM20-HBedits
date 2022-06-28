/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDCore_RainPotETTurc.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInCommon_AtMeanID = MFUnset;
static int _MDInSolRadID        = MFUnset;
static int _MDOutPetID          = MFUnset;

static void _MDRainPotETTurc (int itemID) { // Turc (1961) PE in mm for day 
// Input
	float airT   = MFVarGetFloat (_MDInCommon_AtMeanID, itemID, 0.0); // air temperatur [degree C]
	float solRad = MFVarGetFloat (_MDInSolRadID,        itemID, 0.0); // daily solar radiation on horizontal [MJ/m2]
// Output
	float pet;

	pet = airT > 0.0 ? 0.313 * airT * (solRad + 2.1) / (airT + 15) : 0.0;
	MFVarSetFloat (_MDOutPetID,itemID,pet);
}

int MDCore_RainPotETTurcDef () {
	if (_MDOutPetID != MFUnset) return (_MDOutPetID);

	MFDefEntering ("Rainfed Potential Evapotranspiration (Turc)");
	if (((_MDInSolRadID        = MDCommon_SolarRadDef ())       == CMfailed) ||
        ((_MDInCommon_AtMeanID = MDCommon_AirTemperatureDef ()) == CMfailed) ||
        ((_MDOutPetID          = MFVarGetID (MDVarCore_RainPotEvapotrans, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDRainPotETTurc) == CMfailed)) return (CMfailed);
	MFDefLeaving  ("Rainfed Potential Evapotranspiration (Turc)");
	return (_MDOutPetID);
}
