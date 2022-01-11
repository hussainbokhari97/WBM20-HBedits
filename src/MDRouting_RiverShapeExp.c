/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDRouting_RiverShapeExp.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInAux_MeanDischargeID              = MFUnset;
static int _MDInRiverSlopeID          = MFUnset;
// Output
static int _MDOutRiverAvgDepthMeanID  = MFUnset;
static int _MDOutRiverWidthMeanID     = MFUnset;
static int _MDOutRiverVelocityMeanID  = MFUnset;
static int _MDOutRiverShapeExponentID = MFUnset;

static void _MDRiverShapeExponent (int itemID) {
// Input
	float slope;       // River slope [m/km]
	float discharge;   // Mean annual discharge [m3/s]
// Output
	float yMean = 0.0; // River average depth at mean discharge [m]
	float wMean = 0.0; // River width at mean discharge [m]
	float vMean = 0.0; // River velocity at mean discharge [m/s]
// Local
	float dL;          // Reach length [m]
//	float eta = 0.25, nu = 0.4,  tau = 8.0,  phi = 0.58;    //old
//	float eta = 0.36, nu = 0.37, tau = 3.55, phi = 0.51;	//new based on Knighton (avg)
	float eta = 0.33, nu = 0.35, tau = 3.67, phi = 0.45;	// Hey and Thorn (1986)

	dL        = MFModelGetLength (itemID);
	discharge = fabs(MFVarGetFloat(_MDInAux_MeanDischargeID,  itemID, 0.0));
	if (discharge > 0.0) {
		if ((_MDInRiverSlopeID == MFUnset) || CMmathEqualValues (dL, 0.0) || MFVarTestMissingVal (_MDInRiverSlopeID, itemID)) {
		// Slope independent riverbed geometry
			yMean = eta * pow (discharge, nu);
			wMean = tau * pow (discharge, phi);
			vMean = discharge / (yMean * wMean);
		}
		else { // Slope dependent riverbed geometry
			slope = MFVarGetFloat(_MDInRiverSlopeID,      itemID, 0.01) / 1000.0;
			yMean = eta * pow (discharge, nu);
			wMean = tau * pow (discharge, phi);
			vMean = discharge / (yMean * wMean);
		}
	}
	MFVarSetFloat (_MDOutRiverAvgDepthMeanID,  itemID, yMean);
	MFVarSetFloat (_MDOutRiverWidthMeanID,     itemID, wMean);
	MFVarSetFloat (_MDOutRiverVelocityMeanID,  itemID, vMean);
	MFVarSetFloat (_MDOutRiverShapeExponentID, itemID, 2.0);
}

enum { MDhelp, MDinput, MDindependent, MDdependent };

int MDRouting_RiverShapeExponentDef () {
	int  optID = MDinput;
	const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, "slope-independent", "slope-dependent", (char *) NULL };

	if (_MDOutRiverShapeExponentID != MFUnset) return (_MDOutRiverShapeExponentID);

	MFDefEntering ("River Shape Exponent");
	if ((optStr = MFOptionGet (MDOptRouting_Riverbed)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		default:      MFOptionMessage (MDOptRouting_Riverbed, optStr, options); return (CMfailed);
		case MDhelp:  MFOptionMessage (MDOptRouting_Riverbed, optStr, options);
		case MDinput:
			if (((_MDOutRiverAvgDepthMeanID  = MFVarGetID (MDVarRouting_RiverAvgDepthMean, "m", MFInput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutRiverWidthMeanID     = MFVarGetID (MDVarRouting_RiverWidthMean, "m", MFInput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutRiverVelocityMeanID  = MFVarGetID (MDVarRouting_RiverVelocityMean, "m/s", MFInput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutRiverShapeExponentID = MFVarGetID (MDVarRouting_RiverShapeExponent, MFNoUnit, MFInput, MFState, MFBoundary)) == CMfailed))
				return (CMfailed);
			break;
		case MDdependent:
			if ((_MDInRiverSlopeID           = MFVarGetID (MDVarRouting_RiverSlope, "m/km", MFInput, MFState, MFBoundary)) == CMfailed)
				return (CMfailed);
		case MDindependent:
			if (((_MDInAux_MeanDischargeID = MDAux_MeanDischargeDef()) == CMfailed) ||
                ((_MDOutRiverAvgDepthMeanID  = MFVarGetID (MDVarRouting_RiverAvgDepthMean, "m", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutRiverWidthMeanID     = MFVarGetID (MDVarRouting_RiverWidthMean, "m", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutRiverVelocityMeanID  = MFVarGetID (MDVarRouting_RiverVelocityMean, "m/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutRiverShapeExponentID = MFVarGetID (MDVarRouting_RiverShapeExponent, MFNoUnit, MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDRiverShapeExponent) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("River Shape Exponent");
	return (_MDOutRiverShapeExponentID);
}
