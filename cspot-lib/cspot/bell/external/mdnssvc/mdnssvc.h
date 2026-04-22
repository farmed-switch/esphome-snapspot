

#ifndef __TINYSVCMDNS_H__
#define __TINYSVCMDNS_H__

#include <stdint.h>
#include <stdbool.h>
#ifdef _WIN32
#include <inaddr.h>
#else
#include <netinet/in.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct mdnsd;
struct mdns_service;

struct mdnsd *mdnsd_start(struct in_addr host, bool verbose);

void mdnsd_stop(struct mdnsd *s);

void mdnsd_set_hostname(struct mdnsd *svr, const char *hostname, struct in_addr addr);

struct mdns_service *mdnsd_register_svc(struct mdnsd *svr, const char *instance_name,
		const char *type, uint16_t port, const char *hostname, const char *txt[]);

void mdns_service_destroy(struct mdns_service *srv);

void mdns_service_remove(struct mdnsd *svr, struct mdns_service *svc);

#ifdef __cplusplus
}
#endif

#endif
