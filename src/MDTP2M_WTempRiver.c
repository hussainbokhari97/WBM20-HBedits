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
static int _MDInCommon_AirTemperatureID        = MFUnset;
static int _MDInCommon_HumidityDewPointID      = MFUnset; // FOR NEW TEMP MODULE
static int _MDInCommon_SolarRadID              = MFUnset;
static int _MDInCommon_WindSpeedID             = MFUnset;
static int _MDInCore_RunoffVolumeID            = MFUnset;
static int _MDInRouting_DischargeID            = MFUnset;
static int _MDInRouting_DischargeIncomingID    = MFUnset;
static int _MDInRouting_RiverWidthID           = MFUnset;
static int _MDInRouting_RiverStorageID         = MFUnset;
static int _MDInRouting_RiverStorageChgID      = MFUnset;
static int _MDInReservoir_ReleaseID     	   = MFUnset;
static int _MDInReservoir_CapacityID     	   = MFUnset;
static int _MDInReservoir_StorageID            = MFUnset;
static int _MDInReservoir_StorageChgID         = MFUnset;
static int _MDInTP2M_WTempRunoffID             = MFUnset;
static int _MDInTP2M_Storage_QxTID             = MFUnset; // Initial
static int _MDInTP2M_Flux_QxTID                = MFUnset; // Route
// Output
static int _MDOutTP2M_LocalIn_QxTID            = MFUnset;
static int _MDOutTP2M_Removal_QxTID            = MFUnset;
static int _MDOutTP2M_DeltaStorage_QxTID       = MFUnset;
static int _MDOutTP2M_WTempDeltaT_QxTID        = MFUnset;
static int _MDOutTP2M_Equil_Temp	           = MFUnset;
static int _MDOutTP2M_WTempRiverID             = MFUnset;


static void _MDWTempRiver (int itemID) {
// Input
    float Tair;         // Air temperature in degC
    float dew_point;    // Dewpoint temperature in degC
 	float solarRad;     // Solar radiation in W/m2
 	float windSpeed;    // Winds speed in m/s
    float RO_Vol;       // RO volume in m3
    float Q;            // Outflowing discharge in m3/s 
   	float Q_incoming;   // Incoming discharge including local runoff in m3/s
   	float channelWidth;
   	float waterStorage;
 	float waterStorageChange;
   	float RO_WTemp;
    float resCapacity;		//RJS 071511	Reservoir capacity [km3]
    float ResWaterStorage       = 0.0;
    float ResWaterStorageChange = 0.0;
// Initial
    float StorexT;
// Routed
    float QxT;
// Output
    float StorexT_new;  
	float QxTout = 0;
	float QxT_input;
    float DeltaStorexT;
    float Q_WTemp_new;
    float deltaT;
// Local
	float QxTnew = 0;
    float Q_WTemp;
    
    // processing variable
    float channelLength;
    float Tequil = 0;
    float HeatLoss_int   = 4396.14; // is intercept assuming no wind and clouds
    float HeatLoss_slope = 1465.38; // is slope assuming no wind and clouds
     
    float ReservoirArea;
    float ReservoirDepth;
    float ReservoirVelocity;

    // conservative mixing variables (parallel to those above_
    float QxTRemoval;
    float dt = MFModelGet_dt ();

    Tair               = MFVarGetFloat (_MDInCommon_AirTemperatureID,     itemID, 0.0);
    dew_point          = MFVarGetFloat (_MDInCommon_HumidityDewPointID,   itemID, 0.0);
 	solarRad           = MFVarGetFloat (_MDInCommon_SolarRadID,           itemID, 0.0);
 	windSpeed          = MFVarGetFloat (_MDInCommon_WindSpeedID,          itemID, 0.0);
    RO_Vol             = MFVarGetFloat (_MDInCore_RunoffVolumeID,         itemID, 0.0);
    Q                  = MFVarGetFloat (_MDInRouting_DischargeID,         itemID, 0.0);
   	Q_incoming         = MFVarGetFloat (_MDInRouting_DischargeIncomingID, itemID, 0.0);
   	channelWidth       = MFVarGetFloat (_MDInRouting_RiverWidthID,        itemID, 0.0);
   	waterStorage       = MFVarGetFloat (_MDInRouting_RiverStorageID,      itemID, 0.0);
 	waterStorageChange = MFVarGetFloat (_MDInRouting_RiverStorageChgID,   itemID, 0.0);
   	RO_WTemp           = MFVarGetFloat (_MDInTP2M_WTempRunoffID,          itemID, 0.0);
    StorexT            = MFVarGetFloat (_MDInTP2M_Storage_QxTID,          itemID, 0.0);
    QxT                = MFVarGetFloat (_MDInTP2M_Flux_QxTID,             itemID, 0.0);
    
    if (itemID == 282180) {
        dt = MFModelGet_dt ();
    }
     if (_MDInReservoir_StorageID != MFUnset) {
         resCapacity           = MFVarGetFloat ( _MDInReservoir_CapacityID,   itemID, 0.0) * 1e9; // convert to m3
         ResWaterStorage       = MFVarGetFloat ( _MDInReservoir_StorageID,    itemID, 0.0) * 1e9; // convert to m3 
         ResWaterStorageChange = MFVarGetFloat ( _MDInReservoir_StorageChgID, itemID, 0.0) * 1e9; // convert to m3/
     } else {
         ResWaterStorageChange =
         ResWaterStorage       = 
         resCapacity           = 0.0; //RJS 071511
    }
    waterStorage = waterStorageChange = 0.0; // TODO FBM disabling water storage
    if(Q          < 0.0) Q = 0.0;          //RJS 120409
    if(Q_incoming < 0.0) Q_incoming = 0.0; //RJS 120409

    if(resCapacity > 0.0) {
    	waterStorage = waterStorage + ResWaterStorage;
    	waterStorageChange = waterStorageChange + ResWaterStorageChange;
    	ReservoirArea = pow(((ResWaterStorage / 1e6) / 9.208),(1 / 1.114)) * 1e6;  // m2, from Takeuchi 1997 - original equation has V in 10^6 m3 and A in km2
    	ReservoirDepth = (ResWaterStorage / ReservoirArea); //m
    	ReservoirVelocity = Q / (ReservoirArea); // m/s
    	channelWidth = MDMaximum(channelWidth, (Q / (ReservoirDepth * ReservoirVelocity))); // m

    	QxT_input  = RO_Vol * RO_WTemp * dt;       //RJS 071511 // m3*degC/d
    	QxTnew     =     QxT + QxT_input + StorexT;     //RJS 071511 // m3*degC/d

    	if (Q_incoming > 0.000001) {
    		Q_WTemp     = QxTnew     / ((Q_incoming) * dt + (waterStorage - waterStorageChange)); 			//RJS 071511					//degC
    	} else {
    		if (waterStorage > 0) {
    			Q_WTemp	    = StorexT / waterStorage;		// RJS 071511	//degC
    		} else
                Q_WTemp 	= RO_WTemp; // FBM 2022-06-06
    	}

    	Q_WTemp_new = Q_WTemp;														//RJS 071511

    	StorexT_new      = waterStorage * Q_WTemp_new; 							//RJS 071511	//m3*degC
    	DeltaStorexT     = StorexT_new - StorexT; 									//RJS 071511
    	QxTout           = Q * dt * Q_WTemp_new ; 							//RJS 071511	//m3*degC/d
    	QxTRemoval       = QxTnew - (StorexT_new + QxTout); 						//RJS 071511	//m3*degC/d

    	MFVarSetFloat(_MDInTP2M_Storage_QxTID,             itemID, StorexT_new);     // Initial
    	MFVarSetFloat(_MDInTP2M_Flux_QxTID,                itemID, QxTout);          // Route
    	MFVarSetFloat(_MDOutTP2M_LocalIn_QxTID,            itemID, QxT_input);
    	MFVarSetFloat(_MDOutTP2M_DeltaStorage_QxTID,       itemID, DeltaStorexT);
    	MFVarSetFloat(_MDOutTP2M_WTempRiverID,             itemID, Q_WTemp_new);
    	MFVarSetFloat(_MDOutTP2M_WTempDeltaT_QxTID,        itemID, deltaT);
    } else {
    	ReservoirArea     = 0.0;
    	ReservoirVelocity = 0.0;
    	ReservoirDepth    = 0.0;

        QxT_input = RO_Vol * RO_WTemp * dt; //m3*degC/d

        if((Q_incoming) > 0.000001) { //do not include water storage in this check - will screw up mixing estimates
            QxTnew      = QxT     + QxT_input + StorexT; //m3*degC/d

            if (Q_incoming * dt + (waterStorage - waterStorageChange) > 0.0) {
                Q_WTemp     = QxTnew     / ((Q_incoming) * MFModelGet_dt () + (waterStorage - waterStorageChange));
            } else
                Q_WTemp = RO_WTemp; //degC

            /// Temperature Processing using Dingman 1972 

            ////////// NEW EQUILIBRIUM TEMP MODEL ///// Edinger et al. 1974: Heat Exchange and Transport in the Environment /////

            float wind_f;
            float Tm;
            float beta;
            float kay;
            float solar_KJ;
            float equil2;
            float initial_riverT;
            float RivTemp;
            int x;

            initial_riverT = Q_WTemp;

            wind_f = 9.2+(0.46*pow(windSpeed,2)); // wind function

            for (x = 0; x < 4; x++) {
	            Tm = (dew_point + initial_riverT) / 2; // mean of rivertemp initial and dew point
	            beta = 0.35 + (0.015 * Tm) + (0.0012 * pow(Tm, 2.0)); //beta
	            kay = (4.5 + (0.05 * initial_riverT) + (beta * wind_f) + (0.47 * wind_f)); // K in W/m2/degC
	            equil2 = dew_point + solarRad / kay; // Solar radiation is in W/m2
	            initial_riverT = equil2;
            }

            channelLength = MFModelGetLength(itemID) * 1000; // converting from km to m
            if ((channelLength > 0.0) && (channelWidth > 0.0)) // FBM channel length can be zero
                 RivTemp = equil2 + (Q_WTemp - equil2) * exp(-kay * channelLength * channelWidth / (4181.3 * Q * dt));
            else RivTemp = initial_riverT;

            /// Resetting outgoing temperature:
            Q_WTemp_new = MDMaximum(RivTemp, 0.0);
            MFVarSetFloat(_MDOutTP2M_Equil_Temp, itemID, equil2);

            deltaT           = Q_WTemp_new - Q_WTemp;
            StorexT_new      = waterStorage * Q_WTemp_new; //m3*degC
            DeltaStorexT     = StorexT_new - StorexT; //
            QxTout           = Q * dt * Q_WTemp_new ; //m3*degC/d
            QxTRemoval       = QxTnew - (StorexT_new + QxTout); //m3*degC/d

            // end

            MFVarSetFloat(_MDInTP2M_Storage_QxTID,             itemID, StorexT_new);     // Initial
            MFVarSetFloat(_MDInTP2M_Flux_QxTID,                itemID, QxTout);          // Route
            MFVarSetFloat(_MDOutTP2M_LocalIn_QxTID,            itemID, QxT_input);
            MFVarSetFloat(_MDOutTP2M_Removal_QxTID,            itemID, QxTRemoval);
            MFVarSetFloat(_MDOutTP2M_DeltaStorage_QxTID,       itemID, DeltaStorexT);
            MFVarSetFloat(_MDOutTP2M_WTempRiverID,             itemID, Q_WTemp_new);
            MFVarSetFloat(_MDOutTP2M_WTempDeltaT_QxTID,        itemID, deltaT);
   	    } else {
        	if (waterStorage > 0){
                QxTnew = QxT_input + StorexT; //m3*degC
        	} else Q_WTemp = RO_WTemp; // FBM 2022-06-06
        	StorexT_new      = 0.0; //m3*degC
        	DeltaStorexT     = StorexT_new - StorexT; //
        	QxTout           = 0.0; //m3*degC/dStorexT_new_mix  = 0; //m3*degC
            QxTRemoval       = 0.0; //m3*degC/d

        	MFVarSetFloat(_MDInTP2M_Storage_QxTID,             itemID, StorexT_new);     // Initial
        	MFVarSetFloat(_MDInTP2M_Flux_QxTID,                itemID, QxTout);          // Route
            MFVarSetFloat(_MDOutTP2M_LocalIn_QxTID,            itemID, 0.0);
            MFVarSetFloat(_MDOutTP2M_Removal_QxTID,            itemID, QxTRemoval);
        	MFVarSetFloat(_MDOutTP2M_DeltaStorage_QxTID,       itemID, DeltaStorexT);
            MFVarSetFloat(_MDOutTP2M_WTempRiverID,             itemID, Tair);        // FBM 2022-06-16 Setting to valid value instead of missing 
            MFVarSetFloat(_MDOutTP2M_WTempDeltaT_QxTID,        itemID, 0.0);         // FBM 2022-06-16 Setting to valid value instead of missing 
        }
  	float mb;
  	mb = QxT_input + QxT - QxTRemoval - QxTout - DeltaStorexT;
    }	//RJS 071511
}

int MDTP2M_WTempRiverDef () {

	if (_MDOutTP2M_WTempRiverID != MFUnset) return (_MDOutTP2M_WTempRiverID);

	MFDefEntering ("Route river temperature");
	if (((_MDInCommon_AirTemperatureID        = MDCommon_AirTemperatureDef ())              == CMfailed) ||
        ((_MDInCommon_HumidityDewPointID      = MDCommon_HumidityDewPointTemperatureDef ()) == CMfailed) ||
        ((_MDInCommon_SolarRadID              = MDCommon_SolarRadDef ())                    == CMfailed) ||
        ((_MDInCore_RunoffVolumeID            = MDCore_RunoffVolumeDef ())                  == CMfailed) ||
        ((_MDInRouting_DischargeID            = MDRouting_DischargeDef ())                  == CMfailed) ||
        ((_MDInTP2M_WTempRunoffID             = MDTP2M_WTempRunoffDef ())                   == CMfailed) ||
        ((_MDInRouting_RiverWidthID           = MDRouting_RiverWidthDef ())                 == CMfailed) ||
        ((_MDInReservoir_ReleaseID            = MDReservoir_ExtractableReleaseDef ())       == CMfailed) ||
        ((_MDInCommon_WindSpeedID             = MFVarGetID (MDVarCommon_WindSpeed,            "m/s",       MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInRouting_DischargeIncomingID    = MFVarGetID (MDVarRouting_Discharge0,          "m3/s",      MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDInRouting_RiverStorageID         = MFVarGetID (MDVarRouting_RiverStorage,        "m3",        MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDInRouting_RiverStorageChgID      = MFVarGetID (MDVarRouting_RiverStorageChg,     "m3",        MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDInReservoir_ReleaseID != MFUnset ) &&
          (((_MDInReservoir_CapacityID        = MFVarGetID (MDVarReservoir_Capacity,          "km3",       MFInput,  MFState, MFBoundary)) == CMfailed) ||
           ((_MDInReservoir_StorageID         = MFVarGetID (MDVarReservoir_Storage,           "km3",       MFInput,  MFState, MFInitial))  == CMfailed) ||
           ((_MDInReservoir_StorageChgID      = MFVarGetID (MDVarReservoir_StorageChange,     "km3",       MFInput,  MFState, MFBoundary)) == CMfailed))) ||
        ((_MDInTP2M_Storage_QxTID             = MFVarGetID (MDVarTP2M_Storage_QxT,            "m3*degC",   MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDInTP2M_Flux_QxTID                = MFVarGetID (MDVarTP2M_Flux_QxT,               "m3*degC/d", MFRoute,  MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutTP2M_LocalIn_QxTID            = MFVarGetID (MDVarTP2M_WTLocalIn_QxT,          "m3*degC/d", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutTP2M_Removal_QxTID            = MFVarGetID (MDVarTP2M_Removal_QxT,            "m3*degC/d", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutTP2M_DeltaStorage_QxTID       = MFVarGetID (MDVarTP2M_DeltaStorage_QxT,       "m3*degC/d", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutTP2M_WTempDeltaT_QxTID        = MFVarGetID (MDVarTP2M_WTempDeltaT_QxT,        "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutTP2M_Equil_Temp   	          = MFVarGetID (MDVarTP2M_Equil_Temp,             "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutTP2M_WTempRiverID             = MFVarGetID (MDVarTP2M_WTempRiver,             "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRiver) == CMfailed)) return (CMfailed);
	   MFDefLeaving ("Route river temperature");
	   return (_MDOutTP2M_WTempRiverID);
}
