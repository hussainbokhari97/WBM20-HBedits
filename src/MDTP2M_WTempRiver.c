/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDWTempRiverRoute.c

wil.wollheim@unh.edu

amiara@ccny.cuny.edu - updated river temperature calculations as of Sep 2016

Route temperature through river network

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AirTemperatureID   = MFUnset;
static int _MDInCommon_HumidityDewPointID = MFUnset; // FOR NEW TEMP MODULE
static int _MDInCommon_SolarRadID         = MFUnset;
static int _MDInRouting_DischargeID       = MFUnset;
static int _MDInDischargeIncomingID       = MFUnset;
static int _MDInCore_RunoffVolumeID       = MFUnset;
static int _MDInWTempRunoffID             = MFUnset;
static int _MDInRiverWidthID              = MFUnset;
static int _MDInRiverStorageChgID         = MFUnset;
static int _MDInRiverStorageID            = MFUnset;
static int _MDInWindSpeedID               = MFUnset;
static int _MDInResReleaseID              = MFUnset;
static int _MDInResStorageChangeID        = MFUnset;
static int _MDInResStorageID              = MFUnset;
static int _MDInResCapacityID     	      = MFUnset;
// Output
static int _MDOutLocalIn_QxTID            = MFUnset;
static int _MDOutRemoval_QxTID            = MFUnset;
static int _MDOutFlux_QxTID               = MFUnset;
static int _MDOutStorage_QxTID            = MFUnset;
static int _MDOutDeltaStorage_QxTID       = MFUnset;
static int _MDOutWTempDeltaT_QxTID        = MFUnset;
static int _MDOutFluxMixing_QxTID         = MFUnset;
static int _MDOutStorageMixing_QxTID      = MFUnset;
static int _MDOutDeltaStorageMixing_QxTID = MFUnset;
static int _MDOutWTempMixing_QxTID        = MFUnset;
static int _MDOutEquil_Temp	              = MFUnset;
static int _MDOutWTempRiverID             = MFUnset;


static void _MDWTempRiver (int itemID) {
    float Q;
	float Q_incoming;
    float RO_Vol;
	float RO_WTemp;
	float QxT_input;
	float QxT;
	float QxTnew = 0;
	float QxTout = 0;
    float Q_WTemp;
    float Q_WTemp_new;
    float StorexT;
    float StorexT_new;
    float DeltaStorexT;
    
    //processing variables
    float channelWidth;
    float channelLength;
    float waterStorageChange;
    float waterStorage;
    float ResWaterStorageChange = 0;
    float ResWaterStorage = 0;
    float solarRad;
    float windSpeed;
    float Tair;
    float Tequil = 0;
    float HeatLoss_int   = 4396.14; // is intercept assuming no wind and clouds
    float HeatLoss_slope = 1465.38; // is slope assuming no wind and clouds
    float deltaT;
     
    float ReservoirArea;
    float ReservoirDepth;
    float ReservoirVelocity;

    // conservative mixing variables (parallel to those above_
    float QxT_mix;
    float QxTnew_mix = 0;
    float QxTout_mix = 0;
    float Q_WTemp_mix;
    float StorexT_mix;  
    float StorexT_new_mix;  
    float DeltaStorexT_mix; 
    float QxTRemoval;
    float resCapacity;		//RJS 071511	Reservoir capacity [km3]
    float dt = MFModelGet_dt ();

    Q                  = MFVarGetFloat (_MDInRouting_DischargeID,     itemID, 0.0);
   	Q_incoming         = MFVarGetFloat (_MDInDischargeIncomingID,     itemID, 0.0); // already includes local runoff
    RO_Vol             = MFVarGetFloat (_MDInCore_RunoffVolumeID,     itemID, 0.0);
   	RO_WTemp           = MFVarGetFloat (_MDInWTempRunoffID,           itemID, 0.0);
 	waterStorageChange = MFVarGetFloat (_MDInRiverStorageChgID,       itemID, 0.0);
   	waterStorage       = MFVarGetFloat (_MDInRiverStorageID,          itemID, 0.0);
   	channelWidth       = MFVarGetFloat (_MDInRiverWidthID,            itemID, 0.0);
 	solarRad           = MFVarGetFloat (_MDInCommon_SolarRadID,       itemID, 0.0); //MJ/m2/d - CHECK UNITS
 	windSpeed          = MFVarGetFloat (_MDInWindSpeedID,             itemID, 0.0);
    Tair               = MFVarGetFloat (_MDInCommon_AirTemperatureID, itemID, 0.0);
    QxT                = MFVarGetFloat (_MDOutFlux_QxTID,             itemID, 0.0);
    StorexT            = MFVarGetFloat (_MDOutStorage_QxTID,          itemID, 0.0);
    QxT_mix            = MFVarGetFloat (_MDOutFluxMixing_QxTID,       itemID, 0.0);
    StorexT_mix        = MFVarGetFloat (_MDOutStorageMixing_QxTID,    itemID, 0.0);

     if (_MDInResStorageID != MFUnset) {
         ResWaterStorageChange = MFVarGetFloat ( _MDInResStorageChangeID, itemID, 0.0) * pow(1000,3); // convert to m3/
         ResWaterStorage       = MFVarGetFloat ( _MDInResStorageID,       itemID, 0.0) * pow(1000,3); // convert to m3 
         resCapacity           = MFVarGetFloat ( _MDInResCapacityID,      itemID, 0.0);	//RJS 071511
     } else {
         ResWaterStorageChange =
         ResWaterStorage       = 
         resCapacity           = 0.0; //RJS 071511
    }
     
    if(Q          < 0.0) Q = 0.0;          //RJS 120409
    if(Q_incoming < 0.0) Q_incoming = 0.0; //RJS 120409

    if(resCapacity > 0.0) {
    	waterStorage = waterStorage + ResWaterStorage;
    	waterStorageChange = waterStorageChange + ResWaterStorageChange;
    	ReservoirArea = pow(((ResWaterStorage / pow(10,6)) / 9.208),(1 / 1.114)) * 1000 * 1000;  // m2, from Takeuchi 1997 - original equation has V in 10^6 m3 and A in km2
    	ReservoirDepth = (ResWaterStorage / ReservoirArea); //m
    	ReservoirVelocity = Q / (ReservoirArea); // m/s
    	channelWidth = MDMaximum(channelWidth, (Q / (ReservoirDepth * ReservoirVelocity))); // m

    	QxT_input  = RO_Vol * RO_WTemp * dt;       //RJS 071511 // m3*degC/d
    	QxTnew     =     QxT + QxT_input + StorexT;     //RJS 071511 // m3*degC/d
    	QxTnew_mix = QxT_mix + QxT_input + StorexT_mix; //RJS 071511

    	if (Q_incoming > 0.000001) {
    		Q_WTemp     = QxTnew     / ((Q_incoming) * dt + (waterStorage - waterStorageChange)); 			//RJS 071511					//degC
    		Q_WTemp_mix = QxTnew_mix / ((Q_incoming) * dt + (waterStorage - waterStorageChange));	//RJS 071511					//degC
    	} else {
    		if (waterStorage > 0) {
    			Q_WTemp	    = StorexT / waterStorage;		// RJS 071511	//degC
    			Q_WTemp_mix = StorexT_mix / waterStorage;	// RJS 071511	//degC
    		} else
                Q_WTemp 	= Q_WTemp_mix = RO_WTemp; // FBM 2022-06-06
    	}

    	Q_WTemp_new = Q_WTemp;														//RJS 071511

    	StorexT_new      = waterStorage * Q_WTemp_new; 							//RJS 071511	//m3*degC
    	DeltaStorexT     = StorexT_new - StorexT; 									//RJS 071511
    	QxTout           = Q * dt * Q_WTemp_new ; 							//RJS 071511	//m3*degC/d
    	QxTRemoval       = QxTnew - (StorexT_new + QxTout); 						//RJS 071511	//m3*degC/d
    	StorexT_new_mix  = waterStorage * Q_WTemp_mix; 							//RJS 071511	//m3*degC
    	DeltaStorexT_mix = StorexT_new_mix - StorexT_mix;							//RJS 071511
    	QxTout_mix       = Q * dt * Q_WTemp_mix; 								//RJS 071511	//m3*degC/s

    	MFVarSetFloat(_MDOutLocalIn_QxTID,            itemID, QxT_input);
    	MFVarSetFloat(_MDOutFlux_QxTID,               itemID, QxTout);
    	MFVarSetFloat(_MDOutStorage_QxTID,            itemID, StorexT_new);
    	MFVarSetFloat(_MDOutDeltaStorage_QxTID,       itemID, DeltaStorexT);
    	MFVarSetFloat(_MDOutWTempRiverID,             itemID, Q_WTemp_new);
    	MFVarSetFloat(_MDOutWTempDeltaT_QxTID,        itemID, deltaT);
    	MFVarSetFloat(_MDOutFluxMixing_QxTID,         itemID, QxTout_mix);
    	MFVarSetFloat(_MDOutStorageMixing_QxTID,      itemID, StorexT_new_mix);
    	MFVarSetFloat(_MDOutDeltaStorageMixing_QxTID, itemID, DeltaStorexT_mix);
    	MFVarSetFloat(_MDOutWTempMixing_QxTID,        itemID, Q_WTemp_mix);
    } else {
    	ReservoirArea     = 0.0;
    	ReservoirVelocity = 0.0;
    	ReservoirDepth    = 0.0;

        QxT_input = RO_Vol * RO_WTemp * dt; //m3*degC/d

        if((Q_incoming) > 0.000001) { //do not include water storage in this check - will screw up mixing estimates
            QxTnew      = QxT     + QxT_input + StorexT; //m3*degC/d
   	        QxTnew_mix  = QxT_mix + QxT_input + StorexT_mix;

            if (Q_incoming * dt + (waterStorage - waterStorageChange) > 0.0) {
                Q_WTemp     = QxTnew     / ((Q_incoming) * MFModelGet_dt () + (waterStorage - waterStorageChange));
                Q_WTemp_mix = QxTnew_mix / ((Q_incoming) * MFModelGet_dt () + (waterStorage - waterStorageChange));
            } else
                Q_WTemp = Q_WTemp_mix = RO_WTemp; //degC

            /// Temperature Processing using Dingman 1972 

            ////////// NEW EQUILIBRIUM TEMP MODEL ///// Edinger et al. 1974: Heat Exchange and Transport in the Environment /////

            float dew_point;
            float wind_f;
            float Tm;
            float beta;
            float kay;
            float solar_KJ;
            float equil2;
            float initial_riverT;
            float RivTemp;
            int x;

            dew_point = MFVarGetFloat (_MDInCommon_HumidityDewPointID, itemID, 0.0);
            initial_riverT = Q_WTemp;

            wind_f = 9.2+(0.46*pow(windSpeed,2)); // wind function

            for (x = 0; x < 4; x++) {
	            Tm = (dew_point + initial_riverT) / 2; // mean of rivertemp initial and dew point
	            beta = 0.35 + (0.015 * Tm) + (0.0012 * pow(Tm, 2.0)); //beta
	            kay = (4.5 + (0.05 * initial_riverT) + (beta * wind_f) + (0.47 * wind_f)) * dt; // K in daily J
	            equil2 = dew_point + solarRad * 1e6 / kay; // solarRad is in MJ/day
	            initial_riverT = equil2;
            }

            channelLength = MFModelGetLength(itemID) * 1000; // converting from km to m
            if ((channelLength > 0.0) && (channelWidth > 0.0)) // FBM channel length can be zero
                 RivTemp = equil2 + (Q_WTemp - equil2) * exp(-kay * channelLength * channelWidth / (4181.3 * Q * dt));
            else RivTemp = initial_riverT;

            /// Resetting outgoing temperature:
            Q_WTemp_new = MDMaximum(RivTemp, 0.0);
            MFVarSetFloat(_MDOutEquil_Temp, itemID, equil2);

            deltaT           = Q_WTemp_new - Q_WTemp;
            StorexT_new      = waterStorage * Q_WTemp_new; //m3*degC
            DeltaStorexT     = StorexT_new - StorexT; //
            QxTout           = Q * dt * Q_WTemp_new ; //m3*degC/d
            QxTRemoval       = QxTnew - (StorexT_new + QxTout); //m3*degC/d
            StorexT_new_mix  = waterStorage * Q_WTemp_mix; //m3*degC
            DeltaStorexT_mix = StorexT_new_mix - StorexT_mix;
            QxTout_mix       = Q * dt * Q_WTemp_mix; //m3*degC/s

            // end

            MFVarSetFloat(_MDOutLocalIn_QxTID, itemID, QxT_input);
            MFVarSetFloat(_MDOutRemoval_QxTID, itemID, QxTRemoval);
            MFVarSetFloat(_MDOutFlux_QxTID, itemID, QxTout);
            MFVarSetFloat(_MDOutStorage_QxTID, itemID, StorexT_new);
            MFVarSetFloat(_MDOutDeltaStorage_QxTID, itemID, DeltaStorexT);
            MFVarSetFloat(_MDOutWTempRiverID, itemID, Q_WTemp_new);
            MFVarSetFloat(_MDOutWTempDeltaT_QxTID, itemID, deltaT);
            MFVarSetFloat(_MDOutFluxMixing_QxTID, itemID, QxTout_mix);
            MFVarSetFloat(_MDOutStorageMixing_QxTID, itemID, StorexT_new_mix);
            MFVarSetFloat(_MDOutDeltaStorageMixing_QxTID, itemID, DeltaStorexT_mix);
            MFVarSetFloat(_MDOutWTempMixing_QxTID, itemID, Q_WTemp_mix);
   	    } else {
        	if (waterStorage > 0){
                QxTnew = QxT_input + StorexT; //m3*degC
                QxTnew_mix = QxT_input + StorexT_mix;
        	} else Q_WTemp = Q_WTemp_mix  = RO_WTemp; // FBM 2022-06-06
        	StorexT_new      = 0.0; //m3*degC
        	DeltaStorexT     = StorexT_new - StorexT; //
        	QxTout           = 0.0; //m3*degC/dStorexT_new_mix  = 0; //m3*degC
            QxTRemoval       = 0.0; //m3*degC/d
            StorexT_new_mix  = 0.0; //m3*degC
        	DeltaStorexT_mix = StorexT_new_mix - StorexT_mix;
        	QxTout_mix       = 0.0; //m3*degC/s

            MFVarSetFloat(_MDOutLocalIn_QxTID,            itemID, 0.0);
            MFVarSetFloat(_MDOutRemoval_QxTID,            itemID, QxTRemoval);
        	MFVarSetFloat(_MDOutFlux_QxTID,               itemID, QxTout);
        	MFVarSetFloat(_MDOutStorage_QxTID,            itemID, StorexT_new);
        	MFVarSetFloat(_MDOutDeltaStorage_QxTID,       itemID, DeltaStorexT);
            MFVarSetFloat(_MDOutWTempRiverID,             itemID, Tair);        // FBM 2022-06-16 Setting to valid value instead of missing 
            MFVarSetFloat(_MDOutWTempDeltaT_QxTID,        itemID, 0.0);         // FBM 2022-06-16 Setting to valid value instead of missing 
           	MFVarSetFloat(_MDOutFluxMixing_QxTID,         itemID, QxTout_mix);  // FBM 2022-06-16 Setting to valid value instead of missing 
        	MFVarSetFloat(_MDOutStorageMixing_QxTID,      itemID, StorexT_new_mix);
        	MFVarSetFloat(_MDOutDeltaStorageMixing_QxTID, itemID, DeltaStorexT_mix);
        }
  	float mb;
  	float mbmix;
  	mb = QxT_input + QxT - QxTRemoval - QxTout - DeltaStorexT;
  	mbmix = (QxT_input + QxT_mix - QxTout_mix - DeltaStorexT_mix);
    }	//RJS 071511
}

int MDTP2M_WTempRiverDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutWTempRiverID != MFUnset) return (_MDOutWTempRiverID);

	MFDefEntering ("Route river temperature");
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
	if (((_MDInCore_RunoffVolumeID       = MDCore_RunoffVolumeDef ())                  == CMfailed) ||
        ((_MDInWTempRunoffID             = MDTP2M_WTempRunoffDef ())                   == CMfailed) ||
        ((_MDInRouting_DischargeID       = MDRouting_DischargeDef ())                  == CMfailed) ||
        ((_MDInCommon_AirTemperatureID   = MDCommon_AirTemperatureDef ())              == CMfailed) ||
        ((_MDInCommon_HumidityDewPointID = MDCommon_HumidityDewPointTemperatureDef ()) == CMfailed) ||
        ((_MDInCommon_SolarRadID         = MDCommon_SolarRadDef ())                    == CMfailed) ||
        ((_MDInRiverWidthID              = MDRouting_RiverWidthDef ())                 == CMfailed) ||
        ((_MDInResReleaseID              = MDReservoir_ReleaseDef ())                  == CMfailed) ||
        ((optID == MFon ) &&
          (((_MDInResStorageChangeID     = MFVarGetID (MDVarReservoir_StorageChange,     "km3",       MFInput,  MFState, MFBoundary)) == CMfailed) ||
           ((_MDInResStorageID           = MFVarGetID (MDVarReservoir_Storage,           "km3",       MFInput,  MFState, MFInitial))  == CMfailed) ||
           ((_MDInResCapacityID          = MFVarGetID (MDVarReservoir_Capacity,          "km3",       MFInput,  MFState, MFBoundary)) == CMfailed))) ||
        ((_MDInDischargeIncomingID       = MFVarGetID (MDVarRouting_Discharge0,          "m3/s",      MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDInWindSpeedID               = MFVarGetID (MDVarCommon_WindSpeed,            "m/s",       MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInRiverStorageChgID         = MFVarGetID (MDVarRouting_RiverStorageChg,     "m3",        MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDInRiverStorageID            = MFVarGetID (MDVarRouting_RiverStorage,        "m3",        MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDOutLocalIn_QxTID            = MFVarGetID (MDVarTP2M_WTLocalIn_QxT,          "m3*degC/d", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutRemoval_QxTID            = MFVarGetID (MDVarTP2M_Removal_QxT,            "m3*degC/d", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutFlux_QxTID               = MFVarGetID (MDVarTP2M_Flux_QxT,               "m3*degC/d", MFRoute,  MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutStorage_QxTID            = MFVarGetID (MDVarTP2M_Storage_QxT,            "m3*degC",   MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutDeltaStorage_QxTID       = MFVarGetID (MDVarTP2M_DeltaStorage_QxT,       "m3*degC/d", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutWTempDeltaT_QxTID        = MFVarGetID (MDVarTP2M_WTempDeltaT_QxT,        "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutEquil_Temp   	         = MFVarGetID (MDVarTP2M_Equil_Temp,             "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutFluxMixing_QxTID         = MFVarGetID (MDVarTP2M_FluxMixing_QxT,         "m3*degC/d", MFRoute,  MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutStorageMixing_QxTID      = MFVarGetID (MDVarTP2M_StorageMixing_QxT,      "m3*degC",   MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutDeltaStorageMixing_QxTID = MFVarGetID (MDVarTP2M_DeltaStorageMixing_QxT, "m3*degC/d", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutWTempMixing_QxTID        = MFVarGetID (MDVarTP2M_WTempMixing_QxT,        "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTempRiverID             = MFVarGetID (MDVarTP2M_WTempRiver,             "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRiver) == CMfailed)) return (CMfailed);
	   MFDefLeaving ("Route river temperature");
	   return (_MDOutWTempRiverID);
}
