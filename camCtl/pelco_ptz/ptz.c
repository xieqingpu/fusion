/*-------------------------------- pelco.c -------------------------------
*	Copyright 2002 Briggs Media Systems
*	may 24 02	 modified for pelco camera	J Briggs
*----------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include "ptz.h"
#include "utils_log.h"

unsigned char pelco[7];
int addr = 1;

struct baud_rates {
        unsigned int rate;
        unsigned int cflag;
};

static const struct baud_rates baud_rates[] = {
        { 921600, B921600 },
        { 460800, B460800 },
        { 230400, B230400 },
        { 115200, B115200 },
        {  57600, B57600  },
        {  38400, B38400  },
        {  19200, B19200  },
        {   9600, B9600   },
        {   4800, B4800   },
        {   2400, B2400   },
        {   1200, B1200   },
        {      0, B38400  }
};

int fdOut = -1;
int fdIn = -1;
int *fd=&fdOut;


error_code  pelco_close()
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;
    close(*fd);
    ret=RET_OK;
	return ret;
}

error_code pelco_Init( char* device, u_int32 baud_rate)
{
    int ret = RET_ERR;
	struct termios newtio;
	
    if (NULL == device) {
		return ret;
    }

	if (device && 0 != access(device, F_OK)) {
		UTIL_ERR("device==%s no exsit!!!", device);
		return ret;
	}
	
	if( (*fd = open(device,O_RDWR|O_NOCTTY)) < 0 )
	{
		UTIL_ERR("device:%s,ERROR:%d(%s)!!", device, errno, strerror(errno));
		return ret;
	}

	//tcgetattr(*fd,&oldtio);
	bzero(&newtio,sizeof(newtio));

    cfsetospeed(&newtio, baud_rate);
    
    /*
     * raw mode
     */
    newtio.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    newtio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | INLCR);
    newtio.c_oflag &= ~(OPOST);
    newtio.c_cflag &= ~(CSIZE | PARENB);
	newtio.c_cflag &= ~PARODD;
	newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag |= CS8;

    /*
     * 'DATA_LEN' bytes can be read by serial
     */
    newtio.c_cc[VMIN]   =   0;                                      
    newtio.c_cc[VTIME]  =   10;

	tcflush(*fd,TCIFLUSH);
	tcsetattr(*fd,TCSANOW,&newtio);
    fdIn=fdOut;
    ret=RET_OK;

	return ret;
}


/*
*	pelcoput - place a single char into the transmit buffer
*/
error_code pelcoPut( unsigned char c )
{
	if(fdOut)
	{
		if( write(fdOut,&c,1) < 0)
		{
			UTIL_ERR("error:pelcoput:write");
			return -1;
		}
		return 0;
	}
	return -1;
}
/*
*	pelcowrite - normal entry point - write a block of data to the
*	serial port.
*/
error_code pelcoWrite( unsigned char *buf, u_int32 size )
{
	// error_code ec = 0;
	error_code ec = RET_OK;
	int count = 0;

	if( size == 0 ) return RET_ERR;
    usleep(100);
	if( (count = write(fdOut,buf,size)) != size )
	{
		if( count < 0 )
		{
			UTIL_ERR("ERROR: %s:WRITE %s", __FUNCTION__, strerror(errno));
			ec = RET_ERR;
			sleep(2);
		}
	}
	
	return ec;
}
/*
*	error_code pelcoget - fetch a char from the receive buffer or a
*	error code.
*/
error_code pelcoGet( unsigned char *c,int fd )
{
	if(fd)
	{   
		if( read(fd,c,1) < 0 )
		{
			UTIL_ERR("ERROR:%s[%u] %s", __FUNCTION__, fd, strerror(errno));
			return -1;
		}
        //printf("%s,%d\r\n",__FUNCTION__,__LINE__);
		return 0;
	}
	return -1;
}
/*
*	pelcowrite - normal entry point - write a block of data to the
*	serial port.
*/
error_code pelcoRead( unsigned char *buf, u_int32 size,int fd )
{
	int count = 0;
	error_code ec = 0;

    if( fd != fdIn ) return 0;
	memset(buf,0,size);
	while( !pelcoGet(&buf[0],fd) && (buf[0] != 0xff) )
	{
        UTIL_ERR("read data start:%02x",buf[0]);
    }
	if( (count = read(fd,&buf[1],size-1)) != size-1)
	{
		UTIL_ERR("WARNING:%s: %u vs. %u bytes",__FUNCTION__,count,size);
		ec = 1;
	}

	return ec;
}
/*
*	pelcowrite - normal entry point - write a block of data to the
*	serial port.
*/
error_code pelcoRead_test( unsigned char *buf, u_int32 size,int fd)
{
	int count = 0;
    int read_times=25;
	error_code ec = 0;

    if( fd != fdIn ) return 0;
	memset(buf,0,size);
    //printf("%s,%d\r\n",__FUNCTION__,__LINE__);
    while( !pelcoGet(&buf[0],fd) && (buf[0] != 0xff)&& read_times--)
    {
        sleep(1);
        UTIL_ERR("read unknow data start:%02x",buf[0]);
    }
	if( buf[0] != 0xff) 
	{
          UTIL_ERR("read error data start:%02x",buf[0]);

    }
    else
    {
    	if( (count = read(fd,&buf[1],size-1)) != size-1)
    	{
    		UTIL_ERR("WARNING:%s: %u vs. %u bytes", __FUNCTION__, count, size);
    		ec = 1;
    	}
    }
	return ec;
}
unsigned char get_checksum(unsigned char * buf ,int size)
{
	int i;
	unsigned char ret = 0;
	for( i = 0; i < size; ret += buf[i++] );
	return ret;
}

/* 校验码(第7字节),第2-6个字节和 */
void pelcoChecksum(void)
{
	int i;
	unsigned char c = 0;
	for( i = 1; i < (sizeof(pelco)-1); c += pelco[i++] );
	pelco[i] = c;

}

//////////////////////////
error_code pelco_Left(unsigned short force)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;
    
	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x04;
	pelco[p_data1] = force;
	pelco[p_data2] = 0x00;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_Right(unsigned short force)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;
	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x02;
	pelco[p_data1] = force;
	pelco[p_data2] = 0x00;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));

	// printf("pelco_Right,pelco[p_data1] = %x\n",pelco[p_data1]);
	return ret;

}

error_code pelco_Up(unsigned short force)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;
	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x08;
	pelco[p_data1] = 0x00;
	pelco[p_data2] = force;
	pelcoChecksum();
    ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_Down(unsigned short force)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;

	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x10;
	pelco[p_data1] = 0x00;
	pelco[p_data2] = force;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;
}
error_code pelco_left_down(unsigned short force)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;

	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x14;
	pelco[p_data1] = force;
	pelco[p_data2] = force;
	// pelco[p_data2] = force>>8;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_left_up(unsigned short force)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;

	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x0c;
	pelco[p_data1] = force;
	pelco[p_data2] = force;
	// pelco[p_data2] = force>>8;
	pelcoChecksum();	//校验码(第7字节)
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_right_down(unsigned short force)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;

	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x12;
	pelco[p_data1] = force;
	pelco[p_data2] = force;
	// pelco[p_data2] = force>>8;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_right_up(unsigned short force)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;

	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x0a;
	pelco[p_data1] = force;
	pelco[p_data2] = force;
	// pelco[p_data2] = force>>8;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_Stop(void)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;

	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x00;
	pelco[p_data1] = 0x00;
	pelco[p_data2] = 0x00;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_set_point(unsigned short location)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;

	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x03;
	pelco[p_data1] = 0x00;
	pelco[p_data2] = location;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_get_point(unsigned short  location)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;
	memset(pelco,0,sizeof(pelco));
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x07;
	pelco[p_data1] = 0x00;
	pelco[p_data2] = location;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}

error_code pelco_set_vertical(unsigned short location)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;
	memset(pelco,0,sizeof(pelco));
    //FF 01 00 02 10 00 13
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x4d;
	pelco[p_data2] = location;
	pelco[p_data1] = location>>8;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
    
	return ret;

}
error_code pelco_get_vertical(unsigned short *location)
{
    int ret=-1;
    if(fdOut<0)return ret;
    unsigned short real_data=0;
	memset(pelco,0,sizeof(pelco));
    //FF 01 00 02 10 00 13
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x53;
	pelco[p_data2] = 0;
	pelco[p_data1] = 0;
	pelcoChecksum();
	pelcoWrite(pelco,sizeof(pelco));
    usleep(10000);
    memset(pelco,0,sizeof(pelco));
    pelcoRead(pelco,sizeof(pelco),fdIn);
    if(pelco[p_cksum]==get_checksum(pelco+1,5)&&pelco[p_cmd2]==0x5b)
    {
        real_data|=pelco[p_data2];
        real_data=real_data<<8;
        real_data|=pelco[p_data1];
        if(location)*location=real_data;
        ret=RET_OK;
    }
	return ret;
}
error_code pelco_get_horizontal(unsigned short* location)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;
    unsigned short real_data=0;
	memset(pelco,0,sizeof(pelco));
    //FF 01 00 02 10 00 13
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x51;
	pelco[p_data2] = 0;
	pelco[p_data1] = 0;
	pelcoChecksum();
	pelcoWrite(pelco,sizeof(pelco));
    usleep(10000);
    memset(pelco,0,sizeof(pelco));
    pelcoRead(pelco,sizeof(pelco),fdIn);
    if(pelco[p_cksum]==get_checksum(pelco+1,5)&&pelco[p_cmd2]==0x59)
    {

        real_data|=pelco[p_data2];
        real_data=real_data<<8;
        real_data|=pelco[p_data1];
        if(location)*location=real_data;
        ret=RET_OK;
    }
	return ret;
}

error_code pelco_set_horizontal(unsigned short location)
{
    int ret=RET_ERR;
    if(fdOut<0)return ret;
	memset(pelco,0,sizeof(pelco));
    //FF 01 00 02 10 00 13
	pelco[p_sync] = 0xff;
	pelco[p_addr] = addr;
	pelco[p_cmd1] = 0x00;
	pelco[p_cmd2] = 0x4b;
	pelco[p_data2] = location;
	pelco[p_data1] = location>>8;
	pelcoChecksum();
	ret = pelcoWrite(pelco,sizeof(pelco));
	
	return ret;
}

