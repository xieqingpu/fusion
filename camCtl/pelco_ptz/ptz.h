/*---------------------------- pelco.h---------------------------------------
*	Serial control port for 1650A.
*	01/18/97	started	Joe Briggs
*-------------------------------------------------------------------------*/
#ifndef __PTZ_H
#define __PTZ_H
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define RET_OK  0
#define RET_ERR  -1

typedef int error_code;
typedef unsigned int u_int32;

typedef enum {p_sync,p_addr,p_cmd1,p_cmd2,p_data1,p_data2,p_cksum}pelco_t;
/*
ʮ�����������Ӧ�ò����������ϳ��ḻ���ӵĿ�����̨����
���º�������ֻ�ǵ�ǰ�������д״̬��ȷ��񣬲����ܱ�֤��̨��ȷ���պ�ִ�У���ҪӦ�ò�ͨ����ѯִ��ǰ��ı仯�����ж�
*/


/* device:�򿪵Ĵ����豸����,baud_rate:baud_rate */
error_code pelco_Init(char* device, u_int32 baud_rate);
error_code  pelco_close();
/* force:Speed  ���������ٶ�ֻȡ��8λ*/
error_code pelco_Left(unsigned short force);
error_code pelco_Right(unsigned short force);
error_code pelco_Up(unsigned short force);
error_code pelco_Down(unsigned short force);

/* force:Speed  ��8λ��ˮƽ�����ٶ�ȡֵ����(0x00-0x3F)����8λ�Ǵ�ֱ������ٶ�ȡֵ����(0x00-0x3F)*/

error_code pelco_left_down(unsigned short force);
error_code pelco_right_down(unsigned short force);
error_code pelco_left_up(unsigned short force);
error_code pelco_right_up(unsigned short force);

/* Ԥ�õ����ú͵�������
location:location  ֻ�õ�8λ,ȡֵ��Χ0-0x80 
*/
error_code pelco_set_point(unsigned short location);
error_code pelco_get_point(unsigned short location);


/*����λ��  location:locationҪ���õ�λ������   */
error_code pelco_set_vertical(unsigned short location);
error_code pelco_set_horizontal(unsigned short location);


/*��ȡλ��  location:location ���ص�λ������ */

error_code pelco_get_vertical(unsigned short *location);
error_code pelco_get_horizontal(unsigned short* location);

/*ֹͣ�˶�*/
error_code pelco_Stop(void);


#endif /* ptz */
