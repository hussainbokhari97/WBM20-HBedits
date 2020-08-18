/******************************************************************************

GHAAS Water Balance/Transport Model V2.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2020, UNH - ASRC/CUNY

MDIrrigation.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

#define MDParIrrigationCropFileName "CropParameterFileName"
 
typedef struct {
    int   ID;
    int   DW_ID;
    char  cropName [80];
    char  cropDistrFileName [80];
    int   cropIsRice;
    float cropSeasLength [4];
    float cropKc [4 - 1];
    float cropRootingDepth;
    float cropDepletionFactor;
    float cropLeachReq;
} MDIrrigatedCrop;

enum { FAO_ID = 0, IWMI_ID = 1 };

static MDIrrigatedCrop *_MDirrigCropStruct = (MDIrrigatedCrop *) NULL;

//Input
static int  _MDInIrrAreaFracID          = MFUnset;

static int  _MDInIrrRefEvapotransID     = MFUnset;
static int  _MDInIrrIntensityID         = MFUnset;
static int  _MDInIrrEfficiencyID        = MFUnset;
static int  _MDInPrecipID               = MFUnset;
static int  _MDInSPackChgID             = MFUnset;
static int  _MDGrowingSeason1ID         = MFUnset;
static int  _MDGrowingSeason2ID         = MFUnset;

static int  _MDInFldCapaID              = MFUnset;
static int  _MDInWltPntID               = MFUnset;
static int  _MDNumberOfIrrCrops         = MFUnset;
static int *_MDInCropFractionIDs        = (int *) NULL;
//Output
static int  _MDOutIrrNetDemandID        = MFUnset;
static int  _MDOutIrrEvapotranspID      = MFUnset;
static int  _MDOutIrrGrossDemandID      = MFUnset;
static int  _MDOutIrrReturnFlowID       = MFUnset;
static int  _MDOutIrrSoilMoistID        = MFUnset;
static int  _MDOutIrrSMoistChgID        = MFUnset;
static int *_MDOutCropDeficitIDs        = (int *) NULL;
static int *_MDOutCropETIDs             = (int *) NULL;
static int *_MDOutCropGrossDemandIDs    = (int *) NULL;
static int  _MDNonIrrFractionID         = MFUnset;
static int  _MDRicePoindingDepthID      = MFUnset;

static int  _MDRicePercolationRateID     = MFUnset;
static int  _MDIrrigatedAreaMap;
static bool _MDIntensityDistributed      = true;
 
static const char *CropParameterFileName;

static int getDaysSincePlanting (int dayOfYearModel, int *dayOfYearPlanting,int numGrowingSeasons,const MDIrrigatedCrop * pIrrCrop) {
	int i, ret = -888;
	int   daysSincePlanted ;	// Default> crop is not grown!

	for (i = 0; i < numGrowingSeasons; i++) {
		daysSincePlanted = dayOfYearModel - dayOfYearPlanting [i];
		if (daysSincePlanted < 0)  daysSincePlanted = 365 + (dayOfYearModel - dayOfYearPlanting [i]);
		if (daysSincePlanted < pIrrCrop->cropSeasLength[0]
		                     + pIrrCrop->cropSeasLength[1]
							 + pIrrCrop->cropSeasLength[2]
							 + pIrrCrop->cropSeasLength[3]) ret = daysSincePlanted;
	}
	return (ret);
}

static int getCropStage (const MDIrrigatedCrop *pIrrCrop, int daysSincePlanted) {
	int stage = 0;

    if      (daysSincePlanted <= pIrrCrop->cropSeasLength[0]) stage = 1;
    else if (daysSincePlanted <= pIrrCrop->cropSeasLength[0]
	                           + pIrrCrop->cropSeasLength[1]) stage = 2;
    else if (daysSincePlanted <= pIrrCrop->cropSeasLength[0]
	                           + pIrrCrop->cropSeasLength[1]
	                           + pIrrCrop->cropSeasLength[2]) stage = 3;
	else if (daysSincePlanted <= pIrrCrop->cropSeasLength[0]
	                           + pIrrCrop->cropSeasLength[1]
	                           + pIrrCrop->cropSeasLength[2]
	                           + pIrrCrop->cropSeasLength[3]) stage = 4;
	return (stage);
}

static float getCropKc (const MDIrrigatedCrop *pIrrCrop, int daysSincePlanted, int curCropStage)
{
	float kc;

   //Returns kc depending on the current stage of the growing season
   switch(curCropStage) {
		default:
		case 0: kc = 0.0; break; //crop is not currently grown
		case 1: kc = pIrrCrop->cropKc [0]; break;
		case 2: kc = pIrrCrop->cropKc [0] + (pIrrCrop->cropKc [1] - pIrrCrop->cropKc [0])
		                                  * (daysSincePlanted - pIrrCrop->cropSeasLength [0])
										  /  pIrrCrop->cropSeasLength [1];
			break;
 		case 3: kc = pIrrCrop->cropKc [1]; break;
		case 4: kc = pIrrCrop->cropKc [1] + (pIrrCrop->cropKc[2]-pIrrCrop->cropKc[1])
		                                  * (daysSincePlanted - (pIrrCrop->cropSeasLength [0] + pIrrCrop->cropSeasLength [1] + pIrrCrop->cropSeasLength [2]))
										  / pIrrCrop->cropSeasLength[3];
			break;
	}
 	return (kc);
}

static float getCurCropRootingDepth (MDIrrigatedCrop *pIrrCrop, int dayssinceplanted) {
	float rootDepth;
	float totalSeasonLenth = pIrrCrop->cropSeasLength [0]
	                       + pIrrCrop->cropSeasLength [1]
					       + pIrrCrop->cropSeasLength [2]
					       + pIrrCrop->cropSeasLength [3];
    rootDepth = pIrrCrop->cropRootingDepth * (0.5 + 0.5 * sin (3.03 * (dayssinceplanted  /  totalSeasonLenth) - 1.47));
 	if (0.15 > rootDepth) rootDepth = 0.15;
	return (rootDepth);
}

static float getCorrDeplFactor (const MDIrrigatedCrop *pIrrCrop, float dailyETP) {

	float cropdeplFactor = pIrrCrop->cropDepletionFactor + 0.04 * (5 - dailyETP);

    if (0.1 >= cropdeplFactor) cropdeplFactor = 0.1;
	if (0.8 <= cropdeplFactor) cropdeplFactor = 0.8;
	return (cropdeplFactor);
}

static int readCropParameters (const char *filename) {
	FILE *inputCropFile;
	int i = 0, die;
	if ((inputCropFile = fopen (filename, "r")) == (FILE *) NULL) {
		CMmsgPrint (CMmsgUsrError,"Crop Parameter file could not be opned, filename: %s\n", filename);
		return (CMfailed);
	}
	else {
		char buffer[512];
		// read headings..
		fgets (buffer,sizeof (buffer),inputCropFile);

		while (fgets(buffer, sizeof(buffer), inputCropFile) != NULL) {
			_MDirrigCropStruct = (MDIrrigatedCrop *) realloc (_MDirrigCropStruct, (i + 1) * sizeof (MDIrrigatedCrop));
			_MDInCropFractionIDs     = (int *) realloc (_MDInCropFractionIDs,     (i + 1) * sizeof (int));
			_MDOutCropDeficitIDs     = (int *) realloc (_MDOutCropDeficitIDs,     (i + 1) * sizeof (int));
			_MDOutCropETIDs          = (int *) realloc (_MDOutCropETIDs,          (i + 1) * sizeof (int));
			_MDOutCropGrossDemandIDs = (int *) realloc (_MDOutCropGrossDemandIDs, (i + 1) * sizeof (int));
			_MDInCropFractionIDs [i] = _MDOutCropETIDs[i] =  _MDOutCropDeficitIDs [i] = _MDOutCropGrossDemandIDs[i] = MFUnset;
			sscanf (buffer, "%i" "%i" "%s" "%s" "%f" "%f" "%f" "%f" "%f" "%f" "%f" "%f" "%f",
		       &(_MDirrigCropStruct [i].ID),
		       &(_MDirrigCropStruct [i].DW_ID),
		         _MDirrigCropStruct [i].cropName,
		         _MDirrigCropStruct [i].cropDistrFileName,
		       &(_MDirrigCropStruct [i].cropKc [0]),
		       &(_MDirrigCropStruct [i].cropKc [1]),
		       &(_MDirrigCropStruct [i].cropKc [2]),
		       &(_MDirrigCropStruct [i].cropSeasLength [0]),
		       &(_MDirrigCropStruct [i].cropSeasLength [1]),
		       &(_MDirrigCropStruct [i].cropSeasLength [2]),
		       &(_MDirrigCropStruct [i].cropSeasLength [3]),
		       &(_MDirrigCropStruct [i].cropRootingDepth),
		       &(_MDirrigCropStruct [i].cropDepletionFactor));

			_MDirrigCropStruct [i].cropIsRice = strcmp (_MDirrigCropStruct [i].cropName , "Rice") == 0 ? 1 : 0;
			i += 1;
		}
	}
	_MDNumberOfIrrCrops = i - 1;
	return (CMsucceeded);
}

static int getNumGrowingSeasons(float irrIntensity) { return ceil (irrIntensity); }

static float getIrrGrossWaterDemand(float netIrrDemand, float IrrEfficiency) {
	if (0.0 >= IrrEfficiency) IrrEfficiency = 38.0; // no data, set to average value
	return (netIrrDemand * 100.0 / IrrEfficiency);
}

static void _MDIrrGrossDemand (int itemID) {
//Input
	float irrAreaFrac;
	float irrEffeciency;
	float dailyPrecip    = 0.0;
	float dailyEffPrecip = 0.0;
	float refETP;
	float cropFraction [_MDNumberOfIrrCrops + 1];
	float snowpackChg = 0.0;
	int   seasStart [3];
	float dailyPercolation;
	float wltPnt;
	float fldCap;
//Output:
	float totalNetIrrDemand   = 0.0;
	float totalIrrPercolation = 0.0;
// Local
	int   i;
	float reqPondingDepth ;
	float pondingDepth;
    float irrIntensity;
	float cropDepletionFactor;
	float meanSMChange;
	float meanSMoist;
	int   daysSincePlanted;
	float totAvlWater, readAvlWater, curDepl, prevSoilMstDepl;
	float addBareSoil        = 0.0;
	float bareSoilBalance    = 0.0;
	float cropCoeff;
	float croppedArea        = 0.0;
	float cropWR             = 0.0;
	float deepPercolation    = 0.0;
	float loss               = 0.0;
	float netIrrDemand       = 0.0;
	float relCropFraction    = 0.0;
	float rootDepth;
	float sumOfCropFractions = 0.0;
	float totalCropETP       = 0.0;
	float totGrossDemand     = 0.0;
	float riceWaterBalance;
	float nonRiceWaterBalance;
	float smChange;
	int   numGrowingSeasons;
	float curCropFraction;
	float returnFlow;
	float cropETPlusEPloss;
	float debug = 0.0;
	int   stage;
	int   curDay;
	
	curDay = MFDateGetDayOfYear ();

	reqPondingDepth  =MFVarGetFloat (_MDRicePoindingDepthID, itemID, 2.0);
	irrAreaFrac     = MFVarGetFloat (_MDInIrrAreaFracID,     itemID, 0.0);
	
	switch (_MDIrrigatedAreaMap) {
		case FAO_ID:  irrIntensity = MFVarGetFloat (_MDInIrrIntensityID, itemID, 100.0) / 100.0; break;
		case IWMI_ID: irrIntensity = 1.0; _MDIntensityDistributed = true; break;
	}
	
	seasStart [0]= MFVarGetFloat (_MDGrowingSeason1ID,      itemID, -100);
	seasStart [1]= MFVarGetFloat (_MDGrowingSeason2ID,      itemID, -100);
	if ((seasStart[0] < 0) || (seasStart[0]< 0)) CMmsgPrint (CMmsgDebug,"Growing Season! \n");

	if (0.0 < irrAreaFrac) {
		for (i = 0;i < _MDNumberOfIrrCrops + 1; ++i) { cropFraction[i] = 0.0; }
		for (i = 0;i < _MDNumberOfIrrCrops;     ++i) {
			if (MFVarTestMissingVal (_MDInCropFractionIDs [i],itemID)) {
				MFVarSetFloat(_MDInCropFractionIDs [i],itemID,0.0);
			}
		}
		irrEffeciency   = MFVarGetFloat(_MDInIrrEfficiencyID,    itemID, 38);
		dailyPrecip     = MFVarGetFloat(_MDInPrecipID,           itemID, 0.0);
		refETP          = MFVarGetFloat(_MDInIrrRefEvapotransID, itemID, 0.0);
		if (0.0 >= irrEffeciency) irrEffeciency = 38.0;
		if (0.0 >= refETP)               refETP = 0.01;

		snowpackChg = MFVarGetFloat (_MDInSPackChgID, itemID, 0.0);
		dailyEffPrecip = 0.0 >= snowpackChg ? dailyPrecip + fabs (snowpackChg) : 0.0;
 
	 	dailyPercolation = MFVarGetFloat (_MDRicePercolationRateID, itemID, 3.0);
	 	wltPnt           = MFVarGetFloat (_MDInWltPntID,  itemID, 0.15);
		fldCap           = MFVarGetFloat (_MDInFldCapaID, itemID, 0.25);
		if (0.0 >= fldCap) { fldCap = 0.35; wltPnt = 0.2; }

		if (1.2 > irrIntensity && 1.0 < irrIntensity) irrIntensity = 1.0;
		if (2.0 < irrIntensity)                       irrIntensity = 2.0;

		curDepl = sumOfCropFractions = 0.0;
		for (i = 0; i < _MDNumberOfIrrCrops; i++) { sumOfCropFractions += MFVarGetFloat(_MDInCropFractionIDs [i],itemID, 0.0);	}
		if (0.0 >= sumOfCropFractions) { // No Cropdata for irrigated cell: default to some cereal crop
			MFVarSetFloat(_MDInCropFractionIDs [2], itemID, 0.3);
			sumOfCropFractions = 0.3;
		}

		meanSMChange = totalCropETP = 0.0;
		for (i = 0; i < _MDNumberOfIrrCrops; ++i) { // cropFraction[_MDNumberOfIrrCrops] is bare soil Area!
			numGrowingSeasons = 0 == _MDIrrigatedAreaMap ? getNumGrowingSeasons(irrIntensity) : 2; // FAO MAP or IWMI
			curCropFraction   = MFVarGetFloat (_MDInCropFractionIDs [i], itemID, 0.0);
			relCropFraction   = 0.0 < curCropFraction ? curCropFraction / sumOfCropFractions : 0.0;
			daysSincePlanted  = getDaysSincePlanting (curDay, seasStart, numGrowingSeasons, &_MDirrigCropStruct [i]);

			if (_MDIntensityDistributed) {
				if (0 < daysSincePlanted) {
					addBareSoil = relCropFraction - irrIntensity / ceil(irrIntensity) * relCropFraction;
					if (0.0 < relCropFraction) cropFraction [i] = relCropFraction - addBareSoil;
					cropFraction [_MDNumberOfIrrCrops] += addBareSoil;
			 	}
				else {
					cropFraction [i] = 0.0;
					cropFraction [_MDNumberOfIrrCrops] += relCropFraction;
				}
			}
			else { // try to grow all crops in Growing Season 1 (always assumed to be the first season!)
				if (0 < daysSincePlanted) { // Growing season
					if (curDay < seasStart [1] || (daysSincePlanted > seasStart[1] - seasStart[0])) // First or perennial growing season
						addBareSoil = 1.0 > irrIntensity  ? relCropFraction * (1.0 - irrIntensity) : 0.0;
					else  // second crop
						addBareSoil = 1.0 < irrIntensity ? relCropFraction - (irrIntensity - 1.0) * relCropFraction : relCropFraction;
					if (0.0 < relCropFraction) cropFraction [i] = relCropFraction - addBareSoil;
					cropFraction [_MDNumberOfIrrCrops] += addBareSoil;
				}
				else { //  Non-growing season
					cropFraction [i] = 0.0;
					cropFraction [_MDNumberOfIrrCrops] += relCropFraction;
				}
			}
		}
		croppedArea = 0.0;

		for (i = 0; i < _MDNumberOfIrrCrops; i++) croppedArea+=cropFraction[i];

		for (i = 0; i < _MDNumberOfIrrCrops; i++) {
			netIrrDemand = cropWR = deepPercolation = smChange=0.0;
			relCropFraction = cropFraction [i];
			numGrowingSeasons = 0 == _MDIrrigatedAreaMap ? getNumGrowingSeasons(irrIntensity) : 2; // FAO MAP or IWMI
			if (0.0 < relCropFraction) {
				netIrrDemand = 0.0;
			 	daysSincePlanted = getDaysSincePlanting(curDay, seasStart, numGrowingSeasons, _MDirrigCropStruct + i);
			 	if (0 < daysSincePlanted) {
					prevSoilMstDepl = MFVarGetFloat(_MDOutCropDeficitIDs [i],itemID, 0.0);
					stage     = getCropStage (_MDirrigCropStruct + i, daysSincePlanted);
					cropCoeff = getCropKc    (_MDirrigCropStruct + i, daysSincePlanted, stage);
					cropWR    = refETP * cropCoeff;
					rootDepth = getCurCropRootingDepth (_MDirrigCropStruct + i,daysSincePlanted);
					rootDepth = 400; // TODO
				    cropDepletionFactor = getCorrDeplFactor (_MDirrigCropStruct + i, cropWR);
					if (_MDirrigCropStruct [i].cropIsRice==1) {
					    pondingDepth = prevSoilMstDepl + dailyEffPrecip - cropWR - dailyPercolation;
						if (pondingDepth >= reqPondingDepth) {
							deepPercolation = pondingDepth - reqPondingDepth;
							pondingDepth = reqPondingDepth;
						}
						if (pondingDepth < reqPondingDepth) {
							netIrrDemand = reqPondingDepth-pondingDepth;
							pondingDepth = reqPondingDepth;
						}
						curDepl  = pondingDepth; //so that current ponding depth gets set..		
						smChange = curDepl - prevSoilMstDepl;
						deepPercolation += dailyPercolation;
						riceWaterBalance = dailyEffPrecip + netIrrDemand - cropWR - deepPercolation - smChange;
					}
					else {
						totAvlWater  = (fldCap - wltPnt) * rootDepth;
						 
						readAvlWater = totAvlWater * cropDepletionFactor;
					 
						curDepl  = prevSoilMstDepl - dailyEffPrecip + cropWR;
						if (0.0 > curDepl) { curDepl = 0; deepPercolation = dailyEffPrecip - prevSoilMstDepl -cropWR; }
						if (curDepl >= totAvlWater) {
							curDepl =totAvlWater;
						}
						if (curDepl >= readAvlWater) {
							netIrrDemand = curDepl;
							netIrrDemand = curDepl;
							curDepl = prevSoilMstDepl - netIrrDemand-dailyEffPrecip+cropWR;
						}
						smChange = prevSoilMstDepl - curDepl;
					
						nonRiceWaterBalance = dailyEffPrecip + netIrrDemand - cropWR - deepPercolation - smChange;
						smChange = dailyEffPrecip + netIrrDemand - cropWR - deepPercolation - nonRiceWaterBalance;
					}
				 	MFVarSetFloat(_MDOutCropDeficitIDs [i], itemID, curDepl);
				}
				totalNetIrrDemand += netIrrDemand      * relCropFraction;
				totalCropETP        += cropWR          * relCropFraction;
				meanSMChange        += smChange        * relCropFraction;
				totalIrrPercolation += deepPercolation * relCropFraction;
	 		}
			MFVarSetFloat (_MDOutCropETIDs [i], itemID, netIrrDemand * relCropFraction * irrAreaFrac); 		
			MFVarSetFloat (_MDOutCropGrossDemandIDs[i],itemID,netIrrDemand * relCropFraction * irrAreaFrac/(irrEffeciency/100));
		} //for all crops
		// Add Water Balance for bare soil
		cropWR = curDepl = 0.0;
		relCropFraction = cropFraction [_MDNumberOfIrrCrops];
		netIrrDemand    = 0.0;
		if (0.0 < relCropFraction) { //Crop is not currently grown. ET from bare soil is equal to ET (initial)
			cropWR = 0.2 * refETP;
			
			prevSoilMstDepl = MFVarGetFloat (_MDOutCropDeficitIDs [_MDNumberOfIrrCrops],itemID, 0.0);
			totAvlWater = (fldCap - wltPnt) * 250;	//assumed RD = 0.25 m
			deepPercolation = 0.0;
			curDepl  = prevSoilMstDepl - dailyEffPrecip + cropWR;
			if (0.0 > curDepl) { curDepl = 0; deepPercolation=dailyEffPrecip - prevSoilMstDepl - cropWR; }
			if (curDepl >= totAvlWater) {
				cropWR = totAvlWater - prevSoilMstDepl + dailyEffPrecip;
				deepPercolation = 0.0;
				curDepl = totAvlWater;
			}
 			smChange = prevSoilMstDepl - curDepl;
			bareSoilBalance = dailyEffPrecip - smChange - cropWR - netIrrDemand - deepPercolation;
  			MFVarSetFloat (_MDOutCropDeficitIDs [_MDNumberOfIrrCrops], itemID, curDepl);
		}
		else {
			cropWR          = 0.0;
			smChange        = 0.0;
			deepPercolation = 0.0;
			curDepl         = 0.0;
		}
		MFVarSetFloat (_MDOutCropETIDs [_MDNumberOfIrrCrops], itemID, cropWR);
  		MFVarSetFloat (_MDNonIrrFractionID, itemID, cropFraction [_MDNumberOfIrrCrops]);
		totalNetIrrDemand   += netIrrDemand    * relCropFraction;
		totalCropETP        += cropWR          * relCropFraction;
		meanSMoist          += curDepl         * relCropFraction;
		meanSMChange        += smChange        * relCropFraction;
		totalIrrPercolation += deepPercolation * relCropFraction;

		totGrossDemand = getIrrGrossWaterDemand (totalNetIrrDemand, irrEffeciency);

		loss = (totGrossDemand - totalNetIrrDemand) + (dailyPrecip - dailyEffPrecip);
		returnFlow = totalIrrPercolation + loss * 0.1;
		cropETPlusEPloss = totalCropETP  + loss * 0.9;
		for (i = 0; i < _MDNumberOfIrrCrops; i++) { debug += MFVarGetFloat (_MDOutCropETIDs [i], itemID, 0.0); }

		MFVarSetFloat(_MDInIrrRefEvapotransID, itemID, refETP            * irrAreaFrac);
		MFVarSetFloat(_MDOutIrrSMoistChgID,    itemID, meanSMChange      * irrAreaFrac);
		MFVarSetFloat(_MDOutIrrNetDemandID,    itemID, totalNetIrrDemand * irrAreaFrac);
		MFVarSetFloat(_MDOutIrrGrossDemandID,  itemID, totGrossDemand    * irrAreaFrac);
		MFVarSetFloat(_MDOutIrrReturnFlowID,   itemID, returnFlow        * irrAreaFrac);
		MFVarSetFloat(_MDOutIrrEvapotranspID,  itemID, cropETPlusEPloss  * irrAreaFrac);	
	}
	else { // cell is not irrigated
		MFVarSetFloat(_MDInIrrRefEvapotransID, itemID, 0.0);
		MFVarSetFloat(_MDOutIrrSoilMoistID,    itemID, 0.0);
		MFVarSetFloat(_MDOutIrrSMoistChgID,    itemID, 0.0);
 		MFVarSetFloat(_MDOutIrrNetDemandID,    itemID, 0.0);
		MFVarSetFloat(_MDOutIrrGrossDemandID,  itemID, 0.0);
		MFVarSetFloat(_MDOutIrrReturnFlowID,   itemID, 0.0);
		MFVarSetFloat(_MDOutIrrEvapotranspID,  itemID, 0.0);
		for (i = 0; i < _MDNumberOfIrrCrops; i++) {MFVarSetFloat(_MDOutCropETIDs [i], itemID, 0.0);}
	}
}

enum { MDnone, MDinput, MDcalculate };

int MDIrrGrossDemandDef () {
	int optID = MFUnset;
	const char *optStr, *optName = MDOptIrrigation;
	const char *options [] = { MDNoneStr, MDInputStr, MDCalculateStr, (char *) NULL };
	const char *mapOptions   [] = { "FAO", "IWMI", (char *) NULL };
	const char *distrOptions [] = { "FirstSeason","Distributed", (char *) NULL };
	int i;
	char varname [20];
	char cropETName [20];
	char cropGrossDemandName[20];
	int mapOptionID = MFUnset;
	int irrDistribuedID = MFUnset;

	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);

	if ((optID == MDnone) || (_MDOutIrrGrossDemandID != MFUnset)) return (_MDOutIrrGrossDemandID);

	MFDefEntering ("Irrigation Gross Demand");

	switch (optID) {
		case MDinput:
			 if(((_MDOutIrrGrossDemandID = MFVarGetID (MDVarIrrGrossDemand,        "mm", MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
		    	((_MDOutIrrReturnFlowID  = MFVarGetID (MDVarIrrReturnFlow,         "mm", MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
				((_MDOutIrrEvapotranspID = MFVarGetID (MDVarIrrEvapotranspiration, "mm", MFInput,  MFFlux,  MFBoundary)) == CMfailed))
				return (CMfailed);
			break;
		case MDcalculate:		
			if (((optStr = MFOptionGet (MDOptIrrIntensity)) == (char *) NULL) || ((irrDistribuedID = CMoptLookup (distrOptions, optStr, true)) == CMfailed)) {
				CMmsgPrint(CMmsgUsrError,"Irrigation Distribution not specifed! Options = 'Distributed' or 'FirstSeason'\n");
				return (CMfailed);
			}
			if (((optStr = MFOptionGet (MDOptIrrigatedAreaMap))  == (char *) NULL) || ((mapOptionID = CMoptLookup (mapOptions, optStr, true)) == CMfailed)) {
				CMmsgPrint(CMmsgUsrError,"Typ of Irr Area not specifed! Options = 'FAO' or 'IWMI'\n");
				return (CMfailed);
			}
			_MDIrrigatedAreaMap=mapOptionID;
	
			if (_MDIrrigatedAreaMap == 1) { //read irrArea for both seasons from IWMI data; Irr Intensity not needed!
				_MDIntensityDistributed = true; //Distributed ; 					
			}
			else { // FAO irrigated Area Map; read Irr Area and Intensity
				if (((optStr = MFOptionGet (MDOptIrrIntensity)) == (char *) NULL) || ((irrDistribuedID = CMoptLookup (distrOptions, optStr, true)) == CMfailed)) {
					CMmsgPrint(CMmsgUsrError,"Irrigation Distribution not specifed! Options = 'Distributed' or 'FirstSeason'\n");
					return CMfailed;
				}
				_MDIntensityDistributed = irrDistribuedID == 0 ? false : true;

				if (((_MDInIrrIntensityID = MFVarGetID (MDVarIrrIntensity,           "-",    MFInput,  MFState, MFBoundary)) == CMfailed)) return (CMfailed);
			}
			if ((optStr = MFOptionGet (MDParIrrigationCropFileName)) != (char *) NULL) CropParameterFileName = optStr;
			if (readCropParameters (CropParameterFileName) == CMfailed) {
				CMmsgPrint(CMmsgUsrError,"Error reading crop parameter file   : %s \n", CropParameterFileName);
				return CMfailed;
			}
			if (((_MDInPrecipID              = MDPrecipitationDef    ()) == CMfailed) ||	 
			    ((_MDInSPackChgID            = MDSPackChgDef         ()) == CMfailed) ||
			    ((_MDInIrrRefEvapotransID    = MDIrrRefEvapotransDef ()) == CMfailed) ||
			    ((_MDInIrrAreaFracID         = MDIrrigatedAreaDef    ())==  CMfailed) ||
			    ((_MDInWltPntID              = MFVarGetID (MDVarSoilWiltingPoint,           "mm/m",   MFInput,   MFState, MFBoundary)) == CMfailed) ||
			    ((_MDInFldCapaID             = MFVarGetID (MDVarSoilFieldCapacity,          "mm/m",   MFInput,   MFState, MFBoundary)) == CMfailed) ||
			    ((_MDGrowingSeason1ID        = MFVarGetID (MDVarIrrGrowingSeason1Start,     "DoY",    MFInput,   MFState, MFBoundary)) == CMfailed) ||
			    ((_MDGrowingSeason2ID        = MFVarGetID (MDVarIrrGrowingSeason2Start,     "DoY",    MFInput,   MFState, MFBoundary)) == CMfailed) ||
				((_MDNonIrrFractionID        = MFVarGetID (MDNonIrrigatedFraction,          "-",      MFOutput,  MFState, MFBoundary)) == CMfailed) ||
				((_MDRicePercolationRateID   = MFVarGetID (MDVarIrrDailyRicePerolationRate, "mm/day", MFInput ,  MFState, MFBoundary)) == CMfailed) ||
				((_MDInIrrEfficiencyID       = MFVarGetID (MDVarIrrEfficiency,              "-",      MFInput,   MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutIrrGrossDemandID     = MFVarGetID (MDVarIrrGrossDemand,             "mm",     MFOutput,  MFFlux,  MFBoundary)) == CMfailed) ||
			    ((_MDOutIrrReturnFlowID      = MFVarGetID (MDVarIrrReturnFlow,              "mm",     MFOutput,  MFFlux,  MFBoundary)) == CMfailed) ||
			    ((_MDOutIrrNetDemandID       = MFVarGetID (MDVarIrrNetWaterDemand,          "mm",     MFOutput,  MFFlux,  MFBoundary)) == CMfailed) ||
			    ((_MDOutIrrSoilMoistID       = MFVarGetID (MDVarIrrSoilMoisture,            "mm",     MFOutput,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDRicePoindingDepthID     = MFVarGetID (MDVarIrrRicePondingDepth,        "mm",     MFInput,   MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutIrrSMoistChgID       = MFVarGetID (MDVarIrrSoilMoistChange,         "mm",     MFOutput,  MFFlux,  MFBoundary)) == CMfailed) ||
			    ((_MDOutIrrEvapotranspID     = MFVarGetID (MDVarIrrEvapotranspiration,      "mm",     MFOutput,  MFFlux,  MFBoundary)) == CMfailed))
				return (CMfailed);

			for (i = 0; i < _MDNumberOfIrrCrops; i++) {
				sprintf (varname, "CropFraction_%02d", i + 1); // Input Fraction of crop type per cell
				if ((_MDInCropFractionIDs [i] = MFVarGetID (varname, "mm", MFInput, MFState, MFBoundary)) == CMfailed) {
					CMmsgPrint (CMmsgUsrError, "CMfailed in MDInCropFractionID \n");
					return CMfailed;
				}
			}
			for (i = 0; i < _MDNumberOfIrrCrops + 1;i++) {
				sprintf (cropETName,          "CropET%02d",           i + 1);  // Keep track of crop ET for each crop seperately z
				sprintf (cropGrossDemandName, "CropGrossDemand%02d",  i + 1);  // Keep track of crop ET for each crop seperately z
				sprintf (varname,             "CropSMDeficiency%02d", i + 1);  // Output Soil Moisture Deficit, crop ET and WithDrawal per croptype
			    if ((_MDOutCropETIDs [i]          = MFVarGetID (cropETName,          "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) return (CMfailed);
			    if ((_MDOutCropGrossDemandIDs [i] = MFVarGetID (cropGrossDemandName, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) return (CMfailed);
			    if ((_MDOutCropDeficitIDs [i]     = MFVarGetID (varname,             "mm", MFOutput, MFState, MFInitial)) == CMfailed) return (CMfailed);
			}
			if (MFModelAddFunction (_MDIrrGrossDemand) == CMfailed) return (CMfailed);
			break;
		default: MFOptionMessage (optName, optStr, options); return (CMfailed);
	}
	MFDefLeaving("Irrigation Gross Demand");
	return (_MDOutIrrGrossDemandID);
}

int MDIrrReturnFlowDef() {
	int ret;

	if (_MDOutIrrReturnFlowID != MFUnset) return (_MDOutIrrReturnFlowID);

	if ((ret = MDIrrGrossDemandDef ()) == CMfailed) return (CMfailed);
	if (ret == MFUnset) return (MFUnset);
	_MDOutIrrReturnFlowID = MFVarGetID (MDVarIrrReturnFlow,     "mm",   MFInput, MFFlux,  MFBoundary);
    return (_MDOutIrrReturnFlowID);
}