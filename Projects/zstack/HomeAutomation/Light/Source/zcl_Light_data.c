/**************************************************************************************************
  Filename:       zcl_samplelight_data.c
  Revised:        $Date: 2014-05-12 13:14:02 -0700 (Mon, 12 May 2014) $
  Revision:       $Revision: 38502 $


  Description:    Zigbee Cluster Library - sample device application.


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
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"


#include "zcl_Light.h"

/*********************************************************************
 * CONSTANTS
 */

#define LIGHT_DEVICE_VERSION     1
#define LIGHT_FLAGS              0

#define LIGHT_HWVERSION          1
#define LIGHT_ZCLVERSION         1

#define DEFAULT_PHYSICAL_ENVIRONMENT 0
#define DEFAULT_DEVICE_ENABLE_STATE DEVICE_ENABLED
#define DEFAULT_IDENTIFY_TIME 0

#define DEFAULT_ON_OFF_STATE            LIGHT_OFF

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
 
//global attributes
const uint16 zclLight_clusterRevision_all = 0x0001; //currently all cluster implementations are according to ZCL6, which has revision #1. In the future it is possible that different clusters will have different revisions, so they will have to use separate attribute variables.

// Basic Cluster
const uint8 zclLight_HWRevision_1 = LIGHT_HWVERSION;
const uint8 zclLight_ZCLVersion_1 = LIGHT_ZCLVERSION;
const uint8 zclLight_ManufacturerName_1[] = { 16, 'I','T','S','C','-','R','D','-','T','e','c','h',' ',' ',' ',' ' };
const uint8 zclLight_ModelId_1[] = { 16, 'I','T','S','C','0','0','0','2',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclLight_DateCode_1[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclLight_PowerSource_1 = POWER_SOURCE_MAINS_1_PHASE;
uint8 zclLight_LocationDescription_1[17];
uint8 zclLight_PhysicalEnvironment_1 = 0x0B;    //office
uint8 zclLight_DeviceEnable_1;

const uint8 zclLight_HWRevision_2 = LIGHT_HWVERSION;
const uint8 zclLight_ZCLVersion_2 = LIGHT_ZCLVERSION;
const uint8 zclLight_ManufacturerName_2[] = { 16, 'I','T','S','C','-','R','D','-','T','e','c','h',' ',' ',' ',' ' };
const uint8 zclLight_ModelId_2[] = { 16, 'I','T','S','C','0','0','0','3',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclLight_DateCode_2[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclLight_PowerSource_2 = POWER_SOURCE_MAINS_1_PHASE;
uint8 zclLight_LocationDescription_2[17];       
uint8 zclLight_PhysicalEnvironment_2 = 0x0B;    //office
uint8 zclLight_DeviceEnable_2;

// Identify Cluster
uint16 zclLight_IdentifyTime_1;         // set 0 value for identify (mandatory)
uint16 zclLight_IdentifyTime_2;         // set 0 value for identify (mandatory)

// Groups Cluster
uint8 zclLight_GroupsNameSupport_1 = 0;
uint8 zclLight_GroupsNameSupport_2 = 0;

// On/Off Cluster
uint8  zclLight_OnOff_1;
uint8  zclLight_OnOff_2;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

// NOTE: The attributes listed in the AttrRec must be in ascending order 
// per cluster to allow right function of the Foundation discovery commands
 
CONST zclAttrRec_t zclLight_Attrs_1[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_ZCLVersion_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zclLight_HWRevision_1      // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclLight_ManufacturerName_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclLight_ModelId_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclLight_DateCode_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_PowerSource_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)zclLight_LocationDescription_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclLight_PhysicalEnvironment_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclLight_DeviceEnable_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  },
#ifdef ZCL_IDENTIFY
  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclLight_IdentifyTime_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  },  
#endif

  // *** On/Off Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    { // Attribute record
      ATTRID_ON_OFF,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ | ACCESS_REPORTABLE,
      (void *)&zclLight_OnOff_1
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  },
  
#ifdef ZCL_GROUPS
  {
    ZCL_CLUSTER_ID_GEN_GROUPS,
    {
      ATTRID_GROUP_NAME_SUPPORT,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void*)&zclLight_GroupsNameSupport_1
    }
  },

  {
    ZCL_CLUSTER_ID_GEN_GROUPS,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  },
#endif  
  {
    ZCL_CLUSTER_ID_GEN_SCENES,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  }
};
uint8 CONST zclLight_NumAttributes_1 = ( sizeof(zclLight_Attrs_1) / sizeof(zclLight_Attrs_1[0]) );


CONST zclAttrRec_t zclLight_Attrs_2[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_ZCLVersion_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zclLight_HWRevision_2  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclLight_ManufacturerName_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclLight_ModelId_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclLight_DateCode_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_PowerSource_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)zclLight_LocationDescription_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclLight_PhysicalEnvironment_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclLight_DeviceEnable_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  },
#ifdef ZCL_IDENTIFY
  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclLight_IdentifyTime_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  },  
#endif

  // *** On/Off Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    { // Attribute record
      ATTRID_ON_OFF,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ | ACCESS_REPORTABLE,
      (void *)&zclLight_OnOff_2
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  },
  
#ifdef ZCL_GROUPS
  {
    ZCL_CLUSTER_ID_GEN_GROUPS,
    {
      ATTRID_GROUP_NAME_SUPPORT,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void*)&zclLight_GroupsNameSupport_2
    }
  },

  {
    ZCL_CLUSTER_ID_GEN_GROUPS,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  },
#endif  
  {
    ZCL_CLUSTER_ID_GEN_SCENES,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclLight_clusterRevision_all
    }
  }
};
uint8 CONST zclLight_NumAttributes_2 = ( sizeof(zclLight_Attrs_2) / sizeof(zclLight_Attrs_2[0]) );


/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
const cId_t zclLight_InClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_ON_OFF
};

#define ZCLLIGHT_MAX_INCLUSTERS   (sizeof(zclLight_InClusterList) / sizeof(zclLight_InClusterList[0]))
 
SimpleDescriptionFormat_t zclLight_SimpleDesc_1 =
{
  LIGHT_ENDPOINT_1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                 //  uint16 AppProfId;
  ZCL_HA_DEVICEID_ON_OFF_LIGHT,      //  uint16 AppDeviceId;
  LIGHT_DEVICE_VERSION,              //  int   AppDevVer:4;
  LIGHT_FLAGS,                       //  int   AppFlags:4;
  ZCLLIGHT_MAX_INCLUSTERS,           //  byte  AppNumInClusters;
  (cId_t *)zclLight_InClusterList,   //  byte *pAppInClusterList;
  0,                                 //  byte  AppNumInClusters;
  NULL                               //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclLight_SimpleDesc_2 =
{
  LIGHT_ENDPOINT_2,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                 //  uint16 AppProfId;
  ZCL_HA_DEVICEID_ON_OFF_LIGHT,      //  uint16 AppDeviceId;
  LIGHT_DEVICE_VERSION,              //  int   AppDevVer:4;
  LIGHT_FLAGS,                       //  int   AppFlags:4;
  ZCLLIGHT_MAX_INCLUSTERS,           //  byte  AppNumInClusters;
  (cId_t *)zclLight_InClusterList,   //  byte *pAppInClusterList;
  0,                                 //  byte  AppNumInClusters;
  NULL                               //  byte *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
 
/*********************************************************************
 * @fn      zclLight_ResetAttributesToDefaultValues
 *
 * @brief   Reset all writable attributes to their default values.
 *
 * @param   none
 *
 * @return  none
 */
void zclLight_ResetAttributesToDefaultValues_1(void)
{
  int i;
  
  zclLight_LocationDescription_1[0] = 16;
  for (i = 1; i <= 16; i++)
  {
    zclLight_LocationDescription_1[i] = ' ';
  }
  
  zclLight_PhysicalEnvironment_1 = DEFAULT_PHYSICAL_ENVIRONMENT;
  zclLight_DeviceEnable_1 = DEFAULT_DEVICE_ENABLE_STATE;
  
#ifdef ZCL_IDENTIFY
  zclLight_IdentifyTime_1 = DEFAULT_IDENTIFY_TIME;
#endif

  zclLight_OnOff_1 = DEFAULT_ON_OFF_STATE;
  
}

void zclLight_ResetAttributesToDefaultValues_2(void)
{
  int i;
  
  zclLight_LocationDescription_2[0] = 16;
  for (i = 1; i <= 16; i++)
  {
    zclLight_LocationDescription_2[i] = ' ';
  }
  
  zclLight_PhysicalEnvironment_2 = DEFAULT_PHYSICAL_ENVIRONMENT;
  zclLight_DeviceEnable_2 = DEFAULT_DEVICE_ENABLE_STATE;
  
#ifdef ZCL_IDENTIFY
  zclLight_IdentifyTime_2 = DEFAULT_IDENTIFY_TIME;
#endif

  zclLight_OnOff_2 = DEFAULT_ON_OFF_STATE;
  
}


#ifdef ZCL_REPORT
zclConfigReportRec_t zclLight_ConfigReportRecs_Rpt1[] =
{
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,  //clusterID
    0xFFFF,                     // timeup
    (void *)&zclLight_OnOff_1,  // lastReportValue
    { // CfgReportRec
      ZCL_REPORT_SEND,          // direction
      ATTRID_ON_OFF,            // attrID
      ZCL_DATATYPE_BOOLEAN,     // dataType
      0,                        // minReportInt
      0xFFFF,                   // maxReportInt
      0,                        // timeoutPeriod
      NULL                      // *reportableChange, no need to set for DISCRETE datatype
    }
  }
};

zclConfigReportRec_t zclLight_ConfigReportRecs_Rpt2[] =
{
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,  //clusterID
    0xFFFF,                     // timeup
    (void *)&zclLight_OnOff_2,  // lastReportValue
    { // CfgReportRec
      ZCL_REPORT_SEND,          // direction
      ATTRID_ON_OFF,            // attrID
      ZCL_DATATYPE_BOOLEAN,     // dataType
      0,                        // minReportInt
      0xFFFF,                   // maxReportInt
      0,                        // timeoutPeriod
      NULL                      // *reportableChange, no need to set for DISCRETE datatype
    }
  }
};

uint8 CONST zclLight_NumConfigReportRecs_1 = ( sizeof(zclLight_ConfigReportRecs_Rpt1) / sizeof(zclLight_ConfigReportRecs_Rpt1[0]) );
uint8 CONST zclLight_NumConfigReportRecs_2 = ( sizeof(zclLight_ConfigReportRecs_Rpt2) / sizeof(zclLight_ConfigReportRecs_Rpt2[0]) );

#endif  // ZCL_REPORT
/****************************************************************************
****************************************************************************/
