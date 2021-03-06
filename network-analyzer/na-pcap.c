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
#include <net/ppp_defs.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap/pcap.h>

/* In order to identify the process using a TCP connection, we need to know
 * the source and destination ports, which are stored in the TCP header.
 * To be sure to capture these data, on ethernet for example, we need to capture:
 *
 *   Ethernet header       at most 18 bytes +
 *   IP header             at most 60 bytes +
 *   mandatory TCP header          20 bytes
 *
 * which gives 98 bytes. Let's be ultra conservative and just set the snapshot
 * length to the more-or-less arbitrary number of 200 bytes.
 */
#define PCAP_SNAPLEN 200

#define TCP_PROTOCOL_NUMBER 6
#define UDP_PROTOCOL_NUMBER 17

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

  pcap_t *pcap_handle = pcap_create (iface, errbuf);

  if (pcap_handle == NULL)
    {
      g_set_error (error,
                   NA_PCAP_ERROR,
                   NA_PCAP_ERROR_OPEN,
                   "pcap failed to create handle for iface: %s", errbuf);
      return NULL;
    }

  pcap_set_snaplen (pcap_handle, PCAP_SNAPLEN);
  pcap_set_timeout (pcap_handle, 100);

  if (pcap_activate (pcap_handle) != 0)
    {
      g_set_error (error,
                   NA_PCAP_ERROR,
                   NA_PCAP_ERROR_OPEN,
                   "pcap failed to activate handle for iface");
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
push_packet (const struct pcap_pkthdr *header,
             const IPAddress          *address,
             gushort                   s_port,
             gushort                   d_port)
{
  NAPacket *na_packet;
  switch (address->sa_family)
    {
    case (AF_INET):
      na_packet = na_packet_new_from_ip4 (address->src.ip,
                                          s_port,
                                          address->dst.ip,
                                          d_port,
                                          header->len,
                                          header->ts,
                                          NA_PACKET_DIRECTION_UNKNOWN);
      break;
    case (AF_INET6):
      na_packet = na_packet_new_from_ip6 (address->src.ip6,
                                          s_port,
                                          address->dst.ip6,
                                          d_port,
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
      na_process_find_from_connection (connection);
    }

  na_packet_free (na_packet);
}

static void
parse_tcp (const struct pcap_pkthdr *header,
           const u_char             *packet,
           const IPAddress          *address)
{
  struct tcphdr *tcp = (struct tcphdr *)packet;
  push_packet (header, address, ntohs (tcp->source), ntohs (tcp->dest));
}

static void
parse_udp (const struct pcap_pkthdr *header,
           const u_char             *packet,
           const IPAddress          *address)
{
  struct udphdr *udp = (struct udphdr *)packet;
  push_packet (header, address, ntohs (udp->source), ntohs (udp->dest));
}

static void
parse_ip (const struct pcap_pkthdr *header,
          const u_char             *packet)
{
  const struct ip *ip = (struct ip *)packet;
  const u_char *payload = packet + sizeof (struct ip);

  IPAddress address;
  address.sa_family = AF_INET;
  address.src.ip = ip->ip_src;
  address.dst.ip = ip->ip_dst;

  switch (ip->ip_p)
    {
    case (TCP_PROTOCOL_NUMBER):
      parse_tcp (header, payload, &address);
      break;
    case (UDP_PROTOCOL_NUMBER):
      parse_udp (header, payload, &address);
      break;
    default:
      break;
    }
}

static void
parse_ip6 (const struct pcap_pkthdr *header,
           const u_char             *packet)
{
  const struct ip6_hdr *ip6 = (struct ip6_hdr *)packet;
  const u_char *payload = packet + sizeof (struct ip6_hdr);

  IPAddress address;
  address.sa_family = AF_INET6;
  address.src.ip6 = ip6->ip6_src;
  address.dst.ip6 = ip6->ip6_dst;

  switch ((ip6->ip6_ctlun).ip6_un1.ip6_un1_nxt)
    {
    case (TCP_PROTOCOL_NUMBER):
      parse_tcp (header, payload, &address);
      break;
    case (UDP_PROTOCOL_NUMBER):
      parse_udp (header, payload, &address);
      break;
    default:
      break;
    }
}

static void
parse_ethernet (const struct pcap_pkthdr *header,
                const u_char             *packet)
{
  const struct ether_header *ethernet = (struct ether_header *)packet;
  const u_char * payload = packet + sizeof (struct ether_header);

  switch (ntohs (ethernet->ether_type))
    {
    case (ETHERTYPE_IP):
      parse_ip (header, payload);
      break;
    case (ETHERTYPE_IPV6):
      parse_ip6 (header, payload);
      break;
    default:
      break;
    }
}

static void
parse_ppp (const struct pcap_pkthdr *header,
           const u_char             *packet)
{
  const u_char *payload = packet + PPP_HDRLEN;

  switch (PPP_PROTOCOL (packet))
    {
    case (PPP_IP):
      parse_ip (header, payload);
      break;
    case (PPP_IPV6):
      parse_ip6 (header, payload);
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
parse_linux_cooked (const struct pcap_pkthdr *header,
                    const u_char             *packet)
{
  const struct sll_header *sll = (struct sll_header *)packet;
  u_char *payload = (u_char *)packet + sizeof (struct sll_header);

  switch (sll->sll_protocol)
    {
    case (0x0008):
      parse_ip (header, payload);
      break;
    case (0xDD86):
      parse_ip6 (header, payload);
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
      parse_ethernet (header, packet);
      break;
    case (DLT_PPP):
      parse_ppp (header, packet);
      break;
    case (DLT_LINUX_SLL):
      parse_linux_cooked (header, packet);
      break;
    case (DLT_RAW):
    case (DLT_NULL):
      // hope for the best
      parse_ip (header, packet);
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
