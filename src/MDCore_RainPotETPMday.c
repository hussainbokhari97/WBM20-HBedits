/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_RainPotETPMday.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

static int _MDInDayLengthID     = MFUnset;
static int _MDInI0HDayID        = MFUnset;
static int _MDInCParamAlbedoID  = MFUnset;
static int _MDInCParamCHeightID = MFUnset;
static int _MDInCParamR5ID      = MFUnset;
static int _MDInCParamCDID      = MFUnset;
static int _MDInCParamCRID      = MFUnset;
static int _MDInCParamGLMaxID   = MFUnset;
static int _MDInCParamZ0gID     = MFUnset;
static int _MDInLeafAreaIndexID = MFUnset;
static int _MDInStemAreaIndexID = MFUnset;

static int _MDInCommon_AtMeanID        = MFUnset;
static int _MDInSolRadID        = MFUnset;
static int _MDInVPressID        = MFUnset;
static int _MDInWSpeedID        = MFUnset;
static int _MDOutPetID          = MFUnset;

static void _MDRainPotETPMday (int itemID) { // daily Penman-Monteith PE in mm for day
// Input
	float dayLen  = MFVarGetFloat (_MDInDayLengthID,      itemID, 12.0); // daylength in fraction of day
 	float i0hDay  = MFVarGetFloat (_MDInI0HDayID,         itemID,  0.0); // daily potential insolation on horizontal [MJ/m2]
	float albedo  = MFVarGetFloat (_MDInCParamAlbedoID,   itemID,  0.0); // albedo 
	float height  = MFVarGetFloat (_MDInCParamCHeightID,  itemID,  0.0); // canopy height [m]
	float r5      = MFVarGetFloat (_MDInCParamR5ID,       itemID,  0.0); // solar radiation at which conductance is halved [W/m2]
	float cd      = MFVarGetFloat (_MDInCParamCDID,       itemID,  0.0); // vpd at which conductance is halved [kPa]
	float cr      = MFVarGetFloat (_MDInCParamCRID,       itemID,  0.0); // light extinction coefficient for projected LAI
	float glMax   = MFVarGetFloat (_MDInCParamGLMaxID,    itemID,  0.0); // maximum leaf surface conductance for all sides of leaf [m/s]
	float z0g     = MFVarGetFloat (_MDInCParamZ0gID,      itemID,  0.0); // z0g - ground surface roughness [m]
 	float lai     = MFVarGetFloat (_MDInLeafAreaIndexID,  itemID,  0.0); // projected leaf area index
	float sai     = MFVarGetFloat (_MDInStemAreaIndexID,  itemID,  0.0); // projected stem area index
	float airT    = MFVarGetFloat (_MDInCommon_AtMeanID,  itemID,  0.0); // air temperatur degree C
	float solRad  = MFVarGetFloat (_MDInSolRadID,         itemID,  0.0); // daily solar radiation on horizontal [MJ/m2/day] TODO wrong unit!!!!
	float vPress  = MFVarGetFloat (_MDInVPressID,         itemID,  0.0) / 1000.0; // daily average vapor pressure [kPa] TODO wrong unit!!!
	float wSpeed  = fabs (MFVarGetFloat (_MDInWSpeedID,   itemID,  0.0)); // average wind speed for the day [m/s] 
// Output
	float pet;
// Local
	float sHeat = 0.0; // average subsurface heat storage for day [W/m2]
	float solNet;  // average net solar radiation for daytime [W/m2]
	float lngNet;  // average net longwave radiation for day  [W/m2]
	float za;      // reference height [m]
 	float disp;    // height of zero-plane [m]
	float z0;      // roughness parameter [m] 
	float aa;      // available energy [W/m2]
	float es;      // vapor pressure at airT [kPa]
	float delta;   // dEsat/dTair [kPa/K]
 	float dd;      // vapor pressure deficit [kPa]
 	float rc;      // canopy resistance [s/m]
	float ra;      // aerodynamic resistance [s/ma]
	float le;      // latent heat [W/m2]

	if (wSpeed < 0.2) wSpeed = 0.2;

	solNet = (1.0 - albedo) * solRad / MDConstIGRATE;
	lngNet = MDSRadNETLong (i0hDay,airT,solRad,vPress);

	za     = height + MDConstZMINH;
	disp   = MDPETlibZPDisplacement (height,lai,sai,z0g);
	z0     = MDPETlibRoughness (disp,height,lai,sai,z0g);
	aa     = solNet + lngNet - sHeat;
	es     = MDPETlibVPressSat (airT);
	delta  = MDPETlibVPressDelta (airT);
	dd     = es - vPress; 
	rc     = MDPETlibCanopySurfResistance (airT,solRad,dd,lai,sai,r5,cd,cr,glMax);
	ra     = log ((za - disp) / z0);
	ra     = ra * ra / (0.16 * wSpeed);
	le     = MDPETlibPenmanMontieth (aa, dd, delta, ra, rc);

	pet = MDConstEtoM * MDConstIGRATE * le; 
	if (pet <= -2.0) CMmsgPrint (CMmsgUsrError,"PMday negativ = %f solnet = %f le = %f height = %f wSpeed = %f SolRad =%f \n",pet,solNet,le,height,wSpeed, solRad);
   MFVarSetFloat (_MDOutPetID,itemID,pet);
}

int MDCore_RainPotETPMdayDef () {
	if (_MDOutPetID != MFUnset) return (_MDOutPetID);

	MFDefEntering ("Rainfed Potential Evapotranspiration (Penman Monteith [day])");
	if (((_MDInDayLengthID     = MDCommon_SolarRadDayLengthDef ()) == CMfailed) ||
        ((_MDInI0HDayID        = MDCommon_SolarRadI0HDayDef ())    == CMfailed) ||
        ((_MDInCParamAlbedoID  = MDParam_LCAlbedoDef ())           == CMfailed) ||
        ((_MDInCParamCHeightID = MDParam_LCHeightDef ())           == CMfailed) ||
        ((_MDInCParamR5ID      = MDParam_LCR5Def ())               == CMfailed) ||
        ((_MDInCParamCDID      = MDParam_LCCDDef ())               == CMfailed) ||
        ((_MDInCParamCRID      = MDParam_LCCRDef ())               == CMfailed) ||
        ((_MDInCParamGLMaxID   = MDParam_LCGLMaxDef ())            == CMfailed) ||
        ((_MDInCParamZ0gID     = MDParam_LCZ0gDef ())              == CMfailed) ||
        ((_MDInLeafAreaIndexID = MDParam_LeafAreaIndexDef ())      == CMfailed) ||
        ((_MDInStemAreaIndexID = MDParam_LCStemAreaIndexDef ())    == CMfailed) ||
        ((_MDInSolRadID        = MDCommon_SolarRadDef ())          == CMfailed) ||
        ((_MDInCommon_AtMeanID = MDCommon_AirTemperatureDef ())    == CMfailed) ||
		((_MDInVPressID        = MFVarGetID (MDVarCommon_HumidityVaporPressure, "Pa",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
		((_MDInWSpeedID        = MFVarGetID (MDVarCommon_WindSpeed,             "m/s", MFInput,  MFState, MFBoundary)) == CMfailed) ||
		((_MDOutPetID          = MFVarGetID (MDVarCore_RainPotEvapotrans,       "mm",  MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
		(MFModelAddFunction (_MDRainPotETPMday) == CMfailed)) return (CMfailed);
	MFDefLeaving  ("Rainfed Potential Evapotranspiration (Penman Monteith [day])");
	return(_MDOutPetID);
}
