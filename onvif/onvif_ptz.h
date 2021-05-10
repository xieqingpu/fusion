/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2014-2019, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifndef ONVIF_PTZ_H
#define ONVIF_PTZ_H

#include "sys_inc.h"
#include "onvif.h"

#define MAX_PRESETS_TOUR    10 
#define MAX_PRESETS_T    64
#define MAX_TIMER    16

typedef struct
{
	uint32	TimeoutFlag 	: 1;				// Indicates whether the field Timeout is valid
	uint32  Reserved	 	: 31;
	
	char	ProfileToken[ONVIF_TOKEN_LEN];		// required, A reference to the MediaProfile
	
	onvif_PTZSpeed	Velocity;					// required, A Velocity vector specifying the velocity of pan, tilt and zoom
	
	int		Timeout;							// optional, An optional Timeout parameter, unit is second
} ContinuousMove_REQ;

typedef struct 
{
	uint32	PanTiltFlag 	: 1;				// Indicates whether the field PanTilt is valid
	uint32	ZoomFlag 		: 1;				// Indicates whether the field Zoom is valid
	uint32  Reserved	 	: 30;
	
	char	ProfileToken[ONVIF_TOKEN_LEN];		// required, A reference to the MediaProfile that indicate what should be stopped

	BOOL	PanTilt;							// optional, Set true when we want to stop ongoing pan and tilt movements.If PanTilt arguments are not present, this command stops these movements
	BOOL	Zoom;								// optional, Set true when we want to stop ongoing zoom movement.If Zoom arguments are not present, this command stops ongoing zoom movement
} PTZ_Stop_REQ;

typedef struct
{
	uint32	SpeedFlag 		: 1;				// Indicates whether the field Speed is valid
	uint32  Reserved	 	: 31;
	
    char	ProfileToken[ONVIF_TOKEN_LEN];		// required, A reference to the MediaProfile

	onvif_PTZVector	Position;					// required, A Position vector specifying the absolute target position
	onvif_PTZSpeed	Speed;						// optional, An optional Speed    
} AbsoluteMove_REQ;

typedef struct
{
	uint32	SpeedFlag 		: 1;				// Indicates whether the field Speed is valid
	uint32  Reserved	 	: 31;
	
    char	ProfileToken[ONVIF_TOKEN_LEN];		// required, A reference to the MediaProfile
    
    onvif_PTZVector	Translation;				// required, A positional Translation relative to the current position
	onvif_PTZSpeed	Speed;						// optional, An optional Speed parameter
} RelativeMove_REQ;


// add
typedef struct 
{
	float x;		//顶点坐标x(左上角)
	float y;		//顶点坐标y(左上角)
	float w;		//宽
	float h;		//高

	int		dula_type;			//1:温度检测，2：数据识别
	int		dula_model;  		//4:rgb,即可见光模式 5:ir,红外模式。1~3的可查看Dula方面的 
	onvif_FloatRange	temperature;	//温度范围
} onvif_ex_VectorList;

typedef struct
{
	uint32	PresetTokenFlag : 1;				// Indicates whether the field PresetToken is valid
	uint32	PresetNameFlag 	: 1;				// Indicates whether the field PresetName is valid
	uint32  VectorList_Flag	 	: 1;	//xieqingpu
	uint32  ModifyPosition_Flag	 	: 1;	//xieqingpu  ,重命名预置位时是否也修改预置位位置
	uint32  Reserved	 	: 28;
	
    char	ProfileToken[ONVIF_TOKEN_LEN];		// required, A reference to the MediaProfile where the operation should take place
    char	PresetToken[ONVIF_TOKEN_LEN];		// optional, A requested preset token
    char    PresetName[ONVIF_NAME_LEN];			// optional, A requested preset name

    char    ModifyPosition[ONVIF_NAME_LEN];		// add

	uint16  VectorNumber;     //xie
	onvif_ex_VectorList	 VectorList[VECTOR_LIST_LEN];     //xieqingpu
} SetPreset_REQ;

typedef struct
{
	char	ProfileToken[ONVIF_TOKEN_LEN];		// required, A reference to the MediaProfile where the operation should take place
    char	PresetToken[ONVIF_TOKEN_LEN];		// required, A requested preset token
} RemovePreset_REQ;

typedef struct
{
	uint32	SpeedFlag 		: 1;				// Indicates whether the field Speed is valid
	uint32  Reserved	 	: 31;
	
	char	ProfileToken[ONVIF_TOKEN_LEN];		// required, A reference to the MediaProfile where the operation should take place
    char	PresetToken[ONVIF_TOKEN_LEN];		// required, A requested preset token

	onvif_PTZSpeed	Speed;						// optional, A requested speed.The speed parameter can only be specified when Speed Spaces are available for the PTZ Node
} GotoPreset_REQ;

typedef struct
{
	uint32	SpeedFlag 		: 1;				// Indicates whether the field Speed is valid
	uint32  Reserved	 	: 31;
	
    char	ProfileToken[ONVIF_TOKEN_LEN];		// required, A reference to the MediaProfile where the operation should take place

    onvif_PTZSpeed	Speed;						// optional, A requested speed.The speed parameter can only be specified when Speed Spaces are available for the PTZ Node
} GotoHomePosition_REQ;

typedef struct
{
	onvif_PTZConfiguration  PTZConfiguration;	// required, 

	BOOL	ForcePersistence;					// required, 	
} SetConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	    // required, Contains the token of an existing media profile the configurations shall be compatible with
} GetCompatibleConfigurations_REQ;


/* add by xieqingpu */
typedef struct 
{
	char    ProfileToken[ONVIF_TOKEN_LEN];		    /* required element of type tt:ReferenceToken */
    char	PresetTourToken[ONVIF_TOKEN_LEN];		// optional, A requested PresetTour token
    // char    PresetTourName[ONVIF_NAME_LEN];			// optional, A requested PresetTour name
} PresetTour_REQ;


typedef struct 
{
	char	PresetToken[ONVIF_TOKEN_LEN];
	// char    Name[ONVIF_NAME_LEN];

	uint32    index;	  //index指的是哪一个预置位
	int       StayTime;	
	uint16    zoomValue;
} Presets_t;

typedef struct 
{
	uint32  runNumberFlag  : 1;	
	uint32  runTimeFlag    : 1;
	uint32  TimerFlag    : 1;
	uint32	Reserved	   : 29;

	Presets_t presets[MAX_PRESETS_T];
    uint16    presetCount; 		//巡检总共的预置位
    uint16    timerCount; 		//巡航的定时器数

	uint16    direction; 		//巡检类型（顺序，倒序，随机）
	BOOL	  RandomOrder;      //是否随机转动
    uint32    runNumber; 		//运行次数（次数运行完停止）
    uint32    runTime; 		    //运行时间（时间运行完停止）
	onvif_PTZPresetTourTimer  Timer[MAX_TIMER];		// 扩展巡航定时
} PresetsTours_t;

typedef struct 
{
	uint32  UsedFlag  : 1;	
	uint32	Reserved	   : 31;

	char	PresetTourToken[ONVIF_TOKEN_LEN];
	char    Name[ONVIF_NAME_LEN];
	PresetsTours_t	PresetsTour;
}PTZ_PresetsTours_t;

typedef struct 
{
	char    ProfileToken[ONVIF_TOKEN_LEN];		/* required element of type tt:ReferenceToken */
    char	PresetTourToken[ONVIF_TOKEN_LEN];	/* required element of type tt:ReferenceToken */
	onvif_PTZPresetTourOperation Operation;  	/* required element of type tt:PTZPresetTourOperation */
} OperatePresetTour_REQ;

typedef struct 
{
	char    ProfileToken[ONVIF_TOKEN_LEN];		/* required element of type tt:ReferenceToken */
	ONVIF_PresetTour * PresetTour_req;
} ModifyPresetTour_REQ;


#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET onvif_ptz_GetStatus(ONVIF_PROFILE * p_profile, onvif_PTZStatus * p_ptz_status);

ONVIF_RET onvif_ContinuousMove(ContinuousMove_REQ * p_req);
ONVIF_RET onvif_ptz_Stop(PTZ_Stop_REQ * p_req);
ONVIF_RET onvif_AbsoluteMove(AbsoluteMove_REQ * p_req);
ONVIF_RET onvif_RelativeMove(RelativeMove_REQ * p_req);
ONVIF_RET onvif_SetPreset(SetPreset_REQ * p_req);
ONVIF_RET onvif_RemovePreset(RemovePreset_REQ * p_req);
ONVIF_RET onvif_GotoPreset(GotoPreset_REQ * p_req);
ONVIF_RET onvif_GotoHomePosition(GotoHomePosition_REQ * p_req);
ONVIF_RET onvif_SetHomePosition(const char * token);
ONVIF_RET onvif_SetConfiguration(SetConfiguration_REQ * p_req);
// add by xieqingpu
ONVIF_RET onvif_CreatePresetTour(PresetTour_REQ * p_req);
ONVIF_RET onvif_OperatePresetTour(OperatePresetTour_REQ * p_req);
ONVIF_RET onvif_RemovePresetTour(PresetTour_REQ * p_req);
ONVIF_RET onvif_ModifyPresetTour(ModifyPresetTour_REQ * p_req);


#ifdef __cplusplus
}
#endif


#endif


