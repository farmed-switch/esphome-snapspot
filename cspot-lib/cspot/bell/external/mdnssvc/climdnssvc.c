

#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

typedef uint32_t in_addr_t;
#define strcasecmp stricmp

#elif defined (__linux__) || defined (__FreeBSD__) || defined (sun)
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <strings.h>
#include <ifaddrs.h>
#if defined (__FreeBSD__) || defined (sun)
#include <net/if_dl.h>
#include <net/if_types.h>
#endif
#if defined (sun)
#include <sys/sockio.h>
#endif
#elif defined (__APPLE__)
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#endif

#include "mdnssvc.h"

struct mdns_service *svc;
struct mdnsd *svr;

#ifdef _WIN32
static void winsock_init(void) {
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int WSerr = WSAStartup(wVersionRequested, &wsaData);
	if (WSerr != 0) exit(1);
}

static void winsock_close(void) {
	WSACleanup();
}
#endif

struct in_addr get_interface(char* iface) {
	struct in_addr addr;

	addr.s_addr = iface && *iface ? inet_addr(iface) : INADDR_NONE;

	if (addr.s_addr != INADDR_NONE)  return addr;
#ifdef _WIN32
	struct sockaddr_in* host = NULL;
	ULONG size = sizeof(IP_ADAPTER_ADDRESSES) * 32;

	IP_ADAPTER_ADDRESSES* adapters = (IP_ADAPTER_ADDRESSES*)malloc(size);
	int ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_GATEWAYS | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST, 0, adapters, &size);

	for (PIP_ADAPTER_ADDRESSES adapter = adapters; adapter; adapter = adapter->Next) {
		if (adapter->TunnelType == TUNNEL_TYPE_TEREDO ||
			adapter->OperStatus != IfOperStatusUp || 0)
			continue;

		char name[256];
		wcstombs(name, adapter->FriendlyName, sizeof(name));
		if (iface && *iface && strcasecmp(iface, name)) continue;

		for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast;
			unicast = unicast->Next) {
			if (adapter->FirstGatewayAddress && unicast->Address.lpSockaddr->sa_family == AF_INET) {
				addr = ((struct sockaddr_in*)unicast->Address.lpSockaddr)->sin_addr;
				return addr;
			}
		}
	}

	return addr;
#else
	struct ifaddrs* ifaddr;

	if (getifaddrs(&ifaddr) == -1) 	return addr;

	for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET ||
			!(ifa->ifa_flags & IFF_UP) || !(ifa->ifa_flags & IFF_MULTICAST) ||
			ifa->ifa_flags & IFF_LOOPBACK ||
			(iface && *iface && strcasecmp(iface, ifa->ifa_name)))
			continue;

		addr = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
		break;
	}

	freeifaddrs(ifaddr);
		return addr;
#endif
}

#ifdef _WIN32

int asprintf(char** strp, const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	int len = vsnprintf(NULL, 0, fmt, args);
	*strp = malloc(len + 1);

	if (*strp) len = vsprintf(*strp, fmt, args);
	else len = 0;

	va_end(args);

	return len;
}
#endif

static void print_usage(void) {
	printf("[-v] [-o <ip|ifname>] -i <identity> -t <type> -p <port> [<txt>] ...[<txt>]\n");
}

static void sighandler(int signum) {
	mdnsd_stop(svr);
#ifdef _WIN32
	winsock_close();
#endif
	exit(0);
}

int main(int argc, char *argv[]) {
	const char** txt = NULL;
	struct in_addr host;
	char hostname[256],* arg, * identity = NULL, * type = NULL, * addr = NULL;
	int port = 0;
	bool verbose = false;

	if (argc <= 2) {
		print_usage();
		exit(0);
	}

#ifdef _WIN32
	winsock_init();
#endif

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
#if defined(SIGPIPE)
	signal(SIGPIPE, SIG_IGN);
#endif
#if defined(SIGQUIT)
	signal(SIGQUIT, sighandler);
#endif
#if defined(SIGHUP)
	signal(SIGHUP, sighandler);
#endif

	while ((arg = *++argv) != NULL) {
		if (!strcasecmp(arg, "-o") || !strcasecmp(arg, "host")) {
			addr = *++argv;
			argc -= 2;
		} else if (!strcasecmp(arg, "-p")) {
			port = atoi(*++argv);
		} else if (!strcasecmp(arg, "-v")) {
			verbose = true;
		} else if (!strcasecmp(arg, "-t")) {
			(void)! asprintf(&type, "%s.local", *++argv);
		} else if (!strcasecmp(arg, "-i")) {
			identity = *++argv;
		} else {

			if (!identity) identity = *argv;
			else if (!type) (void) !asprintf(&type, "%s.local", *argv);
			else if (!port) port = atoi(*argv);
			else {
				txt = (const char**) malloc((argc + 1) * sizeof(char**));
				memcpy(txt, argv, argc * sizeof(char**));
				txt[argc] = NULL;
				break;
			}
			argc--;
		}
	}

	gethostname(hostname, sizeof(hostname));
	strcat(hostname, ".local");
	host = get_interface(addr);

	svr = mdnsd_start(host, verbose);
	if (svr) {
		printf("host: %s\nidentity: %s\ntype: %s\nip: %s\nport: %u\n", hostname, identity, type, inet_ntoa(host), port);

		mdnsd_set_hostname(svr, hostname, host);
		svc = mdnsd_register_svc(svr, identity, type, port, NULL, txt);

#ifdef _WIN32
		Sleep(INFINITE);
#else
		pause();
#endif
		mdns_service_remove(svr, svc);
		mdnsd_stop(svr);
	} else {
		printf("Can't start server");
		print_usage();
	}

	free(type);
	if (txt) free(txt);

#ifdef _WIN32
	winsock_close();
#endif

	return 0;
}

