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
#include <sys/prctl.h>
#include "sys_inc.h"
#include "onvif_ptz.h"
#include "onvif_utils.h"

#ifdef MEDIA2_SUPPORT
#include "onvif_media2.h"
#endif

// #include "ptz.h"   ///
#include "set_config.h"
#include "visca_api.h"
#include  <math.h>
#include <pthread.h>
#include "utils_log.h"
#include "http_cln.h"

#ifdef PTZ_SUPPORT

#define MSG_VIDEO_FUSIONSNAPJPEGPROCESS	60	//双光融合图像抓拍
#define MSG_VIDEO_IPCSNAPJPEGPROCESS	80	//可见光摄像图像抓拍
#define MSG_VIDEO_IRMODESNAPJPEGPROCESS	81	//IR模块图像抓拍

extern PTZ_PresetsTours_t * onvif_get_idle_PresetTour(int *index);
extern PTZ_PresetsTours_t * onvif_find_PresetTour(const char  * preset_token);
extern PTZ_PresetsTours_t  PTZPresetsTour[MAX_PRESETS_TOUR];

// #define PTOURS_TIME_NUMBER    //add xie

/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;
extern ONVIF_CFG g_onvif_cfg;

static int cur_preset_id = -1;


/***************************************************************************************/
ONVIF_RET onvif_ptz_GetStatus(ONVIF_PROFILE * p_profile, onvif_PTZStatus * p_ptz_status)
{
	// todo : add get ptz status code ...
	
	p_ptz_status->PositionFlag = 1;
	p_ptz_status->Position.PanTiltFlag = 1;
	p_ptz_status->Position.PanTilt.x = 0;
	p_ptz_status->Position.PanTilt.y = 0;
	p_ptz_status->Position.ZoomFlag = 1;
	p_ptz_status->Position.Zoom.x = 0;
	
	p_ptz_status->MoveStatusFlag = 1;
	p_ptz_status->MoveStatus.PanTiltFlag = 1;
	p_ptz_status->MoveStatus.PanTilt = MoveStatus_IDLE;
	p_ptz_status->MoveStatus.ZoomFlag = 1;
	p_ptz_status->MoveStatus.Zoom = MoveStatus_IDLE;

	p_ptz_status->ErrorFlag = 0;
    p_ptz_status->UtcTime = time(NULL);	
	
	return ONVIF_OK;
}


//
uint16_t  switchSpeed(float x, float y, float z)
{
	uint16_t speed;
	
	float x_value =  fabs(x);
	float y_value =  fabs(y);
	float z_value =  fabs(z);

    if ( x_value  > 0.000001 ) {
	 	speed = (int)(63*x_value);
	}
	else  if ( y_value > 0.000001 ) {
	 	speed = (int)(63*y_value);
	}
	else  if ( z_value > 0.000001 ) {
	 	speed = (int)(63*z_value);
	}
	
	return  speed;
}
//

ONVIF_RET onvif_ContinuousMove(ContinuousMove_REQ * p_req)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}
    else if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}
	
	if (p_req->Velocity.PanTiltFlag)
	{
	
		if (p_req->Velocity.PanTilt.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.XRange.Min < -FPP || 
		 	p_req->Velocity.PanTilt.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.XRange.Max > FPP)
		{
			return ONVIF_ERR_InvalidPosition;
		}

		if (p_req->Velocity.PanTilt.y - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.YRange.Min < -FPP || 
			p_req->Velocity.PanTilt.y - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.YRange.Max > FPP)
		{
			return ONVIF_ERR_InvalidPosition;
		}
	}
	
	if (p_req->Velocity.ZoomFlag && 
		(p_req->Velocity.Zoom.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.ContinuousZoomVelocitySpace.XRange.Min < -FPP ||     //Min = -1.0
		 p_req->Velocity.Zoom.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.ContinuousZoomVelocitySpace.XRange.Max > FPP))		//Max = 1.0
	{
		return ONVIF_ERR_InvalidPosition;
	}	

	// todo : add continuous move code ... 
	// printf("\nPTZvelocity :  x=%f , y = %f , z = %f \n",p_req->Velocity.PanTilt.x, p_req->Velocity.PanTilt.y, p_req->Velocity.Zoom.x);
	float x = p_req->Velocity.PanTilt.x;
	float y = p_req->Velocity.PanTilt.y;
	float z = p_req->Velocity.Zoom.x;

    uint16_t ptzSpeed;
	ptzSpeed  = switchSpeed(x,  y,  z);
	
	int ret = controlPtzPos(x, y, z , ptzSpeed);     // add
	if (ret == 0)
		cur_preset_id = -1;


    return ONVIF_OK;
}

ONVIF_RET onvif_ptz_Stop(PTZ_Stop_REQ * p_req)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}
	else if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}

	// todo : add stop PTZ moving code ... 
	ptzStop();

    return ONVIF_OK;
}

ONVIF_RET onvif_AbsoluteMove(AbsoluteMove_REQ * p_req)
{	
	ONVIF_PROFILE * p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}
	else if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}

	if (p_req->Position.PanTiltFlag)
	{
		if (p_req->Position.PanTilt.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpace.XRange.Min < -FPP || 
		 	p_req->Position.PanTilt.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpace.XRange.Max > FPP)
		{
			return ONVIF_ERR_InvalidPosition;
		}

		if (p_req->Position.PanTilt.y - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpace.YRange.Min < -FPP || 
			p_req->Position.PanTilt.y - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpace.YRange.Max > FPP)
		{
			return ONVIF_ERR_InvalidPosition;
		}
	}

	if (p_req->Position.ZoomFlag && 
		(p_req->Position.Zoom.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.AbsoluteZoomPositionSpace.XRange.Min < -FPP || 
		 p_req->Position.Zoom.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.AbsoluteZoomPositionSpace.XRange.Max > FPP))
	{
		return ONVIF_ERR_InvalidPosition;
	}	
	
	
	// todo : add absolute move code ...
	
    return ONVIF_OK;
}

ONVIF_RET onvif_RelativeMove(RelativeMove_REQ * p_req)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}
	else if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}

	if (p_req->Translation.PanTiltFlag)
	{
		if (p_req->Translation.PanTilt.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpace.XRange.Min < -FPP || 
			p_req->Translation.PanTilt.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpace.XRange.Max > FPP)
		{
			return ONVIF_ERR_InvalidPosition;
		}

		if (p_req->Translation.PanTilt.y - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpace.YRange.Min < -FPP || 
			p_req->Translation.PanTilt.y - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpace.YRange.Max > FPP)
		{
			return ONVIF_ERR_InvalidPosition;
		}
	}
	
	if (p_req->Translation.ZoomFlag && 
		(p_req->Translation.Zoom.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.RelativeZoomTranslationSpace.XRange.Min < -FPP || 
		 p_req->Translation.Zoom.x - g_onvif_cfg.ptz_node->PTZNode.SupportedPTZSpaces.RelativeZoomTranslationSpace.XRange.Max > FPP))
	{
		return ONVIF_ERR_InvalidPosition;
	}
	
	// todo : add relative move code ...
	
    return ONVIF_OK;
}

////// add by xieqingpu
int  onvif_find_PTZPreset_index(const char * profile_token, const char  * preset_token)
{
    int i;
    ONVIF_PROFILE * p_profile = onvif_find_profile(profile_token);
    if (NULL == p_profile)
    {
        return  -1;
    }

    for (i = 0; i < ARRAY_SIZE(p_profile->presets); i++)
    {
        if (strcmp(preset_token, p_profile->presets[i].PTZPreset.token) == 0)
        {
            return i;
        }
    }

	return  -1;
}
//////

ONVIF_RET onvif_SetPreset(SetPreset_REQ * p_req)
{
    ONVIF_PTZPreset * p_preset = NULL;
	ONVIF_PROFILE * p_profile;

	p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}
	else if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}
	
    if (p_req->PresetTokenFlag && p_req->PresetToken[0] != '\0')
    {
        p_preset = onvif_find_PTZPreset(p_req->ProfileToken, p_req->PresetToken);
        if (NULL == p_preset)
        {
        	return ONVIF_ERR_NoToken;
        }
    }
    else
    {
        p_preset = onvif_get_idle_PTZPreset(p_req->ProfileToken);
        if (NULL == p_preset)
        {
        	return ONVIF_ERR_TooManyPresets;
        }
     }


	/* 获取Unix时间戳，用在token字段上，以确保token的唯一性 */
    time_t time_utc;
    time(&time_utc);
	
	if (p_req->PresetNameFlag && p_req->PresetName[0] != '\0')     // Preset Name
    {
    	strcpy(p_preset->PTZPreset.Name, p_req->PresetName);
    }
    else
    {
    	sprintf(p_preset->PTZPreset.Name, "PRESET_NAME_%ld", time_utc);
    	strcpy(p_req->PresetName, p_preset->PTZPreset.Name);
    }
    if (p_req->PresetTokenFlag && p_req->PresetToken[0] != '\0')  //有前端发过来‘PresetToken’说明是重命名预置位操作 或者 画检测框
    {
        strcpy(p_preset->PTZPreset.token, p_req->PresetToken);
    }
    else
    {
        sprintf(p_preset->PTZPreset.token, "PRESET_%ld", time_utc);
        strcpy(p_req->PresetToken, p_preset->PTZPreset.token);
    }

	/* 只单单修改名字(没有修改预置位位置) */
	if(p_req->PresetTokenFlag && p_req->ModifyPosition_Flag==0 && strcasecmp(p_req->ModifyPosition, "false")==0 )
	{
		goto write_preset;
	}

	/* 创建新的预置位 或者 画检测框 或者 重命名操作(并且同时重新修改位置) */
	/* 预置位对应是否画检测框 */
	if (p_req->VectorList_Flag )
	{
		int i;
		p_preset->VectorListFlag = 1;

		if (p_req->VectorNumber > VECTOR_LIST_LEN)	//如果画框数量超过了VECTOR_LIST_LEN，则返回错误
		{
			return ONVIF_ERR_OTHER;
		}
		memset(p_preset->Vector_list, 0x0, sizeof(p_preset->Vector_list));

		for (i = 0; i < p_req->VectorNumber; i++)
		{
			p_preset->Vector_Number = p_req->VectorNumber;
			p_preset->Vector_list[i].x = p_req->VectorList[i].x;
			p_preset->Vector_list[i].y = p_req->VectorList[i].y;
			p_preset->Vector_list[i].w = p_req->VectorList[i].w;
			p_preset->Vector_list[i].h = p_req->VectorList[i].h;

			p_preset->Vector_list[i].dulaType = p_req->VectorList[i].dula_type;
			p_preset->Vector_list[i].dulaModel = p_req->VectorList[i].dula_model;
			p_preset->Vector_list[i].temperature.Min = p_req->VectorList[i].temperature.Min;
			p_preset->Vector_list[i].temperature.Max = p_req->VectorList[i].temperature.Max;
		}

		goto write_preset;
	
	}
	else {
		p_preset->VectorListFlag = 0;
	}

	int index = onvif_find_PTZPreset_index(p_req->ProfileToken, p_req->PresetToken);

	short location = index < 0 ? 1 : (index+1);  //该ptz设备可以从0x00~0x3f设置，但好像从0设置不行，所以从1开始
	/* 设置预置位 */
	setPtzPreset(location);
	

	/* 预置位对应的相机焦距 */
	uint16_t z = get_zoom_val();
	p_preset->zoomVal = z;

	p_preset->UsedFlag = 1;	

	p_preset->PTZPreset.PTZPositionFlag = 1;
	p_preset->PTZPreset.PTZPosition.PanTiltFlag = 1;
	p_preset->PTZPreset.PTZPosition.PanTilt.x = 0;
	p_preset->PTZPreset.PTZPosition.PanTilt.y = 0;
	p_preset->PTZPreset.PTZPosition.ZoomFlag = 1;
	p_preset->PTZPreset.PTZPosition.Zoom.x = 0;


write_preset:

	if (writePtzPreset(p_profile->presets, MAX_PTZ_PRESETS) != 0) 
		printf("write Ptz Preset faile.\n");
    
    return ONVIF_OK;
}

ONVIF_RET onvif_RemovePreset(RemovePreset_REQ * p_req)
{
    ONVIF_PROFILE * p_profile;
    ONVIF_PTZPreset * p_preset;

    p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}

    p_preset = onvif_find_PTZPreset(p_req->ProfileToken, p_req->PresetToken); 
    if (NULL == p_preset)
    {
		UTIL_INFO("p_req->PresetToken=%s", p_req->PresetToken);
		return ONVIF_ERR_PresetExist;
    }

    memset(p_preset, 0, sizeof(ONVIF_PTZPreset));

	//// add by xieqingpu
	if (writePtzPreset(p_profile->presets, MAX_PTZ_PRESETS) != 0) 
		printf("write Ptz Preset faile.\n");
	

    return ONVIF_OK;
}

ONVIF_RET onvif_GotoPreset(GotoPreset_REQ * p_req)
{	
	ONVIF_PROFILE * p_profile;
    ONVIF_PTZPreset * p_preset;
    
	p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}

    p_preset = onvif_find_PTZPreset(p_req->ProfileToken, p_req->PresetToken); 
    if (NULL == p_preset)
    {
        return ONVIF_ERR_NoToken;
    }

    // todo : add goto preset code ...
 	//// add by xieqingpu
 	int index = onvif_find_PTZPreset_index(p_req->ProfileToken, p_req->PresetToken); //获取preset的下标只是为了设置预置位

	short location = index < 0 ? 1 : (index+1);  //该ptz设备可以从0x00~0x3f设置，但好像从0设置不行，所以从1开始
	gotoPtzPreset(location);

	cur_preset_id = index;


	uint16_t zoomValue ;
	zoomValue = p_profile->presets[index].zoomVal;
	set_zoom(zoomValue);
 	////

    return ONVIF_OK;
}

ONVIF_RET onvif_GotoHomePosition(GotoHomePosition_REQ * p_req)
{
    ONVIF_PROFILE * p_profile = onvif_find_profile(p_req->ProfileToken);
    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }

    if (NULL == g_onvif_cfg.ptz_node)
    {
    	return ONVIF_ERR_NoPTZProfile;
    }

    // todo : add goto home position code ...
	gotoPtzPreset(126);		  		 //HomePosition其实也是预置位的一个

	CONFIG_Home homePreset;
	memset(&homePreset, 0, sizeof(CONFIG_Home));
	if (readHomePos(&homePreset) != 0) 
		printf("write Ptz Preset faile.\n");

	uint16_t homeZoomVal ;
	homeZoomVal = homePreset.homeZoom;
	set_zoom(homeZoomVal);

    return ONVIF_OK;
}

ONVIF_RET onvif_SetHomePosition(const char * token)      // ssetPreset
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(token);
    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }

    if (NULL == g_onvif_cfg.ptz_node)
    {
    	return ONVIF_ERR_NoPTZProfile;
    }

	if (g_onvif_cfg.ptz_node->PTZNode.FixedHomePositionFlag && 
	    g_onvif_cfg.ptz_node->PTZNode.FixedHomePosition)
	{
		return ONVIF_ERR_CannotOverwriteHome;
	}
	
    // todo : add set home position code ...
	setPtzPreset(126);         //HomePosition其实也是预置位的一个

	CONFIG_Home homePreset;
	memset(&homePreset, 0, sizeof(CONFIG_Home));

	homePreset.homeZoom = get_zoom_val();

	if ( writeHomePos(&homePreset) != 0)
		printf("write Ptz Preset faile.\n");

    return ONVIF_OK;
}

ONVIF_RET onvif_SetConfiguration(SetConfiguration_REQ * p_req)
{
	ONVIF_PTZConfiguration * p_ptz_cfg;
	ONVIF_PTZNode * p_ptz_node;

	p_ptz_cfg = onvif_find_PTZConfiguration(p_req->PTZConfiguration.token);
	if (NULL == p_ptz_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}
	
	p_ptz_node = onvif_find_PTZNode(p_req->PTZConfiguration.NodeToken);
	if (NULL == p_ptz_node)
	{
		return ONVIF_ERR_ConfigModify;
	}

	if (p_req->PTZConfiguration.DefaultPTZTimeoutFlag)
	{
		if (p_req->PTZConfiguration.DefaultPTZTimeout < g_onvif_cfg.PTZConfigurationOptions.PTZTimeout.Min ||
			p_req->PTZConfiguration.DefaultPTZTimeout > g_onvif_cfg.PTZConfigurationOptions.PTZTimeout.Max)
		{
			return ONVIF_ERR_ConfigModify;
		}
	}

	// todo : add set ptz configuration code ...


    if (p_req->PTZConfiguration.MoveRampFlag)
    {
        p_ptz_cfg->Configuration.MoveRamp = p_req->PTZConfiguration.MoveRamp;
    }
    
    if (p_req->PTZConfiguration.PresetRampFlag)
    {
        p_ptz_cfg->Configuration.PresetRamp = p_req->PTZConfiguration.PresetRamp;
    }
    
    if (p_req->PTZConfiguration.PresetTourRampFlag)
    {
        p_ptz_cfg->Configuration.PresetTourRamp = p_req->PTZConfiguration.PresetTourRamp;
    }    

	strcpy(p_ptz_cfg->Configuration.Name, p_req->PTZConfiguration.Name);
	if (p_req->PTZConfiguration.DefaultPTZSpeedFlag)
	{
		if (p_req->PTZConfiguration.DefaultPTZSpeed.PanTiltFlag)
		{
			p_ptz_cfg->Configuration.DefaultPTZSpeed.PanTilt.x = p_req->PTZConfiguration.DefaultPTZSpeed.PanTilt.x;
			p_ptz_cfg->Configuration.DefaultPTZSpeed.PanTilt.y = p_req->PTZConfiguration.DefaultPTZSpeed.PanTilt.y;
		}

		if (p_req->PTZConfiguration.DefaultPTZSpeed.ZoomFlag)
		{
			p_ptz_cfg->Configuration.DefaultPTZSpeed.Zoom.x = p_req->PTZConfiguration.DefaultPTZSpeed.Zoom.x;
		}
	}

	if (p_req->PTZConfiguration.DefaultPTZTimeoutFlag)
	{
		p_ptz_cfg->Configuration.DefaultPTZTimeout = p_req->PTZConfiguration.DefaultPTZTimeout;
	}

	if (p_req->PTZConfiguration.PanTiltLimitsFlag)
	{
		memcpy(&p_ptz_cfg->Configuration.PanTiltLimits, &p_req->PTZConfiguration.PanTiltLimits, sizeof(onvif_PanTiltLimits));
	}

	if (p_req->PTZConfiguration.ZoomLimitsFlag)
	{
		memcpy(&p_ptz_cfg->Configuration.ZoomLimits, &p_req->PTZConfiguration.ZoomLimits, sizeof(onvif_ZoomLimits));
	}

	if (p_req->PTZConfiguration.ExtensionFlag)
	{
		if (p_req->PTZConfiguration.Extension.PTControlDirectionFlag)
		{
			if (p_req->PTZConfiguration.Extension.PTControlDirection.EFlipFlag)
			{
				p_ptz_cfg->Configuration.Extension.PTControlDirection.EFlip = p_req->PTZConfiguration.Extension.PTControlDirection.EFlip;
			}

			if (p_req->PTZConfiguration.Extension.PTControlDirection.ReverseFlag)
			{
				p_ptz_cfg->Configuration.Extension.PTControlDirection.Reverse = p_req->PTZConfiguration.Extension.PTControlDirection.Reverse;
			}
		}
	}
    
#ifdef MEDIA2_SUPPORT
    onvif_MediaConfigurationChangedNotify(p_req->PTZConfiguration.token, "PTZ");
#endif

	return ONVIF_OK;
}

extern float get_max_ir_temp(float* ir, int x, int y, int width, int height);
 
//毫秒
static int onvif_preset_usleep(int milisec, int presetTourStatus)
{
	struct timeval delay;
	int count = 1;
	int basetime = 100000;
	if (milisec/basetime > 1)
		count = milisec/basetime - 1;

	while (count)
	{
		if (presetTourStatus != PTZPresetTourState_Touring)
		{
			break;
		}

		delay.tv_sec = 0;
		//10*1000是10毫秒，如果延时的时间是在一秒以内只需要改下面这句
		//毫秒级别的定时把10改了就好，比如29毫秒延时改成，29*1000
		delay.tv_usec = basetime;
		select(0, NULL, NULL, NULL, &delay);
		count--;
	}
}

void getVectorData(onvif_VectorList * p_VectorData,unsigned int preset_idx, int vector_idx)
{
	p_VectorData->temperature.Min = g_onvif_cfg.profiles->presets[preset_idx].Vector_list[vector_idx].temperature.Min;
	p_VectorData->temperature.Max = g_onvif_cfg.profiles->presets[preset_idx].Vector_list[vector_idx].temperature.Max;

	p_VectorData->x = (g_onvif_cfg.profiles->presets[preset_idx].Vector_list[vector_idx].x+1)/2*IR_WIDTH;
	p_VectorData->y = (1-(g_onvif_cfg.profiles->presets[preset_idx].Vector_list[vector_idx].y))/2*IR_HEIGHT;
	p_VectorData->w = (g_onvif_cfg.profiles->presets[preset_idx].Vector_list[vector_idx].w)/2*IR_WIDTH;
	p_VectorData->h = (g_onvif_cfg.profiles->presets[preset_idx].Vector_list[vector_idx].h)/2*IR_HEIGHT;
}

//成功返回0，失败返回-1
int get_detect_info(void* pvector, int* cnt)
{
	if(cur_preset_id < 0 || !pvector)
		return -1;

	ONVIF_PTZPreset preset = g_onvif_cfg.profiles->presets[cur_preset_id];
	if (preset.UsedFlag == 0)	return -1;

    if (0 == preset.VectorListFlag)
    {
		*cnt = 0;
		return -1;
    }
	else
	{
		memcpy(pvector, preset.Vector_list, sizeof(onvif_VectorList)*VECTOR_LIST_LEN);
		*cnt = preset.Vector_Number;
	}

	if (*cnt <= 0)
		return -1;

	return 0;
}

int get_cur_preset_name(char* presetname, int length)
{
	if (cur_preset_id < 0 || !presetname)
		return -1;

	ONVIF_PTZPreset preset = g_onvif_cfg.profiles->presets[cur_preset_id];
	if (preset.UsedFlag == 0)	return -1;
	/*
    if (0 == preset.VectorListFlag)
    {
		return -1;
    }
	else*/
	{
	    if (strlen(preset.PTZPreset.Name) > 0)
		{
			memset(presetname, 0x0, length);
			memcpy(presetname, preset.PTZPreset.Name, strlen(preset.PTZPreset.Name));
			return 0;
	    }
	}

	return -1;
}

// 有可能swap同一变量，不能用异或版本 
void swap(int *a, int *b)
{
	int t = *a;
	*a = *b;
	*b = t;
}
/* 取随机预置位序号。不重复随机完所有预置位（即随机一个预置位序号后，再随机一个除去随机过的预置位序号，直到随机完所有预置位序号） */
void RandomSort(int a[], int n)
{
    for(int i=0; i<n; ++i)
    {
        int j = rand() % (n-i) + i;   //产生i到n-1间的随机数
		swap(&a[i], &a[j]);           //将随机选择到的预置位序号(a[j])交换到前面，后面会再从i到n-1间随机抽取预置位序号
    }
}

BOOL onvif_presettour_msg(int msgtype, void * msg_buf)
{
    OIMSG msg;
	memset(&msg,0,sizeof(OIMSG));
	
	msg.msg_src = msgtype;
	//msg.msg_dua = (char *)p_user;
	msg.msg_buf = (char *)msg_buf;
	
	if (hqBufPut(g_onvif_cls.msg_queue, (char *)&msg) == FALSE)
	{
		log_print(LOG_ERR, "%s, send rx msg to main task failed!!!\r\n", __FUNCTION__);

		return  FALSE;
	}

	return TRUE;
}

int onvif_VectorListHandle(ONVIF_PTZPreset *preset, int presetTourStatus, Gpt_SendJpegInfo *pSendInfo)
{
	int k;
	onvif_VectorList vectorData;
	float * IrTemperatureData = NULL;
	float ir_max_temp;

	memset(&vectorData, 0x0, sizeof(onvif_VectorList));

	for (k = 0; k < preset->Vector_Number; k++)
	{
	    //不在巡检不做处理
	    if (presetTourStatus != PTZPresetTourState_Touring)
			return 0;
		
	    //1:温度检测，2：数据识别
		if (preset->Vector_list[k].dulaType == 1)
		{
			// getVectorData(&vectorData, preset_idx, k);

			vectorData.temperature.Min = preset->Vector_list[k].temperature.Min;
			vectorData.temperature.Max = preset->Vector_list[k].temperature.Max;

			vectorData.x = ((preset->Vector_list[k].x)+1)/2*IR_WIDTH;
			vectorData.y = (1-(preset->Vector_list[k].y))/2*IR_HEIGHT;
			vectorData.w = (preset->Vector_list[k].w)/2*IR_WIDTH;
			vectorData.h = (preset->Vector_list[k].h)/2*IR_HEIGHT;

			// UTIL_INFO("\033[0;33mx=%.2f, y=%.2f, w=%.2f, h=%.2f\033[0m\n",		
			// 		preset.Vector_list[k].x, preset.Vector_list[k].y,
			// 		preset.Vector_list[k].w, preset.Vector_list[k].h);
			// UTIL_INFO("\033[0;33mvectorData.x=%.2f, vectorData.y=%.2f, vectorData.w=%.2f, vectorData.h=%.2f\033[0m\n",
			// 		vectorData.x, vectorData.y, vectorData.w, vectorData.h);
		
			//获取热成像数据
			IrTemperatureData = getIrTemperatureData();
#ifdef HI3519AV100
			//返回最大的温度值
			ir_max_temp =  get_max_ir_temp(IrTemperatureData, vectorData.x, vectorData.y, vectorData.w, vectorData.h);
#endif
		// UTIL_INFO("get_max_ir_temp = %.2f  vectorData.temperature.Max =%.2f, vectorData.temperature.Min = %.2f\n", ir_max_temp ,vectorData.temperature.Max, vectorData.temperature.Min);
			if (ir_max_temp > vectorData.temperature.Max || ir_max_temp < vectorData.temperature.Min)
			{
				sprintf(pSendInfo->eventdetail, "%s %0.2f", preset->PTZPreset.Name, ir_max_temp);
				/*IR模块图像抓拍*/ //第三参数：
				pSendInfo->eventtype = 1;
				pSendInfo->snaptype = MSG_VIDEO_IRMODESNAPJPEGPROCESS;
				pSendInfo->towhere = 0;//推送图片去哪个服务器，0:事件服务器地址，1:算法服务器地址
				onvif_presettour_msg(ONVIF_HTTPJPEGSEND_SRC, (void *)pSendInfo);
			}
		}
	}

	return 0;
}

void *PresetTour_Operation_Thread(void *args)
{
	ONVIF_PROFILE * p_profile = g_onvif_cfg.profiles;
	ONVIF_PresetTour *presetTours = (ONVIF_PresetTour *)args;
	if (presetTours == NULL)
	{
		goto __Exit;
    }
	
	prctl(PR_SET_NAME, (unsigned long)presetTours->PresetTour.token);
	
	uint32_t i = 0, j = 0, staytime = 0;
	int a[256] = {0};
	uint32_t preset_idx;		//实际的预置位
	uint16 zoomValue;
	uint16 zoomValue_prev;  	//上一个预置位的焦距
	Gpt_SendJpegInfo pSendInfo;
	onvif_PTZPresetTourDirection op;
    PTZ_PresetsTours_t * p_presetTour = NULL;
	
 	p_presetTour = onvif_find_PresetTour(presetTours->PresetTour.token); 
 	if (NULL == p_presetTour)
	{
 		goto __Exit;
 	}
	
	//关闭除此巡航之外的线程
	if (TRUE == presetTours->PresetTour.AutoStart && p_profile && (p_profile->token[0] != '\0'))
		onvif_Idle_OtherPresetTour(p_profile->token, presetTours->PresetTour.token);

	//向前按顺序巡航 		
	if (presetTours->PresetTour.StartingCondition.DirectionFlag
		&& (presetTours->PresetTour.StartingCondition.Direction == PTZPresetTourDirection_Forward))		
	{
		op = PTZPresetTourDirection_Forward;
	}
	//向后按顺序巡航
	else if (presetTours->PresetTour.StartingCondition.DirectionFlag
		&& (presetTours->PresetTour.StartingCondition.Direction == PTZPresetTourDirection_Backward))	
	{
		j = p_presetTour->PresetsTour.presetCount - 1;
		op = PTZPresetTourDirection_Backward;
	}
	//是否随机，true:1,FALSE:0，随机则方向将被忽略，并随机调用巡更的预设值
	if (presetTours->PresetTour.StartingCondition.RandomPresetOrderFlag 
		&& presetTours->PresetTour.StartingCondition.RandomPresetOrder)   
	{
		/* 将巡更的预置位序号保存到数组中，后面进行随机排序 */
		for (j = 0; j < p_presetTour->PresetsTour.presetCount; j++)
		{
			a[j] = j;    	 //巡更的预置位序号从0~n的
		}
		//将巡更的预置位序号进行随机排序
		RandomSort(a, p_presetTour->PresetsTour.presetCount); 
		op = PTZPresetTourDirection_Extended;
		i = 0;
	}
		
	presetTours->PresetTour.presettour_run = 1;
	UTIL_INFO("!!!PresetTour AUTO=%s direct=%d SATRT %s !!!", 
		presetTours->PresetTour.AutoStart?"TRUE":"FALSE", op, presetTours->PresetTour.token);

	while (1)
	{
		if (presetTours->PresetTour.Status.State == PTZPresetTourState_Idle)
		{
			UTIL_INFO("!!!PTZPresetTourState_Idle  %s !!!", presetTours->PresetTour.token);
			goto __Exit;
		}
		else if (presetTours->PresetTour.Status.State == PTZPresetTourState_Paused)
		{
			//UTIL_INFO("!!!PTZPresetTourOperation_Pause Pause Pause !!!");
			onvif_preset_usleep(1000*1000, presetTours->PresetTour.Status.State);
			continue;
		}

        if (op == PTZPresetTourDirection_Extended) {
			j = a[i];
        }
		
		preset_idx = p_presetTour->PresetsTour.presets[j].index;
		/* 实际预置位 */
  		ONVIF_PTZPreset preset = g_onvif_cfg.profiles->presets[preset_idx];  
		
		short location = preset_idx + 1;   //该ptz设备可以从0x00~0x3f设置，但好像从0设置不行，所以从1开始 (在SetPreset中+1，对应的，这里也要+1)
		gotoPtzPreset(location);  

		zoomValue = preset.zoomVal;

		//将转动到哪个预置位赋值给当前预置位变量
		cur_preset_id = preset_idx;

		/* 此处是为了能够可以处理焦距聚焦问题 */
		zoomValue_prev = get_zoom_val();   	 //获取相机焦距，也即上一个预置位的焦距
		if (zoomValue_prev > 50 && fabs(zoomValue - zoomValue_prev) < 10)  //如果该预置位的焦距与上一个预置位的焦距相差小于10
		{
			set_zoom(zoomValue - 50);
			onvif_preset_usleep(500*1000, presetTours->PresetTour.Status.State);
			//usleep(100*1000);
		}
		/*  */
		set_zoom(zoomValue);  

		staytime = (7 + p_presetTour->PresetsTour.presets[j].StayTime)*1000*1000;
		onvif_preset_usleep(staytime, presetTours->PresetTour.Status.State);

		/* 检测预置位画框标记物的温度,是否抓拍 */
		if (preset.VectorListFlag != 0)   //对应的预置位是否有画检测区域Vector，!=0代表有
		{
			onvif_VectorListHandle(&preset, presetTours->PresetTour.Status.State, &pSendInfo);
		}

		/* 可见光摄像图像抓拍 */
		pSendInfo.eventtype = 2;
		pSendInfo.snaptype = MSG_VIDEO_IPCSNAPJPEGPROCESS;
		pSendInfo.towhere = 1;
		strcpy(pSendInfo.eventdetail,  preset.PTZPreset.Name);
		onvif_presettour_msg(ONVIF_HTTPJPEGSEND_SRC, (void *)&pSendInfo);

		if (op == PTZPresetTourDirection_Extended) {
			if (i > p_presetTour->PresetsTour.presetCount - 1) {
				if (presetTours->PresetTour.AutoStart)
				{
					i = 0;
					onvif_preset_usleep(1000*1000, presetTours->PresetTour.Status.State);
					continue;
				}
				else 
				{
					//UTIL_INFO("PTZPresetTourDirection_Extended END %s !!!", presetTours->PresetTour.token);
					break;
				}
			}
			i += 1;
		}
		else if (op == PTZPresetTourDirection_Backward) {
			if (j <= 0) {
				if (presetTours->PresetTour.AutoStart)
				{
					j = p_presetTour->PresetsTour.presetCount-1;
					onvif_preset_usleep(1000*1000, presetTours->PresetTour.Status.State);
					continue;
				}
				else 
				{
					//UTIL_INFO("PTZPresetTourDirection_Backward END %s !!!", presetTours->PresetTour.token);
					break;
				}
			}
			j -= 1;
		}else if (op == PTZPresetTourDirection_Forward) {
			if (j > p_presetTour->PresetsTour.presetCount - 1) {
				if (presetTours->PresetTour.AutoStart)
				{
					j = 0;
					onvif_preset_usleep(1000*1000, presetTours->PresetTour.Status.State);
					continue;
				}
				else 
				{
					//UTIL_INFO("PTZPresetTourDirection_Forward END %s !!!", presetTours->PresetTour.token);
					break;
				}
			}
			j += 1;
		}

	}

__Exit:	
	presetTours->PresetTour.presettour_run = 0;
	presetTours->PresetTour.Status.State = PTZPresetTourState_Idle;
	UTIL_INFO("PresetTour direct=%d END %s!!!!!!!", op, presetTours->PresetTour.token);

	return NULL;
}

ONVIF_RET onvif_OperatePresetTour(OperatePresetTour_REQ * p_req)
{
	ONVIF_PROFILE * p_profile;
    ONVIF_PresetTour * p_PresetTour;
	int ret = ONVIF_OK;
    
	p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}
		
	p_PresetTour = onvif_find_PTZPresetTour(p_req->ProfileToken, p_req->PresetTourToken);
    if (NULL == p_PresetTour)
    {
        return ONVIF_ERR_OTHER;
    }
	
	switch (p_req->Operation)
	{
		case PTZPresetTourOperation_Start:

			if (p_PresetTour->PresetTour.Status.State == PTZPresetTourState_Paused)
			{
				p_PresetTour->PresetTour.Status.State = PTZPresetTourState_Touring;
				UTIL_INFO("PAUSE!!!!!!!!!RESTART!!!!!!!!!!!");
				break;
			}

			if (p_PresetTour->PresetTour.Status.State == PTZPresetTourState_Touring)
			{
				UTIL_INFO("PTZPresetTourState_Touring!!!!!!!!!!!");
				//如果是定制巡航那么强制设置成主动巡航
				if (FALSE == p_PresetTour->PresetTour.AutoStart)
				{
					p_PresetTour->PresetTour.AutoStart = TRUE;
				}
				break;
			}
			
			//自动巡航
			onvif_presettour_msg(ONVIF_AUTOPRESETTOUR_SRC, p_PresetTour);
			break;

		case PTZPresetTourOperation_Stop:
			p_PresetTour->PresetTour.Status.State = PTZPresetTourState_Idle;    /* 停止即是让巡航处于空闲状态 */
			ptzStop();
			break;

		case PTZPresetTourOperation_Pause:
			p_PresetTour->PresetTour.Status.State = PTZPresetTourState_Paused;
			ptzStop();
			break;

		case PTZPresetTourOperation_Extended:
			break;
	}

    return ONVIF_OK;
}

ONVIF_RET onvif_RemovePresetTour(PresetTour_REQ * p_req)
{
	ONVIF_PROFILE * p_profile;
    ONVIF_PresetTour * p_PresetTour;
    
	p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}

    p_PresetTour = onvif_find_PTZPresetTour(p_req->ProfileToken, p_req->PresetTourToken); 
    if (NULL == p_PresetTour)
    {
        return ONVIF_ERR_OTHER;
    }

	//判断巡航有没有处于闲置中，没有处于闲置则不能删除操作
	if ((p_PresetTour->PresetTour.Status.State != PTZPresetTourState_Idle) || 
		(1 == p_PresetTour->PresetTour.presettour_run))
	{
		p_PresetTour->PresetTour.Status.State == PTZPresetTourState_Idle;
	}
	
	onvif_remove_PresetTour(&p_profile->PresetTours, p_PresetTour);

    // todo : add Remove Preset Tour code ...
	PTZ_PresetsTours_t * p_presetTour = NULL;
	p_presetTour = onvif_find_PresetTour(p_req->PresetTourToken); 
	if (NULL == p_presetTour)
	{
		return ONVIF_ERR_OTHER;
	}

	memset(p_presetTour, 0, sizeof(PTZ_PresetsTours_t));

	//删掉某个巡更后,更新保存起来
	if (writePtzPresetTour(PTZPresetsTour, MAX_PRESETS_TOUR) != 0)
	{
		printf("onvif_RemovePresetTour | writePtzPresetTour faile...\n");
	}

    return ONVIF_OK;
}

ONVIF_RET onvif_CreatePresetTour(PresetTour_REQ * p_req)
{
	ONVIF_PROFILE * p_profile;

	p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}
	else if (NULL == g_onvif_cfg.ptz_node)
	{
		return ONVIF_ERR_NoPTZProfile;
	}
	PTZ_PresetsTours_t * PTZ_PresetsTour = onvif_get_idle_PresetTour(&g_onvif_cls.preset_tour_idx);
	if (NULL == PTZ_PresetsTour)
    {
    	return ONVIF_ERR_OTHER;
    }
	
	memset(p_req->PresetTourToken, 0x0, sizeof(p_req->PresetTourToken));
	snprintf(p_req->PresetTourToken, sizeof(p_req->PresetTourToken)-1,
    			"PRESET_TOUR_%d", g_onvif_cls.preset_tour_idx);
	PTZ_PresetsTour->UsedFlag = 0;
	memset(PTZ_PresetsTour->PresetTourToken, 0x0, sizeof(PTZ_PresetsTour->PresetTourToken));
    snprintf(PTZ_PresetsTour->PresetTourToken, sizeof(PTZ_PresetsTour->PresetTourToken)-1,
				"PRESET_TOUR_%d", g_onvif_cls.preset_tour_idx);
	//UTIL_INFO("idle PresetTour idx=%d,token=%s!!!!", g_onvif_cls.preset_tour_idx, PTZ_PresetsTour->PresetTourToken );

	return ONVIF_OK;
}

ONVIF_RET onvif_ModifyPresetTour(ModifyPresetTour_REQ * p_req)
{	
	ONVIF_PROFILE * p_profile;
    ONVIF_PresetTour * p_PresetTour;
	ONVIF_PresetTour * p_tmp;
	ONVIF_PresetTour * p_prev;  
	ONVIF_RET onvifret = -1;
 	ONVIF_PresetTour * PresetTour_p = NULL;
    PTZ_PresetsTours_t * p_presetTour = NULL;
	
	p_profile = onvif_find_profile(p_req->ProfileToken);
	if (NULL == p_profile)
	{
		onvifret = ONVIF_ERR_NoProfile;
		goto __EXIT;
	}

	if (NULL == g_onvif_cfg.ptz_node)
	{
		onvifret = ONVIF_ERR_NoPTZProfile;
		goto __EXIT;
	}

 	//根据token获取对应preset_tour   
 	p_presetTour = onvif_find_PresetTour(p_req->PresetTour_req->PresetTour.token);    //&PTZPresetsTour[i]
 	if (NULL == p_presetTour)
	{
 		onvifret = ONVIF_ERR_OTHER;
		goto __EXIT;
 	}
	
	if (p_presetTour->UsedFlag == 0)   //说明是要新创建的巡更
	{
		PresetTour_p = onvif_add_PresetTour(&p_profile->PresetTours); 
		if (PresetTour_p)
		{
			strcpy(PresetTour_p->PresetTour.token, p_presetTour->PresetTourToken);
			if (p_req->PresetTour_req->PresetTour.StartingCondition.PresetTourTimerFlag)   //是否有设置巡航定时器
			{
				ONVIF_PTZPresetTourTimer * p_tourTimer = onvif_add_Timer(&PresetTour_p->PresetTour.StartingCondition.Timer);
				if (NULL == p_tourTimer)
				{
					free(PresetTour_p);
					PresetTour_p = NULL;
					onvifret = ONVIF_ERR_OTHER;
					goto __EXIT;
				}
			}

			ONVIF_PTZPresetTourSpot * p_tour_spot = onvif_add_TourSpot(&PresetTour_p->PresetTour.TourSpot);
			if (NULL == p_tour_spot)
			{
				free(PresetTour_p);
				PresetTour_p = NULL;
				onvifret = ONVIF_ERR_OTHER;
				goto __EXIT;
			}
		}
	}

	p_tmp = p_req->PresetTour_req;
	p_PresetTour = onvif_find_PTZPresetTour(p_req->ProfileToken, p_tmp->PresetTour.token);
	if (NULL == p_PresetTour)
	{
		if (p_tmp->PresetTour.StartingCondition.PresetTourTimerFlag)   //是否有设置巡航定时器
		{
			onvif_free_Timers(&p_tmp->PresetTour.StartingCondition.Timer);
		}
		onvif_free_TourSpots(&(p_tmp->PresetTour.TourSpot));
		free(p_tmp);
		p_tmp = NULL;
 		onvifret = ONVIF_ERR_OTHER;
		goto __EXIT;
	}

	p_prev = onvif_get_prev_presetTour(&p_profile->PresetTours, p_PresetTour);
	if (NULL == p_prev)
	{
		p_profile->PresetTours = p_tmp;
		p_tmp->next = p_PresetTour->next;
	}
	else
	{
		p_prev->next = p_tmp;
		p_tmp->next = p_PresetTour->next;
	}
	
	if (p_PresetTour->PresetTour.StartingCondition.PresetTourTimerFlag)   //是否有设置巡航定时器
	{
		onvif_free_Timers(&p_PresetTour->PresetTour.StartingCondition.Timer);
	}
	onvif_free_TourSpots(&(p_PresetTour->PresetTour.TourSpot));
	free(p_PresetTour);


	//todo: 以下赋值为了将数据保存于文件
	p_presetTour->UsedFlag = 1;
	strcpy(p_presetTour->Name, p_req->PresetTour_req->PresetTour.Name);

	onvif_PTZPresetTourStartingCondition  startingCondition;
	startingCondition = p_req->PresetTour_req->PresetTour.StartingCondition;

	if (startingCondition.RecurringTimeFlag){
		p_presetTour->PresetsTour.runNumberFlag = 1;
		p_presetTour->PresetsTour.runNumber = startingCondition.RecurringTime;
	}
	if (startingCondition.RecurringDurationFlag){
		p_presetTour->PresetsTour.runTimeFlag = 1;
		p_presetTour->PresetsTour.runTime   = startingCondition.RecurringDuration;
	}
	if (startingCondition.DirectionFlag){    //与RandomPresetOrder互斥
		p_presetTour->PresetsTour.direction = startingCondition.Direction;
	}
	if (startingCondition.RandomPresetOrderFlag){   //与Direction互斥
		p_presetTour->PresetsTour.RandomOrder = startingCondition.RandomPresetOrder;
	}

	//扩展定时巡更
	if (startingCondition.PresetTourTimerFlag)
	{
		p_presetTour->PresetsTour.TimerFlag = 1;
	}

	uint32_t j = 0;
	ONVIF_PTZPresetTourTimer * p_PresetTourTimer = startingCondition.Timer;
	while (p_PresetTourTimer)
	{
		p_presetTour->PresetsTour.Timer[j].Enabled = p_PresetTourTimer->timer.Enabled;;

		if (p_PresetTourTimer->timer.IntervalMinutesFlag){
			p_presetTour->PresetsTour.Timer[j].IntervalMinutesFlag = 1;
			p_presetTour->PresetsTour.Timer[j].IntervalMinutes = p_PresetTourTimer->timer.IntervalMinutes;
		}
		
		if (p_PresetTourTimer->timer.UTCDateTimeFlag){
			p_presetTour->PresetsTour.Timer[j].UTCDateTimeFlag = 1;
			p_presetTour->PresetsTour.Timer[j].UTCDateTime.Date.Year = p_PresetTourTimer->timer.UTCDateTime.Date.Year;
			p_presetTour->PresetsTour.Timer[j].UTCDateTime.Date.Month = p_PresetTourTimer->timer.UTCDateTime.Date.Month;
			p_presetTour->PresetsTour.Timer[j].UTCDateTime.Date.Day = p_PresetTourTimer->timer.UTCDateTime.Date.Day;
			p_presetTour->PresetsTour.Timer[j].UTCDateTime.Time.Hour = p_PresetTourTimer->timer.UTCDateTime.Time.Hour;
			p_presetTour->PresetsTour.Timer[j].UTCDateTime.Time.Minute = p_PresetTourTimer->timer.UTCDateTime.Time.Minute;
			p_presetTour->PresetsTour.Timer[j].UTCDateTime.Time.Second = p_PresetTourTimer->timer.UTCDateTime.Time.Second;
		}
		j++;	//巡更的定时器数量

		p_PresetTourTimer = p_PresetTourTimer->next;
	}
	p_presetTour->PresetsTour.timerCount = j;


	uint32_t i = 0;
	ONVIF_PTZPresetTourSpot * p_TourSpot = p_req->PresetTour_req->PresetTour.TourSpot;
	while (p_TourSpot)
	{
		p_presetTour->PresetsTour.presets[i].index = onvif_find_PTZPreset_index(p_req->ProfileToken, p_TourSpot->PTZPresetTourSpot.PresetDetail.PresetToken);
		p_presetTour->PresetsTour.presets[i].zoomValue = p_profile->presets[p_presetTour->PresetsTour.presets[i].index].zoomVal;
		p_presetTour->PresetsTour.presets[i].StayTime = p_TourSpot->PTZPresetTourSpot.StayTime;
		strcpy(p_presetTour->PresetsTour.presets[i].PresetToken, p_TourSpot->PTZPresetTourSpot.PresetDetail.PresetToken);
		i++;    //巡更的预置位数量

		p_TourSpot = p_TourSpot->next;
	}
	p_presetTour->PresetsTour.presetCount = i;
	
	//更新某个巡更后,将所有巡更保存起来
	if (writePtzPresetTour(PTZPresetsTour, MAX_PRESETS_TOUR) != 0)
	{
		UTIL_ERR("writePtzPresetTour failed...");
 		onvifret = ONVIF_ERR_OTHER;
		goto __EXIT;
	}
	onvifret = ONVIF_OK;
__EXIT:
	return onvifret;
}

static int  onvif_Compare_Time(onvif_PTZPresetTourTimer *pTourTimer, struct tm *tm_t)
{
	int     IntervalMinutes = 0;
	int 	CurMinutes = 0;
	int 	PrevMinutes = 0;
	
	CurMinutes = tm_t->tm_hour*60 + tm_t->tm_min;
	PrevMinutes = pTourTimer->UTCDateTime.Time.Hour*60 + pTourTimer->UTCDateTime.Time.Minute;
	IntervalMinutes = CurMinutes - PrevMinutes;

    //还没到达指定巡航时间
	if (IntervalMinutes <= 0) 
	{
		if (0 == IntervalMinutes)
		{
			return 0;
		}
	}
	else if ((IntervalMinutes > 0) && (1 == pTourTimer->IntervalMinutesFlag) && 
		(pTourTimer->IntervalMinutes > 1)) 
	{
		//UTIL_INFO("PrevMinutes(%d)CurMinutes(%d)=%d,%d,%d!!!!!!!!!!",PrevMinutes,CurMinutes,IntervalMinutes,pTourTimer->IntervalMinutes, IntervalMinutes%pTourTimer->IntervalMinutes);
		if (0 == IntervalMinutes%pTourTimer->IntervalMinutes)
		{
			return 0;
		}
	}
	return -1;
}

static void *PresetTour_Task(void *args)
{
	prctl(PR_SET_NAME, (unsigned long)"PresetTour_Task");

    ONVIF_PROFILE * p_profile = g_onvif_cfg.profiles;
	int index = 0, ret;
	ONVIF_PresetTour * p_PresetTour = NULL;
	ONVIF_PTZPresetTourTimer *pTourTimer = NULL;
	struct tm NowTime; 
	struct timeval structTimeSpec;
	int viscastatus = 0;
	int transformMode = 0;

	while(1)
	{
		if (0 == get_visca_status())
		{
			usleep(1000*1000LL);
			continue;
		}
		
		if (0 == viscastatus)
		{
			viscastatus = 1;
			transformMode = g_onvif_cfg.ImagingSettings.VideoTransformMode.TransformMode;
			if (transformMode == VideoTransformMode1 || transformMode == VideoTransformMode3){	//翻转
				set_img_flip(0);
				set_img_mirror(0);
			}else{
				set_img_flip(1);
				set_img_mirror(1);				
			}
			//在visca正常的情况下保持2分钟
			sleep(120);
		}
		
		for (index = 0; index < MAX_PRESETS_TOUR; index++)
		{
			if ( PTZPresetsTour[index].UsedFlag == 0 )    //如果没有该巡更，跳过该巡更，继续
			{
			    usleep(1000*1000LL);
				continue;
			}
		    if (p_profile && p_profile->token[0] != '\0')
		    {
				p_PresetTour = onvif_find_PTZPresetTour(p_profile->token, PTZPresetsTour[index].PresetTourToken);
				if (NULL == p_PresetTour)
				{
					usleep(1000*1000LL);
					continue;
				}
				

				//查询是否有巡航
				if (ONVIF_OK == onvif_IsIdle_PTZPresetTour(p_profile->token))
				{
					usleep(1000*1000LL);
					continue;
				}
				
				gettimeofday(&structTimeSpec, NULL);
    			localtime_r(&structTimeSpec.tv_sec, &NowTime);
				
				pTourTimer = p_PresetTour->PresetTour.StartingCondition.Timer;

				while (pTourTimer)
				{
					//开启定时巡航操作
					if (pTourTimer->timer.Enabled && 
					   (p_PresetTour->PresetTour.Status.State == PTZPresetTourState_Idle))
					{
						if (0 == onvif_Compare_Time(&pTourTimer->timer, &NowTime))
						{
						    //定制巡航
							onvif_presettour_msg(ONVIF_PRESETTOUR_SRC, p_PresetTour);
							//休眠2分钟
							sleep(2*60);
						}
					}
					pTourTimer = pTourTimer->next;
					usleep(1000*1000LL);
					continue;
				}
				usleep(1000*1000LL);
				continue;
		    }
			usleep(1000*1000LL);
			continue;
		}
	}
	return NULL;
}

ONVIF_RET onvif_presettour_operation(ONVIF_PresetTour * presetTours, BOOL AutoStart)
{
	int ret = ONVIF_OK;
	
	if (1 == presetTours->PresetTour.presettour_run || !presetTours) {
		return ret;
	}
	
	presetTours->PresetTour.AutoStart = AutoStart;
	presetTours->PresetTour.Status.State = PTZPresetTourState_Touring;
    presetTours->PresetTour.presettour_run = 0;
	presetTours->PresetTour.presettour_tid = sys_os_create_thread((void *)PresetTour_Operation_Thread, presetTours);
	if (0 == presetTours->PresetTour.presettour_tid)
    {
		presetTours->PresetTour.Status.State = PTZPresetTourState_Idle;
		presetTours->PresetTour.AutoStart = FALSE;
		presetTours->PresetTour.presettour_run = 0;
    }
	return ret;
}

int onvif_presettour_build()
{
    g_onvif_cls.presettoursch_tid = sys_os_create_thread((void *)PresetTour_Task, NULL);

}
/* add PresetTour end */

#endif // PTZ_SUPPORT


