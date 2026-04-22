

#include "mdns.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#include <winsock.h>
#include <in6addr.h>
#else
#include <netinet/in.h>
#endif

struct name_comp {
	uint8_t *label;
	size_t pos;

	struct name_comp *next;
};

uint8_t *dup_nlabel(const uint8_t *n) {
	assert(n[0] <= 63);
	return (uint8_t *) strdup((char *) n);
}

uint8_t *dup_label(const uint8_t *label) {
	int len = *label + 1;
	uint8_t *newlabel;
	if (len > 63)
		return NULL;
	newlabel = malloc(len + 1);
	strncpy((char *) newlabel, (char *) label, len);
	newlabel[len] = '\0';
	return newlabel;
}

uint8_t *join_nlabel(const uint8_t *n1, const uint8_t *n2) {
	int len1, len2;
	uint8_t *s;

	assert(n1[0] <= 63 && n2[0] <= 63);

	len1 = strlen((char *) n1);
	len2 = strlen((char *) n2);

	s = malloc(len1 + len2 + 1);
	strncpy((char *) s, (char *) n1, len1);
	strncpy((char *) s+len1, (char *) n2, len2);
	s[len1 + len2] = '\0';
	return s;
}

char *nlabel_to_str(const uint8_t *name) {
	char *label, *labelp;
	const uint8_t *p;
	size_t buf_len = 256;

	assert(name != NULL);

	label = labelp = malloc(buf_len);
	for (p = name; *p; p++) {
		uint8_t label_len = *p;
		if (buf_len <= label_len)
			break;

		strncpy(labelp, (char *)p + 1, label_len);
		labelp += label_len;

		*labelp = '.';
		labelp++;

		buf_len -= label_len + 1;

		p += label_len;
	}

	if (buf_len == 0)
		labelp--;

	*labelp = '\0';

	return label;
}

static size_t label_len(uint8_t *pkt_buf, size_t pkt_len, size_t off) {
	uint8_t *p;
	uint8_t *e = pkt_buf + pkt_len;
	size_t len = 0;

	for (p = pkt_buf + off; p < e; p++) {
		if (*p == 0) {
			return len + 1;
		} else if ((*p & 0xC0) == 0xC0) {
			return len + 2;
		} else {
			len += *p + 1;
			p += *p;
		}
	}

	return len;
}

uint8_t *create_label(const char *txt) {
	int len;
	uint8_t *s;

	assert(txt != NULL);
	len = strlen(txt);
	if (len > 63)
		return NULL;

	s = malloc(len + 2);
	s[0] = len;
	strncpy((char *) s + 1, txt, len);
	s[len + 1] = '\0';

	return s;
}

uint8_t *create_nlabel(const char *name) {
	char *label;
	char *p, *e, *lenpos;
	int len = 0;

	assert(name != NULL);

	len = strlen(name);
	label = malloc(len + 1 + 1);
	if (label == NULL)
		return NULL;

	strncpy((char *) label + 1, name, len);
	label[len + 1] = '\0';

	p = label;
	e = p + len;
	lenpos = p;

	while (p < e) {
		char *dot = memchr(p + 1, '.', e - p - 1);
		*lenpos = 0;

		if (dot == NULL)
			dot = e + 1;
		*lenpos = dot - p - 1;

		p = dot;
		lenpos = dot;
	}

	return (uint8_t *) label;
}

static uint8_t *copy_label(uint8_t *pkt_buf, size_t pkt_len, size_t off) {
	int len;

	if (off > pkt_len)
		return NULL;

	len = pkt_buf[off] + 1;
	if (off + len > pkt_len) {
		DEBUG_PRINTF("label length exceeds packet buffer\n");
		return NULL;
	}

	return dup_label(pkt_buf + off);
}

static uint8_t *uncompress_nlabel(uint8_t *pkt_buf, size_t pkt_len, size_t off) {
	uint8_t *p;
	uint8_t *e = pkt_buf + pkt_len;
	size_t len = 0;
	char *str, *sp;
	if (off >= pkt_len)
		return NULL;

	for (p = pkt_buf + off; *p && p < e; p++) {
		size_t llen = 0;
		if ((*p & 0xC0) == 0xC0) {
			uint8_t *p2 = pkt_buf + (((p[0] & ~0xC0) << 8) | p[1]);
			llen = *p2 + 1;
			p = p2 + llen - 1;
		} else {
			llen = *p + 1;
			p += llen - 1;
		}
		len += llen;
	}

	str = sp = malloc(len + 1);
	if (str == NULL)
		return NULL;

	for (p = pkt_buf + off; *p && p < e; p++) {
		size_t llen = 0;
		if ((*p & 0xC0) == 0xC0) {
			uint8_t *p2 = pkt_buf + (((p[0] & ~0xC0) << 8) | p[1]);
			llen = *p2 + 1;
			strncpy(sp, (char *) p2, llen);
			p = p2 + llen - 1;
		} else {
			llen = *p + 1;
			strncpy(sp, (char *) p, llen);
			p += llen - 1;
		}
		sp += llen;
	}
	*sp = '\0';

	return (uint8_t *) str;
}

const char *rr_get_type_name(enum rr_type type) {
	switch (type) {
		case RR_A:		return "A";
		case RR_PTR:	return "PTR";
		case RR_TXT:	return "TXT";
		case RR_AAAA:	return "AAAA";
		case RR_SRV:	return "SRV";
		case RR_NSEC:	return "NSEC";
		case RR_ANY:	return "ANY";
	}
	return NULL;
}

void rr_entry_destroy(struct rr_entry *rr) {
	struct rr_data_txt *txt_rec;
	assert(rr);

	switch (rr->type) {
		case RR_PTR:
			if (rr->data.PTR.name)
				free(rr->data.PTR.name);

			break;

		case RR_TXT:
			txt_rec = &rr->data.TXT;
			while (txt_rec) {
				struct rr_data_txt *next = txt_rec->next;
				if (txt_rec->txt)
					free(txt_rec->txt);

				if (txt_rec != &rr->data.TXT)
					free(txt_rec);

				txt_rec = next;
			}
			break;

		case RR_SRV:
			if (rr->data.SRV.target)
				free(rr->data.SRV.target);
			break;

		case RR_AAAA:
			if (rr->data.AAAA.addr)
				free(rr->data.AAAA.addr);
        	break;

		default:

			break;
	}

	free(rr->name);
	free(rr);
}

void rr_list_destroy(struct rr_list *rr, char destroy_items) {
	struct rr_list *rr_next;

	for (; rr; rr = rr_next) {
		rr_next = rr->next;
		if (destroy_items)
			rr_entry_destroy(rr->e);
		free(rr);
	}
}

int rr_list_count(struct rr_list *rr) {
	int i = 0;
	for (; rr; i++, rr = rr->next);
	return i;
}

struct rr_entry *rr_entry_remove(struct rr_group *group, struct rr_entry *entry, enum rr_type type) {
	struct rr_group *g;

	for (g = group; g; g = g->next) {
		struct rr_list *lrr = g->rr, *prr= NULL;
		for (; lrr; lrr = lrr->next) {
			if (lrr->e->type == type) {
				switch (type) {
				case RR_PTR:
					if (lrr->e->data.PTR.entry == entry) {
						struct rr_entry *e = lrr->e;
						if (prr == NULL) {
							g->rr = lrr->next;
						} else {
							prr->next = lrr->next;
						}
						free(lrr);
						return e;
					}
					break;
				default:
					break;
				}
			}
			prr = lrr;
		}
	}

	return NULL;
}

struct rr_entry *rr_list_remove(struct rr_list **rr_head, struct rr_entry *rr) {
	struct rr_list *le = *rr_head, *pe = NULL;
	for (; le; le = le->next) {
		if (le->e == rr) {
			if (pe == NULL) {
				*rr_head = le->next;
				free(le);
				return rr;
			} else {
				pe->next = le->next;
				free(le);
				return rr;
			}
		}
		pe = le;
	}
	return NULL;
}

void rr_group_clean(struct rr_group **head) {
	struct rr_group *le = *head, *pe = NULL;

	while (le) {
		if (le->rr == NULL) {
			free(le->name);
			if (pe == NULL) {
				*head = le->next;
				 free(le);
				 le = *head;
			 } else {
				pe->next = le->next;
				free(le);
				le = pe->next;
			 }
		} else {
			pe = le;
			le = le->next;
        }
	}
}

int rr_list_append(struct rr_list **rr_head, struct rr_entry *rr) {
	struct rr_list *node = malloc(sizeof(struct rr_list));
	node->e = rr;
	node->next = NULL;

	if (*rr_head == NULL) {
		*rr_head = node;
	} else {
		struct rr_list *e = *rr_head, *taile = NULL;
		for (; e; e = e->next) {

			if (e->e == rr) {
				free(node);
				return 0;
			}
			if (e->next == NULL)
				taile = e;
		}
		taile->next = node;
	}
	return 1;
}

#define FILL_RR_ENTRY(rr, _name, _type)	\
	rr->name = _name;			\
	rr->type = _type;			\
	rr->ttl  = DEFAULT_TTL;		\
	rr->cache_flush = 1;		\
	rr->rr_class  = 1;

struct rr_entry *rr_create_a(uint8_t *name, struct in_addr addr) {
	DECL_MALLOC_ZERO_STRUCT(rr, rr_entry);
	FILL_RR_ENTRY(rr, name, RR_A);
	rr->data.A.addr = addr.s_addr;
	rr->ttl = DEFAULT_TTL_FOR_RECORD_WITH_HOSTNAME;
	return rr;
}

struct rr_entry *rr_create_aaaa(uint8_t *name, struct in6_addr *addr) {
	DECL_MALLOC_ZERO_STRUCT(rr, rr_entry);
	FILL_RR_ENTRY(rr, name, RR_AAAA);
	rr->data.AAAA.addr = addr;
	rr->ttl = DEFAULT_TTL_FOR_RECORD_WITH_HOSTNAME;
	return rr;
}

struct rr_entry *rr_create_srv(uint8_t *name, uint16_t port, uint8_t *target) {
	DECL_MALLOC_ZERO_STRUCT(rr, rr_entry);
	FILL_RR_ENTRY(rr, name, RR_SRV);
	rr->data.SRV.port = port;
	rr->data.SRV.target = target;
	rr->ttl = DEFAULT_TTL_FOR_RECORD_WITH_HOSTNAME;
	return rr;
}

struct rr_entry *rr_create_ptr(uint8_t *name, struct rr_entry *d_rr) {
	DECL_MALLOC_ZERO_STRUCT(rr, rr_entry);
	FILL_RR_ENTRY(rr, name, RR_PTR);
	rr->cache_flush = 0;
	rr->data.PTR.entry = d_rr;
	rr->ttl = DEFAULT_TTL_FOR_RECORD_WITH_HOSTNAME;
	return rr;
}

struct rr_entry *rr_create(uint8_t *name, enum rr_type type) {
	DECL_MALLOC_ZERO_STRUCT(rr, rr_entry);
	FILL_RR_ENTRY(rr, name, type);
	return rr;
}

void rr_set_nsec(struct rr_entry *rr_nsec, enum rr_type type) {
	assert(rr_nsec->type == RR_NSEC);
	assert((type / 8) < sizeof(rr_nsec->data.NSEC.bitmap));

	rr_nsec->data.NSEC.bitmap[ type / 8 ] = 1 << (7 - (type % 8));
}

void rr_add_txt(struct rr_entry *rr_txt, const char *txt) {
	struct rr_data_txt *txt_rec;
	assert(rr_txt->type == RR_TXT);

	txt_rec = &rr_txt->data.TXT;

	if (txt_rec->txt == NULL) {
		txt_rec->txt = create_label(txt);
		return;
	}

	for (; txt_rec->next; txt_rec = txt_rec->next);

	txt_rec->next = malloc(sizeof(struct rr_data_txt));

	txt_rec = txt_rec->next;
	txt_rec->txt = create_label(txt);
	txt_rec->next = NULL;
}

void rr_group_add(struct rr_group **group, struct rr_entry *rr) {
	struct rr_group *g;

	assert(rr != NULL);

	if (*group) {
		g = rr_group_find(*group, rr->name);
		if (g) {
			rr_list_append(&g->rr, rr);
			return;
		}
	}

	MALLOC_ZERO_STRUCT(g, rr_group);
	g->name = dup_nlabel(rr->name);
	rr_list_append(&g->rr, rr);

	g->next = *group;
	*group = g;
}

struct rr_group *rr_group_find(struct rr_group* g, uint8_t *name) {
	for (; g; g = g->next) {
		if (cmp_nlabel(g->name, name) == 0)
			return g;
	}
	return NULL;
}

struct rr_entry *rr_entry_find(struct rr_list *rr_list, uint8_t *name, uint16_t type) {
	struct rr_list *rr = rr_list;
	for (; rr; rr = rr->next) {
		if (rr->e->type == type && cmp_nlabel(rr->e->name, name) == 0)
			return rr->e;
	}
	return NULL;
}

struct rr_entry *rr_entry_match(struct rr_list *rr_list, struct rr_entry *entry) {
	struct rr_list *rr = rr_list;
	for (; rr; rr = rr->next) {
		if (rr->e->type == entry->type && cmp_nlabel(rr->e->name, entry->name) == 0) {
			if (entry->type != RR_PTR) {
				return rr->e;
			} else if (cmp_nlabel(MDNS_RR_GET_PTR_NAME(entry), MDNS_RR_GET_PTR_NAME(rr->e)) == 0) {

				return rr->e;
			}
		}
	}
	return NULL;
}

void rr_group_destroy(struct rr_group *group) {
	struct rr_group *g = group;

	while (g) {
		struct rr_group *nextg = g->next;
		free(g->name);
		rr_list_destroy(g->rr, 1);
		free(g);
		g = nextg;
	}
}

uint8_t *mdns_write_u16(uint8_t *ptr, const uint16_t v) {
	*ptr++ = (uint8_t) (v >> 8) & 0xFF;
	*ptr++ = (uint8_t) (v >> 0) & 0xFF;
	return ptr;
}

uint8_t *mdns_write_u32(uint8_t *ptr, const uint32_t v) {
	*ptr++ = (uint8_t) (v >> 24) & 0xFF;
	*ptr++ = (uint8_t) (v >> 16) & 0xFF;
	*ptr++ = (uint8_t) (v >>  8) & 0xFF;
	*ptr++ = (uint8_t) (v >>  0) & 0xFF;
	return ptr;
}

uint16_t mdns_read_u16(const uint8_t *ptr) {
	return  ((ptr[0] & 0xFF) << 8) |
			((ptr[1] & 0xFF) << 0);
}

uint32_t mdns_read_u32(const uint8_t *ptr) {
	return  ((ptr[0] & 0xFF) << 24) |
			((ptr[1] & 0xFF) << 16) |
			((ptr[2] & 0xFF) <<  8) |
			((ptr[3] & 0xFF) <<  0);
}

void mdns_init_reply(struct mdns_pkt *pkt, uint16_t id) {

	pkt->unicast = 0;

	pkt->id = id;

	pkt->flags = MDNS_FLAG_RESP | MDNS_FLAG_AA;

	rr_list_destroy(pkt->rr_qn,   0);
	rr_list_destroy(pkt->rr_ans,  0);
	rr_list_destroy(pkt->rr_auth, 0);
	rr_list_destroy(pkt->rr_add,  0);

	pkt->rr_qn    = NULL;
	pkt->rr_ans   = NULL;
	pkt->rr_auth  = NULL;
	pkt->rr_add   = NULL;

	pkt->num_qn = 0;
	pkt->num_ans_rr = 0;
	pkt->num_auth_rr = 0;
	pkt->num_add_rr = 0;
}

void mdns_pkt_destroy(struct mdns_pkt *p) {
	rr_list_destroy(p->rr_qn, 1);
	rr_list_destroy(p->rr_ans, 1);
	rr_list_destroy(p->rr_auth, 1);
	rr_list_destroy(p->rr_add, 1);

	free(p);
}

static size_t mdns_parse_qn(uint8_t *pkt_buf, size_t pkt_len, size_t off,
		struct mdns_pkt *pkt) {
	const uint8_t *p = pkt_buf + off;
	struct rr_entry *rr;
	uint8_t *name;

	assert(pkt != NULL);

	rr = malloc(sizeof(struct rr_entry));
	memset(rr, 0, sizeof(struct rr_entry));

	name = uncompress_nlabel(pkt_buf, pkt_len, off);
	p += label_len(pkt_buf, pkt_len, off);
	rr->name = name;

	rr->type = mdns_read_u16(p);
	p += sizeof(uint16_t);

	rr->unicast_query = (*p & 0x80) == 0x80;
	rr->rr_class = mdns_read_u16(p) & ~0x80;
	p += sizeof(uint16_t);

	rr_list_append(&pkt->rr_qn, rr);

	return p - (pkt_buf + off);
}

static size_t mdns_parse_rr(uint8_t *pkt_buf, size_t pkt_len, size_t off,
		struct mdns_pkt *pkt) {
	const uint8_t *p = pkt_buf + off;
	const uint8_t *e = pkt_buf + pkt_len;
	struct rr_entry *rr;
	uint8_t *name;
	size_t rr_data_len = 0;
	struct rr_data_txt *txt_rec;
	int parse_error = 0;

	assert(pkt != NULL);

	if (off > pkt_len)
		return 0;

	rr = malloc(sizeof(struct rr_entry));
	memset(rr, 0, sizeof(struct rr_entry));

	name = uncompress_nlabel(pkt_buf, pkt_len, off);
	p += label_len(pkt_buf, pkt_len, off);
	rr->name = name;

	rr->type = mdns_read_u16(p);
	p += sizeof(uint16_t);

	rr->cache_flush = (*p & 0x80) == 0x80;
	rr->rr_class = mdns_read_u16(p) & ~0x80;
	p += sizeof(uint16_t);

	rr->ttl = mdns_read_u32(p);
	p += sizeof(uint32_t);

	rr_data_len = mdns_read_u16(p);
	p += sizeof(uint16_t);

	if (p + rr_data_len > e) {
		DEBUG_PRINTF("rr_data_len goes beyond packet buffer: %zu > %zu\n", rr_data_len, e - p);
		rr_entry_destroy(rr);
		return 0;
	}

	e = p + rr_data_len;

	switch (rr->type) {
		case RR_A:
			if (rr_data_len < sizeof(uint32_t)) {
				DEBUG_PRINTF("invalid rr_data_len=%zu for A record\n", rr_data_len);
				parse_error = 1;
				break;
			}
			rr->data.A.addr = ntohl(mdns_read_u32(p));
			p += sizeof(uint32_t);
			break;

		case RR_AAAA: {
			int i;
			if (rr_data_len < sizeof(struct in6_addr)) {
				DEBUG_PRINTF("invalid rr_data_len=%zu for AAAA record\n", rr_data_len);
				parse_error = 1;
				break;
			}
			rr->data.AAAA.addr = malloc(sizeof(struct in6_addr));
			for (i = 0; i < sizeof(struct in6_addr); i++)
				rr->data.AAAA.addr->s6_addr[i] = p[i];
			p += sizeof(struct in6_addr);
			break;
			}

		case RR_PTR:
			rr->data.PTR.name = uncompress_nlabel(pkt_buf, pkt_len, p - pkt_buf);
			if (rr->data.PTR.name == NULL) {
				DEBUG_PRINTF("unable to parse/uncompress label for PTR name\n");
				parse_error = 1;
				break;
			}
			p += rr_data_len;
			break;

		case RR_TXT:
			txt_rec = &rr->data.TXT;

			if (rr_data_len == 0) {
				DEBUG_PRINTF("WARN: rr_data_len for TXT is 0\n");
				txt_rec->txt = create_label("");
				break;
			}

			while (1) {
				txt_rec->txt = copy_label(pkt_buf, pkt_len, p - pkt_buf);
				if (txt_rec->txt == NULL) {
					DEBUG_PRINTF("unable to copy label for TXT record\n");
					parse_error = 1;
					break;
				}
				p += txt_rec->txt[0] + 1;

				if (p >= e)
					break;

				txt_rec->next = malloc(sizeof(struct rr_data_txt));
				txt_rec = txt_rec->next;
				txt_rec->next = NULL;
			}
			break;

		default:

			p = e;
	}

	if (parse_error) {
		rr_entry_destroy(rr);
		return 0;
	}

	rr_list_append(&pkt->rr_ans, rr);

	return p - (pkt_buf + off);
}

struct mdns_pkt *mdns_parse_pkt(uint8_t *pkt_buf, size_t pkt_len) {
	uint8_t *p = pkt_buf;
	size_t off;
	struct mdns_pkt *pkt;
	int i;

	if (pkt_len < 12)
		return NULL;

	MALLOC_ZERO_STRUCT(pkt, mdns_pkt);

	pkt->id 			= mdns_read_u16(p); p += sizeof(uint16_t);
	pkt->flags 			= mdns_read_u16(p); p += sizeof(uint16_t);
	pkt->num_qn 		= mdns_read_u16(p); p += sizeof(uint16_t);
	pkt->num_ans_rr 	= mdns_read_u16(p); p += sizeof(uint16_t);
	pkt->num_auth_rr 	= mdns_read_u16(p); p += sizeof(uint16_t);
	pkt->num_add_rr 	= mdns_read_u16(p); p += sizeof(uint16_t);

	off = p - pkt_buf;

	for (i = 0; i < pkt->num_qn; i++) {
		size_t l = mdns_parse_qn(pkt_buf, pkt_len, off, pkt);
		if (! l) {
			DEBUG_PRINTF("error parsing question #%d\n", i);
			mdns_pkt_destroy(pkt);
			return NULL;
		}

		off += l;
	}

	for (i = 0; i < pkt->num_ans_rr; i++) {
		size_t l = mdns_parse_rr(pkt_buf, pkt_len, off, pkt);
		if (! l) {
			DEBUG_PRINTF("error parsing answer #%d\n", i);
			mdns_pkt_destroy(pkt);
			return NULL;
		}

		off += l;
	}

	return pkt;
}

static size_t mdns_encode_name(uint8_t *pkt_buf, size_t pkt_len, size_t off,
		const uint8_t *name, struct name_comp *comp) {
	struct name_comp *c, *c_tail = NULL;
	uint8_t *p = pkt_buf + off;
	size_t len = 0;

	if (name) {
		while (*name) {
			int segment_len;

			DECL_STRUCT(new_c, name_comp);

			for (c = comp; c; c = c->next) {
				if (cmp_nlabel(name, c->label) == 0) {
					mdns_write_u16(p, 0xC000 | (c->pos & ~0xC000));
					return len + sizeof(uint16_t);
				}

				if (c->next == NULL)
					c_tail = c;
			}

			segment_len = *name + 1;
			strncpy((char *) p, (char *) name, segment_len);
			MALLOC_ZERO_STRUCT(new_c, name_comp);

			new_c->label = (uint8_t *) name;
			new_c->pos = p - pkt_buf;
			c_tail->next = new_c;

			p += segment_len;
			len += segment_len;
			name += segment_len;
		}
	}

	*p = '\0';
	len += 1;

	return len;
}

static size_t mdns_encode_rr(uint8_t *pkt_buf, size_t pkt_len, size_t off,
		struct rr_entry *rr, struct name_comp *comp) {
	uint8_t *p = pkt_buf + off, *p_data;
	size_t l;
	struct rr_data_txt *txt_rec;
	uint8_t *label;
	int i;

	assert(off < pkt_len);

	l = mdns_encode_name(pkt_buf, pkt_len, off, rr->name, comp);
	assert(l != 0);
	p += l;

	p = mdns_write_u16(p, rr->type);

	p = mdns_write_u16(p, (rr->rr_class & ~0x8000) | (rr->cache_flush << 15));

	p = mdns_write_u32(p, rr->ttl);

	p += sizeof(uint16_t);

	p_data = p;

	switch (rr->type) {
		case RR_A:

			p = mdns_write_u32(p, htonl(rr->data.A.addr));
			break;

		case RR_AAAA:
			for (i = 0; i < sizeof(struct in6_addr); i++)
				*p++ = rr->data.AAAA.addr->s6_addr[i];
			break;

		case RR_PTR:
			label = rr->data.PTR.name ?
					rr->data.PTR.name :
					rr->data.PTR.entry->name;
			p += mdns_encode_name(pkt_buf, pkt_len, p - pkt_buf, label, comp);
			break;

		case RR_TXT:
			txt_rec = &rr->data.TXT;
			for (; txt_rec; txt_rec = txt_rec->next) {
				int len = txt_rec->txt[0] + 1;
				strncpy((char *) p, (char *) txt_rec->txt, len);
				p += len;
			}
			break;

		case RR_SRV:
			p = mdns_write_u16(p, rr->data.SRV.priority);

			p = mdns_write_u16(p, rr->data.SRV.weight);

			p = mdns_write_u16(p, rr->data.SRV.port);

			p += mdns_encode_name(pkt_buf, pkt_len, p - pkt_buf,
					rr->data.SRV.target, comp);
			break;

		case RR_NSEC:
			p += mdns_encode_name(pkt_buf, pkt_len, p - pkt_buf,
					rr->name, comp);

			*p++ = 0;

			*p++ = sizeof(rr->data.NSEC.bitmap);

			for (i = 0; i < sizeof(rr->data.NSEC.bitmap); i++)
				*p++ = rr->data.NSEC.bitmap[i];

			break;

		default:
			DEBUG_PRINTF("unhandled rr type 0x%02x\n", rr->type);
	}

	l = p - p_data;

	mdns_write_u16(p - l - sizeof(uint16_t), l);

	return p - pkt_buf - off;
}

size_t mdns_encode_pkt(struct mdns_pkt *answer, uint8_t *pkt_buf, size_t pkt_len) {
	struct name_comp *comp;
	uint8_t *p = pkt_buf;

	size_t off;
	int i;
	struct rr_list *rr_set[3];

	assert(answer != NULL);
	assert(pkt_len >= 12);

	if (p == NULL)
		return -1;

	assert(answer->num_qn == 0);

	p = mdns_write_u16(p, answer->id);
	p = mdns_write_u16(p, answer->flags);
	p = mdns_write_u16(p, answer->num_qn);
	p = mdns_write_u16(p, answer->num_ans_rr);
	p = mdns_write_u16(p, answer->num_auth_rr);
	p = mdns_write_u16(p, answer->num_add_rr);

	off = p - pkt_buf;

	comp = malloc(sizeof(struct name_comp));
	if (comp == NULL)
		return -1;
	memset(comp, 0, sizeof(struct name_comp));

	comp->label = (uint8_t *) "";
	comp->pos = 0;

	rr_set[0] =	answer->rr_ans;
	rr_set[1] = answer->rr_auth;
	rr_set[2] =	answer->rr_add;

	for (i = 0; i < sizeof(rr_set) / sizeof(rr_set[0]); i++) {
		struct rr_list *rr = rr_set[i];
		for (; rr; rr = rr->next) {
			size_t l = mdns_encode_rr(pkt_buf, pkt_len, off, rr->e, comp);
			off += l;

			if (off >= pkt_len) {
				DEBUG_PRINTF("packet buffer too small\n");
				return -1;
			}
		}

	}

	while (comp) {
		struct name_comp *c = comp->next;
		free(comp);
		comp = c;
	}

	return off;
}

