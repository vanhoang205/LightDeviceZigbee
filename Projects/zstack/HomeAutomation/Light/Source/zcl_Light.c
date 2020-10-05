/**************************************************************************************************
  Filename:       zcl_sampleLight.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $


  Description:    Zigbee Cluster Library - sample light application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application implements a ZigBee Light, based on Z-Stack 3.0. It can be configured as an
  On/Off light or as a dimmable light, by undefining or defining ZCL_LEVEL_CTRL, respectively.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample applicetion.
  
  Application-specific UI peripherals being used:

  - LEDs:
    LED1 reflect the current light state (On / Off accordingly).

  Application-specific menu system:

    <TOGGLE LIGHT> Toggle the local light and display its status and level
      Press OK to toggle the local light on and off.
      This screen shows the following information
        Line1: (only populated if ZCL_LEVEL_CTRL is defined)
          LEVEL XXX - xxx is the current level of the light if the light state is ON, or the target level
            of the light when the light state is off. The target level is the level that the light will be
            set to when it is switched from off to on using the on or the toggle commands.
        Line2:
          LIGHT OFF / ON: shows the current state of the light.
      Note when ZCL_LEVEL_CTRL is enabled:
        - If the light state is ON and the light level is X, and then the light receives the OFF or TOGGLE 
          commands: The level will decrease gradually until it reaches 1, and only then the light state will
          be changed to OFF. The level then will be restored to X, with the state staying OFF. At this stage
          the light is not lighting, and the level represent the target level for the next ON or TOGGLE 
          commands.
        - If the light state is OFF and the light level is X, and then the light receives the ON or TOGGLE
          commands; The level will be set to 1, the light state will be set to ON, and then the level will
          increase gradually until it reaches level X.
        - Any level-setting command will affect the level directly, and may also affect the on/off state,
          depending on the command's arguments.       

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"

#include "nwk_util.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "zcl_Light.h"
   
#include "bdb.h"
#include "bdb_interface.h"

   
#include "onboard.h"

/* HAL */
#include "hal_led.h"
#include "hal_key.h"

#include "NLMEDE.h"

/*********************************************************************
 * MACROS
 */
#define HAL_RELAY_1     HAL_LED_5       // NOTE !! : P0_0 using for LCD debug default, undefine macro: xLCD_SUPPORTED=DEBUG
#define HAL_RELAY_2     HAL_LED_4

#define zcl_AccessCtrlRead( a )       ( (a) & ACCESS_CONTROL_READ )

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclLight_TaskID;
afAddrType_t zclLight_DstAddr;
extern int16 zdpExternalStateTaskID;
static devStates_t NwkStateShadow = DEV_HOLD;

uint16 gTimeCounter = 0;
uint8 holdKeyCounter;

uint8 check;
/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static endPointDesc_t light_Ep_1 =
{
  LIGHT_ENDPOINT_1,
  0,
  &zclLight_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

static endPointDesc_t light_Ep_2 =
{
  LIGHT_ENDPOINT_2,
  0,
  &zclLight_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

// Helper function to update LED
void zclLight_UpdateLedState_1(void);
void zclLight_UpdateLedState_2(void);

// Functions to process other events relate to hardware
static void zclLight_HandleKeys( byte shift, byte keys );
static void zclLight_BasicResetCB_1( void );
static void zclLight_BasicResetCB_2( void );
static void zclLight_OnOffCB_1( uint8 cmd );
static void zclLight_OnOffCB_2( uint8 cmd );

// Function to process Commisioning
static void zclLight_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);

// Functions to process ZCL Foundation incoming Command/Response messages
static void zclLight_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 zclLight_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 zclLight_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static uint8 zclLight_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );


#ifdef ZCL_REPORT
static uint8 zclLight_ProcessInConfigReportCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclLight_ProcessInReadReportCfgCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclLight_ProcessInReportCmd( zclIncomingMsg_t *pInMsg, uint8 endpoint );
static void zclLight_CheckReportConfig( uint8 endpoint );
static uint8 sendZclAttrReport(uint8 srcEp, uint16 clusterID, zclReportCmd_t *pReportCmd, uint8 dataLen);
static void zclLight_CheckandSendClusterAttrReport( uint16 clusterID, zclConfigReportRecsList *pConfigReportRecsList, uint8 endpoint );
static void sendZclAttrChangeReport(uint8 srcEp, uint16 clusterId, uint16 attrID, uint8 *lastReportValue, uint8 *currentValue);
#endif

#ifdef ZCL_GROUPS
static void zclLight_GroupCB_1( zclGroupRsp_t *pRsp );
static void zclLight_GroupCB_2( zclGroupRsp_t *pRsp );
#endif

#ifdef ZCL_IDENTIFY
static void zclLight_ProcessIdentifyTimeChange( uint8 endpoint );
#endif
/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * STATUS STRINGS
 */


/*********************************************************************
 * REFERENCED EXTERNALS
 */

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclLight_CmdCallbacks_1 =
{
  zclLight_BasicResetCB_1,               // Basic Cluster Reset command
  NULL,                                  // Identify Trigger Effect command
  zclLight_OnOffCB_1,                    // On/Off cluster commands
  NULL,                                  // On/Off cluster enhanced command Off with Effect
  NULL,                                  // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                  // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                  // Level Control Move to Level command
  NULL,                                  // Level Control Move command
  NULL,                                  // Level Control Step command
  NULL,                                  // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  zclLight_GroupCB_1,                    // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                  // Scene Store Request command
  NULL,                                  // Scene Recall Request command
  NULL,                                  // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                  // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                  // Get Event Log command
  NULL,                                  // Publish Event Log command
#endif
  NULL,                                  // RSSI Location command
  NULL                                   // RSSI Location Response command
};

static zclGeneral_AppCallbacks_t zclLight_CmdCallbacks_2 =
{
  zclLight_BasicResetCB_2,               // Basic Cluster Reset command
  NULL,                                  // Identify Trigger Effect command
  zclLight_OnOffCB_2,                    // On/Off cluster commands
  NULL,                                  // On/Off cluster enhanced command Off with Effect
  NULL,                                  // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                  // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                  // Level Control Move to Level command
  NULL,                                  // Level Control Move command
  NULL,                                  // Level Control Step command
  NULL,                                  // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  zclLight_GroupCB_2,                    // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                  // Scene Store Request command
  NULL,                                  // Scene Recall Request command
  NULL,                                  // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                  // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                  // Get Event Log command
  NULL,                                  // Publish Event Log command
#endif
  NULL,                                  // RSSI Location command
  NULL                                   // RSSI Location Response command
};
/*********************************************************************
 * @fn          zclLight_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclLight_Init( byte task_id )
{
  zclLight_TaskID = task_id;

  // Register the Simple Descriptor for this application
  bdb_RegisterSimpleDescriptor( &zclLight_SimpleDesc_1 );
  bdb_RegisterSimpleDescriptor( &zclLight_SimpleDesc_2 );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( LIGHT_ENDPOINT_1, &zclLight_CmdCallbacks_1 );
  zclGeneral_RegisterCmdCallbacks( LIGHT_ENDPOINT_2, &zclLight_CmdCallbacks_2 );

  // Register the application's attribute list
  zclLight_ResetAttributesToDefaultValues_1();
  zclLight_ResetAttributesToDefaultValues_2();
  zcl_registerAttrList( LIGHT_ENDPOINT_1, zclLight_NumAttributes_1, zclLight_Attrs_1 );
  zcl_registerAttrList( LIGHT_ENDPOINT_2, zclLight_NumAttributes_2, zclLight_Attrs_2 );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclLight_TaskID );

#ifdef ZCL_REPORT
  zcl_registerConfigReportRecList( LIGHT_ENDPOINT_1, zclLight_NumConfigReportRecs_1,
                                   zclLight_ConfigReportRecs_Rpt1);
  
  zcl_registerConfigReportRecList( LIGHT_ENDPOINT_2, zclLight_NumConfigReportRecs_2,
                                   zclLight_ConfigReportRecs_Rpt2);
#endif

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zclLight_TaskID );
  bdb_RegisterIdentifyTimeChangeCB( zclLight_ProcessIdentifyTimeChange );
  bdb_RegisterCommissioningStatusCB( zclLight_ProcessCommissioningStatus );
  
  // Register for a test endpoint
  afRegister( &light_Ep_1 );
  afRegister( &light_Ep_2 );
  
  bdb_StartCommissioning( BDB_COMMISSIONING_MODE_NWK_STEERING);
  zdpExternalStateTaskID = zclLight_TaskID;
  
}

/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 zclLight_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclLight_TaskID )) )
    {
      check = MSGpkt->hdr.event;
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          zclLight_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          zclLight_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
           NwkStateShadow = (devStates_t)(MSGpkt->hdr.status);
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

#if ZG_BUILD_ENDDEVICE_TYPE    
  if ( events & LIGHT_END_DEVICE_REJOIN_EVT )
  {
    bdb_ZedAttemptRecoverNwk();
    return ( events ^ LIGHT_END_DEVICE_REJOIN_EVT );
  }
#endif

  //-- MOD START
#ifdef ZCL_REPORT
  if ( events & LIGHT_CHECK_REPORT_EVT_1 )
  {
    zclLight_CheckReportConfig(LIGHT_ENDPOINT_1);
    return ( events ^ LIGHT_CHECK_REPORT_EVT_1 );
  }
  
  if ( events & LIGHT_CHECK_REPORT_EVT_2 )
  {
    zclLight_CheckReportConfig(LIGHT_ENDPOINT_2);
    return ( events ^ LIGHT_CHECK_REPORT_EVT_2 );
  }
#endif
  
  if ( events & LIGHT_CHECK_HOLD_KEY_EVT)
  {
    if ( HalKeyRead() & HAL_KEY_SW_2 )
    {
      holdKeyCounter++;
      osal_start_timerEx( zclLight_TaskID, LIGHT_CHECK_HOLD_KEY_EVT, 1000 );
    }
    else {
      if ( holdKeyCounter >= 5 )
      {
        if(NwkStateShadow == DEV_END_DEVICE)
        {
          NLME_LeaveReq_t leaveReq;        
          leaveReq.extAddr = NULL;
          leaveReq.removeChildren = FALSE;
          leaveReq.rejoin = FALSE;
          leaveReq.silent = FALSE;
          if ( NLME_LeaveReq( &leaveReq ) == ZSuccess )
          {
            HalLedBlink(HAL_LED_1, 12, 50, 100);
          }
        }
        else 
        {
        zgWriteStartupOptions( ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE | ZCD_STARTOPT_DEFAULT_CONFIG_STATE );
        SystemResetSoft();
        }
      }
      holdKeyCounter = 0;
    }
   return ( events ^ LIGHT_CHECK_HOLD_KEY_EVT );   
  }
  
  
  // Discard unknown events
  return 0;
}


/*********************************************************************
 * @fn      zclLight_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_5
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void zclLight_HandleKeys( byte shift, byte keys )
{
  if ( keys & HAL_KEY_SW_2 )  
  {
     osal_set_event( zclLight_TaskID, LIGHT_CHECK_HOLD_KEY_EVT);
  }
}


/*********************************************************************
 * @fn      zclLight_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclLight_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  {
    case BDB_COMMISSIONING_FORMATION:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //After formation, perform nwk steering again plus the remaining commissioning modes that has not been process yet
        bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING | bdbCommissioningModeMsg->bdbRemainingCommissioningModes);
      }
      else
      {
        //Want to try other channels?
        //try with bdb_setChannelAttribute
      }
    break;
    case BDB_COMMISSIONING_NWK_STEERING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
        //We are on the nwk, what now?
        HalLedBlink(HAL_LED_1, 6, 50, 200);
        
      }
      else
      {
        bdb_StartCommissioning( BDB_COMMISSIONING_MODE_NWK_STEERING);
        //See the possible errors for nwk steering procedure
        //No suitable networks found
        //Want to try other channels?
        //try with bdb_setChannelAttribute
      }
    break;
    case BDB_COMMISSIONING_FINDING_BINDING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
      }
      else
      {
        //YOUR JOB:
        //retry?, wait for user interaction?
      }
    break;
    case BDB_COMMISSIONING_INITIALIZATION:
      //Initialization notification can only be successful. Failure on initialization 
      //only happens for ZED and is notified as BDB_COMMISSIONING_PARENT_LOST notification
      
      //YOUR JOB:
      //We are on a network, what now?
      
    break;
#if ZG_BUILD_ENDDEVICE_TYPE    
    case BDB_COMMISSIONING_PARENT_LOST:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
      {
        //We did recover from losing parent
      }
      else
      {
        //Parent not found, attempt to rejoin again after a fixed delay
        osal_start_timerEx(zclLight_TaskID, LIGHT_END_DEVICE_REJOIN_EVT, SAMPLEAPP_END_DEVICE_REJOIN_DELAY);
      }
    break;
#endif 
  }
  
}

/*********************************************************************
 * @fn      zclLight_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclLight_BasicResetCB_1( void )
{
  //Reset every attribute in all supported cluster to their default value.

  zclLight_ResetAttributesToDefaultValues_1();

  zclLight_UpdateLedState_1();
  zclLight_UpdateLedState_2();

}

static void zclLight_BasicResetCB_2( void )
{
  //Reset every attribute in all supported cluster to their default value.

  zclLight_ResetAttributesToDefaultValues_2();

  zclLight_UpdateLedState_1();
  zclLight_UpdateLedState_2();

}

/*********************************************************************
 * @fn      zclLight_OnOffCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an On/Off Command for this application.
 *
 * @param   cmd - COMMAND_ON, COMMAND_OFF or COMMAND_TOGGLE
 *
 * @return  none
 */
static void zclLight_OnOffCB_1( uint8 cmd )
{
  afIncomingMSGPacket_t *pPtr = zcl_getRawAFMsg();

  uint8 OnOff;


  if ( pPtr->srcAddr.addr.shortAddr == 0x0000 )
  {
        // Turn on the light
    if ( cmd == COMMAND_ON )
    {
      OnOff = LIGHT_ON;
    }
    // Turn off the light
    else if ( cmd == COMMAND_OFF )
    {
      OnOff = LIGHT_OFF;
    }
    // Toggle the light
    else if ( cmd == COMMAND_TOGGLE )
    {
      if (zclLight_OnOff_1 == LIGHT_ON)
      {
        OnOff = LIGHT_OFF;
      }
      else
      {
        OnOff = LIGHT_ON;
      }
    }
    zclLight_OnOff_1 = OnOff;
    zclLight_UpdateLedState_1();
  }
}

static void zclLight_OnOffCB_2( uint8 cmd )
{
  afIncomingMSGPacket_t *pPtr = zcl_getRawAFMsg();

  uint8 OnOff;

  if ( pPtr->srcAddr.addr.shortAddr == 0x0000 )
  {
    // Turn on the light
    if ( cmd == COMMAND_ON )
    {
      OnOff = LIGHT_ON;
    }
    // Turn off the light
    else if ( cmd == COMMAND_OFF )
    {
      OnOff = LIGHT_OFF;
    }
    // Toggle the light
    else if ( cmd == COMMAND_TOGGLE )
    {
      if (zclLight_OnOff_2 == LIGHT_ON)
      {
        OnOff = LIGHT_OFF;
      }
      else
      {
        OnOff = LIGHT_ON;
      }
    }
    zclLight_OnOff_2 = OnOff;
    zclLight_UpdateLedState_2();
  }
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclLight_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zclLight_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclLight_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclLight_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT
    case ZCL_CMD_CONFIG_REPORT:
      zclLight_ProcessInConfigReportCmd( pInMsg );
      break;
      
    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclMultiSensor_ProcessInConfigReportRspCmd( pInMsg );
      break;
      
    case ZCL_CMD_READ_REPORT_CFG:
      zclLight_ProcessInReadReportCfgCmd( pInMsg );
      break;
      
    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclMultiSensor_ProcessInReadReportCfgRspCmd( pInMsg );
      break;
      
    case ZCL_CMD_REPORT:
      //bdb_ProcessIncomingReportingMsg( pInMsg );
      zclLight_ProcessInReportCmd( pInMsg, pInMsg->endPoint );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclLight_ProcessInDefaultRspCmd( pInMsg );
      break;
      
    default:
      break;
  }

  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclLight_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclLight_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successfull request, the value of the requested
    // attribute
  }

  return ( TRUE );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclLight_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclLight_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < writeRspCmd->numAttr; i++ )
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return ( TRUE );
}
#endif // ZCL_WRITE

/*********************************************************************
 * @fn      zclLight_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclLight_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}

void zclLight_UpdateLedState_1(void)
{
  // set the RELAY1 based on light (on or off)
  if ( zclLight_OnOff_1 == LIGHT_ON )
  {
    HalLedSet ( HAL_RELAY_1, HAL_LED_MODE_ON );
  }
  else
  {
    HalLedSet ( HAL_RELAY_1, HAL_LED_MODE_OFF );
  }
}

void zclLight_UpdateLedState_2(void)
{
      // set the RELAY2 based on light (on or off)
  if ( zclLight_OnOff_2 == LIGHT_ON )
  {
    HalLedSet ( HAL_RELAY_2, HAL_LED_MODE_ON );
  }
  else
  {
    HalLedSet ( HAL_RELAY_2, HAL_LED_MODE_OFF );
  }
}

#ifdef ZCL_REPORT
/*********************************************************************
 * @fn      zclSampleLight_ProcessInConfigReportCmd
 *
 * @brief   Process the "Profile" Config Report Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclLight_ProcessInConfigReportCmd( zclIncomingMsg_t *pInMsg )
{
  zclCfgReportCmd_t *pCfgReportCmd;          // numAttr, attrList[]: (zclCfgReportRec_t)    direction, attrID, dataType, minReportInt,
                                             //                                             maxReportInt, timeoutPeriod, reportableChange
  zclCfgReportRspCmd_t *pCfgReportRspCmd;    // numAttr, attrList[]: (zclCfgReportStatus_t) status, direction, attrID
  uint8 sendRsp = FALSE;
  uint16 len;
  uint8 j = 0;
  uint8 i;

  pCfgReportCmd = (zclCfgReportCmd_t *)pInMsg->attrCmd;
  
  if ( pInMsg->zclHdr.commandID == ZCL_CMD_CONFIG_REPORT ) 
  {
    // We need to send a response back - allocate space for it
    len = sizeof( zclCfgReportRspCmd_t ) + (pCfgReportCmd->numAttr * sizeof( zclCfgReportStatus_t ));
    pCfgReportRspCmd = (zclCfgReportRspCmd_t *)zcl_mem_alloc( len );
    
    if ( pCfgReportRspCmd == NULL )
    {
      return FALSE;     // EMBEDDED RETURN
    }

    sendRsp = TRUE;     // sendRsp is active when we got correct commandID
  }

  for ( i = 0; i < pCfgReportCmd->numAttr; i++ )
  {
    zclConfigReportRec_t *pConfigReportRec = NULL;    // find the rec and store here
    zclAttrRec_t attrRec;

    zclCfgReportStatus_t *statusRec = &(pCfgReportRspCmd->attrList[i]);
    zcl_memset( statusRec, 0, sizeof( zclCfgReportStatus_t ) );
         
    if ( zclFindConfigReportRec( pInMsg->endPoint, pInMsg->clusterId,
                         pCfgReportCmd->attrList[i].attrID, &pConfigReportRec ) )
    {
        uint8 status = ZCL_STATUS_SUCCESS;

        if ( pCfgReportCmd->attrList[i].dataType != pConfigReportRec->cfgReportRec.dataType ) {   // if dataType is different
            status = ZCL_STATUS_INVALID_DATA_TYPE;
        }
        else
        {
            if ( zclFindAttrRec( pInMsg->endPoint, pInMsg->clusterId,
                                 pCfgReportCmd->attrList[i].attrID, &attrRec ) )
            {
                if ( pCfgReportCmd->attrList[i].dataType != attrRec.attr.dataType ) {   // if dataType is different
                    status = ZCL_STATUS_INVALID_DATA_TYPE;
                }
                else
                {
                    if ( !zcl_AccessCtrlRead( attrRec.attr.accessControl ) )
                    {
                      status = ZCL_STATUS_WRITE_ONLY;
                    }
                }
            }
        }
        // If successful, store the record, and a CfgReportStatus record shall NOT be generated
        if ( sendRsp && status != ZCL_STATUS_SUCCESS )
        {
            // Attribute is write only or invalid data type - move on to the next record
            statusRec->status = status;
            statusRec->direction = pCfgReportCmd->attrList[i].direction;
            statusRec->attrID = pCfgReportCmd->attrList[i].attrID;
            j++;
        } else { // success, set the config report rec
            pConfigReportRec->cfgReportRec.direction = pCfgReportCmd->attrList[i].direction;
            pConfigReportRec->cfgReportRec.minReportInt = pCfgReportCmd->attrList[i].minReportInt;
            pConfigReportRec->cfgReportRec.maxReportInt = pCfgReportCmd->attrList[i].maxReportInt;
            pConfigReportRec->cfgReportRec.timeoutPeriod = pCfgReportCmd->attrList[i].timeoutPeriod;
            pConfigReportRec->timeup = 0xFFFF;
        }
    }
    else
    {
      // Attribute is not supported - move on to the next configReportRec record
      if ( sendRsp )
      {
        statusRec->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        statusRec->direction = pCfgReportCmd->attrList[i].direction;
        statusRec->attrID = pCfgReportCmd->attrList[i].attrID;
        j++;
      }
    }
  } // for loop

  if ( sendRsp )
  {
    pCfgReportRspCmd->numAttr = j;
    if ( pCfgReportRspCmd->numAttr == 0 )
    {
      // Since all records were written successful, include a single status record
      // in the response command with the status field set to SUCCESS and the
      // attribute ID and direction fields omitted.
      pCfgReportRspCmd->attrList[0].status = ZCL_STATUS_SUCCESS;
      pCfgReportRspCmd->numAttr = 1;
    }

    zcl_SendConfigReportRspCmd( pInMsg->endPoint, &(pInMsg->srcAddr),
                    pInMsg->clusterId, pCfgReportRspCmd,
                    !pInMsg->zclHdr.fc.direction, true, pInMsg->zclHdr.transSeqNum );
                    
    zcl_mem_free( pCfgReportRspCmd );
  }

  // When configured, check report config immediately
  zclLight_CheckReportConfig(pInMsg->endPoint);
  // The SAMPLELIGHT_CHECK_REPORT_EVT will then be triggered again and again
  // If we never received the ConfigReportCmd, the SAMPLELIGHT_CHECK_REPORT_EVT has
  // no change to be triggered.

  // We think this makes sense, since there is no reason for your app to perform 
  // constantly report unless the app is configured to report.
  // If your app just need to automatically report after bootup, you can trigger 
  // SAMPLELIGHT_CHECK_REPORT_EVT in zclSampleLight_Init().

  return TRUE;
}


/*********************************************************************
 * @fn      zclSampleLight_ProcessInReadReportCfgCmd
 *
 * @brief   Process the "Profile" Config Report Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclLight_ProcessInReadReportCfgCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadReportCfgCmd_t *pReadReportCfgCmd;         // numAttr, attrList[]: (zclReadReportCfgRec_t) direction, attrID
  zclReadReportCfgRspCmd_t *pReadReportCfgRspCmd;   // numAttr, attrList[]: (zclReportCfgRspRec_t) status, direction, attrID, dataType, minReportInt, maxReportInt, timeoutPeriod, *reportableChange

  uint8 sendRsp = FALSE;
  uint16 len;
  uint8 i;

  pReadReportCfgCmd = (zclReadReportCfgCmd_t *)pInMsg->attrCmd;
  
  if ( pInMsg->zclHdr.commandID == ZCL_CMD_READ_REPORT_CFG ) 
  {
    // We need to send a response back - allocate space for it
    len = sizeof( zclReadReportCfgRspCmd_t ) + (pReadReportCfgCmd->numAttr * sizeof( zclReportCfgRspRec_t ));
    pReadReportCfgRspCmd = (zclReadReportCfgRspCmd_t *)zcl_mem_alloc( len );
    
    if ( pReadReportCfgRspCmd == NULL )
    {
      return FALSE;     // EMBEDDED RETURN
    }

    sendRsp = TRUE;     // sendRsp is active when we got correct commandID
  }

  for ( i = 0; i < pReadReportCfgCmd->numAttr; i++ )
  {
    zclConfigReportRec_t *pConfigReportRec = NULL;    // find the rec and store here
    zclReportCfgRspRec_t *pReportCfgRspRec = &(pReadReportCfgRspCmd->attrList[i]);
    zclAttrRec_t attrRec;
    
    zcl_memset( pReportCfgRspRec, 0, sizeof( zclReportCfgRspRec_t ) );
         
    if ( zclFindConfigReportRec( pInMsg->endPoint, pInMsg->clusterId,
                                 pReadReportCfgCmd->attrList[i].attrID, &pConfigReportRec ) )
    {   // if found configReportRec
        if ( sendRsp )
        {           
            pReportCfgRspRec->status = ZCL_STATUS_SUCCESS;
            pReportCfgRspRec->direction = pConfigReportRec->cfgReportRec.direction;
            pReportCfgRspRec->attrID = pConfigReportRec->cfgReportRec.attrID;
            pReportCfgRspRec->dataType = pConfigReportRec->cfgReportRec.dataType;
            pReportCfgRspRec->minReportInt = pConfigReportRec->cfgReportRec.minReportInt;
            pReportCfgRspRec->maxReportInt = pConfigReportRec->cfgReportRec.maxReportInt;
            pReportCfgRspRec->timeoutPeriod = pConfigReportRec->cfgReportRec.timeoutPeriod;
            pReportCfgRspRec->reportableChange = pConfigReportRec->cfgReportRec.reportableChange;
        }
    }
    else
    {
      // if not found configReportRec, check if the attribute is an un-reportable or an un-supported one
      uint8 status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
      
      if ( zclFindAttrRec( pInMsg->endPoint, pInMsg->clusterId, pReadReportCfgCmd->attrList[i].attrID, &attrRec ) )
      {
        // if found the attr rec, it is there but un-reportable
        status = ZCL_STATUS_UNREPORTABLE_ATTRIBUTE;
      }
      // Attribute is not supported - move on to the next configReportRec record
      if ( sendRsp )
      {
        pReportCfgRspRec->status = status;
        pReportCfgRspRec->direction = pReadReportCfgCmd->attrList[i].direction;
        pReportCfgRspRec->attrID =  pReadReportCfgCmd->attrList[i].attrID;
      }
    }
  } // for loop

  if ( sendRsp )
  {
    pReadReportCfgRspCmd->numAttr = pReadReportCfgCmd->numAttr;

    zcl_SendReadReportCfgRspCmd( pInMsg->endPoint, &(pInMsg->srcAddr), pInMsg->clusterId,
                                 pReadReportCfgRspCmd, !pInMsg->zclHdr.fc.direction, true, pInMsg->zclHdr.transSeqNum );           
    zcl_mem_free( pReadReportCfgRspCmd );
  }

  return TRUE;
}

/*********************************************************************
 * @fn      zclSampleLight_ProcessInReportCmd
 *
 * @brief   ZCL_CMD_REPORT event handler
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static uint8 zclLight_ProcessInReportCmd( zclIncomingMsg_t *pInMsg, uint8 endpoint )
{
  zclReportCmd_t *pReportCmd;                       // numAttr, attrList[]: (zclReport_t) attrID, dataType, *attrData
  pReportCmd = (zclReportCmd_t *)pInMsg->attrCmd;   // *pReportCmd will be free by handler: zclSampleLight_ProcessIncomingMsg()
  afAddrType_t dstAddr;
  

  dstAddr.endPoint = 0;
  dstAddr.addr.shortAddr = 0;
  dstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zcl_SendReportCmd( endpoint, &dstAddr, pInMsg->clusterId, pReportCmd, ZCL_REPORT_SEND, true, NULL);

  // you can send additional reportable attributes as needed

  /** zcl_SendReportCmd ( SAMPLELIGHT_ENDPOINT, &dstAddr, ...); **/

  // i.e. for Thermostat Cluster that reports LocalTemperature, PICoolingDemand, and PIHeatingDemand.
  // see ZIGBEE CLUSTER LIBRARY SPECIFICATION: 6.3.2.5 Attribute Reporting of the Thermostat Cluster

  return ( TRUE );
}
#endif // ZCL_REPORT

//-- MOD START
#ifdef ZCL_REPORT
/*********************************************************************
 * @fn      zclSampleLight_CheckReportConfig
 *
 * @brief   Check if there is a reportable attribute in all clusters is timeout to report
 *
 * @param   none
 *
 * @return  none
 */
static void zclLight_CheckReportConfig( uint8 endpoint )
{
  uint8 x, y; 
  uint8 stopChecking = TRUE;
  zclConfigReportRecsList *pConfigReportRecsList = zclFindConfigReportRecsList(endpoint); 
  
  if ( pConfigReportRecsList != NULL )
  {
    for ( x = 0; x < pConfigReportRecsList->numConfigReportRec; x++ )
    {
        uint8 cIdDuplicate = 0;

        for ( y = 0; y < x; y++ )
        {
            if ( pConfigReportRecsList->configReportRecs[x].clusterId == pConfigReportRecsList->configReportRecs[y].clusterId )
            {
                cIdDuplicate = 1;
            }
        }
        
        if (!cIdDuplicate)
        {
            zclLight_CheckandSendClusterAttrReport( pConfigReportRecsList->configReportRecs[x].clusterId, pConfigReportRecsList, endpoint );
        }

        if (pConfigReportRecsList->configReportRecs[x].cfgReportRec.maxReportInt != 0xFFFF) {
            stopChecking = FALSE;   // If there is any attribute setting to report, don't stop checking
        }
    }
  }

  gTimeCounter++;   // time ticks every second for checking attr report
  if (!stopChecking) {
    if ( endpoint == LIGHT_ENDPOINT_1 )
    {
      osal_start_timerEx( zclLight_TaskID, LIGHT_CHECK_REPORT_EVT_1, 1000 );
    }
    else if ( endpoint == LIGHT_ENDPOINT_2 )
    {
      osal_start_timerEx( zclLight_TaskID, LIGHT_CHECK_REPORT_EVT_2, 1000 );
    }
  }
 // osal_start_timerEx( zclSampleLight_TaskID, TURN_OFF_LIGHT, 500 );
}

/*********************************************************************
 * @fn      zclSampleLight_CheckandSendClusterAttrReport
 *
 * @brief   Check if there is a reportable attribute in a cluster is timeout to report
 *
 * @param   clusterID - which cluster
 *          pConfigReportRecsList - zclConfigReportRecsList of an endpoint
 *
 * @return  none
 */
static void zclLight_CheckandSendClusterAttrReport( uint16 clusterID, zclConfigReportRecsList *pConfigReportRecsList, uint8 endpoint )
{
    uint8 numAttr = 0;
    uint8 x;
    uint16 len;
    zclReportCmd_t *pReportCmd;
    zclConfigReportRec_t *pConfigReportRec = NULL;
    zclAttrRec_t attrRec;
    
    for ( x = 0; x < pConfigReportRecsList->numConfigReportRec; x++ )
    {
        pConfigReportRec = &(pConfigReportRecsList->configReportRecs[x]);
        
        if ( pConfigReportRec->clusterId == clusterID && pConfigReportRec->cfgReportRec.maxReportInt != 0xFFFF) {
            if (pConfigReportRec->timeup == 0xFFFF || pConfigReportRec->timeup == gTimeCounter) {
                numAttr++;
            }
        }
    }
    
    if (numAttr != 0) {
        // We need to send a report - allocate space for it
        len = sizeof( zclReportCmd_t ) + (numAttr * sizeof( zclReport_t ));
        pReportCmd = (zclReportCmd_t *)zcl_mem_alloc( len );
        pReportCmd->numAttr = numAttr;
    }
    
    numAttr = 0;
    
    for ( x = 0; x < pConfigReportRecsList->numConfigReportRec; x++ )
    {   
        zclReport_t *reportRec;
        pConfigReportRec = &(pConfigReportRecsList->configReportRecs[x]);
        
        if ( pConfigReportRec->clusterId == clusterID && pConfigReportRec->cfgReportRec.maxReportInt != 0xFFFF)  // need report
        {   
            if (pConfigReportRec->timeup == 0xFFFF || pConfigReportRec->timeup == gTimeCounter) // timeup to report
            {
               // zclSampleLight_OnOffCB(COMMAND_ON);
                // fill the record in *pReportCmd
                reportRec = &(pReportCmd->attrList[numAttr]);
                zcl_memset( reportRec, 0, sizeof( zclReport_t ) );
                numAttr++;
                zclFindAttrRec( endpoint, pConfigReportRec->clusterId, pConfigReportRec->cfgReportRec.attrID, &attrRec);
                reportRec->attrID = attrRec.attr.attrId;
                reportRec->dataType = attrRec.attr.dataType;
                reportRec->attrData = attrRec.attr.dataPtr;
                
                if (pConfigReportRec->cfgReportRec.minReportInt == 0) {
                    pConfigReportRec->timeup = gTimeCounter + pConfigReportRec->cfgReportRec.maxReportInt; 
                } else {
                    pConfigReportRec->timeup = gTimeCounter + pConfigReportRec->cfgReportRec.minReportInt; 
                }
            }
        }
    }

  if (numAttr != 0) {
      // send report
      sendZclAttrReport( endpoint, clusterID, pReportCmd, len);
  }    
}
#endif
//-- MOD END

/*********************************************************************
 * @fn      sendZclAttrReport
 *
 * @brief   Send the attr report. Let ZCL_CMD_REPORT event handler handle this.
 *
 * @param   srcEp - source endpoint
 *          clusterID - cluster id
 *          pReportCmd - pointer to the report command packet
 *          dataLen - data length of the report command
 *
 * @return  none
 */
 
static uint8 sendZclAttrReport(uint8 srcEp, uint16 clusterID, zclReportCmd_t *pReportCmd, uint8 dataLen)
{
  zclIncomingMsg_t *pMsg; // this is for the inner-app osal msg, not OTA msg, thus some fields are not important

  // pMsg will be released by zclSampleLight_event_loop()
  pMsg = (zclIncomingMsg_t *)osal_msg_allocate( sizeof(zclIncomingMsg_t) + (dataLen));

  if ( pMsg == NULL )
  {
    return FALSE; // EMBEDDED RETURN
  }
  
  if (pMsg)
  {
      pMsg->hdr.event = ZCL_INCOMING_MSG;
      pMsg->hdr.status = 0;
      //pMsg->zclHdr.fc = NULL;         // not important
      pMsg->zclHdr.manuCode = 0;        // not important
      pMsg->zclHdr.transSeqNum = 0;     // not important
      pMsg->zclHdr.commandID = ZCL_CMD_REPORT;
      pMsg->clusterId = clusterID;
      pMsg->srcAddr.addrMode = (afAddrMode_t)Addr16Bit;
      pMsg->srcAddr.addr.shortAddr = 0; // not important
      pMsg->srcAddr.panId = 0;          // inner-PAN, not important
      pMsg->srcAddr.endPoint = srcEp;   // src ep, SAMPLELIGHT_ENDPOINT send to himself
      pMsg->endPoint = srcEp;           // dest ep, send to SAMPLELIGHT_ENDPOINT himself
      pMsg->attrCmd = (zclReportCmd_t *)pReportCmd;
  }
  
  osal_msg_send( zclLight_TaskID, (uint8 *)pMsg );

  return ( TRUE );
}

#ifdef ZCL_GROUPS
static void zclLight_GroupCB_1( zclGroupRsp_t *pRsp )
{
//  if ( pRsp->srcAddr->addr.shortAddr == 0x0000 )
//  {
//    if ( pRsp->cmdID == COMMAND_GROUP_ADD_RSP )
//    {
//      
//    }
//        COMMAND_GROUP_VIEW_RSP
//  }
          HalLedBlink(HAL_LED_1, 6, 50, 200);

}
static void zclLight_GroupCB_2( zclGroupRsp_t *pRsp )
{
            HalLedBlink(HAL_LED_1, 6, 50, 200);

}
#endif

#ifdef ZCL_IDENTIFY
/*********************************************************************
 * @fn      zclSampleLight_ProcessIdentifyTimeChange
 *
 * @brief   Called to process any change to the IdentifyTime attribute.
 *
 * @param   none
 *
 * @return  none
 */
static void zclLight_ProcessIdentifyTimeChange( uint8 endpoint )
{
  check = endpoint;

    if (zclLight_IdentifyTime_1 > 0 )
    {
      HalLedBlink(HAL_LED_1, 6, 50, 200);
    }
    else 
    {
      HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
    }

    if (zclLight_IdentifyTime_2 > 0  )
    {
      HalLedBlink(HAL_LED_2, 6, 50, 200);
    }
    else 
    {
      HalLedSet(HAL_LED_2, HAL_LED_MODE_ON);
    }
}

#endif
/****************************************************************************
****************************************************************************/


