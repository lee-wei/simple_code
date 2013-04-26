#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>

#include <linux/videodev2.h>
#include "uvcdevice.h"

//command
#define VC_EXTENSION_UNIT               0x06
#define ERROR	1
#define OK		0

static pV4L_DEVINFO gpInfo;

struct uvc_xu_control_info {
	__u8 entity[16];
	__u8 index;
	__u8 selector;
	__u16 size;
	__u32 flags;
};

struct uvc_xu_control {
	__u8 unit;
	__u8 selector;
	__u16 size;
	__u8 *data;
};

#define UVCIOC_CTRL_ADD	_IOW('U', 1, struct uvc_xu_control_info)
#define UVCIOC_CTRL_MAP	_IOWR('U', 2, struct uvc_xu_control_mapping)
#define UVCIOC_CTRL_GET	_IOWR('U', 3, struct uvc_xu_control)
#define UVCIOC_CTRL_SET		_IOW('U', 4, struct uvc_xu_control)

/* Control flags */
#define UVC_CONTROL_SET_CUR	(1 << 0)
#define UVC_CONTROL_GET_CUR	(1 << 1)
#define UVC_CONTROL_GET_MIN	(1 << 2)
#define UVC_CONTROL_GET_MAX	(1 << 3)
#define UVC_CONTROL_GET_RES	(1 << 4)
#define UVC_CONTROL_GET_DEF	(1 << 5)
int fd;
char	*s;	
static int xioctl(int  fd, int  request,void *arg)
{
        int r;
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}

static void AddExUnitControl(pV4L_DEVINFO pInfo)
{
	int nCtrl, i, ret;
	struct uvc_xu_control_info extinfos[] = {
		{
			UVC_GUID_AU3820_XU,
			index: 0,
			selector: XU_I2C_ADDR,
			size: 4,
			flags: UVC_CONTROL_GET_CUR | UVC_CONTROL_SET_CUR
		},
		{
			UVC_GUID_AU3820_XU,
			index: 1,
			selector: XU_I2C_DATA,
			size: 2,
			flags: UVC_CONTROL_GET_CUR | UVC_CONTROL_SET_CUR
		},
		{
			UVC_GUID_AU3820_XU,
			index: 3,
			selector: XU_FLIP_CMD,
			size: 1,
			flags: UVC_CONTROL_GET_CUR | UVC_CONTROL_SET_CUR
		},
		{
			UVC_GUID_AU3820_XU,
			index: 4,
			selector: XU_MIRROR_CMD,
			size: 1,
			flags: UVC_CONTROL_GET_CUR | UVC_CONTROL_SET_CUR
		},	
		{
			UVC_GUID_AU3820_XU,
			index: 5,
			selector: XU_REG_SEQUENCIAL_RW,
			size: 11,
			flags: UVC_CONTROL_GET_CUR | UVC_CONTROL_SET_CUR
		},
                {
			UVC_GUID_AU3820_XU,
			index: 6,
			selector: XU_SENSOR_SEQUENCIAL_RW,
			size: 11,
			flags: UVC_CONTROL_GET_CUR | UVC_CONTROL_SET_CUR
		}
	};
	nCtrl = sizeof(extinfos)/sizeof(struct uvc_xu_control_info);
	for(i=0; i<nCtrl; i++){
		ret = xioctl(pInfo->fd, UVCIOC_CTRL_ADD, &extinfos[i]); 
		if(ret == -EPERM){
			printf("Need adminstrator privilege to add extension unit control %d\n", i);
		}else if(ret < 0){
			printf("Add extension control failed %d\n", i);
		}
	}
}



// Init camera device
static pV4L_DEVINFO InitCamera(int fd, char* devname)
{
	struct v4l2_capability cap;
    pV4L_DEVINFO pInfo;
	
	if( xioctl(fd, VIDIOC_QUERYCAP, &cap)  <  0  ) {
		printf("Init camera QUERYCAP failed\n");
		exit(EXIT_FAILURE);
	}
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
		printf("device is not a capture device\n");
		exit(EXIT_FAILURE);
	}
	if(!(cap.capabilities & V4L2_CAP_STREAMING)){
		printf("device is not streamming device\n");
		exit(EXIT_FAILURE);
	}
    pInfo = (pV4L_DEVINFO)malloc(sizeof(V4L_DEVINFO));
    if(pInfo != NULL){
        memset((void*)pInfo, 0, sizeof(V4L_DEVINFO));
		pInfo->devname =  devname;
    }
    return (pInfo);
}

// Open camera device
pV4L_DEVINFO OpenCamera(char* dev)
{
	int  i;
	gpInfo = InitCamera(fd, dev);
	if(gpInfo == NULL)
	{
		printf("Init camera failed\n");
		exit(1);
	}
	else
	{
		gpInfo->fd = fd;
		AddExUnitControl(gpInfo);
	}
	return(gpInfo);
}

void CloseCamera(pV4L_DEVINFO pInfo)
{
	int i;
	if(pInfo != NULL){
		free(pInfo);
	}
}
char Setaddress[11] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char SetDATA[11] = { 0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
int GetASICCommand(int nAsicaddress)
{
	struct uvc_xu_control xctrl;
	__u8 *pvalue;
	int naddLoby=0,naddHiby=0;
	if(gpInfo == NULL)
	{
		printf("err: pInfo is Null!!!\n");
		return 0;
	}
	//set asic address 
	naddLoby=nAsicaddress&(0x00FF);
	naddHiby=nAsicaddress>>8;
	Setaddress[1]=naddHiby;		
	Setaddress[2]=naddLoby;
	memset((void*)&xctrl, 0, sizeof(struct uvc_xu_control));
	xctrl.unit = EXTENSION_ID;
	xctrl.selector = XU_REG_SEQUENCIAL_RW;; 
	xctrl.size = 11;   //in bytes
	xctrl.data = (__u8*)Setaddress; 
  if(xioctl(gpInfo->fd, UVCIOC_CTRL_SET, &xctrl)>=0)
	{
		//Get asic  data
		xctrl.unit = EXTENSION_ID;
		xctrl.selector = XU_REG_SEQUENCIAL_RW; 
		xctrl.size = 11;   //in bytes
		if(xioctl(gpInfo->fd, UVCIOC_CTRL_GET, &xctrl)>=0)
		{         
		   pvalue=xctrl.data;		
       printf("GetASIC 0x%4.4x data=0x%2.2x\n",nAsicaddress,pvalue[3]);			
		}
		else
		{
			 printf("Get ASIC 0x%4.4x failed\n",nAsicaddress);	
			 return ERROR; 	
		}
	}	
	else
	{
		  printf("Set Address command failed\n");
		  return ERROR;
	}
	return OK;
}

int SetASICCommand(int nAsicaddress,int nAsicValue)
{
	struct uvc_xu_control xctrl;
	char* pvalue;
	int naddLoby=0,naddHiby=0;
	if(gpInfo == NULL)
	{
		printf("err: pInfo is Null!!!\n");
		return 0;
	}
	naddLoby=nAsicaddress&(0x00FF);
	naddHiby=nAsicaddress>>8;
	SetDATA[1]=naddHiby;
	SetDATA[2]=naddLoby;	
	SetDATA[3]=nAsicValue;
	//set asic  data
	memset((void*)&xctrl, 0, sizeof(struct uvc_xu_control));
	xctrl.unit = EXTENSION_ID;
	xctrl.selector = XU_REG_SEQUENCIAL_RW; 
	xctrl.size = 11;   //in bytes
  xctrl.data = (__u8*)SetDATA;
	if(xioctl(gpInfo->fd, UVCIOC_CTRL_SET, &xctrl)>=0)
	{
		printf("Set ASIC 0x%4.4x  Data=%2.2x\n",nAsicaddress,nAsicValue);		
	}	
	else
	{
		printf("Set ASIC failed\n");
		return ERROR;
	}
	return OK;
}

int GetBYTESensor(int nSensoraddress)
{
	struct uvc_xu_control xctrl;
	__u8 *pvalue;
	int naddLoby=0,naddHiby=0;
	if(gpInfo == NULL)
	{
		printf("err: pInfo is Null!!!\n");
		return 0;
	}
	//set sensor address 
	naddLoby=nSensoraddress&(0x00FF);
	naddHiby=nSensoraddress>>8;
	Setaddress[1]=naddHiby;		
	Setaddress[2]=naddLoby;
	memset((void*)&xctrl, 0, sizeof(struct uvc_xu_control));
	xctrl.unit = EXTENSION_ID;
	xctrl.selector = XU_SENSOR_SEQUENCIAL_RW; 
	xctrl.size = 11;   //in bytes
	xctrl.data = (__u8*)Setaddress; 
  	if(xioctl(gpInfo->fd, UVCIOC_CTRL_SET, &xctrl)>=0)
	{
		//Get sensor  data
		xctrl.unit = EXTENSION_ID;
		xctrl.selector = XU_SENSOR_SEQUENCIAL_RW; 
		xctrl.size = 11;   //in bytes
		if(xioctl(gpInfo->fd, UVCIOC_CTRL_GET, &xctrl)>=0)
		{         
		    pvalue=xctrl.data;		
        printf("GetByteSensor 0x%4.4x data=0x%2.2x\n",nSensoraddress,pvalue[3]);			
		}
		else
		{
			  printf("GetByteSensor 0x%4.4x failed\n",nSensoraddress);	
			  return ERROR;	
		}
	}	
	else
	{
		   printf("Set Address command failed\n");
		   return ERROR;
	}
	return OK;
}

int SetBYTESensor(int nSensorddress,int nSensorValue)
{
	struct uvc_xu_control xctrl;
	char* pvalue;
	int naddLoby=0,naddHiby=0;
	if(gpInfo == NULL)
	{
		printf("err: pInfo is Null!!!\n");
		return 0;
	}
	naddLoby=nSensorddress&(0x00FF);
	naddHiby=nSensorddress>>8;
	SetDATA[1]=naddHiby;
	SetDATA[2]=naddLoby;	
	SetDATA[3]=nSensorValue;
	//set Sensor  data
	memset((void*)&xctrl, 0, sizeof(struct uvc_xu_control));
	xctrl.unit = EXTENSION_ID;
	xctrl.selector = XU_SENSOR_SEQUENCIAL_RW; 
	xctrl.size = 11;   //in bytes
  xctrl.data = (__u8*)SetDATA;
	if(xioctl(gpInfo->fd, UVCIOC_CTRL_SET, &xctrl)>=0)
	{
		   printf("Set Sensor 0x%4.4x  Data=%2.2x\n",nSensorddress,nSensorValue);		
	}	
	else
	{
		   printf("Set Sensor failed\n");
		   return ERROR;
	}
	return OK;
}

int SetFlip( int bOn)
{
	__u8	data;
	struct uvc_xu_control xctrl;
  if(gpInfo == NULL)
	{
		printf("err: pInfo is Null!!!\n");
		return 0;
	}
	memset((void*)&xctrl, 0, sizeof(struct uvc_xu_control));
	xctrl.unit = EXTENSION_ID;
	xctrl.selector = FLIP_CTRL; 
	xctrl.size = 1;
	xctrl.data = &data;
	if(xioctl(gpInfo->fd, UVCIOC_CTRL_GET, &xctrl) >= 0)
	{
		if(bOn == 1)
		{
			data |= FLIP_BIT_MASK;
		}
	else
		{
			data &= ~(FLIP_BIT_MASK);
		}
	if(xioctl(gpInfo->fd, UVCIOC_CTRL_SET, &xctrl)>=0)
		{
			printf("Set flip %d ok\n", bOn);
		}
	else
		{
			printf("Set flip failed\n");
			return 0;
		}
}
	else
		{
			printf("Get flip failed\n");
		return 0;
	}
	return 1;
}
int GetFlip()
{
	__u8	data;
	int bOn = 0;
	struct uvc_xu_control xctrl;
	if(gpInfo == NULL)
	{
		printf("err: pInfo is Null!!!\n");
		return 0;
	}
	memset((void*)&xctrl, 0, sizeof(struct uvc_xu_control));
	xctrl.unit = EXTENSION_ID;
	xctrl.selector = FLIP_CTRL; 
	xctrl.size = 1;
	xctrl.data = &data;
	if(xioctl(gpInfo->fd, UVCIOC_CTRL_GET, &xctrl) >= 0)
	{
		if(data & FLIP_BIT_MASK)
		{
			bOn = 1;
			printf("Flip is on\n");
		}
		else
		{
			bOn = 0;
			printf("Flip is off\n");
		}
		
	}
	else
	{
		printf("GetFlip error\n");
	}
	return 1;
}

int SetMirror(int bOn)
{
	__u8	data;
	struct uvc_xu_control xctrl;
	if(gpInfo == NULL)
	{
		printf("err: pInfo is Null!!!\n");
		return 0;
	}
	memset((void*)&xctrl, 0, sizeof(struct uvc_xu_control));
	xctrl.unit = EXTENSION_ID;
	xctrl.selector = MIRROR_CTRL; 
	xctrl.size = 1;
	xctrl.data = &data;
	if(xioctl(gpInfo->fd, UVCIOC_CTRL_GET, &xctrl) >= 0)
	{
		if(bOn == 1)
		{
			data |= MIRROR_BIT_MASK;
		}
		else
		{
			data &= ~(MIRROR_BIT_MASK);
		}
		if(xioctl(gpInfo->fd, UVCIOC_CTRL_SET, &xctrl)>=0)
		{
			printf("Set Mirror %d ok\n", bOn);
		}
		else
		{
			printf("Set Mirror failed\n");
			return 0;
		}
	}
	else
	{
		printf("Get Mirror failed\n");
		return 0;
	}
	return 1;
}

int GetMirror()
{
	__u8	data;
	int bOn = 0;
	struct uvc_xu_control xctrl;
	if(gpInfo == NULL)
	{
		printf("err: pInfo is Null!!!\n");
		return 0;
	}
	memset((void*)&xctrl, 0, sizeof(struct uvc_xu_control));
	xctrl.unit = EXTENSION_ID;
	xctrl.selector = MIRROR_CTRL; 
	xctrl.size = 1;
	xctrl.data = &data;
	if(xioctl(gpInfo->fd, UVCIOC_CTRL_GET, &xctrl) >= 0)
	{
		if(data & MIRROR_BIT_MASK)
		{
			bOn = 1;
			printf("Mirror is on\n");
		}
		else
		{
			bOn = 0;
			printf("Mirror is off\n");
		}
		
	}
	else
	{
		printf("GetMirror error\n");
	}
	return 1;
}
static char *itoa(int value,char *string,int radix)
{
	int tmp = value;
	int i =0;
	char str[10];
	if(value == 0)
	{
		string[0]='0';
		string[1]='\0';
		return string;
	}
	while(tmp!=0)
	{
        str[i++]=(tmp%radix)+48;
	    tmp = tmp/radix;	
	}
	int j = 0;
	while(j<i)
	{
      string[j] = str[i-j-1];
	  j++;
	}
	string[i]='\0';
    return string;
}

int main(int argc, char *argv[])
{
	int i=0;
	char ch;
	int nvalue=0;
	int nadd=0;
	int nOn=0;
	char devicepath[24];
	char devicename[12];
	char num[3]="";
		
	for (i=0;i<255;i++)
	{
	
		strcpy(devicepath, "/dev/video");
		strcpy(devicename, "video");
		itoa(i,num,10);
		strcat(devicepath,num);
		strcat(devicename,num);
		fd = open(devicepath,O_RDWR|S_IRUSR|S_IWUSR);
		if (fd==-1)
		{
			printf("Can't open %s\n",devicepath);
			if (i==255)
			{
				return ERROR;
			}
		}
		else
		{
			printf("Open %s OK\n",devicepath);
			break;
		}
	}	
	{				
		printf("********************\n");		
		printf("g) Get ASIC  Value\n");
		printf("s) Set ASIC  Value\n");
		printf("a) Get SenSor BYTE Value\n");
		printf("b) Set SenSor BYTE Value\n");
		printf("e) Get Flip\n");
		printf("f) Set Flip\n");
		printf("m) Get Mirror\n");
		printf("h) Set Mirror\n");
		printf("0) exit\n");
		scanf("%c", &ch);
		if(ch=='g')
			{
				gpInfo=OpenCamera(devicename);
				printf("Please input ASIC address(DEC)=");
				scanf("%d", &nadd); 			
				GetASICCommand(nadd);
				CloseCamera(gpInfo);
				printf("*** Get ASIC 0x%4.4x Value Done ***\n",nadd);
			}
		else if(ch=='s')
			{
				gpInfo=OpenCamera(devicename);
				printf("Please input ASIC address(DEC)=");
				scanf("%d", &nadd); 
                                                                printf("Please input ASIC  value(DEC)=");
				scanf("%d", &nvalue); 
				SetASICCommand(nadd,nvalue);
				CloseCamera(gpInfo); 
				printf("*** Set ASIC 0x%4.4x Value 0x%4.4x Done ***\n",nadd,nvalue);
			}
		else if(ch=='a')
			{
				gpInfo=OpenCamera(devicename);
				printf("Please input Sensor BYTE address(DEC)=");
				scanf("%d", &nadd); 
				GetBYTESensor(nadd);
				CloseCamera(gpInfo);
				printf("*** Get Sensor BYTE 0x%4.4x Value Done ***\n",nadd);
				
			}
		else if(ch=='b')
			{
				gpInfo=OpenCamera(devicename);
				printf("Please input Sensor BYTE address(DEC)=");
				scanf("%d", &nadd); 
        printf("Please input Sensor BYTE value(DEC)=");
				scanf("%d", &nvalue); 
				SetBYTESensor(nadd,nvalue);
				CloseCamera(gpInfo);
				printf("*** Set Sensor BYTE 0x%4.4x Value 0x%4.4x Done ***\n",nadd,nvalue);
			}
			else if(ch=='e')
			{
				gpInfo=OpenCamera(devicename);
			  GetFlip(); 
				CloseCamera(gpInfo);
				printf("*** GetFlip Done ***\n");
				
			}
		else if(ch=='f')
			{
				gpInfo=OpenCamera(devicename);
				printf("Please input Flip value 0:off 1:on =");
				scanf("%d", &nOn); 
        SetFlip(nOn);
				CloseCamera(gpInfo);
				if(nOn==0)
				{
					printf("*** Set Filp Off Done ***\n");
				}
				else
				{
					printf("*** Set Filp On Done ***\n");
				}
			}
			else if(ch=='m')
			{
				gpInfo=OpenCamera(devicename);
			  GetMirror(); 
				CloseCamera(gpInfo);
				printf("*** GetMirror Done ***\n");
				
			}
		else if(ch=='h')
			{
				gpInfo=OpenCamera(devicename);
				printf("Please input Mirror value 0:off 1:on =");
				scanf("%d", &nOn); 
                                                                SetMirror(nOn);
				CloseCamera(gpInfo);
				if(nOn==0)
				{
					printf("*** Set Mirror Off Done ***\n");
				}
				else
				{
					printf("*** Set Mirror On Done ***\n");
				}
			}
		printf("\n\n");
	}
}



