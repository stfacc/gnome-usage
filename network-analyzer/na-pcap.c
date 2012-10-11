/*
 * na-pcap.c
 *
 * Copyright (c) 2012 Stefano Facchini
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
/* Partly derived from NetHogs, Copyright (c) 2004-2011 Arnout Engelen */

#include "na-pcap.h"

#include "na-packet.h"
#include "na-connection.h"

#include <glib.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <pcap/pcap.h>

GQuark
na_pcap_error_quark ()
{
  return g_quark_from_static_string ("na-pcap-error-quark");
}

struct _NAPCapHandle
{
  pcap_t *pcap_handle;
  int linktype;
  const char *iface;
};

NAPCapHandle *
na_pcap_open (const char *iface,
              GError     **error)
{
  char errbuf[PCAP_ERRBUF_SIZE];

  pcap_t *pcap_handle = pcap_open_live (iface, BUFSIZ, 0, 100, errbuf);

  if (pcap_handle == NULL)
    {
      g_set_error (error,
                   NA_PCAP_ERROR,
                   NA_PCAP_ERROR_OPEN,
                   "pcap failed to open iface: %s", errbuf);
      return NULL;
    }

  NAPCapHandle *handle = g_new0 (NAPCapHandle, 1);

  handle->pcap_handle = pcap_handle;
  handle->iface = iface;
  handle->linktype = pcap_datalink (pcap_handle);

  switch (handle->linktype)
    {
    case (DLT_EN10MB):
      g_debug ("Ethernet link detected");
      break;
    case (DLT_PPP):
      g_debug ("PPP link detected");
      break;
    case (DLT_LINUX_SLL):
      g_debug ("Linux Cooked Socket link detected");
      break;
    default:
      g_debug ("No PPP or Ethernet link: %d", handle->linktype);
      break;
    }

  return handle;
}

typedef struct
{
  sa_family_t sa_family;

  union {
    struct in_addr ip;
    struct in6_addr ip6;
  } src;

  union {
    struct in_addr ip;
    struct in6_addr ip6;
  } dst;
} IPAddress;

static void
parse_tcp (NAPCapHandle      *handle,
           const struct pcap_pkthdr *header,
           const u_char      *packet,
           const IPAddress    *address)
{
  struct tcphdr *tcp = (struct tcphdr *)packet;

  NAPacket *na_packet;
  switch (address->sa_family)
    {
    case (AF_INET):
      na_packet = na_packet_new_from_ip4 (address->src.ip,
                                          ntohs(tcp->source),
                                          address->dst.ip,
                                          ntohs(tcp->dest),
                                          header->len,
                                          header->ts,
                                          NA_PACKET_DIRECTION_UNKNOWN);
      break;
    case (AF_INET6):
      na_packet = na_packet_new_from_ip6 (address->src.ip6,
                                          ntohs(tcp->source),
                                          address->dst.ip6,
                                          ntohs(tcp->dest),
                                          header->len,
                                          header->ts,
                                          NA_PACKET_DIRECTION_UNKNOWN);
      break;
    }

  NAConnection *connection = na_connection_find_from_packet (na_packet);

  if (connection != NULL)
    {
      g_debug ("Adding packet to connection");
      na_connection_add_packet (connection, na_packet);
    }
  else
    {
      /* unknown connection, create new */
      connection = na_connection_new (na_packet);
      na_process_find_from_connection (connection, handle->iface);
    }

  na_packet_free (na_packet);
}

#if 0
int process_udp (u_char * userdata, const dp_header * header, const u_char * m_packet) {
	PCapContext *context = (PCapContext *) userdata;
	//struct tcphdr * tcp = (struct tcphdr *) m_packet;
	struct udphdr * udp = (struct udphdr *) m_packet;

	curtime = header->ts;

	/* TODO get info from userdata, then call getPacket */
	NAPacket * packet;
	switch (context->sa_family)
	{
		case (AF_INET):
			packet = na_packet_new_from_ip4 (context->ip_src, ntohs(udp->source), context->ip_dst, ntohs(udp->dest), header->len, header->ts, NA_PACKET_DIRECTION_UNKNOWN);
			break;
		case (AF_INET6):
			packet = na_packet_new_from_ip6 (context->ip6_src, ntohs(udp->source), context->ip6_dst, ntohs(udp->dest), header->len, header->ts, NA_PACKET_DIRECTION_UNKNOWN);
			break;
	}

	NAConnection * connection = na_connection_find_from_packet (packet);

	if (connection != NULL)
	{
		/* add packet to the connection */
		na_connection_add_packet (connection, packet);
	} else {
		/* else: unknown connection, create new */
		connection = na_connection_new (packet);
		na_process_find_from_connection (connection, context->iface);
	}
	na_packet_free (packet);

	/* we're done now. */
	return TRUE;
}

#endif

static void
parse_ip (NAPCapHandle      *handle,
          const struct pcap_pkthdr *header,
          const u_char      *packet)
{
  const struct ip *ip = (struct ip *)packet;
  u_char * payload = (u_char *) packet + sizeof (struct ip);

  IPAddress address;
  address.sa_family = AF_INET;
  address.src.ip = ip->ip_src;
  address.dst.ip = ip->ip_dst;

  switch (ip->ip_p)
    {
    case (6):
      parse_tcp (handle, header, payload, &address);
      break;
    default:
      break;
    }
}

static void
parse_ip6 (NAPCapHandle      *handle,
           const struct pcap_pkthdr *header,
           const u_char      *packet)
{
  const struct ip6_hdr *ip6 = (struct ip6_hdr *)packet;
  u_char *payload = (u_char *)packet + sizeof (struct ip6_hdr);

  IPAddress address;
  address.sa_family = AF_INET6;
  address.src.ip6 = ip6->ip6_src;
  address.dst.ip6 = ip6->ip6_dst;

  switch ((ip6->ip6_ctlun).ip6_un1.ip6_un1_nxt)
    {
    case (6):
      parse_tcp (handle, header, payload, &address);
      break;
    default:
      break;
    }
}

static void
parse_ethernet (NAPCapHandle      *handle,
                const struct pcap_pkthdr *header,
                const u_char      *packet)
{
  const struct ether_header *ethernet = (struct ether_header *)packet;
  u_char * payload = (u_char *)packet + sizeof (struct ether_header);

  switch (ethernet->ether_type)
    {
    case (0x0008):
      parse_ip (handle, header, payload);
      break;
    case (0xDD86):
      parse_ip6 (handle, header, payload);
      break;
    default:
      break;
    }
}

/* ppp header, i hope ;) */
/* glanced from ethereal, it's 16 bytes, and the payload packet type is
 * in the last 2 bytes... */
struct ppp_header {
  u_int16_t dummy1;
  u_int16_t dummy2;
  u_int16_t dummy3;
  u_int16_t dummy4;
  u_int16_t dummy5;
  u_int16_t dummy6;
  u_int16_t dummy7;

  u_int16_t packettype;
};

static void
parse_ppp (NAPCapHandle      *handle,
           const struct pcap_pkthdr *header,
           const u_char      *packet)
{
  const struct ppp_header *ppp = (struct ppp_header *)packet;
  u_char *payload = (u_char *)packet + sizeof (struct ppp_header);

  switch (ppp->packettype)
    {
    case (0x0008):
      parse_ip (handle, header, payload);
      break;
    case (0xDD86):
      parse_ip6 (handle, header, payload);
      break;
    default:
      break;
    }
}

/* linux cooked header, i hope ;) */
/* glanced from libpcap/ssl.h */
#define SLL_ADDRLEN	8		/* length of address field */
struct sll_header {
	u_int16_t sll_pkttype;		/* packet type */
	u_int16_t sll_hatype;		/* link-layer address type */
	u_int16_t sll_halen;		/* link-layer address length */
	u_int8_t sll_addr[SLL_ADDRLEN];	/* link-layer address */
	u_int16_t sll_protocol;		/* protocol */
};

static void
parse_linux_cooked (NAPCapHandle             *handle,
                    const struct pcap_pkthdr *header,
                    const u_char             *packet)
{
  const struct sll_header *sll = (struct sll_header *)packet;
  u_char *payload = (u_char *)packet + sizeof (struct sll_header);

  switch (sll->sll_protocol)
    {
    case (0x0008):
      parse_ip (handle, header, payload);
      break;
    case (0xDD86):
      parse_ip6 (handle, header, payload);
      break;
    default:
      break;
    }
}

static void
pcap_dispatch_cb (u_char                   *u_handle,
                  const struct pcap_pkthdr *header,
                  const u_char             *packet)
{
  NAPCapHandle *handle = (NAPCapHandle *)u_handle;

  switch (handle->linktype)
    {
    case (DLT_EN10MB):
      parse_ethernet (handle, header, packet);
      break;
    case (DLT_PPP):
      parse_ppp (handle, header, packet);
      break;
    case (DLT_LINUX_SLL):
      parse_linux_cooked (handle, header, packet);
      break;
    case (DLT_RAW):
    case (DLT_NULL):
      // hope for the best
      parse_ip (handle, header, packet);
      break;
    default:
      g_debug ("Unknown linktype %d", handle->linktype);
      break;
    }
}

int
na_pcap_dispatch (NAPCapHandle *handle)
{
  return pcap_dispatch (handle->pcap_handle, -1, pcap_dispatch_cb, (u_char *)handle);
}

const char *
na_pcap_handle_get_iface (NAPCapHandle *handle)
{
  return handle->iface;
}
