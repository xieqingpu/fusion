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
#include <sys/types.h>
#include <sys/socket.h>
#include "sys_inc.h"
#include "http.h"
#include "http_cln.h"
#include "onvif.h"
#include "utils_log.h"
#include "cJSON.h"
#include "gptmessage.h"
#include "gptmessagedef.h"

#define HTTP_RFC1123FMT 	"%a, %d %b %Y %H:%M:%S GMT"
#define KEY_FORMAT      "--%s\r\n"\
	                    "Content-Disposition: form-data; name=\"%s\"\r\n\r\n"
#define JPEG_KEY_FORMAT      "--%s\r\n"\
							"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"\
							"Content-Type: image/jpeg\r\n\r\n"
#define JPEG2_KEY_FORMAT      "\r\n--%s\r\n"\
								"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"\
								"Content-Type: image/jpeg\r\n\r\n"
							
extern ONVIF_CFG g_onvif_cfg;

typedef unsigned long long uint64_t;
typedef unsigned int   uint32_t;

static uint64_t _linux_get_time_ms(void)
{
    struct timeval tv = { 0 };
    uint64_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_ms;
}

static uint64_t _linux_time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}

int tcp_sendall(int fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
	int ret;
	uint32_t len_sent;
	uint64_t t_end, t_left;
	fd_set sets;

	t_end = _linux_get_time_ms() + timeout_ms;
	len_sent = 0;
	ret = 1; /* send one time if timeout_ms is value 0 */
	int net_err = 0;

	do {
		t_left = _linux_time_left(t_end, _linux_get_time_ms());

		if (0 != t_left) {
			struct timeval timeout;

			FD_ZERO(&sets);
			FD_SET(fd, &sets);

			timeout.tv_sec = t_left / 1000;
			timeout.tv_usec = (t_left % 1000) * 1000;

			ret = select(fd + 1, NULL, &sets, NULL, &timeout);
			if (ret > 0) {
				if (0 == FD_ISSET(fd, &sets)) {
					log_print(LOG_ERR, "Should NOT arrive");
					/* If timeout in next loop, it will not sent any data */
					ret = 0;
					continue;
				}
			} else if (0 == ret) {
				log_print(LOG_ERR, "select-write timeout %d", (int)fd);
				break;
			} else {
				if (EINTR == errno) {
					log_print(LOG_ERR, "EINTR be caught");
					continue;
				}

				log_print(LOG_ERR,"select-write fail, ret = select() = %d", ret);
				net_err = 1;
				break;
			}
		}

		if (ret > 0) {
			ret = send(fd, buf + len_sent, len - len_sent, 0);
			if (ret > 0) {
				len_sent += ret;
			} else if (0 == ret) {
				log_print(LOG_ERR, "No data be sent");
			} else {
				if (EINTR == errno) {
					log_print(LOG_ERR, "EINTR be caught");
					continue;
				}

				log_print(LOG_ERR, "send fail, ret = send() = %d", ret);
				net_err = 1;
				break;
			}
		}
	} while (!net_err && (len_sent < len) && (_linux_time_left(t_end, _linux_get_time_ms()) > 0));

	if (net_err) {
		return -1;
	} else {
		return len_sent;
	}
}

int tcp_readall(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
	int ret, err_code;
	uint32_t len_recv;
	uint64_t t_end, t_left;
	fd_set sets;
	struct timeval timeout;

	t_end = _linux_get_time_ms() + timeout_ms;
	len_recv = 0;
	err_code = 0;

	do {
		t_left = _linux_time_left(t_end, _linux_get_time_ms());
		if (0 == t_left) {
			break;
		}
		FD_ZERO(&sets);
		FD_SET(fd, &sets);

		timeout.tv_sec = t_left / 1000;
		timeout.tv_usec = (t_left % 1000) * 1000;

		ret = select(fd + 1, &sets, NULL, NULL, &timeout);
		if (ret > 0) {
			ret = recv(fd, buf + len_recv, len - len_recv, 0);
			if (ret > 0) {
				len_recv += ret;
			} else if (0 == ret) {
				//log_print(LOG_ERR, "connection is closed");
				err_code = -1;
				break;
			} else {
				if (EINTR == errno) {
					log_print(LOG_ERR, "EINTR be caught");
					continue;
				}
				log_print(LOG_ERR, "recv fail");
				err_code = -2;
				break;
			}
		} else if (0 == ret) {
			break;
		} else {
			log_print(LOG_ERR, "select-recv fail");
			err_code = -2;
			break;
		}
	} while ((len_recv < len));

	/* priority to return data bytes if any data be received from TCP connection. */
	/* It will get error code on next calling */
	return (0 != len_recv) ? len_recv : err_code;
}

BOOL http_tcp_tx(HTTPREQ * p_req, const char * p_data, int len)
{
    int slen;
    
	if (p_req->cfd <= 0)
	{
		return FALSE;
    }
    
#ifdef HTTPS
	if (p_req->https)
	{
		slen = SSL_write(p_req->ssl, p_data, len);
	}
	else
	{
		slen = tcp_sendall(p_req->cfd, p_data, len, 20000);
	}
#else	
    
	slen = tcp_sendall(p_req->cfd, p_data, len, 20000);
#endif
	if (slen != len)
	{
	    UTIL_ERR("slen = %d, len = %d failed!!!", slen, len);
		return FALSE;
    }
    
	return TRUE;
}

int http_event_tcp_rx(HTTPREQ * p_req, char * p_data, int len)
{
    int slen;
    
	if (p_req->cfd <= 0)
	{
		return FALSE;
    }
    
#ifdef HTTPS
	if (p_req->https)
	{
		slen = SSL_read(p_req->ssl, p_data, len);
	}
	else
	{
		slen = tcp_readall(p_req->cfd, p_data, len, 20*1000);
	}
#else	
	slen = tcp_readall(p_req->cfd, p_data, len, 20*1000);
#endif
    
	return slen;
}

BOOL http_onvif_req(HTTPREQ * p_req, const char * p_xml, int len)
{
    int offset = 0;
	char bufs[1024 * 4];
	
	if (len > 3072)
	{
		return FALSE;
	}
	
	offset += sprintf(bufs+offset, "POST %s HTTP/1.1\r\n", p_req->url);
	offset += sprintf(bufs+offset, "Host: %s:%d\r\n", p_req->host, p_req->port);
	offset += sprintf(bufs+offset, "User-Agent: ltxd/1.0\r\n");
	offset += sprintf(bufs+offset, "Content-Type: application/soap+xml; charset=utf-8; action=\"%s\"\r\n", p_req->action);
	offset += sprintf(bufs+offset, "Content-Length: %d\r\n", len);
	offset += sprintf(bufs+offset, "Connection: close\r\n\r\n");

	if (p_xml && len > 0)
	{
		memcpy(bufs+offset, p_xml, len);
		offset += len;
	}
	
	bufs[offset] = '\0';

    log_print(LOG_DBG, "TX >> %s\r\n", bufs);
    
	return http_tcp_tx(p_req, bufs, offset);
}


BOOL http_onvif_trans(HTTPREQ * p_req, int timeout, const char * bufs, int len)
{
	BOOL ret = FALSE;
	
#ifdef HTTPS
	SSL_CTX* ctx = NULL;
	const SSL_METHOD* meth = NULL;

	if (p_req->https)
	{
		SSLeay_add_ssl_algorithms();  
		meth = SSLv23_client_method();	
		SSL_load_error_strings();  
		ctx = SSL_CTX_new(meth);
		if (NULL == ctx)
		{
			log_print(LOG_ERR, "%s, SSL_CTX_new failed!\r\n", __FUNCTION__);
			return FALSE;
		}
	}
#else
	if (p_req->https)
	{
		log_print(LOG_ERR, "%s, the server require ssl connection, unsupport!\r\n", __FUNCTION__);
		return FALSE;
	}
#endif

	p_req->cfd = tcp_connect_timeout(inet_addr(p_req->host), p_req->port, timeout);
	if (p_req->cfd <= 0)
	{
	    log_print(LOG_ERR, "%s, tcp_connect_timeout\r\n", __FUNCTION__);
		goto FAILED;
    }

#ifdef HTTPS
	if (p_req->https)
	{
		p_req->ssl = SSL_new(ctx); 
		if (NULL == p_req->ssl)
		{
			log_print(LOG_ERR, "%s, SSL_new failed!\r\n", __FUNCTION__);
			goto FAILED;
		}
		
		SSL_set_fd(p_req->ssl, (int)p_req->cfd);
		
		if (SSL_connect(p_req->ssl) == -1)
		{
			log_print(LOG_ERR, "%s, SSL_connect failed!\r\n", __FUNCTION__);
			goto FAILED;
		}
	}
#endif

	ret = http_onvif_req(p_req, bufs, len);

FAILED:

	if (p_req->cfd > 0)
	{
		closesocket(p_req->cfd);
		p_req->cfd = 0;
	}

#ifdef HTTPS
	if (p_req->ssl)
	{
		SSL_free(p_req->ssl);
		p_req->ssl = NULL;
	}

	if (ctx)
	{
		SSL_CTX_free(ctx);
	}
#endif
    
	return ret;
}

unsigned long http_get_filesize(const char * filename)
{
    struct stat buf;  
    if(stat(filename, &buf)<0)  
    {  
        return 0;  
    }  
    return (unsigned long)buf.st_size;  

}

int http_read_file(const char * filename, char * buff, int buflen)
{
    int len;
    int rlen;
    FILE * fp;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        log_print(LOG_WARN, "%s, open file (%s) failed\r\n", __FUNCTION__, filename);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
	
	len = ftell(fp);
	if (len <= 0)
	{
		fclose(fp);
		return 0;
	}
	fseek(fp, 0, SEEK_SET);

    if (len >= buflen)
    {
        log_print(LOG_WARN, "%s, output too large, %d, %d\r\n", __FUNCTION__, len, buflen);
        
        len = buflen - 1;        
    }

	rlen = fread(buff, 1, len, fp);
	
    fclose(fp);

    return rlen;
}

char* http_boundary_create()
{
	int flag, i;
	static char randomstr[64];
    static int seed = 1;
	int length = 32;

    srand((uint32)time(NULL) + seed++);        
	
	for (i = 0; i < length - 1; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
			case 0:
				randomstr[i] = 'A' + rand() % 26;
				break;
			case 1:
				randomstr[i] = 'a' + rand() % 26;
				break;
			case 2:
				randomstr[i] = '0' + rand() % 10;
				break;
			default:
				randomstr[i] = 'x';
				break;
		}
	}
	randomstr[length - 1] = '\0';
	return randomstr;
}

int http_cjson_parse(char *msgPayload)
{
	cJSON *root_json;
	cJSON *item_json;
	int ret = -1;
	/*bool			 请求是否成功 	   布尔值
	  msg 			 请求结果说明 	   字符串
	  status			 请求结果状态码		数值
	  请求成功			{"bool":true,"msg":"OK","status":200}
	*/
	if (!msgPayload)
		return -1;
	
	root_json = cJSON_Parse(msgPayload);
	if(root_json)
	{
		item_json = cJSON_GetObjectItem(root_json, "bool");
		if(item_json && item_json->type == cJSON_True)
		{
			UTIL_INFO("event file upload success!!!");
			ret = 200;
		}
		else {
			item_json = cJSON_GetObjectItem(root_json, "msg");
			if(item_json && item_json->type == cJSON_String)
			{
				UTIL_ERR("msg=%s!!!", item_json->valuestring);
			}
			item_json = cJSON_GetObjectItem(root_json, "status");
			if(item_json && item_json->type == cJSON_Number)
			{
				UTIL_ERR("status=%d!!!", item_json->valueint);
				ret = item_json->valueint;
			}
		}
	}
	return ret;
}

int http_onvif_event_req(HTTPREQ * p_req, Gpt_EventUploadInfo *pUploadInfo)
{
    BOOL ret = FALSE;
    int offset = 0;
	char *bufs = NULL;
	char tempdatabuff[1024*4] = {0};
	char boundary[128] = {0};
    time_t now;
    char tempbuf[512] = {0}; 
	int len = 0;
	int data_length = 0;
	int file_max_len = 1024 * 1024;
	char *p = NULL;
	
    now = time(NULL);
    strftime(tempbuf, sizeof(tempbuf), HTTP_RFC1123FMT, gmtime(&now));
	
    bufs = (char *)malloc(file_max_len);
	if (!bufs) 
	{
		UTIL_ERR("malloc bufs failed!!!");
		return -1;
	}
    snprintf(boundary, sizeof(boundary), "----%s", http_boundary_create());
	
	offset += sprintf(tempdatabuff+offset, KEY_FORMAT, boundary, "uuid");
	offset += sprintf(tempdatabuff+offset, "%s\r\n", g_onvif_cfg.EndpointReference);
	
	offset += sprintf(tempdatabuff+offset, KEY_FORMAT, boundary, "deviceName");
	offset += sprintf(tempdatabuff+offset, "%s\r\n", g_onvif_cfg.DeviceInformation.Model);
	
	offset += sprintf(tempdatabuff+offset, KEY_FORMAT, boundary, "deviceIP");
	offset += sprintf(tempdatabuff+offset, "%s\r\n", g_onvif_cfg.servs[0].serv_ip);
	
	offset += sprintf(tempdatabuff+offset, KEY_FORMAT, boundary, "deviceDateTime");
	offset += sprintf(tempdatabuff+offset, "%s\r\n", tempbuf);

    //上传给算法服务器必须同时上传事件服务器地址
	if (1 == pUploadInfo->towhere && strlen(g_onvif_cfg.network.EventUploadInfo.HttpServerUrl) > 0)
	{
		offset += sprintf(tempdatabuff+offset, KEY_FORMAT, boundary, "eventServerUrl");
		offset += sprintf(tempdatabuff+offset, "%s\r\n", g_onvif_cfg.network.EventUploadInfo.HttpServerUrl);
	}
		
	offset += sprintf(tempdatabuff+offset, KEY_FORMAT, boundary, "eventType");
	offset += sprintf(tempdatabuff+offset, "%d\r\n", pUploadInfo->eventtype);
    //上传事件信息
	if (strlen(pUploadInfo->eventdetail) > 0)
	{
		offset += sprintf(tempdatabuff+offset, KEY_FORMAT, boundary, "eventDetail");
		offset += sprintf(tempdatabuff+offset, "%s\r\n", pUploadInfo->eventdetail);
	}	

    memset(tempbuf, 0x0, sizeof(tempbuf));
	snprintf(tempbuf, sizeof(tempbuf), JPEG_KEY_FORMAT, boundary, "snapshot", pUploadInfo->pFileName);
    strncpy(tempdatabuff+offset, tempbuf, strlen(tempbuf));
	offset += strlen(tempbuf);
	len = offset;

	data_length = offset + http_get_filesize(pUploadInfo->pFileName) + strlen(boundary) + strlen("\r\n----\r\n");

	offset = 0;
	offset += sprintf(bufs+offset, "POST %s HTTP/1.1\r\n", p_req->url);
	offset += sprintf(bufs+offset, "Host: %s\r\n", p_req->host);
	offset += sprintf(bufs+offset, "User-Agent: GPT/1.0\r\n");
	offset += sprintf(bufs+offset, "Content-Type: multipart/form-data; boundary=%s\r\n", boundary);
	offset += sprintf(bufs+offset, "Content-Length: %d\r\n", data_length);
	offset += sprintf(bufs+offset, "Connection: close\r\n\r\n");

	strncpy(bufs+offset, tempdatabuff, len);
	offset += len;
	
    //UTIL_INFO("snap data_length=%d len=%d, offset=%d >> %s\r\n", data_length, len, offset, bufs);
	len = http_read_file(pUploadInfo->pFileName, bufs+offset, file_max_len-offset-1);
	if (len > 0)
	{
		offset += len;
	}

	offset += sprintf(bufs+offset, "\r\n--%s--\r\n", boundary);
	bufs[offset] = '\0';
	
	ret = http_tcp_tx(p_req, bufs, offset);
	
	if (!bufs) 
	{
		free(bufs);
		bufs = NULL;
	}
	
	if (ret)
	{
	    memset(tempdatabuff, 0x0, sizeof(tempdatabuff));
		ret = http_event_tcp_rx(p_req, tempdatabuff, sizeof(tempdatabuff));
		if (ret > 0) 
		{
			p = strstr(tempdatabuff, "Content-Length");//Content-Length: 37^M
			if (p) {
				if (sscanf(p, "Content-Length: %d", &len) < 0)
				{
					UTIL_ERR("parse Content-Length failed. line: %s", p);
					return -1;
				}
			}
			else {
				UTIL_ERR("have no Content-Length. line: %s", p);
				return -1;
			}

			p = strstr(tempdatabuff, "\r\n\r\n");
			if (p) {
				p += strlen("\r\n\r\n");
				p[len] = '\0';
			    UTIL_INFO("http_event_tcp_rx len=%d>> %s", len, p);
				return http_cjson_parse(p);
			}
		}
	}

	return -1;
}


static int http_onvif_event_trans(HTTPREQ * p_req, int timeout, Gpt_EventUploadInfo *pUploadInfo)
{
	int ret = -1;
	
#ifdef HTTPS
	SSL_CTX* ctx = NULL;
	const SSL_METHOD* meth = NULL;

	if (p_req->https)
	{
		SSLeay_add_ssl_algorithms();  
		meth = SSLv23_client_method();	
		SSL_load_error_strings();  
		ctx = SSL_CTX_new(meth);
		if (NULL == ctx)
		{
			log_print(LOG_ERR, "%s, SSL_CTX_new failed!\r\n", __FUNCTION__);
			return FALSE;
		}
	}
#else
	if (p_req->https)
	{
		log_print(LOG_ERR, "%s, the server require ssl connection, unsupport!\r\n", __FUNCTION__);
		return FALSE;
	}
#endif

	p_req->cfd = tcp_connect_timeout(inet_addr(p_req->host), p_req->port, timeout);
	if (p_req->cfd <= 0)
	{
	    UTIL_ERR("tcp_connect_timeout failed");
		goto FAILED;
    }

#ifdef HTTPS
	if (p_req->https)
	{
		p_req->ssl = SSL_new(ctx); 
		if (NULL == p_req->ssl)
		{
			log_print(LOG_ERR, "%s, SSL_new failed!\r\n", __FUNCTION__);
			goto FAILED;
		}
		
		SSL_set_fd(p_req->ssl, (int)p_req->cfd);
		
		if (SSL_connect(p_req->ssl) == -1)
		{
			log_print(LOG_ERR, "%s, SSL_connect failed!\r\n", __FUNCTION__);
			goto FAILED;
		}
	}
#endif

	ret = http_onvif_event_req(p_req, pUploadInfo);
    
FAILED:

	if (p_req->cfd > 0)
	{
		closesocket(p_req->cfd);
		p_req->cfd = 0;
	}

#ifdef HTTPS
	if (p_req->ssl)
	{
		SSL_free(p_req->ssl);
		p_req->ssl = NULL;
	}

	if (ctx)
	{
		SSL_CTX_free(ctx);
	}
#endif
    
	return ret;
}

int http_send_event_jpeg(Gpt_EventUploadInfo *pUploadInfo)
{
	HTTPREQ	req;
	if (!pUploadInfo) return -1;
	
	memset(&req, 0, sizeof(HTTPREQ));
	
	char *host, *path, *p1, *url;
	char hbuf[256] = {0};
	int hlen = 0;

	url = pUploadInfo->hostname;
	if (strncasecmp(url, "http", 4))
	{
	  	return -1;
	}

	p1 = strchr(url+4, ':');
	if (!p1 || strncmp(p1, "://", 3))
	  return -1;
	
	host = p1 + 3;
	path = strchr(host, '/');
	if (!path)
	{
		hlen = strlen(url) - 7;
		strcpy(req.url, "/");
	}
	else 
	{
		hlen = path - host;
		strcpy(req.url, url+hlen+7);
	}
	
	strncpy(hbuf, host, hlen);
	hbuf[hlen] = '\0';
	host = hbuf;
	p1 = strrchr(host, ':');
	if (p1)
	{
		*p1++ = '\0';
		req.port = atoi(p1);
	}
	else
	{
		req.port = 80;
	}

	strcpy(req.host, host);

	req.https = 0;
	return http_onvif_event_trans(&req, 200, pUploadInfo);
}

int http_snap_and_sendto_host(int eventtype, int snaptype, int towhere, const char *eventdetail)
{
    int ret = -1;
	Gpt_EventUploadInfo pUploadInfo;
	int len = 0;
	
	time_t nowtime;
	struct tm *gtime;

	if ((0 == towhere && strlen(g_onvif_cfg.network.EventUploadInfo.HttpServerUrl) > 0) ||
		(1 == towhere && strlen(g_onvif_cfg.network.EventUploadInfo.AlgorithmServerUrl) > 0))
	{
	    //UTIL_INFO("towhere====%d", towhere);
	}
	else 
	{
		 UTIL_ERR("HttpServerUrl is not http addr!!!!");
		 return -1;
	}
	
	if ((snaptype == GPT_MSG_VIDEO_FUSIONSNAPJPEGPROCESS) ||  //双光融合图像抓拍
		 (snaptype == GPT_MSG_VIDEO_IPCSNAPJPEGPROCESS ) ||  //可见光摄像图像抓拍
		 (snaptype == GPT_MSG_VIDEO_IRMODESNAPJPEGPROCESS))  //IR模块图像抓拍
	{
		time(&nowtime);
		gtime = gmtime(&nowtime);
		snprintf(pUploadInfo.pFileName, sizeof(pUploadInfo.pFileName), "/tmp/Snap_%04d%02d%02d%02d%02d%02d_%d_%d.jpg",
				gtime->tm_year+1900, gtime->tm_mon+1, gtime->tm_mday, 
				gtime->tm_hour, gtime->tm_min, gtime->tm_sec, eventtype, snaptype);
			 
		if (0 == GPTMessageSend(snaptype, 0, (int)pUploadInfo.pFileName, strlen(pUploadInfo.pFileName)))
		{
		    if (0 == access(pUploadInfo.pFileName, F_OK))
		    {
		        if (0 == towhere)
		        {
			        len = strlen(g_onvif_cfg.network.EventUploadInfo.HttpServerUrl);
					if (len >= sizeof(pUploadInfo.hostname))
					{
						unlink(pUploadInfo.pFileName);
						return -1;
					}
					strncpy(pUploadInfo.hostname, g_onvif_cfg.network.EventUploadInfo.HttpServerUrl, len);
		        }
				else
				{
					len = strlen(g_onvif_cfg.network.EventUploadInfo.AlgorithmServerUrl);
					if (len >= sizeof(pUploadInfo.hostname))
					{
						unlink(pUploadInfo.pFileName);
						return -1;
					}

					strncpy(pUploadInfo.hostname, g_onvif_cfg.network.EventUploadInfo.AlgorithmServerUrl, len);
				}
				
				pUploadInfo.hostname[len] = '\0';
				if (eventdetail)
				{
				    len = strlen(eventdetail);
				    if (len >= sizeof(pUploadInfo.eventdetail))
						len = sizeof(pUploadInfo.eventdetail) - 1;
					strncpy(pUploadInfo.eventdetail, eventdetail, len);
				}
				
				pUploadInfo.eventtype = eventtype;
				pUploadInfo.towhere = towhere;
				ret = http_send_event_jpeg(&pUploadInfo);
				if (ret < 0)
				{
					UTIL_ERR("Upload file=%s to url=%s failed!!", pUploadInfo.pFileName, pUploadInfo.hostname);
				}
		    }
			else
				UTIL_INFO("pUploadInfo.pFileName=%s not exsit===", pUploadInfo.pFileName);
		}

	    if (0 == access(pUploadInfo.pFileName, F_OK))
	    {
	    	unlink(pUploadInfo.pFileName);
	    }
	}
	
    return ret;
}

int http_snap_and_sendto_host_extend(Gpt_SendJpegInfo *pSendInfo)
{
	if (pSendInfo)
		return http_snap_and_sendto_host(pSendInfo->eventtype, pSendInfo->snaptype, pSendInfo->towhere, pSendInfo->eventdetail);
	return -1;
}

