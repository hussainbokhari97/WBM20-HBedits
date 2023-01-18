/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_RainPotETPstd.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInDayLengthID     = MFUnset;
static int _MDInI0HDayID        = MFUnset;
static int _MDInCParamAlbedoID  = MFUnset;
static int _MDInCommon_AtMeanID = MFUnset;
static int _MDInSolRadID        = MFUnset;
static int _MDInVPressID        = MFUnset;
static int _MDInWSpeedID        = MFUnset;
// Output
static int _MDOutPetID          = MFUnset;

static void _MDRainPotETPstd (int itemID) { // Penman (1948) PE in mm for day also given by Chidley and Pike (1970)
// Input
 	float dayLen  = MFVarGetFloat (_MDInDayLengthID,      itemID, 12.0); // daylength in fraction of day
	float i0hDay  = MFVarGetFloat (_MDInI0HDayID,         itemID,  0.0); // daily potential insolation on horizontal [MJ/m2]
	float albedo  = MFVarGetFloat (_MDInCParamAlbedoID,   itemID,  0.0); // albedo 
	float airT    = MFVarGetFloat (_MDInCommon_AtMeanID,  itemID,  0.0); // air temperatur [degree C]
	float solRad  = MFVarGetFloat (_MDInSolRadID,         itemID,  0.0); // daily solar radiation on horizontal [MJ/m2]
 	float vPress  = MFVarGetFloat (_MDInVPressID,         itemID,  0.0) / 1000.0; // daily average vapor pressure [kPa]
	float wSpeed  = fabs (MFVarGetFloat (_MDInWSpeedID,   itemID,  0.0)); // average wind speed for the day [m/s]
// Output
	float pet;
// Local
	float sHeat = 0.0; // average subsurface heat storage for day [W/m2]
	float solNet; // average net solar radiation for day [W/m2]
	float novern; // sunshine duration fraction of daylength
	float effem;  // effective emissivity from clear sky
	float cldCor; // cloud cover correction to net longwave under clear sky
	float lngNet; // average net longwave radiation for day [W/m2]
	float aa;     // available energy [W/m2]
	float fu;     // Penman wind function, [mm d-1 kPa-1]
	float es;     // vapor pressure at airT [kPa]
	float delta;  // dEsat/dTair [kPa/K]
	
	if (wSpeed < 0.2) wSpeed = 0.2;

	solNet = (1.0 - albedo) * solRad / MDConstIGRATE; // net solar with Penman (1948) albedo of 0.25

	effem = 0.44 + 0.252 * sqrt (vPress); // Brunt method for effective emissivity with Penman (1948) coefficients

	novern = ((solRad / i0hDay) - 0.18) / 0.55; // Penman's relation of SOLRAD / I0HDAY to sunshine duration n/N
	if (novern > 1.0) novern = 1.0;

	cldCor = 0.1 + 0.9 * novern; //Penman's (1948) longwave cloud correction coefficient
	lngNet = (effem - 1.0) * cldCor * MDConstSIGMA * pow (airT + 273.15,4.0);
	aa = solNet + lngNet - sHeat;
	es = MDPETlibVPressSat (airT);
	delta = MDPETlibVPressDelta (airT);

	fu = 2.6 * (1.0 + 0.54 * wSpeed); // Penman wind function given by Brutsaert (1982) eq 10.17

	// Penman equation from Brutsaert eq 10.15
	pet = (delta * MDConstEtoM * MDConstIGRATE * aa + MDConstPSGAMMA * fu * (es - vPress)) / (delta + MDConstPSGAMMA);
   MFVarSetFloat (_MDOutPetID,itemID,pet);
}

int MDCore_RainPotETPstdDef () {
	if (_MDOutPetID != MFUnset) return (_MDOutPetID);

	MFDefEntering ("Rainfed Potential Evapotranspiration (Penman Standard)");
	if (((_MDInDayLengthID     = MDCommon_SolarRadDayLengthDef ()) == CMfailed) ||
        ((_MDInI0HDayID        = MDCommon_SolarRadI0HDayDef ())    == CMfailed) ||
        ((_MDInCParamAlbedoID  = MDParam_LCAlbedoDef ())           == CMfailed) ||
        ((_MDInSolRadID        = MDCommon_SolarRadDef ())          == CMfailed) ||
        ((_MDInCommon_AtMeanID = MDCommon_AirTemperatureDef ())    == CMfailed) ||
        ((_MDInVPressID  = MFVarGetID (MDVarCommon_HumidityVaporPressure, "Pa",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInWSpeedID  = MFVarGetID (MDVarCommon_WindSpeed,             "m/s", MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutPetID    = MFVarGetID (MDVarCore_RainPotEvapotrans,       "mm",  MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDRainPotETPstd) == CMfailed)) return (CMfailed);
	MFDefLeaving  ("Rainfed Potential Evapotranspiration (Penman Standard)");
	return (_MDOutPetID);
}
