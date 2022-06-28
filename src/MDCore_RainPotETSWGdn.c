/*****************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDCore_RainPotETSWGdn.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

static int _MDInDayLengthID     = MFUnset;
static int _MDInI0HDayID        = MFUnset;
static int _MDInCParamAlbedoID  = MFUnset;
static int _MDInCParamCHeightID = MFUnset;
static int _MDInCParamLWidthID  = MFUnset;
static int _MDInCParamRSSID     = MFUnset;
static int _MDInCParamR5ID      = MFUnset;
static int _MDInCParamCDID      = MFUnset;
static int _MDInCParamCRID      = MFUnset;
static int _MDInCParamGLMaxID   = MFUnset;
static int _MDInCParamZ0gID     = MFUnset;
static int _MDInLeafAreaIndexID = MFUnset;
static int _MDInStemAreaIndexID = MFUnset;

static int _MDInCommon_AtMeanID = MFUnset;
static int _MDInAtMinID         = MFUnset;
static int _MDInAtMaxID         = MFUnset;
static int _MDInSolRadID        = MFUnset;
static int _MDInVPressID        = MFUnset;
static int _MDInWSpeedID        = MFUnset;
static int _MDOutPetID          = MFUnset;

static void _MDRainPotETSWGdn (int itemID) {
// Input
	float dayLen  = MFVarGetFloat (_MDInDayLengthID,      itemID, 0.0); // daylength in fraction of day
 	float i0hDay  = MFVarGetFloat (_MDInI0HDayID,         itemID, 0.0); // daily potential insolation on horizontal [MJ/m2]
	float albedo  = MFVarGetFloat (_MDInCParamAlbedoID,   itemID, 0.0); // albedo 
	float height  = MFVarGetFloat (_MDInCParamCHeightID,  itemID, 0.0); // canopy height [m]
	float lWidth  = MFVarGetFloat (_MDInCParamLWidthID,   itemID, 0.0); // average leaf width [m]
	float rss     = MFVarGetFloat (_MDInCParamRSSID,      itemID, 0.0); // soil surface resistance [s/m]
	float r5      = MFVarGetFloat (_MDInCParamR5ID,       itemID, 0.0); // solar radiation at which conductance is halved [W/m2]
	float cd      = MFVarGetFloat (_MDInCParamCDID,       itemID, 0.0); // vpd at which conductance is halved [kPa]
	float cr      = MFVarGetFloat (_MDInCParamCRID,       itemID, 0.0); // light extinction coefficient for projected LAI
	float glMax   = MFVarGetFloat (_MDInCParamGLMaxID,    itemID, 0.0); // maximum leaf surface conductance for all sides of leaf [m/s]
	float z0g     = MFVarGetFloat (_MDInCParamZ0gID,      itemID, 0.0); // z0g       - ground surface roughness [m]
 	float lai     = MFVarGetFloat (_MDInLeafAreaIndexID,  itemID, 0.0); // projected leaf area index
	float sai     = MFVarGetFloat (_MDInStemAreaIndexID,  itemID, 0.0); // projected stem area index
	float airT    = MFVarGetFloat (_MDInCommon_AtMeanID,  itemID, 0.0); // air temperatur [degree C]
	float airTMin = MFVarGetFloat (_MDInAtMinID,          itemID, 0.0); // daily minimum air temperature [degree C]
	float airTMax = MFVarGetFloat (_MDInAtMaxID,          itemID, 0.0); // daily maximum air temperature [degree C]
	float solRad  = MFVarGetFloat (_MDInSolRadID,         itemID, 0.0); // daily solar radiation on horizontal [MJ/m2]
	float vPress  = MFVarGetFloat (_MDInVPressID,         itemID, 0.0) / 1000.0; // daily average vapor pressure [kPa]
	float wSpeed  = MFVarGetFloat (_MDInWSpeedID,         itemID, 0.0); // average wind speed for the day [m/s] 
// Output
	float pet;
// Local
	float sHeat = 0.0; // average subsurface heat storage for day [W/m2]
	float solNet;   // average net solar radiation for daytime [W/m2]
	float airTDtm, airTNtm; // air temperature for daytime and nighttime [degree C]
	float uaDtm,   uaNtm;	// average wind speed for daytime and nighttime [m/s]
	float lngDtm,	lngNtm;	// average net longwave radiation for daytime and nighttime [W/m2]
	float z0;       // roughness parameter [m] 
 	float disp;     // height of zero-plane [m]
	float z0c;      // roughness parameter (closed canopy)
	float dispc;    // zero-plane displacement (closed canopy)
	float aa;       // available energy [W/m2]
	float asubs;    // available energy at ground [W/m2]
	float es;       // vapor pressure at airT [kPa]
	float delta;    // dEsat/dTair [kPa/K]
 	float dd;       // vapor pressure deficit [kPa]
 	float rsc;      // canopy resistance [s/m]
	float led, len;	// daytime and nighttime latent heat [W/m2]
	float rn;       // net radiation [W/m2]
	float rns;		// net radiation at ground [W/m2]
	float raa;		// aerodynamic resistance [s/m]
	float rac;		// leaf boundary layer resistance [s/m]
	float ras;		// ground aerodynamic resistance  [s/m]
	
	if (wSpeed < 0.2) wSpeed = 0.2;

	solNet  = (1.0 - albedo) * solRad / (MDConstIGRATE * dayLen);

	z0c     = MDPETlibRoughnessClosed (height,z0g);
	dispc   = height - z0c / 0.3;
	disp    = MDPETlibZPDisplacement (height,lai,sai,z0g);
	z0      = MDPETlibRoughness (disp,height,lai,sai,z0g);

// daytime
	if (dayLen > 0.0) {
		airTDtm = airT + ((airTMax - airTMin) / (2 * M_PI * dayLen)) * sin (M_PI * dayLen);
		uaDtm   = wSpeed / (dayLen + (1.0 - dayLen) * MDConstWNDRAT);
		lngDtm  = MDSRadNETLong (i0hDay,airTDtm,solRad,vPress);

		rn      = solNet + lngDtm; 
		aa      = rn - sHeat; 
		rns     = rn * exp (-cr * (lai + sai));
		asubs   = rns - sHeat;
		es      = MDPETlibVPressSat (airTDtm);
		delta   = MDPETlibVPressDelta (airTDtm);
		dd      = es - vPress; 

		rsc     = MDPETlibCanopySurfResistance (airTMin,solRad / dayLen,dd,lai,sai,r5,cd,cr,glMax);
		raa     = MDPETlibBoundaryResistance (uaDtm,height,z0g,z0c,dispc,z0,disp);
		rac     = MDPETlibLeafResistance (uaDtm,height,lWidth,z0g,lai,sai,z0c,dispc);
		ras     = MDPETlibGroundResistance (uaDtm,height,z0g,z0c,dispc,z0,disp);
		led     = MDPETlibShuttleworthWallace (rss,aa,asubs,dd,raa,rac,ras,rsc,delta);
	}
	else {
		led = 0.0;
		uaDtm = wSpeed / MDConstWNDRAT;
	}

// nighttime
	if (dayLen < 1.0) {
		airTNtm = airT - ((airTMax - airTMin) / (2 * M_PI * (1 - dayLen))) * sin (M_PI * dayLen);
		uaNtm   = MDConstWNDRAT * uaDtm;
		lngNtm  = MDSRadNETLong (i0hDay,airTNtm,solRad,vPress);

		rn = lngNtm;
		aa = rn - sHeat; 
		rns = rn * exp (-cr * (lai + sai));
		asubs = rns - sHeat; 

		es      = MDPETlibVPressSat (airTNtm);
		delta   = MDPETlibVPressDelta (airTNtm);
		dd      = es - vPress; 
		rsc     = 1.0 / (MDConstGLMIN * lai);
		raa     = MDPETlibBoundaryResistance (uaNtm,height,z0g,z0c,dispc,z0,disp);
		rac     = MDPETlibLeafResistance (uaNtm,height,lWidth,z0g,lai,sai,z0c,dispc);
		ras     = MDPETlibGroundResistance (uaNtm,height,z0g,z0c,dispc,z0,disp);
		len     = MDPETlibShuttleworthWallace (rss,aa,asubs,dd,raa,rac,ras,rsc,delta);
	}
	else len = 0.0;

	pet = MDConstEtoM * MDConstIGRATE * (dayLen * led + (1.0 - dayLen) * len);
//	   if (pet<0)printf("pet <! dayLen=%f\n",dayLen);
   MFVarSetFloat (_MDOutPetID,itemID,pet);
}

int MDCore_RainPotETSWGdnDef () {
	if (_MDOutPetID != MFUnset) return (_MDOutPetID);

	MFDefEntering ("Rainfed Potential Evapotranspiration (Shuttleworth - Wallace [day-night])");
	if (((_MDInDayLengthID     = MDCommon_SolarRadDayLengthDef ())  == CMfailed) ||
        ((_MDInI0HDayID        = MDCommon_SolarRadI0HDayDef ())     == CMfailed) ||
            ((_MDInCParamAlbedoID  = MDParam_LCAlbedoDef ())        == CMfailed) ||
            ((_MDInCParamCHeightID = MDParam_LCHeightDef ())        == CMfailed) ||
            ((_MDInCParamLWidthID  = MDParam_LCLeafWidthDef ())     == CMfailed) ||
            ((_MDInCParamRSSID     = MDParam_LCRSSDef ())           == CMfailed) ||
            ((_MDInCParamR5ID      = MDParam_LCR5Def ())            == CMfailed) ||
            ((_MDInCParamCDID      = MDParam_LCCDDef ())            == CMfailed) ||
            ((_MDInCParamCRID      = MDParam_LCCRDef ())            == CMfailed) ||
            ((_MDInCParamGLMaxID   = MDParam_LCGLMaxDef ())         == CMfailed) ||
            ((_MDInCParamZ0gID     = MDParam_LCZ0gDef ())           == CMfailed) ||
            ((_MDInLeafAreaIndexID = MDParam_LeafAreaIndexDef ())   == CMfailed) ||
            ((_MDInStemAreaIndexID = MDParam_LCStemAreaIndexDef ()) == CMfailed) ||
            ((_MDInSolRadID        = MDCommon_SolarRadDef ())       == CMfailed) ||
            ((_MDInCommon_AtMeanID = MDCommon_AirTemperatureDef ()) == CMfailed) ||
			((_MDInAtMinID   = MFVarGetID (MDVarCommon_AirTempMinimum,        "degC", MFInput, MFState, MFBoundary)) == CMfailed) ||
			((_MDInAtMaxID   = MFVarGetID (MDVarCommon_AirTempMaximum,        "degC", MFInput, MFState, MFBoundary)) == CMfailed) ||
			((_MDInVPressID  = MFVarGetID (MDVarCommon_HumidityVaporPressure, "kPa",  MFInput, MFState, MFBoundary)) == CMfailed) ||
			((_MDInWSpeedID  = MFVarGetID (MDVarCommon_WindSpeed,             "m/s",  MFInput, MFState, MFBoundary)) == CMfailed) ||
			((_MDOutPetID    = MFVarGetID (MDVarCore_RainPotEvapotrans,       "mm",  MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
            (MFModelAddFunction (_MDRainPotETSWGdn) == CMfailed)) return (CMfailed);
	MFDefLeaving  ("Rainfed Potential Evapotranspiration (Shuttleworth - Wallace [day-night])");
	return (_MDOutPetID);
}
