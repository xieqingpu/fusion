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

#ifndef HTTP_CLN_H
#define HTTP_CLN_H

#include "http.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct eventinfo {
	char pFileName[128]; //需要上传的文件(请用绝对路径)
	char hostname[128]; //上传的主机IP
	int eventtype;//事件类型
}Gpt_EventUploadInfo;

BOOL http_onvif_trans(HTTPREQ * p_req, int timeout, const char * bufs, int len);

/*发送图片到服务器
@param Gpt_EventUploadInfo *pUploadInfo : 上传信息

返回200成功，其他失败，失败原因见msg
请求成功			{"bool":true,"msg":"OK","status":200}
缺少uuid参数		{"bool":false,"msg":"\"uuid\" can not be empty","status":400}
uuid校验不通过 	 {"bool":false,"msg":"\"uuid\" must be a string","status":400}
deviceIP校验不通过  {"bool":false,"msg":"\"deviceIP\" must be writed as Dot-decimal notation","status":400}
上传了非法文件	   {"bool":false,"msg":"Unsupported Media Type","status":415}
服务器处理出错	   {"bool":false,"msg":"Internal Server Error","status":500}
请求接口地址错误	{"bool":false,"msg":"Not Found","status":404}
*/
int http_send_event_jpeg(Gpt_EventUploadInfo *pUploadInfo);

/*
抓拍图片并将图片发送到服务器
@param 
	   eventtype: 事件类型
返回与http_send_event_jpeg相同
*/
int http_snap_and_sendto_host(int eventtype);

#ifdef __cplusplus
}
#endif

#endif


