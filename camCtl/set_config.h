
#ifndef __SET_CONFIG_H__
#define	__SET_CONFIG_H__

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ir.h"
#include "onvif.h"
#include "onvif_device.h"
#include "onvif_ptz.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef  int    BOOL;
typedef unsigned int         u32_t;
typedef unsigned char        u8_t;
typedef unsigned char*       u8ptr_t;
typedef float                float32_t;
typedef unsigned short int   u16_t;


/* *********************************** */
#define NAME_LEN    	 100
#define TOKEN_LEN    	 100

#define  g711   "G711"
#define  g726   "G762"
#define  aac    "AAC"

                   
typedef struct Alg_param
{ 
	BOOL	Enabled;				// 是否启用叠加检测框
	float	ConfidenceLevel;		// 置信度，浮点，取值范围：[0, 1]
	float 	NMS;					// 浮点，取值范围：[0, 1]
	int 	Extended1;				// 预留参数1，整形
	float 	Extended2;				// 预留参数2，浮点
} AlgParam_t;


#define MAX_WEEK_NUM            7       //星期天数
#define MAX_TIME_SEG_NUM        4       //时间段个数
#define MAX_DAY_NUM_EX	        32
typedef enum _RECORD_FLAGS
{
	RECORD_NO = 0,
	RECORD_FULLTIME,
	RECORD_ALARM,
	RECORD_NULL
}RECORDING_FLAGS;

/*
 * 时间段结构体定义
 */
typedef struct sdk_sched_time_s
{
    uint8_t enable;	    //激活, 当定时录像时表示录像类型;
    uint8_t res;
    uint8_t start_hour; //开始时间
    uint8_t start_min;
	uint8_t start_sec;
    uint8_t stop_hour;  //结束时间
    uint8_t stop_min;
	uint8_t stop_sec;
} sdk_day_time_t;

typedef struct sdk_schedule_time_s
{
    unsigned char is_allday;  //是否全天
	unsigned char enable;     //开启
    unsigned char unused[2];
    sdk_day_time_t day_sched_info[MAX_TIME_SEG_NUM];//布防时间段每天都有4个部分
} sdk_schedule_time_t;

/*
 * 录像参数
 */
typedef struct sdk_record_cfg_s
{
	unsigned char ipc_ucRecordMode;  // 0：HD:1：SD
	unsigned char ipc_ucRecordType;  // Nornal / Event
	unsigned char ipc_ucRecordSchedule;//本地录像使能
	unsigned char ipc_ucDetectType;//侦测类型:0:移动追踪，1:人行追踪
	unsigned int  ipc_iRecordWeekMask;
	sdk_schedule_time_t RecodSchedTime[MAX_WEEK_NUM];
} sdk_record_cfg_t;

/*设备信息结构体*/
typedef struct _GB28181Conf_t
{
#define GB28181_MAX_VIDEO_CHN				(32)
#define GB28181_MAX_ALARM_CHN				(16)

	char server_id[30];/*SIP服务器ID*//*默认值：34020000002000000001*/
	char server_ip[128];/*SIP服务器IP地址*//*默认值：192.168.10.178*/
	char server_port[10];/*SIP服务器IP端口*//*默认值：5060*/
	
	char ipc_id[30];/*媒体流发送者ID*//*默认值：34020000001180000002*/
	char ipc_username[30];
	char ipc_pwd[20];/*媒体流发送者密码*//*默认值：12345678*/
	char ipc_ip[20];/*媒体流发送者IP地址*//*默认值：192.168.10.144*/
	char ipc_sess_port[10]; /*会话端口，即SIP端口*/
	unsigned int AliveTime;/*注册有效期*/
	unsigned int HeartBeatTime;/*心跳包活时间间隔*/
	unsigned int ReconnctCount;/*重连次数*/
	
	char device_name[30];/*设备/区域/系统名称*//*默认值：IPC*/
	char device_manufacturer[20];/*设备厂商*//*默认值：GPT*/
	char device_model[20];/*设备型号*//*默认值：GB28181*/
	char device_firmware[32];/*设备固件版本*//*默认值：02.38.25*/
	char device_encode[10];/*是否编码*//*取值范围：ON/OFF*//*默认值：ON*/
	char device_record[10];/*是否录像*//*取值范围：ON/OFF*//*默认值：OFF*/
	int VideoNum;
	int AlarmNum;
	char VideoId[GB28181_MAX_VIDEO_CHN][32];
	char AlarmId[GB28181_MAX_ALARM_CHN][32]; /*报警器ID*/
	
	int gb28181_enable;/*GB28181使能 0:关闭 1:打开*/
	
	int proto_nettype;//信令传输网络类型1:TCP 0:UDP
	int stream_nettype; //流媒体传输网络类型1:TCP 0:UDP
}GB28181Conf_t;

typedef struct 
{ 
    u32_t    session_timeout;            // The rtsp session timeout for the related audio stream
    u32_t    sample_rate;                // The output sample rate in kHz
    u32_t    bitrate;                    // The output bitrate in kbps
    char     a_encoding[32];             //  G711 G762 AAC / Audio codec used for encoding the audio input (either G711, G726 or AAC)
} Audio_Encoder;

#define  PROFILE_MAIN      "Main"  
#define  PROFILE_HIGH      "High"
#define  PROFILE_EXTENDED  "Extended"  
#define  PROFILE_MAIN      "Main"
#define  PROFILE_MAIN10    "Main10"
/* typedef enum H264Profile 
{
	H264Profile_Baseline = 0, 
	H264Profile_Main = 1, 
	H264Profile_Extended = 2, 
	H264Profile_High = 3
} onvif_H264Profile; */
typedef struct 
{
    u32_t    gov_length;                  // Group of Video frames length. Determines typically the interval in which the
    onvif_H264Profile    encode_profile;
} Encoding_profile; 


typedef struct 
{
    onvif_VideoEncoding    v_encoding;          
    Encoding_profile    v_encoding_profile; 
} Video_encode;

typedef struct 
{
    u32_t    width;
    u32_t    height;
    u32_t    quality;                   // Relative value for the video quantizers and the quality of the video
    u32_t    session_timeout;           // The rtsp session timeout for the related video stream
    u32_t    framerate;                 // Desired frame rate in fps , Maximum output framerate in fps
    u32_t    encoding_interval;         // Interval at which images are encoded and transmitted,(A value of 1 means
                                                  // that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)
    u32_t    bitrate_limit;             // The maximum output bitrate in kbps
    Video_encode video_encoding;                 
} Video_Encoder;
        

//设备信息
typedef struct 
{
    char    manufacturer[64];                 // The manufactor of the device
    char    model[64];                                // The device model
    char    firmware_version[64];         // The firmware version in the device
    char    serial_number[64];               // The serial number of the device
    char    hardware_id[64];                   // The hardware ID of the device
} CONFIG_Information;

/* **************************************** */


//主位置
typedef struct
{
	u16_t homeZoom;						// required
} CONFIG_Home;

typedef struct
{
	float saturation;         //色饱和度       
	float contrast;             //对比度
	float brightness;         //亮度补偿 
    float sharp;                   //清晰度 (锐度)
} ImgParam_t;

typedef struct
{
	int focal;                //焦点
	float weightIrY;          //亮度权重
	float weightIrC;          //色度权重

	int dula_model;          //融合模式
	signed short int x;      //x偏移
	signed short int y;      //y偏移
	float scale;             //缩放
} DulaInformation_t;

/*********************************************
* FuncName: onvif_get_devinfo       
* Describe:  获取设备信息
* Params  :                                
* [IN]      
*    p_devInfo : 设备信息
* Return  :  成功返回0，非0失败                                                
**********************************************/
int onvif_get_devinfo(CONFIG_Information * p_devInfo);


/*********************************************
* FuncName: readUsers       
* Describe:  从文件读出用户信息
* Params  :                                
* [IN]      
*  users : 用户信息 
*  cnt ：用户个数 
* Return  :  成功返回0，-1失败                                                
**********************************************/
int readUsers(onvif_User *users, int cnt);

//写用户信息到文件  成功返回0，-1失败 
int writeUsers(onvif_User *users, int cnt);


/*********************************************
* FuncName: devInit       
* Describe:  设备初始化
* Params  :                                
**********************************************/
int devInit(char *ptzDevID, const char *cameraDEVID);


/*********************************************
* FuncName: controlPtzLeft       
* Describe:  控制PTZ的转向, 转动哪个方向才有哪个方向的数值，不转动的方向数值为0
* Params  :                                
* [OUT]      
*    Xspeed : 水平转动速度  -为左转，+为右转
*    Yspeed : 垂直转动速度  -为下转，+为上转
*    Zspeed : 水平转动速度  -为缩小焦距，+为放大焦距
* Return  : 成功返回0，失败返回-1                                               
**********************************************/
int controlPtzPos(float X, float Y, float Z , unsigned short Speed);

/*********************************************
* FuncName: ptzStop       
* Describe: 停止转动
* Params  :                                                                          
**********************************************/
void ptzStop();


/* ********************************************
* FuncName: readHomePos       
* Describe:  读取PTZ的主位置
* Params  :                                
* [OUT]      
*    speed : 方位 
* Return  :                                                  
**********************************************/
int readHomePos(CONFIG_Home * p_homPreset);
 // 设置 PTZ的主位置 
int writeHomePos(CONFIG_Home * p_homPreset);

/*********************************************
* FuncName: setPtzPreset       
* Describe:  设置PTZ的预置位
* Params  :                                
* [OUT]      
*    speed : 方位 
* Return  :                                                  
**********************************************/
void setPtzPreset(unsigned short location);

/*********************************************
* FuncName: gotoPtzPreset       
* Describe:  转到PTZ的预置位
* Return  :                                                  
**********************************************/
void gotoPtzPreset(unsigned short location);


/*********************************************
* FuncName: readPtzPresets       
* Describe:  从文件读取ptz预置位
* Params  :                                
* [OUT]      
*   p_preset ：
* Return  :  成功返回0，失败返回-1                                                  
********************************************s**/
int readPtzPreset(ONVIF_PTZPreset * p_presets, int cnt);
// 将ptz预置位保存到文件  成功返回0，失败返回-1
int writePtzPreset(ONVIF_PTZPreset * p_presets, int cnt);


/*********************************************
* FuncName: readPtzPresetTour       
* Describe:  从文件读取ptz巡航
* Params  :                                
* [OUT]      
*   p_preset ：
* Return  :  成功返回0，失败返回-1                                                  
**********************************************/
int readPtzPresetTour(PTZ_PresetsTours_t  *presetTours, int cnt);
// 将ptz巡航PresetTour保存到文件  成功返回0，失败返回-1
int writePtzPresetTour(PTZ_PresetsTours_t  *presetTours,  int cnt);


/*********************************************
* FuncName: focusMove       
* Describe:  摄像头调焦
* Params  :                                
* [OUT]      
*    zoom : 缩放速度 
* Return  :                                                  
**********************************************/
void focusMove(float zoom);

/*********************************************
* FuncName: getImgParam       
* Describe:  获取 摄像头参数
* Params  :  参数数据  >0 时才有效设置，<= 0 表示没有设置                             
* [IN]      
* Return  :  成功返回0，失败返回-1                                                
**********************************************/
int getImgParam(ImgParam_t *imgParams);

/*********************************************
* FuncName: setImgParam       
* Describe:  设置 摄像头参数
* Params  :  参数数据  >0 时才有效设置，<= 0 表示没有设置                             
* [OUT]      
* Return  :  成功返回0，失败返回-1                                                
**********************************************/
int setImgParam(ImgParam_t *imgParams);

int img_Stop();
/*********************************************
* FuncName: getThermalBaseParam       
* Describe:  获取 热成像参数配置_1 , 获取:色板、宽动态、亮度补偿、清晰度 (锐度)
* Params  :                                
* [IN]      
* Return  : 成功返回0，失败返回-1                              
**********************************************/
int getThermalBaseParam(ThermalBaseParam *thermalParam1);

/*********************************************
* FuncName: setThermalParam1       
* Describe:  热成像参数配置设置_1 , 设置:色板、宽动态、亮度补偿、清晰度 (锐度)
* Params  :                                
* [OUT]      
* Return  : 成功返回0，失败返回-1                             
**********************************************/
int setThermalParam1(ThermalBaseParam *thermalParam1);

/*********************************************
* FuncName: getThermalEnvParam       
* Describe:  获取 热成像参数配置设置_1 , 获取:发射率、距离、湿度、修正、反射温度、环境温度
* Params  :                                
* [IN]      
* Return  : 成功返回0，失败返回-1                             
**********************************************/
int getThermalEnvParam(ThermalEnvParam *param);

/*********************************************
* FuncName: setThermalParam2       
* Describe:  热成像参数配置设置_1 , 设置:发射率、距离、湿度、修正、反射温度、环境温度
* Params  :                                
* [OUT]      
* Return  : 成功返回0，失败返回-1                             
**********************************************/
int setThermalParam2(ThermalEnvParam *thermalParam2);

/*********************************************
* FuncName: getFusionParam        
* Describe:  从保存的文件读取 dula参数数据
* Params  :                                
* [IN]      
*   dulaInfo :  获取dulaInfo数据参数
* Return  : 成功返回0，失败返回-1                                                 
**********************************************/
int getFusionParam(DulaInformation_t *dulaInfo);

/*********************************************
* FuncName: setDulaParam        
* Describe:  设置dula参数数据 ,同时保存到文件
* Params  :                                
* [OUT]      
*   dulaInfo :  设置dulaInfo数据
* Return  : 成功返回0，失败返回-1      
**********************************************/
int setDulaParam(DulaInformation_t *dulaInfo);


/*********************************************
* FuncName: setVideoEncoder        
* Describe:  设置视频编码器参数 ,同时保存到文件
* Params  :                                
*   Camera_Encoder : 数据，包含分辨率，码率，帧率，GOP，编码级别
* Return  : 成功返回0，失败返回-123     
**********************************************/
int setVideoEncoder(Video_Encoder *p_video_encoder);
/* 读取 设置视频编码器参数 */
int getVideoEncoder(Video_Encoder *p_video_encoder);


/*********************************************
* FuncName:         
* Describe: 设置音频编码器参数 ,同时保存到文件
* Params  :                                
*   Camera_Encoder : 数据，包含采样率,码率,编码
* Return  : 成功返回0，失败返回-123     
**********************************************/
int getAudioEncoder(Audio_Encoder *p_audio_encoder);
/* 读取 设置音频编码器参数 */
int setAudioEncoder(Audio_Encoder *p_audio_encoder);

/* 设置叠加检测框 */
int set_Alg_Param(AlgParam_t * p_AlgParam);
/* 获取叠加检测框 */
int get_Alg_Param(AlgParam_t * p_AlgParam);


/* 设置GB28181配置 */
int onvif_SIP_Settings(GB28181Conf_t * GB28181Confing);
/* 获取GB28181配置 */
int GetGB28181Confing(GB28181Conf_t *GB28181Confing);


int GetNTPInformation(onvif_NTPInformation		    *pNTPInformation);
	
/* 保存NTPInformation数据参数*/
int SetNTPInformation(onvif_NTPInformation		    *pNTPInformation, BOOL isSave);

/* 读取 SystemDateTime数据参数 */
int GetSystemDateTime(onvif_SystemDateTime *pDataTimeInfo);
int opt_SetSystemDateTime(onvif_DateTime *pUTCDateTime);

time_t SystemTimeToTM(onvif_DateTime stStartTime);
	
/* 保存SystemDateTime数据参数*/
int SetSystemDateTime(onvif_SystemDateTime *pDataTimeInfo, 
								onvif_DateTime *pUTCDateTime, BOOL isTZChange, BOOL isSave);


/*1 读取TCPIP数据参数 */
int GetNetworkInterfaces(onvif_NetworkInterface	*pNetworkInterface);

int opt_SetNetworkInterfaces(onvif_NetworkInterface	*pNetworkInterface);

/* 保存SystemDateTime数据参数*/
int SetNetworkInterfaces(onvif_NetworkInterface	*pNetworkInterface, BOOL isSave);


/*2 读取网关数据参数 */
int GetNetworkGateway(onvif_NetworkGateway		      *pNetworkGateway);

	
/*设置网关数据参数*/
int SetNetworkGateway(onvif_NetworkGateway		     *pNetworkGateway, BOOL isSave);


/*3 读取DNS数据参数 */
int GetDNSInformation(onvif_DNSInformation		     *pDNSInformation);
	
/* 设置DNS数据参数*/
int SetDNSInformation(onvif_DNSInformation		    *pDNSInformation, BOOL isSave);

/*读取事件上传数据参数 */
int GetEventSnapInformation(onvif_EventSnapUploadInfo	       *pEventSnap);
/*设置事件上传数据参数 */
int SetEventSnapInformation(onvif_EventSnapUploadInfo	       *pEventSnap, BOOL isSave);

/*4 读取网络协议数据参数 */
int GetNetworkProtocols(onvif_NetworkProtocol	*pNetworkProtocol);
	
/* 设置网络协议数据参数*/
int SetNetworkProtocols(onvif_NetworkProtocol	*pNetworkProtocol, BOOL isSave);

void SystemReboot();

void SetSystemFactoryDefault(int type /* 0:soft, 1:hard */);
struct tm * GetSystemUTCTime();
int sync_hwclock_tosys(); 
int onvif_message_init();


#ifdef __cplusplus
}
#endif

#endif

