
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

#ifdef __cplusplus
extern "C" {
#endif

/////
typedef  int    BOOL;
typedef unsigned int               u32_t;
typedef unsigned char           u8_t;
typedef unsigned char*         u8ptr_t;
typedef float                               float32_t;
typedef unsigned short int   u16_t;

#define OTHER_FAILE  (-123)

/* *********************************** */
#define NAME_LEN    	 100
#define TOKEN_LEN    	 100

#define  g711   "G711"
#define  g726   "G762"
#define  aac    "AAC"

// typedef struct 
// {
    
// } Audio_Source;

typedef struct 
{ 
    u32_t    session_timeout;            // The rtsp session timeout for the related audio stream
    u32_t    sample_rate;                // The output sample rate in kHz
    u32_t    bitrate;                    // The output bitrate in kbps
    u8_t     a_encoding[32];             //  G711 G762 AAC / Audio codec used for encoding the audio input (either G711, G726 or AAC)
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

#define  VIEDO_ENCODE_JPEG   "JPEG" 
#define  VIEDO_ENCODE_MPEG4  "MPEG4"
#define  VIEDO_ENCODE_H264   "H264"
#define  VIEDO_ENCODE_H265   "H265"
typedef struct 
{
    // u8_t       v_encoding[64];              // VIEDO_ENCODE_XXX
    u8ptr_t    v_encoding;                  // VIEDO_ENCODE_XXX
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
    u8_t    manufacturer[64];                 // The manufactor of the device
    u8_t    model[64];                                // The device model
    u8_t    firmware_version[64];         // The firmware version in the device
    u8_t    serial_number[64];               // The serial number of the device
    u8_t    hardware_id[64];                   // The hardware ID of the device
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
* Return  :                                                  
**********************************************/
void controlPtzPos(float X, float Y, float Z , unsigned short Speed);

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
* Return  : 成功返回0，失败返回-1                                           int      
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


int vpclose(FILE *fp);  
FILE *vpopen(const char* cmdstring, const char *type);  

//尽量使用system_ex替换system函数，因为在很多系统里面发现system存在隐患
#define system_ex(fmt, args...)                   \
    do{                                             \
        char readbuf[512]={0};                      \
        int debug=0;                                    \
        snprintf(readbuf, sizeof(readbuf), fmt, ##args);\
        if(debug==1) printf("%s\n", readbuf);            \
        FILE *fp=vpopen(readbuf,"r");                    \
        if(fp!=NULL){                                     \
			memset(readbuf, 0x0, sizeof(readbuf));         \
			while(fgets(readbuf, sizeof(readbuf), fp) != NULL) {\
				if(debug==1) printf("%s\n", readbuf);\
			}\
			vpclose(fp); \
		}                      \
    }while(0)

void Set_Start_NTP_Server(int flag);
int Get_Start_NTP_Server();


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


/*4 读取网络协议数据参数 */
int GetNetworkProtocols(onvif_NetworkProtocol	*pNetworkProtocol);
	
/* 设置网络协议数据参数*/
int SetNetworkProtocols(onvif_NetworkProtocol	*pNetworkProtocol, BOOL isSave);

void SystemReboot();

void SetSystemFactoryDefault(int type /* 0:soft, 1:hard */);
struct tm * GetSystemUTCTime();
int sync_hwclock_tosys(); 


#ifdef __cplusplus
}
#endif

#endif

