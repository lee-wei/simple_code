#ifndef _UVCDEVICE_HEADER_
#define _UVCDEVICE_HEADER_


// Extension Unit GUID {68bbd0b0-61a4-4b83-90b7-a6215f3c4f70}
#define UVC_GUID_AU3820_XU	{	0xb0, 0xd0, 0xbb, 0x68,\
									 	0xa4, 0x61,\
										0x83, 0x4b,\
										0x90, 0xb7,\
										0xa6, 0x21, 0x5f, 0x3c, 0x4f, 0x70}
#define EXTENSION_ID	(6)
#define EXTENSION_SIZE	(1)
#define	FLIP_CTRL		(4)
#define MIRROR_CTRL		(5)
#define MIRROR_BIT_MASK (0x1)
#define FLIP_BIT_MASK	(0x2)
#define REGmode_Sensor	(0x2)
#define XU_I2C_ADDR		(0x1)
#define XU_I2C_DATA		(0x2)
#define XU_FLIP_CMD		(0x4)
#define XU_MIRROR_CMD	(0x5)
#define XU_REG_SEQUENCIAL_RW	(0x6)
#define XU_SENSOR_SEQUENCIAL_RW	(0x7)


typedef struct{
	int fd;
	int					nCtrlCnt;
	char*				devname;
    __u32               nFmtCnt;
    int                 nCurFmtIdx;
    int                 nCurResIdx;
    __u64               PrevCapSize;
    __u64               CapSize;
//
}V4L_DEVINFO, *pV4L_DEVINFO;


#endif // _UVCDEVICE_HEADER_
