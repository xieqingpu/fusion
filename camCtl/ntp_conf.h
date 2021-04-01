#ifndef __NTP_CONF_H__
#define __NTP_CONF_H__

#ifdef __cplusplus
extern "C"
{
#endif

//ntp是否完成时间同步
int Get_Ntp_Sync_Time_Done();
void Set_Ntp_Sync_Time(int value);

int ntp_update_run();

#ifdef __cplusplus
}
#endif

#endif //__NTP_CONF_H__
