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

#include "sys_inc.h"
#include "onvif.h"
#include "onvif_cm.h"
#include "onvif_cfg.h"
#include "xml_node.h"
#include "onvif_utils.h"

#include "set_config.h"		// add by xieqingpu


/***************************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;


/***************************************************************************************/
void onvif_parse_information_cfg(XMLN * p_node)
{
	XMLN * p_Manufacturer;
	XMLN * p_Model;
	XMLN * p_FirmwareVersion;
	XMLN * p_SerialNumber;
	XMLN * p_HardwareId;

	p_Manufacturer = xml_node_get(p_node, "Manufacturer");
	if (p_Manufacturer && p_Manufacturer->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.Manufacturer, p_Manufacturer->data, sizeof(g_onvif_cfg.DeviceInformation.Manufacturer)-1);
	}

	p_Model = xml_node_get(p_node, "Model");
	if (p_Model && p_Model->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.Model, p_Model->data, sizeof(g_onvif_cfg.DeviceInformation.Model)-1);
	}
	
	p_FirmwareVersion = xml_node_get(p_node, "FirmwareVersion");
	if (p_FirmwareVersion && p_FirmwareVersion->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.FirmwareVersion, p_FirmwareVersion->data, sizeof(g_onvif_cfg.DeviceInformation.FirmwareVersion)-1);
	}

	p_SerialNumber = xml_node_get(p_node, "SerialNumber");
	if (p_SerialNumber && p_SerialNumber->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.SerialNumber, p_SerialNumber->data, sizeof(g_onvif_cfg.DeviceInformation.SerialNumber)-1);
	}

	p_HardwareId = xml_node_get(p_node, "HardwareId");
	if (p_HardwareId && p_HardwareId->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.HardwareId, p_HardwareId->data, sizeof(g_onvif_cfg.DeviceInformation.HardwareId));
	}
}

////// add by xieqingpu
void onvif_set_devinfo(CONFIG_Information * p_devInfo)
{
	if (p_devInfo->manufacturer[0] != '\0'){
		strncpy(g_onvif_cfg.DeviceInformation.Manufacturer, p_devInfo->manufacturer, sizeof(g_onvif_cfg.DeviceInformation.Manufacturer)-1);
	}

	if (p_devInfo->model[0] != '\0'){
		strncpy(g_onvif_cfg.DeviceInformation.Model, p_devInfo->model, sizeof(g_onvif_cfg.DeviceInformation.Model)-1);
	}

	if (p_devInfo->firmware_version[0] != '\0'){
		strncpy(g_onvif_cfg.DeviceInformation.FirmwareVersion, p_devInfo->firmware_version, sizeof(g_onvif_cfg.DeviceInformation.FirmwareVersion)-1);
	}

	if (p_devInfo->serial_number[0] != '\0'){
		strncpy(g_onvif_cfg.DeviceInformation.SerialNumber, p_devInfo->serial_number, sizeof(g_onvif_cfg.DeviceInformation.SerialNumber)-1);
	}

	if (p_devInfo->hardware_id[0] != '\0'){
		strncpy(g_onvif_cfg.DeviceInformation.HardwareId ,p_devInfo->hardware_id, sizeof(g_onvif_cfg.DeviceInformation.HardwareId)-1);
	}
}

/////

BOOL onvif_parse_user(XMLN * p_node, onvif_User * p_user)
{
	XMLN * p_username;
    XMLN * p_password;
    XMLN * p_userlevel;
    
	p_username = xml_node_get(p_node, "username");
	if (p_username && p_username->data)
	{
		strncpy(p_user->Username, p_username->data, sizeof(p_user->Username)-1);
	}
	else
	{
		return FALSE;
	}

	p_password = xml_node_get(p_node, "password");
	if (p_password && p_password->data)
	{
		strncpy(p_user->Password, p_password->data, sizeof(p_user->Password)-1);
	}

	p_userlevel = xml_node_get(p_node, "userlevel");
	if (p_userlevel && p_userlevel->data)
	{
		p_user->UserLevel = onvif_StringToUserLevel(p_userlevel->data);
	}

	return TRUE;
}

/* void onvif_parse_h264_options(XMLN * p_video_encoder, ONVIF_VideoEncoder2Configuration * p_v_enc_cfg)
{
	XMLN * p_h264;
	XMLN * p_gov_length;
	XMLN * p_h264_profile;

	p_h264 = xml_node_get(p_video_encoder, "h264");
	if (NULL == p_h264)
	{
		return;
	}
	
	p_gov_length = xml_node_get(p_h264, "gov_length");
	if (p_gov_length && p_gov_length->data)
	{
	    p_v_enc_cfg->Configuration.GovLengthFlag = 1;
		p_v_enc_cfg->Configuration.GovLength = atoi(p_gov_length->data);
	}

	p_h264_profile = xml_node_get(p_h264, "h264_profile");
	if (p_h264_profile && p_h264_profile->data)
	{
	    p_v_enc_cfg->Configuration.ProfileFlag = 1;
		strncpy(p_v_enc_cfg->Configuration.Profile, p_h264_profile->data, sizeof(p_v_enc_cfg->Configuration.Profile)-1);
	}
} */

void onvif_parse_h264_options(Encoding_profile * p_encoding_profile, ONVIF_VideoEncoder2Configuration * p_v_enc_cfg)
{	
	// printf("xxx onvif_parse_h264_option | gov_length关键帧=%d, encode_profile编码级别=%d.(0:Baseline 1:Main)\n", p_encoding_profile->gov_length,p_encoding_profile->encode_profile);
	
	if(p_encoding_profile->gov_length){
		p_v_enc_cfg->Configuration.GovLengthFlag = 1;
		p_v_enc_cfg->Configuration.GovLength = p_encoding_profile->gov_length;  //关键帧
	}

	if (p_encoding_profile->encode_profile >= 0){
		p_v_enc_cfg->Configuration.ProfileFlag = 1;						//编码级别
		strncpy(p_v_enc_cfg->Configuration.Profile, onvif_H264ProfileToString(p_encoding_profile->encode_profile), sizeof(p_v_enc_cfg->Configuration.Profile)-1);
	}
}

#ifdef MEDIA2_SUPPORT
void onvif_parse_h265_options(Encoding_profile * p_encoding_profile, ONVIF_VideoEncoder2Configuration * p_v_enc_cfg)
{	
	//printf("xxx onvif_parse_h265_option | gov_length关键帧=%d, encode_profile编码级别=%s.\n", p_encoding_profile->gov_length,p_encoding_profile->encode_profile);
	
	if(p_encoding_profile->gov_length){
		p_v_enc_cfg->Configuration.GovLengthFlag = 1;
		p_v_enc_cfg->Configuration.GovLength = p_encoding_profile->gov_length;  //关键帧
	}

	if (p_encoding_profile->encode_profile >= 0){
		p_v_enc_cfg->Configuration.ProfileFlag = 1;						//编码级别
		strncpy(p_v_enc_cfg->Configuration.Profile, onvif_H264ProfileToString(p_encoding_profile->encode_profile), sizeof(p_v_enc_cfg->Configuration.Profile)-1);
	}
}
#endif

void onvif_parse_mpeg4_options(Encoding_profile * p_encoding_profile, ONVIF_VideoEncoder2Configuration * p_v_enc_cfg)
{	
	//printf("xxx onvif_parse_mpeg4_option | gov_length关键帧=%d, encode_profile编码级别=%s.\n", p_encoding_profile->gov_length,p_encoding_profile->encode_profile);
	
	if(p_encoding_profile->gov_length){
		p_v_enc_cfg->Configuration.GovLengthFlag = 1;
		p_v_enc_cfg->Configuration.GovLength = p_encoding_profile->gov_length;  //关键帧
	}

	if (p_encoding_profile->encode_profile >= 0){
		p_v_enc_cfg->Configuration.ProfileFlag = 1;						//编码级别
		strncpy(p_v_enc_cfg->Configuration.Profile, onvif_Mpeg4ProfileToString(p_encoding_profile->encode_profile), sizeof(p_v_enc_cfg->Configuration.Profile)-1);
	}
}

ONVIF_VideoSourceConfiguration * onvif_parse_video_source_cfg()
{
	int w = 0, h = 0;
	ONVIF_VideoSourceConfiguration * p_v_src_cfg = NULL;
	
	onvif_VideoResolution videoSource;
	videoSource.Width = 1280;
	videoSource.Height = 720;

	w = videoSource.Width;
	h = videoSource.Height;

	if (w == 0 || h == 0)
	{
		return NULL;
	}
	
	p_v_src_cfg = onvif_find_VideoSourceConfiguration_by_size(w, h);
	if (p_v_src_cfg)
	{
		return p_v_src_cfg;
	}

	p_v_src_cfg = onvif_add_VideoSourceConfiguration(w, h);

	return p_v_src_cfg;
}

// ONVIF_VideoEncoder2Configuration * onvif_parse_video_encoder_cfg(XMLN * p_video_encoder)
ONVIF_VideoEncoder2Configuration * onvif_parse_video_encoder_cfg()
{
    ONVIF_VideoEncoder2Configuration * p_v_enc_cfg;
	ONVIF_VideoEncoder2Configuration v_enc_cfg;
	
	memset(&v_enc_cfg, 0, sizeof(v_enc_cfg));

	//
	Video_Encoder encoder;
	memset(&encoder, 0, sizeof(encoder));

	if( getVideoEncoder(&encoder) !=0 )		//读取 视频编码器参数
		printf("get Video Encoder para faile.\n");

	//printf("xxx 1.getVideoEncoder | encoder:width=%d,height=%d, bitrate_limit=%d, framerate帧率=%d, gov关键帧=%d, 编码级别=%d(0:Baseline 1:Main)\n",
			// encoder.width,encoder.height,
			// encoder.bitrate_limit,
			// encoder.framerate,
			// encoder.video_encoding.v_encoding_profile.gov_length,
			// encoder.video_encoding.v_encoding_profile.encode_profile);
	//printf("xxx 2.getVideoEncoder | encoder.video_encoding.v_encoding = %s xxx\n",encoder.video_encoding.v_encoding);

	// p_width = xml_node_get(p_video_encoder, "width");
	// if (p_width && p_width->data)
	if (encoder.width){
		// v_enc_cfg.Configuration.Resolution.Width = atoi(p_width->data);		
		v_enc_cfg.Configuration.Resolution.Width = encoder.width;	//分辨率 宽
	}
	if (encoder.height){
		v_enc_cfg.Configuration.Resolution.Height = encoder.height;	 //分辨率 高
	}

	if (encoder.quality){
		v_enc_cfg.Configuration.Quality = (float)(encoder.quality);
	}		

	if (encoder.session_timeout){
		v_enc_cfg.Configuration.SessionTimeout = encoder.session_timeout; 
	}

	v_enc_cfg.Configuration.RateControlFlag = 1;
	
	if (encoder.framerate){
		v_enc_cfg.Configuration.RateControl.FrameRateLimit = encoder.framerate; //帧率
	}

	if (encoder.encoding_interval){
		v_enc_cfg.Configuration.RateControl.EncodingInterval = encoder.encoding_interval;
	}

	if (encoder.bitrate_limit){
		v_enc_cfg.Configuration.RateControl.BitrateLimit = encoder.bitrate_limit; //码率
	}

	if (encoder.video_encoding.v_encoding){
		strncpy(v_enc_cfg.Configuration.Encoding, encoder.video_encoding.v_encoding, sizeof(v_enc_cfg.Configuration.Encoding)-1);
		v_enc_cfg.Configuration.VideoEncoding = onvif_StringToVideoEncoding(encoder.video_encoding.v_encoding);
	}

 //printf("xxx xml_node_get(p_video_encoder, encoding )/v_enc_cfg.Configuration.Encoding = %s \n",v_enc_cfg.Configuration.Encoding);

	if (strcasecmp(v_enc_cfg.Configuration.Encoding, "MPEG4") == 0 || 
	    strcasecmp(v_enc_cfg.Configuration.Encoding, "MPV4-ES") == 0)
	{
	    strcpy(v_enc_cfg.Configuration.Encoding, "MPV4-ES");
	    v_enc_cfg.Configuration.VideoEncoding = VideoEncoding_MPEG4;
	    
		// onvif_parse_mpeg4_options(p_video_encoder, &v_enc_cfg);
		onvif_parse_mpeg4_options(&encoder.video_encoding.v_encoding_profile, &v_enc_cfg); //修改的
	}
	else if (strcasecmp(v_enc_cfg.Configuration.Encoding, "H264") == 0)
	{
		// onvif_parse_h264_options(p_video_encoder, &v_enc_cfg);
		onvif_parse_h264_options(&encoder.video_encoding.v_encoding_profile, &v_enc_cfg);  //修改的
	}
#ifdef MEDIA2_SUPPORT	
	else if (strcasecmp(v_enc_cfg.Configuration.Encoding, "H265") == 0)
	{
		// onvif_parse_h265_options(p_video_encoder, &v_enc_cfg);
		onvif_parse_h265_options(&encoder.video_encoding.v_encoding_profile, &v_enc_cfg);  //修改的
	}	
#endif
    else if (strcasecmp(v_enc_cfg.Configuration.Encoding, "JPEG"))
	{
	    return NULL;   
	}

	p_v_enc_cfg = onvif_find_VideoEncoderConfiguration_by_param(&v_enc_cfg);
	if (p_v_enc_cfg)
	{
		return p_v_enc_cfg;
	}

	p_v_enc_cfg = onvif_add_VideoEncoderConfiguration(&v_enc_cfg);

	return p_v_enc_cfg;
}

#ifdef AUDIO_SUPPORT

// ONVIF_AudioSourceConfiguration * onvif_parse_audio_source_cfg(XMLN * p_audio_source)
ONVIF_AudioSourceConfiguration * onvif_parse_audio_source_cfg()
{		
	ONVIF_AudioSourceConfiguration * p_a_src_cfg = g_onvif_cfg.a_src_cfg;
	if (p_a_src_cfg)
	{
		return p_a_src_cfg;
	}

	p_a_src_cfg = onvif_add_AudioSourceConfiguration();

	return p_a_src_cfg;
}

ONVIF_AudioEncoder2Configuration * onvif_parse_audio_encoder_cfg(XMLN * p_audio_encoder)
{
    XMLN * p_session_timeout;
    XMLN * p_sample_rate;
    XMLN * p_bitrate;
    XMLN * p_encoding;
    ONVIF_AudioEncoder2Configuration * p_a_enc_cfg;    
	ONVIF_AudioEncoder2Configuration a_enc_cfg;
	
	memset(&a_enc_cfg, 0, sizeof(ONVIF_AudioEncoder2Configuration));
	
	p_session_timeout = xml_node_get(p_audio_encoder, "session_timeout");
	if (p_session_timeout && p_session_timeout->data)
	{
		a_enc_cfg.Configuration.SessionTimeout = atoi(p_session_timeout->data);
	}

	p_sample_rate = xml_node_get(p_audio_encoder, "sample_rate");
	if (p_sample_rate && p_sample_rate->data)
	{
		a_enc_cfg.Configuration.SampleRate = atoi(p_sample_rate->data);
	}

	p_bitrate = xml_node_get(p_audio_encoder, "bitrate");
	if (p_bitrate && p_bitrate->data)
	{
		a_enc_cfg.Configuration.Bitrate = atoi(p_bitrate->data);
	}	

	p_encoding = xml_node_get(p_audio_encoder, "encoding");
	if (p_encoding && p_encoding->data)
	{
		strncpy(a_enc_cfg.Configuration.Encoding, p_encoding->data, sizeof(a_enc_cfg.Configuration.Encoding)-1);
		a_enc_cfg.Configuration.AudioEncoding = onvif_StringToAudioEncoding(p_encoding->data);
	}

#ifdef MEDIA2_SUPPORT
    if (strcasecmp(a_enc_cfg.Configuration.Encoding, "PCMU") == 0 || 
	    strcasecmp(a_enc_cfg.Configuration.Encoding, "G711") == 0)
	{
	    strcpy(a_enc_cfg.Configuration.Encoding, "PCMU");
	    a_enc_cfg.Configuration.AudioEncoding = AudioEncoding_G711;
	}
	else if (strcasecmp(a_enc_cfg.Configuration.Encoding, "AAC") == 0 || 
	    strcasecmp(a_enc_cfg.Configuration.Encoding, "MP4A-LATM") == 0)
	{
	    strcpy(a_enc_cfg.Configuration.Encoding, "MP4A-LATM");
	    a_enc_cfg.Configuration.AudioEncoding = AudioEncoding_AAC;
	}
#endif

	p_a_enc_cfg = onvif_find_AudioEncoderConfiguration_by_param(&a_enc_cfg);
	if (p_a_enc_cfg)
	{
		return p_a_enc_cfg;
	}

	p_a_enc_cfg = onvif_add_AudioEncoderConfiguration(&a_enc_cfg);

	return p_a_enc_cfg;
}
// ONVIF_AudioEncoder2Configuration * onvif_parse_audio_encoder_cfg(XMLN * p_audio_encoder)
ONVIF_AudioEncoder2Configuration * onvif_parse_audio_encoder_cfg_t()
{
    ONVIF_AudioEncoder2Configuration * p_a_enc_cfg;    
	ONVIF_AudioEncoder2Configuration a_enc_cfg;
	
	memset(&a_enc_cfg, 0, sizeof(ONVIF_AudioEncoder2Configuration));

	Audio_Encoder audio_encoder;
	memset(&audio_encoder, 0, sizeof(audio_encoder));
	if( getAudioEncoder(&audio_encoder) !=0 )	  //读取音频编码器参数
		printf("get Camera Encoder para faile.\n");

	//printf("xxx 1.getAudioEncode | sample_rate采样率 = %d,bitrate码率 = %d,a_encoding编码 = %s\n",
			// audio_encoder.sample_rate, audio_encoder.bitrate, audio_encoder.a_encoding);

	
	if (audio_encoder.session_timeout){
		// a_enc_cfg.Configuration.SessionTimeout = atoi(p_session_timeout->data);
		a_enc_cfg.Configuration.SessionTimeout = audio_encoder.session_timeout;
	}

	if (audio_encoder.sample_rate){
		a_enc_cfg.Configuration.SampleRate = audio_encoder.sample_rate;
	}

	if (audio_encoder.bitrate){
		a_enc_cfg.Configuration.Bitrate = audio_encoder.bitrate;
	}	

	// p_encoding = xml_node_get(p_audio_encoder, "encoding");
	// if (p_encoding && p_encoding->data)
	if (audio_encoder.a_encoding[0] != '\0')
	{
		strncpy(a_enc_cfg.Configuration.Encoding, audio_encoder.a_encoding, sizeof(a_enc_cfg.Configuration.Encoding)-1);
		a_enc_cfg.Configuration.AudioEncoding = onvif_StringToAudioEncoding(audio_encoder.a_encoding);
	//    printf("xxx 2.1a_enc_cfg.Configuration.Encoding = %s, a_enc_cfg.Configuration.AudioEncoding = %d(0:g711)\n",
	// 		a_enc_cfg.Configuration.Encoding, a_enc_cfg.Configuration.AudioEncoding);
	}

#ifdef MEDIA2_SUPPORT
    if (strcasecmp(a_enc_cfg.Configuration.Encoding, "PCMU") == 0 || 
	    strcasecmp(a_enc_cfg.Configuration.Encoding, "G711") == 0)
	{
		//printf("xxx strcasecmp(a_enc_cfg.Configuration.Encoding, PCMU/G711) == 0\n");
	    strcpy(a_enc_cfg.Configuration.Encoding, "PCMU");
	    a_enc_cfg.Configuration.AudioEncoding = AudioEncoding_G711;
	}
	else if (strcasecmp(a_enc_cfg.Configuration.Encoding, "AAC") == 0 || 
	    strcasecmp(a_enc_cfg.Configuration.Encoding, "MP4A-LATM") == 0)
	{
		//printf("xxx strcasecmp(a_enc_cfg.Configuration.Encoding, AAC/GMP4A) == 0\n");
	    strcpy(a_enc_cfg.Configuration.Encoding, "MP4A-LATM");
	    a_enc_cfg.Configuration.AudioEncoding = AudioEncoding_AAC;
	}
#endif

	p_a_enc_cfg = onvif_find_AudioEncoderConfiguration_by_param(&a_enc_cfg);
	if (p_a_enc_cfg)
	{
		return p_a_enc_cfg;
	}

	p_a_enc_cfg = onvif_add_AudioEncoderConfiguration(&a_enc_cfg);

	return p_a_enc_cfg;
}

#endif // end of AUDIO_SUPPORT

// void onvif_parse_profile(XMLN * p_profile)
void onvif_parse_profile()
{
	ONVIF_PROFILE * profile;

	profile = onvif_add_profile(TRUE);	//g_onvif_cfg.profiles 结构为链表类型不是数组
	if (NULL == profile)
	{
		return;
	}
	
	profile->v_src_cfg = onvif_parse_video_source_cfg();	//g_onvif_cfg.v_src_cfg
	profile->v_src_cfg->Configuration.UseCount++;

	profile->v_enc_cfg = onvif_parse_video_encoder_cfg();   //g_onvif_cfg.v_enc_cfg 
	if (profile->v_enc_cfg)
	{
		profile->v_enc_cfg->Configuration.UseCount++;
	}

#ifdef AUDIO_SUPPORT
	profile->a_src_cfg = onvif_parse_audio_source_cfg();
	profile->a_src_cfg->Configuration.UseCount++;

	profile->a_enc_cfg = onvif_parse_audio_encoder_cfg_t();
	profile->a_enc_cfg->Configuration.UseCount++;
#endif
}

// void onvif_parse_event_cfg(XMLN * p_event)
void onvif_parse_event_cfg()
{
	g_onvif_cfg.evt_renew_time = 60;

	g_onvif_cfg.evt_sim_flag = 1;

	g_onvif_cfg.evt_sim_interval = 10;
}

BOOL onvif_parse_server(ONVIF_SRV * p_req)
{
    /* p_serv_ip = xml_node_get(p_node, "server_ip");
	if (NULL == p_serv_ip)
	{
	    return FALSE;
	}
	else if (p_serv_ip->data)
	{
		strncpy(p_req->serv_ip, p_serv_ip->data, sizeof(p_req->serv_ip)-1);
	}
	*/
	p_req->serv_port = 8000;
	if (p_req->serv_port < 0 || p_req->serv_port > 65535)
		return FALSE;

	return TRUE;
}


// void onvif_parse_cfg(char * xml_buff, int rlen)
// void onvif_parse_cfg()
void onvif_load_cfg()
{
	/* 服务端口 */
    if (onvif_parse_server(&g_onvif_cfg.servs[0]))
    {
        g_onvif_cfg.servs_num++;
    }

/* 	p_servers = xml_node_get(p_node, "servers");
	while (p_servers && strcmp(p_servers->name, "servers") == 0)
	{
	    int idx = g_onvif_cfg.servs_num;

	    if (onvif_parse_server(p_servers, &g_onvif_cfg.servs[idx]))
	    {
	        g_onvif_cfg.servs_num++;

	        if (g_onvif_cfg.servs_num >= ARRAY_SIZE(g_onvif_cfg.servs))
	        {
	            break;
	        }
	    }
	    
	    p_servers = p_servers->next;
	} */

	g_onvif_cfg.http_max_users = 16;

#ifdef HTTPS
	g_onvif_cfg.https_enable = 0;   // 0:diable 1:enable
#endif
	g_onvif_cfg.need_auth = 1;    // 0:不鉴权 1:鉴权
	
	/* 设备信息 */
	int ret;
	CONFIG_Information devInfo;
	memset(&devInfo, 0, sizeof(CONFIG_Information));

    ret = onvif_get_devinfo(&devInfo);
	if ( ret == 0)
		onvif_set_devinfo(&devInfo);


	/* 用户 */
	add_to_Gusers();  //从文件读取用户复制给全区变量g_onvif_cfg.users，给soap_GetUsers显示在前端
	
	/* 音频视频编码器参数 */
    onvif_parse_profile();  

	/* scope */
	int i, scope_item_len;
	onvif_Scope scope[MAX_SCOPE_NUMS];
	scope_item_len = ARRAY_SIZE(scope[0].ScopeItem);

	strncpy(scope[0].ScopeItem, "onvif://www.onvif.org/Profile/Streaming" ,scope_item_len-1);
	strncpy(scope[1].ScopeItem, "onvif://www.onvif.org/location/country/china" ,scope_item_len-1);
	strncpy(scope[2].ScopeItem, "onvif://www.onvif.org/type/video_encoder" ,scope_item_len-1);
	strncpy(scope[3].ScopeItem, "onvif://www.onvif.org/name/IP-Camera" ,scope_item_len-1);
	strncpy(scope[4].ScopeItem, "onvif://www.onvif.org/hardware/HI3518C" ,scope_item_len-1);
	// for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.scopes); i++)
	for (i = 0; i < 5; i++) {
		// printf("xxx scope[%d].scope_item = %s xxx\n",i, scope[i].ScopeItem);
		if (scope[i].ScopeItem[0] != '\0') {
			onvif_add_scope(scope[i].ScopeItem,TRUE);
		}
		else {
			break;
		}
	}

	onvif_parse_event_cfg();
}

/* void onvif_load_cfg()
{
	int len;
    int rlen;
    FILE * fp;
    char * xml_buff;

    // read config file
	fp = fopen("config.xml", "r");
	if (NULL == fp)
	{
		return;
	}
	
	fseek(fp, 0, SEEK_END);
	
	len = ftell(fp);
	if (len <= 0)
	{
		fclose(fp);
		return;
	}
	fseek(fp, 0, SEEK_SET);
	
	xml_buff = (char *) malloc(len + 1);	
	if (NULL == xml_buff)
	{
		fclose(fp);
		return;
	}

	rlen = fread(xml_buff, 1, len, fp);
	
	fclose(fp);

	// onvif_parse_cfg(xml_buff, rlen);
	// onvif_parse_cfg();    // by xieqingpu

	free(xml_buff);
} */


BOOL onvif_read_device_uuid(char * buff, int bufflen)
{
    int len, rlen;
    FILE * fp;
    
	fp = fopen("uuid.txt", "r");
	if (NULL == fp)
	{
		return FALSE;
	}

	fseek(fp, 0, SEEK_END);

	// get file length
	len = ftell(fp);
	if (len <= 0)
	{
		fclose(fp);
		return FALSE;
	}
	fseek(fp, 0, SEEK_SET);

    if (bufflen <= len)
    {
        printf("filelen = %d, bufflen = %d\r\n", len, bufflen);
    }
    else
    {
    	rlen = fread(buff, 1, len, fp);

        // remove \r\n
    	while (rlen > 0 && (buff[rlen-1] == '\r' || buff[rlen-1] == '\n'))
    	{
    	    rlen--;
    	    buff[rlen] = '\0';
    	}
	}
	
	fclose(fp);

	return TRUE;
}

BOOL onvif_save_device_uuid(char * buff)
{
    FILE * fp;
	
	fp = fopen("uuid.txt", "w");
	if (NULL == fp)
	{
	    printf("open file uuid.txt failed\r\n");
		return FALSE;
	}

	fwrite(buff, 1, strlen(buff), fp);

	fclose(fp);

	return TRUE;
}

#ifdef PROFILE_Q_SUPPORT

int onvif_read_device_state()
{
    int state = 0, rlen;
    char buff[8] = {'\0'};
    FILE * fp;
    
	fp = fopen("devst.txt", "r");
	if (NULL == fp)
	{
		return state;
	}
	
	rlen = fread(buff, 1, 1, fp);
	if (1 == rlen)
	{
	    state = atoi(buff);
	}
	
	fclose(fp);

	return state;
}

BOOL onvif_save_device_state(int state)
{
    char buff[8] = {'\0'};
    FILE * fp;
	
	fp = fopen("devst.txt", "w");
	if (NULL == fp)
	{
	    printf("open file devst.txt failed\r\n");
		return FALSE;
	}

    sprintf(buff, "%d", state);
	fwrite(buff, 1, strlen(buff), fp);

	fclose(fp);

	return TRUE;
}

#endif // end of PROFILE_Q_SUPPORT


