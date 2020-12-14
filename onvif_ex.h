#ifndef __ONVIF_EXTEND_H__
#define	__ONVIF_EXTEND_H__

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************
* FuncName: onvifStart       
* Describe:  开始onvif服务端
* Params  :  bdebug  是否打开日志  true打开                            
**********************************************/
void onvifStart(int	 bdebug);

/*********************************************
* FuncName: onvifStop       
* Describe:  结束onvif服务端并清空服务
* Params  :                                
**********************************************/
void onvifStop(void);

#ifdef __cplusplus
}
#endif

#endif

