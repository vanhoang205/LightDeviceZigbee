/**************************************************************************************************
  Filename:       zcl_samplelight.h
  Revised:        $Date: 2014-06-19 08:38:22 -0700 (Thu, 19 Jun 2014) $
  Revision:       $Revision: 39101 $

  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


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
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

#ifndef ZCL_LIGHT_H
#define ZCL_LIGHT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
   
/*********************************************************************
 * CONSTANTS
 */
#define LIGHT_ENDPOINT_1            8
#define LIGHT_ENDPOINT_2            9

  
#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Application Events   
#define LIGHT_END_DEVICE_REJOIN_EVT           0x0001
#define SAMPLEAPP_END_DEVICE_REJOIN_DELAY     10000

#define LIGHT_CHECK_HOLD_KEY_EVT              0x0002  
  
#define LIGHT_CHECK_REPORT_EVT_1              0x0004 

#define LIGHT_CHECK_REPORT_EVT_2              0x0008 

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */

extern zclConfigReportRec_t zclLight_ConfigReportRecs_Rpt1[];
extern zclConfigReportRec_t zclLight_ConfigReportRecs_Rpt2[];
extern uint8 CONST zclLight_NumConfigReportRecs_1;
extern uint8 CONST zclLight_NumConfigReportRecs_2;

extern SimpleDescriptionFormat_t zclLight_SimpleDesc_1;
extern SimpleDescriptionFormat_t zclLight_SimpleDesc_2;

extern CONST zclCommandRec_t zclLight_Cmds_1[];
extern CONST zclCommandRec_t zclLight_Cmds_2[];

extern CONST uint8 zclCmdsArraySize_1;
extern CONST uint8 zclCmdsArraySize_2;

// attribute list
extern CONST zclAttrRec_t zclLight_Attrs_1[];
extern CONST zclAttrRec_t zclLight_Attrs_2[];

extern CONST uint8 zclLight_NumAttributes_1;
extern CONST uint8 zclLight_NumAttributes_2;


// Identify attributes
extern uint16 zclLight_IdentifyTime_1;
extern uint16 zclLight_IdentifyTime_2;

extern uint8  zclLight_IdentifyCommissionState;

// Scenes attributes
extern uint8        zclLight_ScenesSceneCount;
extern uint8        zclLight_ScenesCurrentScene;
extern uint16       zclLight_ScenesCurrentGroup;
extern uint8        zclLight_ScenesSceneValid;
extern CONST uint8  zclLight_ScenesNameSupport;

// OnOff attributes
extern uint8  zclLight_OnOff_1;
extern uint8  zclLight_OnOff_2;

// In/Out cluster list
extern const cId_t zclLight_InClusterList[];

// Groups attributes
extern uint8 zclLight_GroupsNameSupport_1;
extern uint8 zclLight_GroupsNameSupport_2;
/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void zclLight_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern UINT16 zclLight_event_loop( byte task_id, UINT16 events );

/*
 *  Reset all writable attributes to their default values.
 */
extern  void zclLight_ResetAttributesToDefaultValues_1(void); //implemented in zcl_samplelight_data.c
extern  void zclLight_ResetAttributesToDefaultValues_2(void); //implemented in zcl_samplelight_data.c

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_LIGHT_H */
