#include "onvif_device_interface.h"
#include "onvif_debug.h"

#define LEN_NAME_ADAPTER            10  
#define _PATH_PROCNET_DEV           "/proc/net/dev"
#define _PATH_PROCNET_IFINET6       "/proc/net/if_inet6"
#define _PATH_PROCNET_ROUTE         "/proc/net/route"
#define _PATH_DNS_CONT              "/etc/resolv.conf"


OnvifUserMng_S gOnvifUsers = {0};
OnvifProductInfos_S gOnvifProductInfo;
OnvifDeviceScopes_S gOnvifDevScopes;
OnvifDeviceInfos_S gOnvifDevInfo;
OnvifServices_S gOnvifServices;
OnvifVideoEncCfg_S gOnvifVideoEncCfg;
OnvifVideoSrcCfg_S gOnvifVideoSrcCfg;
SystemCfg_S gSystemCfg;
SystemStatus_S gSystemStatus;

// call back function
OnvifCallback_S gOnvifCallback = {0};

//static struct interface
// network
static int get_name(char *name[], int *count_adapter);
static int get_mtu(const char* adapter_name, int *mtu);
static int set_mtu(const char* adapter_name, const int mtu);
static int get_hwaddr(const char *adapter_name, unsigned char hwaddr[]);
static int set_hwaddr(const char *adapter_name, const unsigned char hwaddr[]);
static int get_ip(const char *adapter_name, struct sockaddr *addr);
static int set_ip(const char *adapter_name, const struct sockaddr *addr);
static int set_dhcp(const char *adapter_name);

static int get_netmask(const char *adapter_name, struct sockaddr *netmask);
static int set_netmask(const char *adapter_name, const struct sockaddr *netmask);
static int get_broadaddr(const char *adapter_name, struct sockaddr *broadaddr);
static int set_broadaddr(const char *adapter_name, struct sockaddr *broadaddr);
static int get_ipv6(const char *adapter_name, struct sockaddr_in6 *ipv6);
static int get_gateway(const char *adapter_name, struct sockaddr *gateway[], int *count_gateway);
static int set_gateway(const char *adapter_name, struct sockaddr *gateway[], int count_gateway);
static int get_dns(char ***search, char ***nameserver, char **domain, int *size_search, int *size_ns);
static int set_dns(char **search, char **nameserver, char *domain, int size_search, int size_ns);
static int is_ipaddr(const char *ip_addr, int iptype);
static int covprefixlen(struct sockaddr *paddr);

// video
static OnvifVideoEncCfg_S get_video_enc();
static int set_video_enc(OnvifVideoEncCfg_S enc);
static int set_ve_name(const char *name);
static int set_ve_useCount(const int count);
static int set_ve_token(const char* token);
static int set_ve_encoding(const VideoEnc_E enc);
static int set_ve_resolution(const VideoResolution_S reso);
static int set_ve_quality(const float quality);
static int set_ve_sessionTimeout(const char *timeout);
static int set_ve_gop(const int gop);
static int set_ve_frameRateLimit(const int limit);
static int set_ve_bitrate(const int rate);
static int set_ve_advanceEncType(const VideoEnc_E type);
static int set_ve_h264(const VideoH264Cfg_S h264);

// system
static int set_local_time(const SystemTime_S local);
static int get_local_time(const SystemTime_S *local);
static int set_utc_time(const SystemTime_S utc);
static int get_utc_time(const SystemTime_S *utc);
static int sys_reboot(void);

OnvifNetworkCallback_S gOnvifNetworkCallback =
{
    .get_name       = get_name,
    .get_mtu        = get_mtu,
    .set_mtu        = set_mtu,
    .get_hwaddr     = get_hwaddr,
    .set_hwaddr     = set_hwaddr,
    .get_ip         = get_ip,
    .set_ip         = set_ip,
    .get_netmask    = get_netmask,
    .set_netmask    = set_netmask,
    .get_broadaddr  = get_broadaddr,
    .set_broadaddr  = set_broadaddr,
    .get_ipv6       = get_ipv6,
    .get_gateway    = get_gateway,
    .set_gateway    = set_gateway,
    .get_dns        = get_dns,
    .set_dns        = set_dns,
    .is_ipaddr      = is_ipaddr,
    .covprefixlen   = covprefixlen,
    .set_dhcp       = set_dhcp,
};

OnvifVideoEncCallback_S gOnvifVideoEncCallback = 
{
    .set_video_enc          = set_video_enc,
    .get_video_enc          = get_video_enc,
    .set_ve_name            = set_ve_name,
    .set_ve_useCount        = set_ve_useCount, 
    .set_ve_token           = set_ve_token,
    .set_ve_encoding        = set_ve_encoding,
    .set_ve_resolution      = set_ve_resolution,
    .set_ve_quality         = set_ve_quality,
    .set_ve_sessionTimeout  = set_ve_sessionTimeout,
    .set_ve_gop             = set_ve_gop,
    .set_ve_frameRateLimit  = set_ve_frameRateLimit,
    .set_ve_bitrate         = set_ve_bitrate,
    .set_ve_advanceEncType  = set_ve_advanceEncType,
    .set_ve_h264            = set_ve_h264,
};

OnvifSysCfgCallback_S gOnvifSysCfgCallback = 
{
    .set_local_time = set_local_time,
    .get_local_time = get_local_time,
    .set_utc_time = set_utc_time,
    .get_utc_time = get_utc_time,
    .sys_reboot = sys_reboot,
    .sys_reboot = sys_reboot,
};


static int set_dhcp(const char *adapter_name)
{
    FILE *fp = NULL;
    char cmd[128];
    char buffer[128];

    if (0 == sprintf(cmd, "udhcpc -i %s  2>&1", adapter_name)) {
        perror("ERROR: ");
    }
    if (NULL == (fp = popen(cmd, "r"))) {
        perror("ERROR: ");
    }
    if (NULL != fgets(buffer, sizeof(buffer), fp)) {
        perror("ERROR: ");
    }
    if (-1 == pclose(fp)) {
        perror("ERROR: ");
    }
    return 0;   
}

static int covprefixlen(struct sockaddr *paddr)
{
    unsigned int nmask = 0;
    int countbit=0;
    /*IPv4 */
    if (AF_INET == paddr->sa_family) {
        nmask = (*((struct sockaddr_in *)paddr)).sin_addr.s_addr;
        while (nmask) {
            nmask >>= 1;
            countbit++;
        }
    }
    return countbit;
}

static int is_ipaddr(const char *ip_addr, int iptype)
{
    unsigned int ip[5]={0};
    int ret;

    if (DEVICE_IPv4 == iptype) {
        ret = sscanf(ip_addr, "%3d.%3d.%3d.%3d", &ip[0], &ip[1], &ip[2],&ip[3]);
        if (ret != 4) {
            return 0;
        }
        if (0xFFFFFFFF == inet_addr(ip_addr)) {
            return 0;
        }
    }
    else {
        return 0;
    }
    return 1;
}

static int set_dns(char **search, char **nameserver, char *domain, int size_search, int size_ns)
{
    int i=0;
    FILE *fp = NULL;
    char buf[]={"# Dynamic resolv.conf file for onvif resolver generated by onvif_server\n#     DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"};

    while (i<size_search) {
        printf("error search[%d]:[%s]\n", i, search[i]);
        i++;
    }
    i = 0;
    while (i<size_ns) {
        printf("error nameserver[%d]:[%s]\n", i, nameserver[i]);
        i++;
    }    
    i=0;
    if (domain) {
        printf("error domain[%s]\n", domain);   
    }

    fp= fopen(_PATH_DNS_CONT, "w+");
    if (fp == NULL) {
        return -1;  
    }

    fprintf(fp, "%s", buf);
    while (i<size_ns) {
        fprintf(fp, "nameserver %s\n", nameserver[i]);
        i++;
    }
    i=0;
    if (size_search!=0) {
        fprintf(fp, "%s", "search");
    }
    while (i<size_search) {
        fprintf(fp, " %s", search[i]);
        i++;
    }   
    fprintf(fp, "%s", "\n");
    if (domain) {
        fprintf(fp, "domain %s\n", domain);
    }
    fclose(fp);
    return 0;
}


static int get_dns(char ***search, char ***nameserver, char **domain, int *size_search, int *size_ns)
{
    char buf[100]={0};
    int r=0;
    char ns[100]={0};
    char sch[100]={0};
    char dm[100]={0};
    int count_ns=0;
    int havedm=0;
    int count_sch=0;
    int i=0;
    char *delimPos = NULL;

    FILE *fp = fopen(_PATH_DNS_CONT, "r");
    if (fp == NULL) {
        return -1;   
    } 
    while ((r = fscanf(fp, "%s", buf)) == 1) {
        if (buf[0]=='#') {
            if (fscanf(fp, "%*[^\n]\n") < 0) {  /* Skip the first line. */
                goto error;                     /* Empty or missing line, or read error. */
            } 
            continue;
        }

        if (0 == strncmp(buf, "nameserver", sizeof ("nameserver"))) {
            r = fscanf(fp, "%s", buf);
            if (r!=1) {
                break; 
            }
            count_ns++;
            if (count_ns>1) {
                strcat(ns, " ");
            }
            strcat(ns, buf);
        }
        else if (0 == strncmp(buf, "search", sizeof ("search"))) {
            r = fscanf(fp, "%*[ ]%[^\n]s", buf); /* skip space */
            if (r!=1)
                break; 
            count_sch++;
            if (count_sch>1) {
                strcat(sch, " ");
            }
            strcat(sch, buf);
        }
        else if (0 == strncmp(buf, "domain", sizeof ("domain"))) {
            r = fscanf(fp, "%s", buf);
            if (r!=1)
                break; 
            strcat(dm, buf);
            havedm = 1;
        }
    }
    fclose(fp);

    if (count_sch) {
        *search = (char **)malloc((5) * sizeof(char *));
        delimPos = strtok(sch, " ");
        while (delimPos) {
            (*search)[i] = (char *)malloc(100 * sizeof(char));
                strcpy((*search)[i], delimPos);
                i++;
            delimPos = strtok(NULL, " \n");
        }
        (*search)[i] = NULL;
    }
    if (count_ns) {
        *nameserver = (char **)malloc(count_ns * sizeof(char *));
        i = 0;
        delimPos = strtok(ns, " ");
        while (delimPos) {
            (*nameserver)[i] = (char *)malloc(100 * sizeof(char));
            strcpy((*nameserver)[i], delimPos);
            i++;
            delimPos = strtok(NULL, " \n");
        }
        (*nameserver)[i] = NULL;    
    }
    if (havedm) {
        *domain = (char *)malloc(64 * sizeof(char));
        strcpy(*domain, dm);
    }
    *size_search = count_sch;
    *size_ns = count_ns;

    return 0;
error:
    return -1;
}


static int set_mtu(const char* adapter_name, const int mtu)
{
	struct ifreq ifr;
	const char *ifname = adapter_name;
	int skfd;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);

    ifr.ifr_mtu = mtu;
	if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0) {
        perror("set mtu error!!\n");
    }

	close(skfd);
	return 0;
}


static int set_hwaddr(const char *adapter_name, const unsigned char hwaddr[])
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    memcpy((void *) ifr.ifr_hwaddr.sa_data,hwaddr, 8);
    
	if (ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0) {
        perror("set hwaddr error!!\n");
    }
	close(skfd);
	return 0;
}


static int set_netmask(const char *adapter_name, const struct sockaddr *netmask)
{
	struct ifreq ifr;
	const char *ifname = adapter_name;
	int skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr = *netmask;

    if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0) {
        perror("?");
    }
	close(skfd);
	return 0;
}


static int set_ip(const char *adapter_name, const struct sockaddr *addr)
{
	struct ifreq ifr;
	const char *ifname = adapter_name;
    int ret=0;
	int skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr = *addr;

	if ((ret = ioctl(skfd, SIOCSIFADDR, &ifr)) != 0) {
        perror("?");
    }
	close(skfd);
	return ret;
}


static int set_gateway(const char *adapter_name, struct sockaddr *gateway[], int count_gateway)
{
    FILE *fp = NULL;
    char cmd[DEVICE_CMD_LEN];
    char buffer[DEVICE_ROUTE_BUF_LEN];
    int i=0;
    for(i=0;i<count_gateway;i++) {
        if (0 == sprintf(cmd, "route add default gw %s %s ", 
                        inet_ntoa((*(struct sockaddr_in *)gateway[i]).sin_addr), 
                        adapter_name)) {
            goto err;
        }

        if (NULL == (fp = popen(cmd, "r"))) {
            goto err;
        }
        if (NULL != fgets(buffer, sizeof(buffer), fp)) {
            goto err;
        }
        if (-1 == pclose(fp)) {
            goto err;
        }
    }
    return 0;
err:
    return -1;
}


static int get_gateway(const char *adapter_name, struct sockaddr *gateway[], int *count_gateway)
{
    char devname[64] /*flags[16],*/ /* * sgw*//**sdest*/;
    char *sdest = NULL;//[18];
    unsigned long d, g, m;
    int flgs, ref, use, metric, mtu, win, ir;
    struct sockaddr_in s_addr;
    int count = 0;
    
    FILE *fp = fopen(_PATH_PROCNET_ROUTE, "r");

    if (fscanf(fp, "%*[^\n]\n") < 0) {  /* Skip the first line. */
        goto ERROR;                     /* Empty or missing line, or read error. */
	}
	while (1) {
		int r;
		r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
				devname, &d, &g, &flgs, &ref, &use, &metric, &m,
				&mtu, &win, &ir);
		if (r != 11) {
			if ((r < 0) && feof(fp)) { /* EOF with no (nonspace) chars read. */
				break;
			}
 ERROR:
            printf("ERROR read route table!");
		}

        memset(&s_addr, 0, sizeof(struct sockaddr_in));
        s_addr.sin_family = AF_INET;
        s_addr.sin_addr.s_addr = d;

        sdest = strdup(inet_ntoa(s_addr.sin_addr));

        if (0 == strcmp(sdest, "0.0.0.0")) {
            s_addr.sin_addr.s_addr = g;
            gateway[count] = (struct sockaddr *)malloc(sizeof(struct sockaddr));
            memcpy(gateway[count], (void *)&s_addr, sizeof(struct sockaddr));
            count++;
        }
        free(sdest);
	}
    *count_gateway = count;
	fclose(fp);
    return 0;
}


//hwaddr
static int get_ipv6(const char *adapter_name, struct sockaddr_in6 *ipv6)
{
    FILE *f;
    char addr6[40], devname[20];
    struct sockaddr_in6 sap;
    int plen, scope, dad_status, if_idx;
    char addr6p[8][5];

    memset(ipv6, 0, sizeof(struct sockaddr_in6));
    f = fopen(_PATH_PROCNET_IFINET6, "r");

    if (f == NULL) {
        return -1;
    }

    while (fscanf
            (f, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n",
             addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
             addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
             &dad_status, devname) != EOF
          ) {
        if (!strcmp(devname, adapter_name)) {
            sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
                    addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                    addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
            inet_pton(AF_INET6, addr6,
                    (struct sockaddr *) &sap.sin6_addr);
            sap.sin6_family = AF_INET6;
            *ipv6 = sap;
        }
    }
	fclose(f);
	return 0;
}


static int get_broadaddr(const char *adapter_name, struct sockaddr *broadaddr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    memset(broadaddr, 0, sizeof(struct sockaddr));

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
        if (ioctl(skfd, SIOCGIFBRDADDR, &ifr) >= 0) {
            *broadaddr = ifr.ifr_broadaddr;
        }
    }

    close(skfd);
    return 0;
}


static int set_broadaddr(const char *adapter_name, struct sockaddr *broadaddr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    ifr.ifr_broadaddr = *broadaddr;

    if (ioctl(skfd, SIOCSIFADDR, &ifr) == 0) {
        if (ioctl(skfd, SIOCSIFBRDADDR, &ifr) < 0) {
            printf("set netmask error!!\n");
        }
    }

    close(skfd); 
    return 0;
}


static int get_netmask(const char *adapter_name, struct sockaddr *netmask)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    memset(netmask, 0, sizeof(struct sockaddr));

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
        if (ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0) {
            *netmask = ifr.ifr_netmask;
        }
    }

    close(skfd);
    return 0;
}


static int get_ip(const char *adapter_name, struct sockaddr *addr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    memset(addr, 0, sizeof(struct sockaddr));

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
        *addr = ifr.ifr_addr;
    }
    close(skfd);
    return 0;
}


static int get_hwaddr(const char *adapter_name, unsigned char hwaddr[])
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);

    memset(hwaddr, 0, 6);
    if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0) {
        memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);
    }

    close(skfd);
    return 0;
}


static int get_mtu(const char *adapter_name, int *mtu)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);

    *mtu = 0;
    if (ioctl(skfd, SIOCGIFMTU, &ifr) >= 0) {
        *mtu = ifr.ifr_mtu;
    }

    close(skfd);
    return 0;
}


static int get_name(char *name[], int *count_adapter)
{
    int numreqs = 30;
    struct ifconf ifc;
    struct ifreq *ifr;
    int n, err = -1;
    int skfd;

    *count_adapter = 0;
    ifc.ifc_buf = NULL;

    /* SIOCGIFCONF currently seems to only work properly on AF_INET sockets
       (as of 2.1.128) */
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0) {
        printf("error: no inet socket available\n");
        return -1;
    }
    for (;;) {
        ifc.ifc_len = sizeof(struct ifreq) * numreqs;
        ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len);

        if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
            goto out;
        }
        if (ifc.ifc_len == (int)(sizeof(struct ifreq) * numreqs)) {
            /* assume it overflowed and try again */
            numreqs += 10;
            continue;
        }
        break;
    }

    ifr = ifc.ifc_req; 
    for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) {
        name[*count_adapter] = (char *)malloc(LEN_NAME_ADAPTER * sizeof(char)); 
        strcpy(name[*count_adapter], ifr->ifr_name);
        (*count_adapter)++ ; 
        ifr++;
    }
    err = 0;
out:
    close(skfd);
    free(ifc.ifc_buf);
    return err;
}


static OnvifVideoEncCfg_S get_video_enc()
{
    return gOnvifVideoEncCfg;
}


static int set_video_enc(const OnvifVideoEncCfg_S enc)
{
    // TODO: (Sam) to be finished
    return 0;
}

// set video enc cfg
static int set_ve_name(const char *name)
{
    printf("%s: name %s len %d\n", __FUNCTION__, name, strlen(name));
    if(name == NULL) {
        printf("error: empty name\n");
        return -1;
    }
    memset(gOnvifVideoEncCfg.name, 0, sizeof(gOnvifVideoEncCfg.name));
    memcpy(gOnvifVideoEncCfg.name, name, strlen(name));
    return 0;
}

static int set_ve_useCount(const int count)
{
    printf("%s: count %d\n", __FUNCTION__, count);
    gOnvifVideoEncCfg.useCount = count;
    return 0;
}

static int set_ve_token(const char* token)
{
    printf("%s: token %s len %d\n", __FUNCTION__, token, strlen(token));
    if(token == NULL) {
        printf("error: token is NULL\n");
        return -1;
    }
    memset(gOnvifVideoEncCfg.token, 0, sizeof(gOnvifVideoEncCfg.token));
    memcpy(gOnvifVideoEncCfg.token, token, strlen(token));
    return 0;
}

static int set_ve_encoding(const VideoEnc_E enc)
{
    printf("%s: enc %d\n", __FUNCTION__, enc);
    gOnvifVideoEncCfg.encoding = enc;
    return 0;
}

static int set_ve_resolution(const VideoResolution_S reso)
{
    printf("%s: resolution %d\n", __FUNCTION__, reso);
    gOnvifVideoEncCfg.resolution = reso;
    return 0;
}

static int set_ve_quality(const float quality)
{
    printf("%s: quality %f\n", __FUNCTION__, quality);
    gOnvifVideoEncCfg.quality = quality;
    return 0;
}

static int set_ve_sessionTimeout(const char *timeout)
{
    printf("%s: session timeout %s len %d\n", __FUNCTION__, timeout, strlen(timeout));
    if(timeout == NULL) {
        printf("error: no session timeout\n");
        return -1;
    }
    memset(gOnvifVideoEncCfg.sessionTimeout, 0, sizeof(gOnvifVideoEncCfg.sessionTimeout));
    memcpy(gOnvifVideoEncCfg.sessionTimeout, timeout, strlen(timeout));
    return 0;
}

static int set_ve_gop(const int gop)
{
    printf("%s: gop %d\n", __FUNCTION__, gop);
    gOnvifVideoEncCfg.gop = gop;
    return 0;
}


static int set_ve_frameRateLimit(const int limit)
{
    printf("%s: limit %d\n", __FUNCTION__, limit);
    gOnvifVideoEncCfg.frameRateLimit = limit;
    return 0;
}

static int set_ve_bitrate(const int rate)
{
    printf("%s: rate %d\n", __FUNCTION__, rate);
    gOnvifVideoEncCfg.bitrate = rate;
    return 0;
}

static int set_ve_advanceEncType(const VideoEnc_E type)
{
    printf("%s: type %d\n", __FUNCTION__, type);
    gOnvifVideoEncCfg.advanceEncType = type;
    return 0;
}

static int set_ve_h264(const VideoH264Cfg_S h264)
{
    printf("%s: gov %d profile %d\n", __FUNCTION__, h264.govLength, h264.profile);
    gOnvifVideoEncCfg.h264 = h264;
    return 0;
}

int onvif_product_info_init()
{
    memset(&gOnvifProductInfo, 0, sizeof(OnvifProductInfos_S));
    memcpy(gOnvifProductInfo.manufacturer, DEVICE_INFO_MANUFACTURER, DEVICE_INFO_LENGTH);
    memcpy(gOnvifProductInfo.model, DEVICE_INFO_MODEL, DEVICE_INFO_LENGTH);
    memcpy(gOnvifProductInfo.firmwareVersion, DEVICE_INFO_FIRMWARE_VER, DEVICE_INFO_LENGTH);
    memcpy(gOnvifProductInfo.serialNumber, DEVICE_INFO_SN, DEVICE_INFO_LENGTH);
    memcpy(gOnvifProductInfo.hardwareID, DEVICE_INFO_HARDWARE_ID, DEVICE_INFO_LENGTH);
    return 0;
}

int onvif_device_scopes_init()
{
    memset(&gOnvifDevScopes, 0, sizeof(OnvifDeviceScopes_S));
    gOnvifDevScopes.size = DEVICE_SCOPES_COUNT;
    memcpy(&gOnvifDevScopes.scopes[0], DEVICE_SCOPE1, DEVICE_SCOPE_LENGTH);
    memcpy(&gOnvifDevScopes.scopes[1], DEVICE_SCOPE2, DEVICE_SCOPE_LENGTH);
    memcpy(&gOnvifDevScopes.scopes[2], DEVICE_SCOPE3, DEVICE_SCOPE_LENGTH);
    memcpy(&gOnvifDevScopes.scopes[3], DEVICE_SCOPE4, DEVICE_SCOPE_LENGTH);
    memcpy(&gOnvifDevScopes.scopes[4], DEVICE_SCOPE5, DEVICE_SCOPE_LENGTH);
    memcpy(&gOnvifDevScopes.scopes[5], DEVICE_SCOPE6, DEVICE_SCOPE_LENGTH);
    memcpy(&gOnvifDevScopes.scopes[6], DEVICE_SCOPE7, DEVICE_SCOPE_LENGTH);
    memcpy(&gOnvifDevScopes.scopes[7], DEVICE_SCOPE8, DEVICE_SCOPE_LENGTH);
    memcpy(&gOnvifDevScopes.scopes[8], DEVICE_SCOPE9, DEVICE_SCOPE_LENGTH);
    return 0;
}

int onvif_device_info_init()
{
    // int i = 0;
    memset(&gOnvifDevInfo, 0, sizeof(OnvifDeviceInfos_S));
    // get uuid
    unsigned char uuid[DEVICE_UUID_LEN];
    // gOnvifNetworkCallback.get_hwaddr(DEVICE_ADAPTER_NAME, uuid);
    gOnvifCallback.network.get_hwaddr(DEVICE_ADAPTER_NAME, uuid);
    sprintf(gOnvifDevInfo.uuid, "urn:uuid:20200908-abcd-efgh-ijkl-%02X%02X%02X%02X%02X%02X",
            uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5]);
    printf("init uuid: %s\n", gOnvifDevInfo.uuid);

    // get mac
    sprintf(gOnvifDevInfo.mac, "%02X:%02X:%02X:%02X:%02X:%02X",
        uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5]);
    printf("init mac: %s\n", gOnvifDevInfo.mac);
    // get ip
    struct sockaddr addr;
    gOnvifCallback.network.get_ip(DEVICE_ADAPTER_NAME, &addr);
    strcpy(gOnvifDevInfo.ip, inet_ntoa((*(struct sockaddr_in *)&addr).sin_addr));
    printf("get ip: %s\n", gOnvifDevInfo.ip);
    return 0;
}

int onvif_services_init()
{
    memset(&gOnvifServices, 0, sizeof(OnvifServices_S));
    printf("test service struct size %lu\n", sizeof(OnvifServices_S));
    gOnvifServices.size = 6;
    memcpy(&gOnvifServices.namespaces[0], DEVICE_SERVICE1, DEVICE_SERVICE_LENGTH);
    memcpy(&gOnvifServices.namespaces[1], DEVICE_SERVICE2, DEVICE_SERVICE_LENGTH);
    memcpy(&gOnvifServices.namespaces[2], DEVICE_SERVICE3, DEVICE_SERVICE_LENGTH);
    memcpy(&gOnvifServices.namespaces[3], DEVICE_SERVICE4, DEVICE_SERVICE_LENGTH);
    memcpy(&gOnvifServices.namespaces[4], DEVICE_SERVICE5, DEVICE_SERVICE_LENGTH);
    memcpy(&gOnvifServices.namespaces[5], DEVICE_SERVICE6, DEVICE_SERVICE_LENGTH);
    return 0;
}

int onvif_video_config_init()
{
    // init video encode cfg
    memset(&gOnvifVideoEncCfg, 0, sizeof(OnvifVideoEncCfg_S));
    memcpy(gOnvifVideoEncCfg.name, "Name_VideoEnc", DEVICE_COMMON_LENGTH);
    gOnvifVideoEncCfg.useCount = 1;
    memcpy(gOnvifVideoEncCfg.token, "Token_VideoEnc", DEVICE_COMMON_LENGTH);
    gOnvifVideoEncCfg.encoding = VideoEnc_H264;
    gOnvifVideoEncCfg.resolution.Width = DEVICE_VIDEO_WIDTH;
    gOnvifVideoEncCfg.resolution.Height = DEVICE_VIDEO_HEIGHT;
    gOnvifVideoEncCfg.quality = 1;      // FIXME: what for
    memcpy(gOnvifVideoEncCfg.sessionTimeout, "PT5S", DEVICE_COMMON_LENGTH);
    gOnvifVideoEncCfg.gop = 1;          // FIXME: what for
    gOnvifVideoEncCfg.frameRateLimit = 30;
    gOnvifVideoEncCfg.bitrate = 1024;   // FIXME: what for
    gOnvifVideoEncCfg.advanceEncType = VideoEnc_H265;
    gOnvifVideoEncCfg.h264.govLength = 60;
    gOnvifVideoEncCfg.h264.profile = H264Profile_High;

    // init video source cfg
    memset(&gOnvifVideoSrcCfg, 0, sizeof(OnvifVideoSrcCfg_S));
    memcpy(gOnvifVideoSrcCfg.name, "Name_VideoSrc", DEVICE_COMMON_LENGTH);
    gOnvifVideoSrcCfg.useCount = 1;
    memcpy(gOnvifVideoSrcCfg.token, "Token_VideoSrc", DEVICE_COMMON_LENGTH);
    memcpy(gOnvifVideoSrcCfg.sourceToken, "Token_VideoSrc", DEVICE_COMMON_LENGTH);
    gOnvifVideoSrcCfg.resolution.Width = DEVICE_VIDEO_WIDTH;
    gOnvifVideoSrcCfg.resolution.Height = DEVICE_VIDEO_HEIGHT;
    gOnvifVideoSrcCfg.framerate = 25.0;
    gOnvifVideoSrcCfg.bounds.x = 0;
    gOnvifVideoSrcCfg.bounds.y = 0;
    gOnvifVideoSrcCfg.bounds.width = DEVICE_VIDEO_WIDTH;
    gOnvifVideoSrcCfg.bounds.height = DEVICE_VIDEO_HEIGHT;
    return 0;
}

int onvif_audio_config_init()
{
    return 0;
}

int onvif_system_config_init()
{
    gSystemCfg.timeType = TimeType_Manual;
    gSystemCfg.timeZoom = 8;
    return 0;
}

int onvif_system_status_init()
{
    gSystemStatus.discover = 0;
    gSystemStatus.webservices = 0;
    gSystemStatus.hello_bye = 0;
    gSystemStatus.exit_discover = 0;
    gSystemStatus.exit_webservices = 0;
    gSystemStatus.exit_hellobye = 0;
    gSystemStatus.hello_bye = 0;
    sem_init(&gSystemStatus.hello_bye_sem, 0, 0);
    return 0;
}

int onvif_device_init()
{
    printf("onvif_device_init()\n");
    onvif_users_init();
    onvif_product_info_init();
    onvif_device_info_init();
    onvif_device_scopes_init();
    onvif_services_init();
    onvif_video_config_init();
    onvif_audio_config_init();
    onvif_system_config_init();
    onvif_system_status_init();
    return 0;
}

int onvif_register_callback()
{
    gOnvifCallback.network = gOnvifNetworkCallback;
    gOnvifCallback.videoEnc = gOnvifVideoEncCallback;
    gOnvifCallback.system = gOnvifSysCfgCallback;
    return 0;
}


int onvif_users_init()
{
    // TODO: init users, need db operation
    gOnvifUsers.count = 1;
    strcpy(gOnvifUsers.users[0].name, "admin");
    strcpy(gOnvifUsers.users[0].password, "123456");
    gOnvifUsers.users[0].level = UserLevel__Administrator;

    gOnvifUsers.current = &gOnvifUsers.users[0];

    printf("current user: %s %s %d\n", 
        gOnvifUsers.current->name, gOnvifUsers.current->password, gOnvifUsers.current->level);
    return 0;
}

int onvif_add_user(OnvifUser user)
{
    // add user, need db operation
    printf("%s %s\n", user.name, user.password);
    return 0;
}

int onvif_delete_user(OnvifUser user)
{
    // delete user, need db operation
    printf("%s %s\n", user.name, user.password);
    return 0;
}

int onvif_set_user(OnvifUser user)
{
    // set user, need db operation
    printf("%s %s\n", user.name, user.password);
    return 0;
}


static int set_local_time(const SystemTime_S local)
{
    printf("%d-%d-%d\n", local.hour, local.minute, local.second);
    return 0;    
}   

static int get_local_time(const SystemTime_S *local)
{
    if(local == NULL) return -1;
    return 0;
}

static int set_utc_time(const SystemTime_S utc)
{
    printf("%d-%d-%d\n", utc.hour, utc.minute, utc.second);
    return 0;
}

static int get_utc_time(const SystemTime_S *utc)
{
    if(utc == NULL) return -1;
    return 0;
}

static int sys_reboot(void)
{
    system("reboot -f");
    return 0;
}
