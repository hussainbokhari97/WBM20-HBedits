/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_RainPotETJensen.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInCommon_AtMeanID = MFUnset;
static int _MDInSolRadID = MFUnset;
static int _MDOutPetID   = MFUnset;

static void _MDRainPotETJensen (int itemID) { // Jensen-Haise (1963) PE in mm for day
// Input
	float airT   = MFVarGetFloat (_MDInCommon_AtMeanID, itemID, 0.0); // air temperatur [degree C]
	float solRad = MFVarGetFloat (_MDInSolRadID, itemID, 0.0);        // daily solar radiation on horizontal [MJ/m2/day] TODO wrong unit!!!!
// Output 
	float pet;

	pet = 0.41 * (0.025 * airT + .078) * solRad;
	MFVarSetFloat (_MDOutPetID,itemID,pet);
}

int MDCore_RainPotETJensenDef () {
	if (_MDOutPetID != MFUnset) return (_MDOutPetID);

	MFDefEntering ("Rainfed Potential Evapotranspiration (Jensen)");
	if (((_MDInSolRadID        = MDCommon_SolarRadDef())        == CMfailed) ||
        ((_MDInCommon_AtMeanID = MDCommon_AirTemperatureDef ()) == CMfailed) ||
        ((_MDOutPetID          = MFVarGetID (MDVarCore_RainPotEvapotrans, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDRainPotETJensen) == CMfailed)) return (CMfailed);
	MFDefLeaving  ("Rainfed Potential Evapotranspiration (Jensen)");
	return (_MDOutPetID);
}
