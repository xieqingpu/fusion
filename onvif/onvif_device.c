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
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <string.h>
#include "onvif_device.h"
#include "sys_inc.h"
#include "onvif.h"
#include "xml_node.h"
#include "onvif_utils.h"
#include "onvif_cfg.h"
#include "onvif_probe.h"
#include "onvif_event.h"
#include "set_config.h"
#include "utils_log.h"
#include "gpt_utils.h"
#include "rfc_md5.h"

/***************************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;
extern void onvif_init_capabilities();
/***************************************************************************************/

void onvif_LastClockSynchronizationNotify()
{
    ONVIF_NotificationMessage * p_message = onvif_add_NotificationMessage(NULL);
	if (p_message)
	{
	    ONVIF_SimpleItem * p_simpleitem;
	    
		strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
		strcpy(p_message->NotificationMessage.Topic, "tns1:Monitoring/OperatingTime/LastClockSynchronization");
		p_message->NotificationMessage.Message.PropertyOperationFlag = 1;
		p_message->NotificationMessage.Message.PropertyOperation = PropertyOperation_Changed;
		p_message->NotificationMessage.Message.UtcTime = time(NULL)+1;

        p_message->NotificationMessage.Message.DataFlag = 1;
        
		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{		    
			strcpy(p_simpleitem->SimpleItem.Name, "Status");
			strcpy(p_simpleitem->SimpleItem.Value, onvif_format_datetime_str(time(NULL), 1, "%Y-%m-%dT%H:%M:%SZ"));
		}

		onvif_put_NotificationMessage(p_message);
	}
}


#ifdef PROFILE_Q_SUPPORT

void onvif_switchDeviceState(int state)
{
    // 0 - Factory Default state, 1 - Operational State

    if (state != g_onvif_cfg.device_state)
    {
        g_onvif_cfg.device_state = state;

	    onvif_save_device_state(state);
    }
    
    // When switching from Factory Default State to Operational State, the device may reboot if necessary.
    
}

#endif

ONVIF_RET onvif_GetSystemLog(GetSystemLog_REQ * p_req, GetSystemLog_RES * p_res)
{
    if (SystemLogType_Access == p_req->LogType)
    {
        strcpy(p_res->String, "test access log");
    }
    else
    {
        strcpy(p_res->String, "test system log");
    }
    
    return ONVIF_OK;
}

ONVIF_RET onvif_SetSystemDateAndTime(SetSystemDateAndTime_REQ * p_req)
{
	// check datetime
	BOOL isTZChange = 0;
	onvif_SystemDateTime *pSystemDateTime = &g_onvif_cfg.SystemDateTime;
	if (p_req->SystemDateTime.DateTimeType == SetDateTimeType_Manual)
	{
		if (p_req->UTCDateTime.Date.Month < 1 || p_req->UTCDateTime.Date.Month > 12 ||
			p_req->UTCDateTime.Date.Day < 1 || p_req->UTCDateTime.Date.Day > 31 ||
			p_req->UTCDateTime.Time.Hour < 0 || p_req->UTCDateTime.Time.Hour > 23 ||
			p_req->UTCDateTime.Time.Minute < 0 || p_req->UTCDateTime.Time.Minute > 59 ||
			p_req->UTCDateTime.Time.Second < 0 || p_req->UTCDateTime.Time.Second > 61)
		{
			return ONVIF_ERR_InvalidDateTime;
		}
	}

	// check timezone
	if (p_req->SystemDateTime.TimeZoneFlag && 
		p_req->SystemDateTime.TimeZone.TZ[0] != '\0' && 
		onvif_is_valid_timezone(p_req->SystemDateTime.TimeZone.TZ) == FALSE)
	{
		return ONVIF_ERR_InvalidTimeZone;
	}
		
    pSystemDateTime->TimeZoneFlag = p_req->SystemDateTime.TimeZoneFlag;
	pSystemDateTime->DateTimeType = p_req->SystemDateTime.DateTimeType;
	pSystemDateTime->DaylightSavings = p_req->SystemDateTime.DaylightSavings;
	if ((p_req->SystemDateTime.TimeZone.TZ[0] != '\0') && 
		(0 != strncasecmp(pSystemDateTime->TimeZone.TZ, p_req->SystemDateTime.TimeZone.TZ, 
				strlen(p_req->SystemDateTime.TimeZone.TZ))))
	{
	    memset(pSystemDateTime->TimeZone.TZ, 0x0, sizeof(pSystemDateTime->TimeZone.TZ));
		strncpy(pSystemDateTime->TimeZone.TZ, p_req->SystemDateTime.TimeZone.TZ, strlen(p_req->SystemDateTime.TimeZone.TZ));
		isTZChange = 1;
	}
	
	// todo : add set system date time code ...
	SetSystemDateTime(pSystemDateTime, &p_req->UTCDateTime, isTZChange, TRUE);

    // send notify message ...
    onvif_LastClockSynchronizationNotify();
	
	return ONVIF_OK;
}

ONVIF_RET onvif_SystemReboot()
{
    // send onvif bye message    
    sleep(3);
    onvif_bye();
	// todo : reboot system ...
	SystemReboot();

    // please comment the code below
    // send onvif hello message, just for test
    //sleep(3);
    //onvif_hello();
    
	return ONVIF_OK;
}

ONVIF_RET onvif_SetSystemFactoryDefault(int type /* 0:soft, 1:hard */)
{
	// todo : set system factory default

#ifdef PROFILE_Q_SUPPORT
    onvif_switchDeviceState(0); // Devices conformant to Profile Q shall be in Factory Default State 
                                // out-of-the-box and after hard factory reset
#endif

    // please note the following lines, just for test
    // send onvif hello message
    //sleep(3);
    //onvif_hello();
	SetSystemFactoryDefault(type);
	return ONVIF_OK;
}

ONVIF_RET onvif_SetHostname(const char * name, BOOL fromdhcp)
{
    if (fromdhcp)
    {
    }
    else if (onvif_is_valid_hostname(name) == FALSE)
    {
    	return ONVIF_ERR_InvalidHostname;
    }
    
    // todo : set hostname ...

	if (name)
	{
    	strncpy(g_onvif_cfg.network.HostnameInformation.Name, name, sizeof(g_onvif_cfg.network.HostnameInformation.Name)-1);
    }
    g_onvif_cfg.network.HostnameInformation.FromDHCP = fromdhcp;

    return ONVIF_OK;
}

ONVIF_RET onvif_SetGPTSettings(const char * eventName, const char * AlgorithmName)
{
    ONVIF_RET ret = 0;
    if (eventName && NULL == strstr(eventName, "http"))
    {
        g_onvif_cfg.network.EventUploadInfo.EventHttpFlag = 0;
    	ret = ONVIF_ERR_InValidEventHttpUrl;
    }
    else
	{
		g_onvif_cfg.network.EventUploadInfo.EventHttpFlag = 1;
		memset(g_onvif_cfg.network.EventUploadInfo.HttpServerUrl, 0x0, sizeof(g_onvif_cfg.network.EventUploadInfo.HttpServerUrl));
    	strncpy(g_onvif_cfg.network.EventUploadInfo.HttpServerUrl, eventName, strlen(eventName));
		ret = ONVIF_OK;
    }

	if (AlgorithmName && NULL == strstr(AlgorithmName, "http"))
    {
        g_onvif_cfg.network.EventUploadInfo.AlgorithmServerFlag = 0;
    	ret = ONVIF_ERR_InValidAlgorithmServerUrl;
    }
    else
	{
		g_onvif_cfg.network.EventUploadInfo.AlgorithmServerFlag = 1;
		memset(g_onvif_cfg.network.EventUploadInfo.AlgorithmServerUrl, 0x0, sizeof(g_onvif_cfg.network.EventUploadInfo.AlgorithmServerUrl));
    	strncpy(g_onvif_cfg.network.EventUploadInfo.AlgorithmServerUrl, AlgorithmName, strlen(AlgorithmName));
		ret = ONVIF_OK;
    }
	
	SetEventSnapInformation(&g_onvif_cfg.network.EventUploadInfo, TRUE);

    return ret;
}

ONVIF_RET onvif_SetDNS(SetDNS_REQ * p_req)
{
	// todo : add set DNS code ...
    SetDNSInformation(&p_req->DNSInformation, TRUE);
	
	g_onvif_cfg.network.DNSInformation.FromDHCP = p_req->DNSInformation.FromDHCP;

	if (g_onvif_cfg.network.DNSInformation.FromDHCP == FALSE)
	{
		if (p_req->DNSInformation.SearchDomainFlag)
		{
			memcpy(g_onvif_cfg.network.DNSInformation.SearchDomain, 
			    p_req->DNSInformation.SearchDomain, 
			    sizeof(g_onvif_cfg.network.DNSInformation.SearchDomain));
		}

		memcpy(g_onvif_cfg.network.DNSInformation.DNSServer, 
		    p_req->DNSInformation.DNSServer, 
		    sizeof(g_onvif_cfg.network.DNSInformation.DNSServer));
    }
	else
	{
		// todo : get dns server from dhcp ...
		strcpy(g_onvif_cfg.network.DNSInformation.DNSServer[0], "8.8.8.8");
		strcpy(g_onvif_cfg.network.DNSInformation.DNSServer[0], "114.114.114.114");
	}
    
	return ONVIF_OK;
}

ONVIF_RET onvif_SetNTP(SetNTP_REQ * p_req)
{
	// todo : add set NTP code ...
	SetNTPInformation(&g_onvif_cfg.network.NTPInformation, TRUE);

	g_onvif_cfg.network.NTPInformation.FromDHCP = p_req->NTPInformation.FromDHCP;

	if (g_onvif_cfg.network.NTPInformation.FromDHCP == FALSE)
	{
		memcpy(g_onvif_cfg.network.NTPInformation.NTPServer, 
		    p_req->NTPInformation.NTPServer, 
		    sizeof(g_onvif_cfg.network.NTPInformation.NTPServer));
    }
    else
    {
    	// todo : get ntp server from dhcp ...
    	
    }

	return ONVIF_OK;
}

ONVIF_RET onvif_SetZeroConfiguration(SetZeroConfiguration_REQ * p_req)
{
    if (strcmp(p_req->InterfaceToken, g_onvif_cfg.network.ZeroConfiguration.InterfaceToken))
    {
        return ONVIF_ERR_InvalidToken;
    }

    // todo : add set zero configuration code ...

    g_onvif_cfg.network.ZeroConfiguration.Enabled = p_req->Enabled;

    // please comment the code below, just for test
    if (p_req->Enabled)
    {
        g_onvif_cfg.network.ZeroConfiguration.sizeAddresses = 1;
        strcpy(g_onvif_cfg.network.ZeroConfiguration.Addresses[0], "192.168.1.100");
    }
    else
    {
        g_onvif_cfg.network.ZeroConfiguration.sizeAddresses = 0;
    }

    return ONVIF_OK;
}

ONVIF_RET onvif_SetNetworkProtocols(SetNetworkProtocols_REQ * p_req)
{
#ifndef HTTPS
    if (p_req->NetworkProtocol.HTTPSFlag && p_req->NetworkProtocol.HTTPSEnabled)
    {
        return ONVIF_ERR_ServiceNotSupported;
    }
#endif

	// todo : add set network protocols code ...	
	SetNetworkProtocols(&p_req->NetworkProtocol, TRUE);

	if (p_req->NetworkProtocol.HTTPFlag)
	{
		g_onvif_cfg.network.NetworkProtocol.HTTPEnabled = p_req->NetworkProtocol.HTTPEnabled;
		memcpy(g_onvif_cfg.network.NetworkProtocol.HTTPPort, 
		    p_req->NetworkProtocol.HTTPPort, 
		    sizeof(g_onvif_cfg.network.NetworkProtocol.HTTPPort));
	}

	if (p_req->NetworkProtocol.HTTPSFlag)
	{
		g_onvif_cfg.network.NetworkProtocol.HTTPSEnabled = p_req->NetworkProtocol.HTTPSEnabled;
		memcpy(g_onvif_cfg.network.NetworkProtocol.HTTPSPort, 
		    p_req->NetworkProtocol.HTTPSPort, 
		    sizeof(g_onvif_cfg.network.NetworkProtocol.HTTPSPort));
	}

	if (p_req->NetworkProtocol.RTSPFlag)
	{
		g_onvif_cfg.network.NetworkProtocol.RTSPEnabled = p_req->NetworkProtocol.RTSPEnabled;
		memcpy(g_onvif_cfg.network.NetworkProtocol.RTSPPort, 
		    p_req->NetworkProtocol.RTSPPort, 
		    sizeof(g_onvif_cfg.network.NetworkProtocol.RTSPPort));
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_SetNetworkDefaultGateway(SetNetworkDefaultGateway_REQ * p_req)
{
	memcpy(g_onvif_cfg.network.NetworkGateway.IPv4Address, 
	    p_req->IPv4Address, 
	    sizeof(g_onvif_cfg.network.NetworkGateway.IPv4Address));

	// todo : add set network default gateway code ...
	SetNetworkGateway(&g_onvif_cfg.network.NetworkGateway, TRUE);

	return ONVIF_OK;
}

ONVIF_RET onvif_GetSystemUris(const char * lip, uint32 lport, GetSystemUris_RES * p_res)
{
    p_res->AccessLogUriFlag = 1;
    p_res->SystemLogUriFlag = 1;
    p_res->SupportInfoUriFlag = 1;
    p_res->SystemBackupUriFlag = 1;
    
    sprintf(p_res->SystemLogUri, "%s://%s:%d/SystemLog", g_onvif_cfg.https_enable ? "https" : "http", lip, lport);
    sprintf(p_res->AccessLogUri, "%s://%s:%d/AccessLog", g_onvif_cfg.https_enable ? "https" : "http", lip, lport);
    sprintf(p_res->SupportInfoUri, "%s://%s:%d/SupportInfo", g_onvif_cfg.https_enable ? "https" : "http", lip, lport);
    sprintf(p_res->SystemBackupUri, "%s://%s:%d/SystemBackup", g_onvif_cfg.https_enable ? "https" : "http", lip, lport);

    return ONVIF_OK;
}

ONVIF_RET onvif_SetNetworkInterfaces(SetNetworkInterfaces_REQ * p_req)
{
    ONVIF_NetworkInterface * p_net_inf = onvif_find_NetworkInterface(p_req->NetworkInterface.token);
    if (NULL == p_net_inf)
    {
        return ONVIF_ERR_InvalidNetworkInterface;
    }

    if (p_req->NetworkInterface.InfoFlag && 
    	p_req->NetworkInterface.Info.MTUFlag && 
    	(p_req->NetworkInterface.Info.MTU < 0 || p_req->NetworkInterface.Info.MTU > 1530))
    {
        return ONVIF_ERR_InvalidMtuValue;
    }

    if (p_req->NetworkInterface.Enabled && 
        p_req->NetworkInterface.IPv4Flag && 
        p_req->NetworkInterface.IPv4.Enabled && 
        p_req->NetworkInterface.IPv4.Config.DHCP == FALSE)
    {
        if (is_ip_address(p_req->NetworkInterface.IPv4.Config.Address) == FALSE)
        {
            return ONVIF_ERR_InvalidIPv4Address;
        }
    }

    // todo : add set network interfaces code ...
    //NOTE:只有在有线环境下才可以设置ip地址
    if (NULL != strstr(p_req->NetworkInterface.token, "eth")) 
	{
	    if (SetNetworkInterfaces(&p_req->NetworkInterface, TRUE) !=0 )
			return -1;
		memset(g_onvif_cfg.servs[0].serv_ip, 0x0, sizeof(g_onvif_cfg.servs[0].serv_ip));
		strcpy(g_onvif_cfg.servs[0].serv_ip, p_req->NetworkInterface.IPv4.Config.Address);
		onvif_init_capabilities();
    }
		
	p_net_inf->NetworkInterface.Enabled = p_req->NetworkInterface.Enabled;

	if (p_req->NetworkInterface.InfoFlag && p_req->NetworkInterface.Info.MTUFlag)
	{
		p_net_inf->NetworkInterface.Info.MTU = p_req->NetworkInterface.Info.MTU;
	}
    	
	if (p_req->NetworkInterface.IPv4Flag)
	{
		p_net_inf->NetworkInterface.IPv4.Enabled = p_req->NetworkInterface.IPv4.Enabled;
		p_net_inf->NetworkInterface.IPv4.Config.DHCP = p_req->NetworkInterface.IPv4.Config.DHCP;
		
		if (p_net_inf->NetworkInterface.IPv4.Config.DHCP == FALSE)
		{
	    	strcpy(p_net_inf->NetworkInterface.IPv4.Config.Address, p_req->NetworkInterface.IPv4.Config.Address);
	    	p_net_inf->NetworkInterface.IPv4.Config.PrefixLength = p_req->NetworkInterface.IPv4.Config.PrefixLength;
	    }
    }
    
	return ONVIF_OK;
}

ONVIF_RET onvif_SetDiscoveryMode(SetDiscoveryMode_REQ * p_req)
{
	g_onvif_cfg.network.DiscoveryMode = p_req->DiscoveryMode;

	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_InvalidNetworkInterface
	ONVIF_ERR_InvalidDot11
	ONVIF_ERR_NotDot11
	ONVIF_ERR_NotConnectedDot11
 */
ONVIF_RET onvif_GetDot11Status(GetDot11Status_REQ * p_req, GetDot11Status_RES * p_res)
{    
    ONVIF_NetworkInterface * p_net_inf = onvif_find_NetworkInterface(p_req->InterfaceToken);
    if (NULL == p_net_inf)
    {
        return ONVIF_ERR_InvalidNetworkInterface;
    }
    
    // todo : add get dot11 status code ...

    
    return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_InvalidNetworkInterface
	ONVIF_ERR_InvalidDot11
	ONVIF_ERR_NotDot11
	ONVIF_ERR_NotScanAvailable
 */
ONVIF_RET onvif_ScanAvailableDot11Networks(ScanAvailableDot11Networks_REQ * p_req, ScanAvailableDot11Networks_RES * p_res)
{
    ONVIF_NetworkInterface * p_net_inf = onvif_find_NetworkInterface(p_req->InterfaceToken);
    if (NULL == p_net_inf)
    {
        return ONVIF_ERR_InvalidNetworkInterface;
    }

    if (g_onvif_cfg.Capabilities.dot11.ScanAvailableNetworks == 0)
    {
        return ONVIF_ERR_NotScanAvailable;
    }

    // todo : add scan availabe dot11 networks code ...

    
    return ONVIF_OK;
}

ONVIF_RET onvif_CreateUsers(CreateUsers_REQ * p_req)
{
    int i;
	for (i = 0; i < ARRAY_SIZE(p_req->User); i++)
	{
	    int len;
	    onvif_User * p_idle_user;
	    
		if (p_req->User[i].Username[0] == '\0')
		{
			break;
		}

		len = strlen(p_req->User[i].Username);
		if (len <= 1)
		{
			return ONVIF_ERR_UsernameTooShort;
		}

		if (onvif_is_user_exist(p_req->User[i].Username))
		{
			return ONVIF_ERR_UsernameClash;
		}
		
		p_idle_user = onvif_get_idle_user();	//g_onvif_cfg.users[i]
		if (p_idle_user)
		{
			memcpy(p_idle_user, &p_req->User[i], sizeof(onvif_User));

#ifdef PROFILE_Q_SUPPORT
            if (UserLevel_Administrator == p_req->User[i].UserLevel && 
                p_req->User[i].Password[0] != '\0')
            {
                onvif_switchDeviceState(1); // creates a new admin user, switch to Operational State
            }
#endif			
		}
		else
		{
			return ONVIF_ERR_TooManyUsers;
		}
	}
	//// add by xieqingpu 
	if (writeUsers(g_onvif_cfg.users, ARRAY_SIZE(g_onvif_cfg.users)) != 0)   //写用户到文件保存起来
		printf(" write user faile.\n");	

	return ONVIF_OK;
}

ONVIF_RET onvif_DeleteUsers(DeleteUsers_REQ * p_req)
{
    int i;
	onvif_User * p_item = NULL;
	
	for (i = 0; i < ARRAY_SIZE(p_req->Username); i++)
	{
		if (p_req->Username[i][0] == '\0')
		{
			break;
		}

		p_item = onvif_find_user(p_req->Username[i]);
		if (NULL == p_item)
		{
			return ONVIF_ERR_UsernameMissing;
		}
		else if (p_item->fixed)
		{
			return ONVIF_ERR_FixedUser;
		}
	}	

	for (i = 0; i < ARRAY_SIZE(p_req->Username); i++)
	{
		if (p_req->Username[i][0] == '\0')
		{
			break;
		}

		p_item = onvif_find_user(p_req->Username[i]);
		if (NULL != p_item && p_item->fixed == FALSE)
		{
			memset(p_item, 0, sizeof(onvif_User));
		}
	}

	//删除后更新用户文件
	if (writeUsers(g_onvif_cfg.users, ARRAY_SIZE(g_onvif_cfg.users)) != 0)   //写用户到文件保存起来
	printf(" write user faile.\n");	

	return ONVIF_OK;
}

ONVIF_RET onvif_SetUser(SetUser_REQ * p_req)
{
    int i;
	onvif_User * p_item = NULL;
	
	for (i = 0; i < ARRAY_SIZE(p_req->User); i++)
	{
		if (p_req->User[i].Username[0] == '\0')
		{
			break;
		}
		
		p_item = onvif_find_user(p_req->User[i].Username);
		if (NULL == p_item)
		{
			return ONVIF_ERR_UsernameMissing;
		}
		else if (p_item && p_item->fixed)
		{
			return ONVIF_ERR_FixedUser;
		}		
	}
	
	for (i = 0; i < ARRAY_SIZE(p_req->User); i++)
	{
		if (p_req->User[i].Username[0] == '\0')
		{
			break;
		}
		
		p_item = onvif_find_user(p_req->User[i].Username);
		if (p_item && FALSE == p_item->fixed)
		{
			strcpy(p_item->Password, p_req->User[i].Password);
			p_item->UserLevel = p_req->User[i].UserLevel;

#ifdef PROFILE_Q_SUPPORT
            if (UserLevel_Administrator == p_item->UserLevel && 
                p_item->Password[0] != '\0')
            {
                onvif_switchDeviceState(1); // modifies the password of an existing admin user, switch to Operational State
            }
#endif
		}
	}

	//删除后更新用户文件   add by xieqingpu
	if (writeUsers(g_onvif_cfg.users, ARRAY_SIZE(g_onvif_cfg.users)) != 0)   //写用户到文件保存起来
	printf(" write user faile.\n");	

	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_NotRemoteUser
	ONVIF_OK
 */ 
ONVIF_RET onvif_GetRemoteUser(GetRemoteUser_RES * p_res)
{
    // todo : get the remote user ...


    // return ONVIF_ERR_NotRemoteUser;

    if (g_onvif_cfg.RemoteUser.Username[0] == '\0')
    {
        p_res->RemoteUserFlag = 0;
    }
    else
    {
        p_res->RemoteUserFlag = 1;
        strcpy(p_res->RemoteUser.Username, g_onvif_cfg.RemoteUser.Username);
    }
    
    p_res->RemoteUser.UseDerivedPassword = g_onvif_cfg.RemoteUser.UseDerivedPassword;

    return ONVIF_OK; 
}

/**
 The possible return values:
	ONVIF_ERR_NotRemoteUser
	ONVIF_OK
 */
ONVIF_RET onvif_SetRemoteUser(SetRemoteUser_REQ * p_req)
{
    // todo : set the remote user ...

    // To remove the remote user SetRemoteUser should be called without the RemoteUser parameter

    // return ONVIF_ERR_NotRemoteUser;
    
    if (p_req->RemoteUserFlag)
    {
        g_onvif_cfg.RemoteUser.UseDerivedPassword = p_req->RemoteUser.UseDerivedPassword;
        strcpy(g_onvif_cfg.RemoteUser.Username, p_req->RemoteUser.Username);

        if (p_req->RemoteUser.PasswordFlag)
        {
            strcpy(g_onvif_cfg.RemoteUser.Password, p_req->RemoteUser.Password);
        }
    }
    else
    {
        g_onvif_cfg.RemoteUser.UseDerivedPassword = FALSE;
        strcpy(g_onvif_cfg.RemoteUser.Username, "");
        strcpy(g_onvif_cfg.RemoteUser.Password, "");
    }
    
    return ONVIF_OK; 
}

ONVIF_RET onvif_AddScopes(AddScopes_REQ * p_req)
{
    int i;
	for (i = 0; i < ARRAY_SIZE(p_req->ScopeItem); i++)
	{
	    onvif_Scope * p_item;
	    
		if (p_req->ScopeItem[i][0] == '\0')
		{
			break;
		}

		if (onvif_is_scope_exist(p_req->ScopeItem[i]))
		{
			continue;
		}
		
		p_item = onvif_get_idle_scope();
		if (p_item)
		{
			p_item->ScopeDef = ScopeDefinition_Configurable;
			strcpy(p_item->ScopeItem, p_req->ScopeItem[i]);
		}
		else
		{
			return ONVIF_ERR_TooManyScopes;
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_SetScopes(SetScopes_REQ * p_req)
{
    int i;
	onvif_Scope * p_item = NULL;
	
	for (i = 0; i < ARRAY_SIZE(p_req->Scopes); i++)
	{
		if (p_req->Scopes[i][0] == '\0')
		{
			break;
		}
		
		p_item = onvif_find_scope(p_req->Scopes[i]);
		if (p_item && ScopeDefinition_Fixed == p_item->ScopeDef)
		{
			return ONVIF_ERR_ScopeOverwrite;
		}
	}
	
	for (i = 0; i < ARRAY_SIZE(p_req->Scopes); i++)
	{
		if (p_req->Scopes[i][0] == '\0')
		{
			break;
		}
		
		p_item = onvif_find_scope(p_req->Scopes[i]);
		if (p_item && ScopeDefinition_Configurable == p_item->ScopeDef)
		{
			strcpy(p_item->ScopeItem, p_req->Scopes[i]);
		}
		else
		{
		    p_item = onvif_get_idle_scope();
    		if (p_item)
    		{
    			p_item->ScopeDef = ScopeDefinition_Configurable;
    			strcpy(p_item->ScopeItem, p_req->Scopes[i]);
    		}
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_RemoveScopes(RemoveScopes_REQ * p_req)
{
    int i;
	onvif_Scope * p_item = NULL;
	
	for (i = 0; i < ARRAY_SIZE(p_req->ScopeItem); i++)
	{
		if (p_req->ScopeItem[i][0] == '\0')
		{
			break;
		}

		p_item = onvif_find_scope(p_req->ScopeItem[i]);
		if (NULL == p_item)
		{
			return ONVIF_ERR_NoScope;
		}
		else if (ScopeDefinition_Fixed == p_item->ScopeDef)
		{
			return ONVIF_ERR_FixedScope;
		}
	}	

	for (i = 0; i < ARRAY_SIZE(p_req->ScopeItem); i++)
	{
		if (p_req->ScopeItem[i][0] == '\0')
		{
			break;
		}

		p_item = onvif_find_scope(p_req->ScopeItem[i]);
		if (NULL != p_item && ScopeDefinition_Configurable == p_item->ScopeDef)
		{
			memset(p_item, 0, sizeof(onvif_Scope));
		}
	}	

	return ONVIF_OK;
}

BOOL onvif_StartFirmwareUpgrade(const char * lip, uint32 lport, StartFirmwareUpgrade_RES * p_res)
{
	// todo : do some file upload prepare ...
	
#ifdef HTTPS
	if (g_onvif_cfg.https_enable)
	{
		sprintf(p_res->UploadUri, "https://%s:%d/FirmwareUpgrade", lip, lport);
	}
	else
	{
		sprintf(p_res->UploadUri, "http://%s:%d/FirmwareUpgrade", lip, lport);
	}
#else
	sprintf(p_res->UploadUri, "http://%s:%d/FirmwareUpgrade", lip, lport);
#endif
	UTIL_INFO("UploadUri = %s\n",p_res->UploadUri);

	p_res->UploadDelay = 5;				// 5 seconds
	p_res->ExpectedDownTime = 5 * 60; 	// 5 minutes

	return TRUE;
}

/***
  * do some check before the upgrade
  *
  * buff : pointer the upload content
  * len  : the upload content length
  **/
BOOL onvif_FirmwareUpgradeCheck(const char * buff, int len)
{
	// todo : add the check code ...

    // the firmware length is too short
    if (len < 1024*1024)
    {
        return FALSE;
    }
	
	return TRUE;
}

char *onvif_FileMd5Sum (char *path, int md5_len, char *md5_buf)
{
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	uint8 digest[16];
	int i;

	if (NULL == path)
	{
		return NULL;
	}
	
	FILE *fp = fopen (path, "rb");
	if (fp == NULL) {
		fprintf (stderr, "fopen %s failed\n", path);
		return NULL;
	}
	
	MD5Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, fp)) != 0)
	{
		MD5Update (&mdContext, data, bytes);
	}
	MD5Final (digest, &mdContext);

	if(md5_len == 16)
	{
		for(i=4; i<12; i++)
		{
			sprintf(&md5_buf[(i-4)*2], "%02x", digest[i]);
		}
	}
	else if(md5_len == 32)
	{
		for(i=0; i<16; i++)
		{
			sprintf(&md5_buf[i*2], "%02x", digest[i]);
		}
	}
	else
	{
		fclose(fp);
		return NULL;
	}

	fclose (fp);

	return md5_buf;
}

int onvif_FileMd5Verify(char *path, char *decodepath)
{
	char filebuffer[128] = {0};
	char *password = "gptohQBsL4TB2tPAaJ7LAMBK99VFMGuX";
	char md5str[64] = {0};
	FILE *fp = NULL;
	FILE *decodefp = NULL;
	int bytes;
	unsigned char data[1024*1024];
	struct stat buf;
	int count = 0;

	
	if (NULL == path || NULL == decodepath)
	{
		return -1;
	}
	
    stat(path, &buf);
	if (buf.st_size <= 1024*1024)
	{
		UTIL_ERR("path(%s)st_size <= 1024*1024 error!!!!!", path);
		return -1;
	}
	
	fp = fopen (path, "rb");
	if (fp == NULL) {
		UTIL_ERR ("fopen %s failed", path);
		return -1;
	}
	
	decodefp = fopen (decodepath, "wb+");
	if (decodefp == NULL) {
		UTIL_ERR ("decode fopen %s failed\n", decodepath);
		fclose(fp);
		return -1;
	}

	while ((bytes = fread (data, 1, 1024*1024, fp)) != 0)
	{
		fwrite (data, 1, bytes, decodefp);
		if (0 == count)
		{
			bytes = fread(filebuffer, 1, 64, fp);
			count = 1;
			if (strstr(filebuffer, password) == NULL)
			{
				UTIL_ERR ("failed!!!filebuffer %s,password=%s", filebuffer, password);
				count = 0;
				break;
			}
			else 
			{
			    //获取原始升级文件的MD5验证文件的完整性
				memcpy(md5str, &filebuffer[32],  32);
				md5str[strlen(md5str)] = '\0';
			}
		}
	}

	fclose(fp);
	fclose(decodefp);
	unlink(path);

	if (0 == count)
	{
		return -1;
	}

	onvif_FileMd5Sum(decodepath, 32, filebuffer);
	filebuffer[strlen(filebuffer)] = '\0';
	if (strcasecmp(filebuffer, md5str) == 0)
	{
		UTIL_INFO ("Verify success!!!!!!!");
		return 0;
	}
	return -1;
}

/***
  * begin firmware upgrade
  *
  * buff : pointer the upload content
  * len  : the upload content length
  **/
BOOL onvif_FirmwareUpgrade(const char * buff, int len, char *decodefile)
{ 
	// todo : add the upgrade code ... 
	FILE *pfd = NULL;
	BOOL isOK = FALSE;
	char  *Content_Type = NULL, *nchrBegin = NULL; 
	const char *pbuffer = NULL, *nchrEnd = NULL;
	char *endneedle = NULL; 
	int   endlength = 0, firstlen = 0, length = 0;
	int i =0;
	char md5str[64] = {0};
	char wholefilemd5str[64] = {0};
	char *uploadfile = "/user/tempgpt";
	
	if (NULL == buff) {
		return FALSE;
	}
	
	pfd = fopen(uploadfile, "wb+");
	if (NULL == pfd) { 
		UTIL_ERR("############open %s error", uploadfile);  
		return FALSE; 
	} 
	
	pbuffer = buff;
	//Content-Type: application/octet-stream
	Content_Type = strcasestr(pbuffer, (const char *)"Content-Type");
	if (NULL == Content_Type) {
		isOK = FALSE;
		UTIL_ERR("############NULL == Content-Type"); 
		goto _EXIT1;
	}
	
	nchrBegin = strstr(Content_Type, "\r\n\r\n");  
	if (NULL == nchrBegin) {
		isOK = FALSE;
		UTIL_ERR("############NULL == nchrBegin"); 
		goto _EXIT1;
	}
	
	firstlen = (nchrBegin + 4) - pbuffer;
	nchrEnd = pbuffer + (len - 200);
	for (i = 0; i < 200;i++) {
		if (nchrEnd[i]== 0xD && nchrEnd[i+1]== 0xA) {
			break;
		}
	}
	endlength = 200 - i;
	length	= len - endlength - firstlen;
	UTIL_INFO("firstlen=%d, endlength=%d, length=%d",firstlen, endlength, length);  
	//获取MD5值
	memcpy(wholefilemd5str, nchrBegin+4, 32);
	wholefilemd5str[32] = '\0';
	//减去md5 32位
	length -= 32;
	nchrBegin += 32;

	if (fwrite(nchrBegin+4, 1, length,	pfd) != length) 
	{  
		isOK = FALSE;
		goto _EXIT1;
	} 

	//无论如何先关闭文件然后计算md5
	if (pfd)
	{
		fclose(pfd); 
		pfd = NULL;
	}
	
	onvif_FileMd5Sum(uploadfile, 32, md5str);
	if ((strcasecmp(wholefilemd5str, md5str) == 0) && 
		(0 == onvif_FileMd5Verify(uploadfile, decodefile)))
	{
		UTIL_INFO("FirmwareUpgrade OK!!!"); 
		isOK = TRUE;
	}
	else
	{	
		UTIL_ERR("FirmwareUpgrade failed!!!");
	}
	
_EXIT1:	
	if (pfd)
	{
		fclose(pfd); 
		pfd = NULL;
	}

	return isOK;
}

/***
  * After the upgrade is complete do some works, such as reboot device ...
  *  
  **/
void onvif_FirmwareUpgradePost(char *decodefile)
{
	if (0 != access("/user/upload.sh", F_OK)) {
		FILE *pConf = NULL;
		pConf = fopen("/user/upload.sh", "w+");
		if(pConf == NULL)
		{
			return ;
		}
		else
		{  
			fprintf(pConf, "#/bin/bash\n");
			fprintf(pConf, "uploadfile=$1\n");
			fprintf(pConf, "echo \"uploadfile:\"$uploadfile\n");
			fprintf(pConf, "if [ -e $uploadfile ] ; then\n");
			fprintf(pConf, "	if [ -e /user/gpt.tar.gz ] ; then\n");
			fprintf(pConf, "		rm /user/gpt.tar.gz -rf\n");
			fprintf(pConf, "	fi\n");
			fprintf(pConf, "mv $uploadfile  /user/gpt.tar.gz\n");
			fprintf(pConf, "if [ -d /user/gpt ] ; then\n");
			fprintf(pConf, "	rm /user/gpt -rf\n");
			fprintf(pConf, "fi\n");
			fprintf(pConf, "cd /user/\n");
			fprintf(pConf, "tar xf gpt.tar.gz\n");
			fprintf(pConf, "reboot;sleep 1;reboot -f\n");
			fprintf(pConf, "fi\n");
		}
	}
	system_ex("/user/upload.sh %s &", decodefile);
}

BOOL onvif_StartSystemRestore(const char * lip, uint32 lport, StartSystemRestore_RES * p_res)
{
    // todo : do some file upload prepare ...
	
#ifdef HTTPS
	if (g_onvif_cfg.https_enable)
	{
		sprintf(p_res->UploadUri, "https://%s:%d/SystemRestore", lip, lport);
	}
	else
	{
		sprintf(p_res->UploadUri, "http://%s:%d/SystemRestore", lip, lport);
	}
#else
	sprintf(p_res->UploadUri, "http://%s:%d/SystemRestore", lip, lport);
#endif

	p_res->ExpectedDownTime = 5 * 60; 	// 5 minutes

	return TRUE;
}

/***
  * do some check before the restore
  *
  * buff : pointer the upload content
  * len  : the upload content length
  **/
BOOL onvif_SystemRestoreCheck(const char * buff, int len)
{
	// todo : add the check code ...
	
	if (NULL == buff || len < 10)
	{
	    return FALSE;
	}
	
	return TRUE;
}

/***
  * begin system restore
  *
  * buff : pointer the upload content
  * len  : the upload content length
  **/
BOOL onvif_SystemRestore(const char * buff, int len)
{
	// todo : add the system restore code ...

	
	return TRUE;
}

/***
  * After the system restore is complete do some works, such as reboot device ...
  *  
  **/
void onvif_SystemRestorePost()
{
    // please comment the code below
    // send onvif hello message, just for test
    sleep(3);
    onvif_hello();
}

#ifdef IPFILTER_SUPPORT	

ONVIF_RET onvif_SetIPAddressFilter(SetIPAddressFilter_REQ * p_req)
{
	memcpy(&g_onvif_cfg.ipaddr_filter, &p_req->IPAddressFilter, sizeof(onvif_IPAddressFilter));

	// todo : add your handler code ...
	
	
	return ONVIF_OK;
}

ONVIF_RET onvif_AddIPAddressFilter(AddIPAddressFilter_REQ * p_req)
{
    int i;
    onvif_PrefixedIPAddress * p_item;
    
	for (i = 0; i < ARRAY_SIZE(p_req->IPAddressFilter.IPv4Address); i++)
	{
		if (p_req->IPAddressFilter.IPv4Address[i].Address[0] == '\0')
		{
			break;
		}

		if (onvif_is_ipaddr_filter_exist(g_onvif_cfg.ipaddr_filter.IPv4Address, ARRAY_SIZE(g_onvif_cfg.ipaddr_filter.IPv4Address), &p_req->IPAddressFilter.IPv4Address[i]))
		{
			continue;
		}
		
		p_item = onvif_get_idle_ipaddr_filter(g_onvif_cfg.ipaddr_filter.IPv4Address, ARRAY_SIZE(g_onvif_cfg.ipaddr_filter.IPv4Address));
		if (p_item)
		{
			p_item->PrefixLength = p_req->IPAddressFilter.IPv4Address[i].PrefixLength;
			strcpy(p_item->Address, p_req->IPAddressFilter.IPv4Address[i].Address);
		}
		else
		{
			return ONVIF_ERR_IPFilterListIsFull;
		}
	}

	for (i = 0; i < ARRAY_SIZE(p_req->IPAddressFilter.IPv6Address); i++)
	{	    
		if (p_req->IPAddressFilter.IPv6Address[i].Address[0] == '\0')
		{
			break;
		}

		if (onvif_is_ipaddr_filter_exist(g_onvif_cfg.ipaddr_filter.IPv6Address, ARRAY_SIZE(g_onvif_cfg.ipaddr_filter.IPv6Address), &p_req->IPAddressFilter.IPv6Address[i]))
		{
			continue;
		}
		
		p_item = onvif_get_idle_ipaddr_filter(g_onvif_cfg.ipaddr_filter.IPv6Address, ARRAY_SIZE(g_onvif_cfg.ipaddr_filter.IPv6Address));
		if (p_item)
		{
			p_item->PrefixLength = p_req->IPAddressFilter.IPv6Address[i].PrefixLength;
			strcpy(p_item->Address, p_req->IPAddressFilter.IPv6Address[i].Address);
		}
		else
		{
			return ONVIF_ERR_IPFilterListIsFull;
		}
	}

	// todo : add your handler code ...

	
	return ONVIF_OK;
}

ONVIF_RET onvif_RemoveIPAddressFilter(RemoveIPAddressFilter_REQ * p_req)
{
    int i;
	onvif_PrefixedIPAddress * p_item = NULL;
	
	for (i = 0; i < ARRAY_SIZE(p_req->IPAddressFilter.IPv4Address); i++)
	{
		if (p_req->IPAddressFilter.IPv4Address[i].Address[0] == '\0')
		{
			break;
		}

		p_item = onvif_find_ipaddr_filter(g_onvif_cfg.ipaddr_filter.IPv4Address, ARRAY_SIZE(g_onvif_cfg.ipaddr_filter.IPv4Address), &p_req->IPAddressFilter.IPv4Address[i]);
		if (NULL == p_item)
		{
			return ONVIF_ERR_NoIPv4Address;
		}
	}

	for (i = 0; i < ARRAY_SIZE(p_req->IPAddressFilter.IPv6Address); i++)
	{
		if (p_req->IPAddressFilter.IPv6Address[i].Address[0] == '\0')
		{
			break;
		}

		p_item = onvif_find_ipaddr_filter(g_onvif_cfg.ipaddr_filter.IPv6Address, ARRAY_SIZE(g_onvif_cfg.ipaddr_filter.IPv6Address), &p_req->IPAddressFilter.IPv6Address[i]);
		if (NULL == p_item)
		{
			return ONVIF_ERR_NoIPv6Address;
		}
	}

	// todo : add your handler code ...
	

	for (i = 0; i < ARRAY_SIZE(p_req->IPAddressFilter.IPv4Address); i++)
	{
		if (p_req->IPAddressFilter.IPv4Address[i].Address[0] == '\0')
		{
			break;
		}

		p_item = onvif_find_ipaddr_filter(g_onvif_cfg.ipaddr_filter.IPv4Address, ARRAY_SIZE(g_onvif_cfg.ipaddr_filter.IPv4Address), &p_req->IPAddressFilter.IPv4Address[i]);
		if (NULL != p_item)
		{
			memset(p_item, 0, sizeof(onvif_PrefixedIPAddress));
		}
	}

	for (i = 0; i < ARRAY_SIZE(p_req->IPAddressFilter.IPv6Address); i++)
	{
		if (p_req->IPAddressFilter.IPv6Address[i].Address[0] == '\0')
		{
			break;
		}

		p_item = onvif_find_ipaddr_filter(g_onvif_cfg.ipaddr_filter.IPv6Address, ARRAY_SIZE(g_onvif_cfg.ipaddr_filter.IPv6Address), &p_req->IPAddressFilter.IPv6Address[i]);
		if (NULL != p_item)
		{
			memset(p_item, 0, sizeof(onvif_PrefixedIPAddress));
		}
	}
    
	return ONVIF_OK;
}

#endif // end of IPFILTER_SUPPORT



