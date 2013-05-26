#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>
#include <asm/ioctls.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>

#include <linux/if_packet.h>
#include <net/if.h>
#include <linux/wireless.h>
#include <linux/rtnetlink.h>

#include "wps.h"

#define MAX_SHELL_LINE 	128
#define MAX_SHELL_BUF		512
#define KEY_SEM_OPTION  2012
#define PPPD_PROCESS "/usr/lib/wpa_supplicant"

#define	PATH_DNS_CONF			"/etc/resolv.conf"
static int driver_ready_flags=0;
static int linkingwps=0;
int wps_net_status=0;
int ap_comming=0;
static int we_version_compiled=22;
static int ap_wait =0;

 //#define WPS_DEBUG
 #ifdef WPS_DEBUG
 #define BRUCE_DEBUG   printf
 #else
 #define BRUCE_DEBUG
 #endif

char *GetValueFromShell(const char *cmdstring)
{
	char line[MAX_SHELL_LINE];
	char *increase, *getstring;
	FILE *fpin;
	int buf_size = MAX_SHELL_BUF;
	if ((fpin = popen(cmdstring, "r")) == NULL)
	{
		BRUCE_DEBUG("popen error!\n");
		return NULL;
	}	
	if (!(getstring = (char *)malloc(MAX_SHELL_BUF)))
	{
		BRUCE_DEBUG("malloc error!\n");
		return NULL;
	}
	memset(getstring, 0, sizeof(getstring));
	memset(line, 0, sizeof(line));
	while(fgets(line, MAX_SHELL_LINE, fpin) != NULL)
	{
		buf_size -= strlen(line);
		if ( buf_size <= 0 )
		{
			buf_size += 2 * MAX_SHELL_BUF;
			if(!(increase = (char *)realloc(getstring, buf_size+MAX_SHELL_BUF)))
			{
				BRUCE_DEBUG("realloc error!\n");
				return NULL;
			}
			getstring = increase;
		}
		strcat(getstring, line);
	}	
	if(pclose(fpin) == -1)
	{
		BRUCE_DEBUG("pclose error!\n");
		free(getstring);
		return NULL;
	}
	return (char *)getstring;
}

int Check_Wps_Net_Status(void)
{
	char *p,*str0,*str1;
	char ssid[128]={0};
	char cmd0[] = "iwconfig";
	
	p = GetValueFromShell(cmd0);
	if(strstr(p,"wlan0")==NULL)
	{
		
			BRUCE_DEBUG("no wireless\n");
			return 0;
	}
	else
	{
		if(strstr(p,"Access Point: Not-Associated")!=NULL)
		{
			BRUCE_DEBUG("Access Point:Not-Associated wps not connected!\n");
			return wps_off;
		}
		str0 = strstr(p,"ESSID:\"");
				BRUCE_DEBUG("str0===%s\n",str0);
		if(str0 > 0)
		{
			p = str0 = str0 + strlen("ESSID:\"");
		
				BRUCE_DEBUG("str0+ESSID===%s\n",str0);
			
			str1 = strstr(p,"\"");
			
				BRUCE_DEBUG("str1===%s\n",str1);
			
			if((int)(str1-str0)>0)
			strncpy(ssid,str0,(int)(str1-str0));
			
			if(strlen(ssid)<1)
			{
			
				BRUCE_DEBUG("wireless is not connect!\n");
				return wps_off;
			}
			else
			{
				
				BRUCE_DEBUG("ssid=%s\n",ssid);
				return wps_on;
			}
		}
		else
			return 0;
	}
  
	
}
void changppoeconf(int networkflags)
{
	char buf[128]={0};
	int i=0;
	char *p=NULL;
	FILE *fd=fopen("/etc/ppp/pppoe.conf","r+b");
	while(fgets(buf,127,fd)!=NULL)
	{
		i++;
		if((networkflags==0)&&(p=strstr(buf,"ETH=wlan0")!=NULL))
		{
			fseek(fd,-10,SEEK_CUR);
			fwrite("ETH=eth0\n",9,1,fd);
			fclose(fd);
		}
		else if((networkflags==1)&&(p=strstr(buf,"ETH=eth0")!=NULL))
		{
			fseek(fd,-9,SEEK_CUR);
			fwrite("ETH=wlan0\n",10,1,fd);
			fclose(fd);
		}
		if(i>10)
		{
			fclose(fd);
			break;	
		}
	}
}
#if 1
int check_wpa_supplicant_process(void)
{
	
	FILE *log;
	int i,index;
	char prolist[200][128];	//process list
	char *delim = " ";
	int pid;
	char *process;
	int found_pppd= 0;
	char tmp[]="ps -A > /tmp/wpsprocess";
		system("ps -A > /tmp/wpsprocess"); 
		log = fopen("/tmp/wpsprocess","r");
		for (i=0; i < 200; i++)
		{
			if (fgets(prolist[i],128,log) == NULL)
				break; 
				
			pid = atoi((char *)strtok(prolist[i], delim));     //pid of process
			
			for (index=0; index < 3; index++)
			{
				process = strtok(NULL, delim);		//name of process
			}
			if (*process != '[')
				process = strtok(NULL, delim);
				
			process[strlen(process)] = '\0';
			if (strcmp(process,PPPD_PROCESS) == 0)
				found_pppd=1;
			
			
		}
		fclose(log);
		BRUCE_DEBUG("found_pppd=%d\n",found_pppd);
  if(found_pppd)
  	return 1;
  else
  	return 0;
}
#endif
void EnableWifiThread(void)
{
	char *p0;
	char cmd6 [] = "/usr/sbin/iwconfig";
		BRUCE_DEBUG("start check_on\n");
		
				 	p0 =GetValueFromShell(cmd6);
					if(strstr(p0,"wlan0")==NULL)/*检查wlan0 是否已经启用*/
					{
						
							BRUCE_DEBUG("install wlan0 driver lose\n");
							API_Set_Wps_Status(4); /*无线网卡没有连接好或是损坏，退出*/
							driver_ready_flags =0;
							
					}
					else
					{
						driver_ready_flags =1;
						BRUCE_DEBUG("install wlan0 driver and ifconfig wlan0 up success and do wpa_supplicant over\n");
					}
		free(p0);
	
}
void strtolow(char *str)
{
	char *p;
	p = str;
	while(*p!=0)
	{
		if( (*p>='A') && (*p<='Z') )
			*p = *p +('a'-'A');
		p++;
	}
}
void LedThread(void)
{
	while(1)
	{
		if(wps_net_status==wps_on)
		{
			ap_comming=0;
			API_GPIOB_Set_Value(24,1);
			sleep(1);
			API_GPIOB_Set_Value(24,0);
			sleep(1);
			continue;
		}
		if(ap_comming==1)
		{
			API_GPIOB_Set_Value(24,1);
			usleep(50000);
			API_GPIOB_Set_Value(24,0);
			usleep(50000);
			continue;
		}
		sleep(3);
	}


}

void driver_event_wireless(char *data, int len)
{
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end, *custom, *buf, *assoc_info_buf, *info_pos;
	
        assoc_info_buf = info_pos = NULL;
	pos = data;
	end = data + len;

	while (pos + IW_EV_LCP_LEN <= end)
	{
		/* Event data may be unaligned, so make a local, aligned copy
		 * before processing. */
		memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);

		if (iwe->len <= IW_EV_LCP_LEN)
			return;

		custom = pos + IW_EV_POINT_LEN;
		
		if (we_version_compiled > 18 &&
		    (iwe->cmd == IWEVMICHAELMICFAILURE ||
		     iwe->cmd == IWEVCUSTOM ||
		     iwe->cmd == IWEVASSOCREQIE ||
		     iwe->cmd == IWEVASSOCRESPIE ||
		     iwe->cmd == IWEVPMKIDCAND))
		{
			/* WE-19 removed the pointer from struct iw_point */
			char *dpos = (char *) &iwe_buf.u.data.length;
			int dlen = dpos - (char *) &iwe_buf;
			memcpy(dpos, pos + IW_EV_LCP_LEN,
			       sizeof(struct iw_event) - dlen);
		}
		else
		{
			memcpy(&iwe_buf, pos, sizeof(struct iw_event));
		    custom += IW_EV_POINT_OFF;
                  }
		
		switch (iwe->cmd)
		{
		
			case IWEVCUSTOM:
       			    if (custom + iwe->u.data.length > end) {
       				return;
       			    }
	
                               if (memcmp(custom, "WPS_CONNECTING", 14) == 0) {
							   	
						ap_comming =1;
						//API_Set_Wps_Status(7);
						BRUCE_DEBUG("***get wps connecting event***\n");
	                       }
			         break;

			default:
				break;
		
		}

		pos += iwe->len;
	}
}
void driver_rtm_newlink(struct nlmsghdr *h,int len)
{
	struct ifinfomsg *ifi;
	int attrlen, nlmsg_len, rta_len;
	struct rtattr * attr;

	if (len < sizeof(*ifi))
		return;

	ifi = NLMSG_DATA(h);
	
	nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));
       
	attrlen = h->nlmsg_len - nlmsg_len;
	//printf("attrlen=%d\n",attrlen);
	if (attrlen < 0)
		return;

	attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);
	rta_len = RTA_ALIGN(sizeof(struct rtattr));
	while (RTA_OK(attr, attrlen))
	{
		//printf("rta_type=%02x\n",attr->rta_type);
		if (attr->rta_type == IFLA_WIRELESS)
		{
		       // printf("Rec wireless event!!!\n");
			driver_event_wireless(((char *) attr) + rta_len, attr->rta_len - rta_len);
		} 
		attr = RTA_NEXT(attr, attrlen);
	}
}	

void CheckAPIDThread(void)
{
	while(1)
	{
	if(ap_wait!=1)
	{
		BRUCE_DEBUG("wait ap connecting !\n");	
		sleep(2);
		 continue;
	}


	int s;
	struct sockaddr_nl local;
	
	char buf[1024];
	int left;
	struct sockaddr_nl from;
	socklen_t fromlen;
	struct nlmsghdr *h;
	 we_version_compiled = WIRELESS_EXT;
	s = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	if (s < 0)
	{
		BRUCE_DEBUG("socket(PF_NETLINK,SOCK_RAW,NETLINK_ROUTE)");
		pthread_exit(NULL);
	}
	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;

	if (bind(s, (struct sockaddr *) &local, sizeof(local)) < 0)
	{
		BRUCE_DEBUG("bind(netlink)");
		close(s);
		pthread_exit(NULL);
	}
   	 while(1)
	{
	fromlen = sizeof(from);
	left = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &from, &fromlen);
	if (left < 0)
	{
               BRUCE_DEBUG("Recv wrong length?\n");
		continue;
	}
	h = (struct nlmsghdr *) buf;
	while (left >= sizeof(*h))
	{
		int len, plen;

		len = h->nlmsg_len;              //total message length including header
		plen = len - sizeof(*h);       //message body length
		if (len > left || plen < 0)
		{
			BRUCE_DEBUG( "Malformed netlink message:len=%d left=%d plen=%d", len, left, plen);
			break;
		}

		switch (h->nlmsg_type)
		{
		case RTM_NEWLINK:
			driver_rtm_newlink(h, plen);
			break;
		}

		len = NLMSG_ALIGN(len);
		left -= len;
		h = (struct nlmsghdr *) ((char *) h + len);
		BRUCE_DEBUG("=============\n");
	}

	if (left > 0)
	{
		BRUCE_DEBUG( "%d extra bytes in the end of netlink ", left);
	}
	if(ap_comming)
	{
		//ap_comming=0;
		break;
	}
	}
	
	//pthread_exit(NULL);

		}
	ap_wait=0;

}
void StartWpsThread(void)
{
	int checkwpslow=0;
	int countchek=0;
	int count=0;
	int wps_ked_down_flags =0;
	int Running=1;
	int wpsconnected=0;
	int wifimode;
	int wps_onoroff;
	int process_on;
	int process_flags=0;
	
	EnableWifiThread();
	if(driver_ready_flags==0)
	{
		BRUCE_DEBUG("wait for driver_ready_flags==1\n");
		return;
	}
	 wifimode = API_WLAN_GetPhyMode();
	 process_on=check_wpa_supplicant_process();
	 process_flags = process_on;
	 if(process_flags)
	 {
		while(1)
		{
			  wps_net_status = Check_Wps_Net_Status();
			  if(wps_net_status==wps_off)
			  {	
				if(count++>5)
					break;	
			 }
			 if(wps_net_status==wps_on)
			 {
				if(wifimode==1)
					linkingwps =1;
				//API_GPIOB_Set_Value(24,1);
				//API_Set_Wps_Status(5);
				break;
			 }
			sleep(5); 
		}

	 }
	while(driver_ready_flags)
	{
							
		while(1)
			{ 
					if(!API_GPIOB_Get_Value(25))
						{
							checkwpslow=1;
						}
		 	 			countchek++;
					if(countchek==100)
                 			 {
						if(!checkwpslow)
						{
							wps_ked_down_flags=0;
						}
						else
						{
							wps_ked_down_flags=1;
						}
							checkwpslow=0;
							countchek=0;
							break;
		 			}
					usleep(10000);
			}
			 if( !wps_ked_down_flags)
					BRUCE_DEBUG("Wait for Key down\n");
			   wps_onoroff = API_Get_Wps_Options();
		          BRUCE_DEBUG("wps_onorff=%d\n",wps_onoroff);
						 
			  wps_net_status = Check_Wps_Net_Status();
					
			  if(wps_net_status==wps_on)/*wps 已经连接*/	
				{
					if(wpsconnected)
					{
						API_Set_Wps_Status(5);
						linkingwps =1;
						wpsconnected =0;
					}
					//API_GPIOB_Set_Value(24,1);
					sleep(5);
					continue;
				}
			if(wps_net_status==wps_off)
					API_Set_Wps_Status(0);
			
			if(wps_ked_down_flags||wps_onoroff)
		   	{
						
						BRUCE_DEBUG("Wps Key down or software start,wait for router connect!\n");
						API_Set_Wps_Status(6);
						API_GPIOB_Set_Value(24,1);
						//API_GPIO_Set_Value(5,1);
						
						 wps_net_status = Check_Wps_Net_Status();
							
		   				if(wps_net_status==wps_off)/*执行脚本连接wps路由器*/
		   				{
								if(!check_wpa_supplicant_process())/*用户执行无线连接，但是脚本邋wpa_supplicant未运行，再次运行*/
										Running =1;

								if(process_flags)/*wps未连接上，重启启动wps前，杀掉上电启动的进程*/
								{
									system("killall wpa_supplicant");
									process_flags =0;
								}
								
								if(Running)
								{
									if(check_wpa_supplicant_process())
										system("killall wpa_supplicant");
									BRUCE_DEBUG("start wpa_supplicant script\n");
									system("/usr/lib/wpa_supplicant -iwlan0 -Dvia -c  /usr/sbin/wps.conf -B");
									//system("/usr/lib/wpa_supplicant -iwlan0 -Dvia -c  /usr/sbin/wps.conf -dd &");
									sleep(1);
									Running = 0;
									wpsconnected =1;
									ap_wait=1;
								}
								system("/usr/sbin/wpa_cli -iwlan0 wps_pbc");		
								BRUCE_DEBUG("wait for router key down wps key 30 s\n");
		   						sleep(30);	
		   				}
						if(!wifimode)
						{
							API_WLAN_SetPhyMode(1);
							API_System_SyncFlash();
						}
						
		   		}	
			API_GPIOB_Set_Value(24,0);
			//API_GPIO_Set_Value(5,0);
			sleep(1);	
	}

}

void signal_catch(int msgno) {
  BRUCE_DEBUG("***Exit***\n");			
   exit(0);
}
int main(int *argc,char *argv[])
{
	int forkflag;
	int opt;
	pthread_t WPSThreadID;
	pthread_t WpsThreadLedID;
	int checklanlow=0;
	int countchek=0;
	int loopflags=1;
	int lanworkflags;
	int wps_pro_flgs=0;
	int WlanLinked;
	int mustgolan;
	char cmd[512];
	char *net,ipaddr[32]={0},netmask[32]={0},gateway[32]={0};
	unsigned long Ipaddr,Netmask,Net;
	int first_loop_flags =1;
	pthread_t CheckAPID;
	pthread_attr_t attr;
	signal(SIGINT,signal_catch);
	signal(SIGTERM,signal_catch);
	IPCInit();
	
	#if 1	
// set mac same as rt3070
{
	char *p,*pStr,oldmac[32]={0},*mac,EthIP[20],WlanIP[20];
	char String[]="ifconfig | grep wlan0 | sed 's/^.*HWaddr //g'";				//API_Net_SetMacAddress
	if(strlen(p = GetValueFromShell(String)) != 0)
	{
		pStr = p;
		while(*pStr)
		{
			if(*pStr==':')
				*pStr = '-';
			pStr++;
		}
		
		API_Net_GetMac(oldmac,"eth0");
		mac = oldmac;
		strtolow(p);
		strtolow(mac);
		if(strncmp(mac,p,strlen(mac)))
		{
			printf("eth0:%s\trt3070:%s\n",mac,p);
			API_Net_SetMacAddress(p);
			API_System_SyncFlash();
			API_System_SyncFlash();
#ifdef	LOG_APP
	WriteLogFile(LOG_PATH,"At wifi_restart:sync mac");
#endif				
			system("reboot");
		}	
	}
	free(p);
}
#endif
	
	
	#if 1
	API_WLAN_GetFixIP(ipaddr);
	API_WLAN_GetFixNetmask(netmask);
	API_WLAN_GetFixGateway(gateway);

	 while(loopflags)
		{
			if(!API_GPIOB_Get_Value(26))
			{
				checklanlow=1;
			}
		 	 countchek++;
			if(countchek==100)
     			 {
				if(!checklanlow)
				{
					lanworkflags=0;
				}
				if(checklanlow)
				{
					lanworkflags=1;
				}
				checklanlow=0;
				countchek=0;
				loopflags=0;
							
		 	}
			usleep(10000);		
		 }
	
		WlanLinked =Check_Wps_Net_Status();	
	
	       BRUCE_DEBUG("WlanLinked=%d,lanworkflags=%d\n",WlanLinked,lanworkflags);
		if((WlanLinked==wps_on)&&!lanworkflags)
		{
			API_NET_SetUsedInterface(0);/*走无线*/
		}
		else if((WlanLinked==wps_off)&&!lanworkflags)
		{

			API_NET_SetUsedInterface(0);/*走无线*/
		}
		else
		{
			API_NET_SetUsedInterface(1);	
			mustgolan=1;/*走有线*/
		}

	// wps_pro_flgs = ((API_WLAN_GetPhyMode())&&(check_wpa_supplicant_process));
    	 driver_ready_flags = 0;
	pthread_create(&WPSThreadID,NULL,(void *)StartWpsThread,(void *)0);
	pthread_create(&WpsThreadLedID,NULL,(void*)LedThread,(void *)0);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&CheckAPID, &attr, (void *)CheckAPIDThread, (void *)0);
			
	while(1)
	{
		
		BRUCE_DEBUG("lanworkflags=%d\n",lanworkflags);
		if(((API_NET_GetUsedInterface())&&lanworkflags)||mustgolan)
		{
			mustgolan=0;
			API_NET_SetUsedInterface(0);
			changppoeconf(0);
			BRUCE_DEBUG("go to lan work!\n");
			if(API_NET_DHCPGetEnable())
			{
				system("killall udhcpc");
				open(PATH_DNS_CONF, O_WRONLY | O_CREAT | O_TRUNC,0666);
				system("/sbin/udhcpc -s /etc/init.d/udhcpc.default -i eth0 &");
				system("/bin/ip address flush wlan0");
				
			}	
			else
			{
				system("/bin/ip address flush wlan0");		
				sprintf(cmd,"/sbin/ifconfig eth0 %s netmask %s",ipaddr,netmask);
				system(cmd);
				sprintf(cmd,"/sbin/route add default gw %s dev eth0",gateway);
				system(cmd);
			}
				
			BRUCE_DEBUG("\nwireless is down!\n");
		}
		else if(!lanworkflags)
		{	
			
			
			BRUCE_DEBUG("wps_net_status:%d\n", wps_net_status);	
		 	if(!(API_NET_GetUsedInterface())&&(wps_net_status==wps_on||linkingwps==1))
				{
				 if(linkingwps==1)
				 {
					 sleep(10);
					 linkingwps=0;
					 if(wps_net_status !=wps_on)
					 	continue;
				 }
				changppoeconf(1);
				API_NET_SetUsedInterface(1);
				if(API_NET_DHCPGetEnable())
				{
					system("killall udhcpc");
					open(PATH_DNS_CONF, O_WRONLY | O_CREAT | O_TRUNC,0666);
					system("/sbin/udhcpc -s /etc/init.d/udhcpc.default -i wlan0 &");
					system("/bin/ip address flush eth0");
				}	
				else
				{
					system("/bin/ip address flush eth0");
					sprintf(cmd,"/sbin/ifconfig wlan0 %s netmask %s",ipaddr,netmask);
					system(cmd);
					sprintf(cmd,"/sbin/route add default gw %s dev wlan0",gateway);
					system(cmd);
				}
				BRUCE_DEBUG("\nwireless is up!\n");
				}
			
		}
		loopflags=1;
		while(loopflags)
		{
			if(!API_GPIOB_Get_Value(26))
			{
				checklanlow=1;
			}
		 	 countchek++;
			if(countchek==100)
                      {
				
				if(!checklanlow)
				{
					lanworkflags=0;
					if(API_NET_GetUsedInterface()==0)
					{
						if(API_ALARM_Getlanalarm())
						{
							API_MOD_SetRecordLanTrig(0);/*network out*/	
				
							BRUCE_DEBUG("network trig sd\n");	
						
						}
						
					}
					
				}
				else
				{
					lanworkflags=1;
				}
				checklanlow=0;
				countchek=0;
				loopflags=0;
							
		 	}
			usleep(10000);
				
		   }
		sleep(2);
		
	}
#else
  		while(1)
  			sleep(1);
  #endif
	return 0;
}
