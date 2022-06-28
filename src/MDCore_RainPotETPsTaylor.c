/****************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDCore_RainPotETPsTaylor.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInDayLengthID     = MFUnset;
static int _MDInI0HDayID        = MFUnset;
static int _MDInCParamAlbedoID  = MFUnset;

static int _MDInCommon_AtMeanID = MFUnset;
static int _MDInSolRadID        = MFUnset;
static int _MDInVPressID        = MFUnset;
static int _MDOutPetID          = MFUnset;

static void _MDRainPotETPsTaylor (int itemID) { // Priestley and Taylor (1972) PE in mm for day
// Input
	float dayLen  = MFVarGetFloat (_MDInDayLengthID,     itemID, 0.0); // daylength in fraction of day
	float i0hDay  = MFVarGetFloat (_MDInI0HDayID,        itemID, 0.0); // daily potential insolation on horizontal [MJ/m2]
	float albedo  = MFVarGetFloat (_MDInCParamAlbedoID,  itemID, 0.0); // albedo
	float airT    = MFVarGetFloat (_MDInCommon_AtMeanID, itemID, 0.0); // air temperatur [degree C]
	float solRad  = MFVarGetFloat (_MDInSolRadID,        itemID, 0.0); // daily solar radiation on horizontal [MJ/m2]
	float vPress  = MFVarGetFloat (_MDInVPressID,       itemID, 0.0) / 1000.0; // daily average vapor pressure [kPa]
// Output
	float pet;
// Local	
	float sHeat = 0.0;  // average subsurface heat storage for day [W/m2]
	float solNet;  // average net solar radiation for daytime [W/m2]
	float lngNet;  // average net longwave radiation for day  [W/m2]
	float aa;      // available energy [W/m2]
	float es;      // vapor pressure at airT [kPa]
	float delta;   // dEsat/dTair [kPa/K]
 	float dd;      // vapor pressure deficit [kPa]
	float le;      // latent heat [W/m2]

	solNet = (1.0 - albedo) * solRad / MDConstIGRATE;
	lngNet = MDSRadNETLong (i0hDay,airT,solRad,vPress);

	aa     = solNet + lngNet - sHeat;
	es     = MDPETlibVPressSat (airT);
	delta  = MDPETlibVPressDelta (airT);

	dd     = es - vPress; 
   le     = MDConstPTALPHA * delta * aa / (delta + MDConstPSGAMMA);

	pet = MDConstEtoM * MDConstIGRATE * le; 
   MFVarSetFloat (_MDOutPetID,itemID,pet);
}

int MDCore_RainPotETPsTaylorDef () {
	if (_MDOutPetID != MFUnset) return (_MDOutPetID);

	MFDefEntering ("Rainfed Potential Evapotranspiration (Priestley - Taylor)");
	if (((_MDInDayLengthID     = MDCommon_SolarRadDayLengthDef ()) == CMfailed) ||
        ((_MDInI0HDayID        = MDCommon_SolarRadI0HDayDef ())    == CMfailed) ||
        ((_MDInCParamAlbedoID  = MDParam_LCAlbedoDef ())           == CMfailed) ||
        ((_MDInSolRadID        = MDCommon_SolarRadDef ())          == CMfailed) ||
        ((_MDInCommon_AtMeanID = MDCommon_AirTemperatureDef ())    == CMfailed) ||
        ((_MDInVPressID  = MFVarGetID (MDVarCommon_HumidityVaporPressure, "Pa",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutPetID    = MFVarGetID (MDVarCore_RainPotEvapotrans,       "mm",  MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDRainPotETPsTaylor) == CMfailed)) return (CMfailed);
	MFDefLeaving  ("Rainfed Potential Evapotranspiration (Priestley - Taylor)");
	return (_MDOutPetID);
}
