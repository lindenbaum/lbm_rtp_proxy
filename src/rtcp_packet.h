/**
 * Copyright (C) 2015  Lindenbaum GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _RTCP_PACKET_H_
#define _RTCP_PACKET_H_

#define SR_PACKET_TYPE   200
#define RR_PACKET_TYPE   201
#define SDES_PACKET_TYPE 202
#define BYE_PACKET_TYPE  203
#define APP_PACKET_TYPE  204

struct rtcp_packet {
#if defined(__BIG_ENDIAN_BITFIELD)
  uint8_t V:2;
  uint8_t P:1;
  uint8_t RC:5;
#elif defined(__LITTLE_ENDIAN_BITFIELD)
  uint8_t RC:5;
  uint8_t P:1;
  uint8_t V:2;
#else
# error "this endianness is not supported"
#endif
  uint8_t PT:8;

  __be16  length;

  __be32  SSRC;

  // ...
};

/*
struct sr_packet {
  struct rtcp_packet header;
  __be64 NTP_timestamp;
  __be32 RTP_timestamp;
  __be32 packet_count;
  __be32 octet_count;
};
*/


/* [RFC 1889]

6.1 RTCP Packet Format

This specification defines several RTCP packet types to carry a
variety of control information:

SR
Sender report, for transmission and reception statistics from
participants that are active senders

RR
Receiver report, for reception statistics from participants that are
not active senders

SDES
Source description items, including CNAME

BYE
Indicates end of participation

APP
Application specific functions

Each RTCP packet begins with a fixed part similar to that of RTP data
packets, followed by structured elements that may be of variable
length according to the packet type but always end on a 32-bit
boundary. The alignment requirement and a length field in the fixed
part are included to make RTCP packets "stackable". Multiple RTCP
packets may be concatenated without any intervening separators to form
a compound RTCP packet that is sent in a single packet of the lower
layer protocol, for example UDP. There is no explicit count of
individual RTCP packets in the compound packet since the lower layer
protocols are expected to provide an overall length to determine the
end of the compound packet.

Each individual RTCP packet in the compound packet may be processed
independently with no requirements upon the order or combination of
packets. However, in order to perform the functions of the protocol,
the following constraints are imposed:

Reception statistics (in SR or RR) should be sent as often as
bandwidth constraints will allow to maximize the resolution of the
statistics, therefore each periodically transmitted compound RTCP
packet should include a report packet.  New receivers need to receive
the CNAME for a source as soon as possible to identify the source and
to begin associating media for purposes such as lip-sync, so each
compound RTCP packet should also include the SDES CNAME.  The number
of packet types that may appear first in the compound packet should be
limited to increase the number of constant bits in the first word and
the probability of successfully validating RTCP packets against
misaddressed RTP data packets or other unrelated packets.  Thus, all
RTCP packets must be sent in a compound packet of at least two
individual packets, with the following format recommended:

Encryption prefix If and only if the compound packet is to be
encrypted, it is prefixed by a random 32-bit quantity redrawn for
every compound packet transmitted.  SR or RR The first RTCP packet in
the compound packet must always be a report packet to facilitate
header validation as described in Appendix A.2. This is true even if
no data has been sent nor received, in which case an empty RR is sent,
and even if the only other RTCP packet in the compound packet is a
BYE.  Additional RRs If the number of sources for which reception
statistics are being reported exceeds 31, the number that will fit
into one SR or RR packet, then additional RR packets should follow the
initial report packet.  SDES An SDES packet containing a CNAME item
must be included in each compound RTCP packet. Other source
description items may optionally be included if required by a
particular application, subject to bandwidth constraints (see Section
6.2.2).  BYE or APP Other RTCP packet types, including those yet to be
defined, may follow in any order, except that BYE should be the last
packet sent with a given SSRC/CSRC. Packet types may appear more than
once.  It is advisable for translators and mixers to combine
individual RTCP packets from the multiple sources they are forwarding
into one compound packet whenever feasible in order to amortize the
packet overhead (see Section 7). An example RTCP compound packet as
might be produced by a mixer is shown in Fig. 1. If the overall length
of a compound packet would exceed the maximum transmission unit (MTU)
of the network path, it may be segmented into multiple shorter
compound packets to be transmitted in separate packets of the
underlying protocol. Note that each of the compound packets must begin
with an SR or RR packet.

An implementation may ignore incoming RTCP packets with types unknown
to it. Additional RTCP packet types may be registered with the
Internet Assigned Numbers Authority (IANA).

   if encrypted: random 32-bit integer
    |
    |[------- packet -------][----------- packet -----------][-packet-]
    |
    |             receiver reports          chunk        chunk
    V                                    item  item     item  item
   --------------------------------------------------------------------
   |R[SR|# sender #site#site][SDES|# CNAME PHONE |#CNAME LOC][BYE##why]
   |R[  |# report #  1 #  2 ][    |#             |#         ][   ##   ]
   |R[  |#        #    #    ][    |#             |#         ][   ##   ]
   |R[  |#        #    #    ][    |#             |#         ][   ##   ]
   --------------------------------------------------------------------
   |<------------------  UDP packet (compound packet) --------------->|


   #: SSRC/CSRC

              Figure 1: Example of an RTCP compound packet


6.3.1 SR: Sender report RTCP packet

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|    RC   |   PT=SR=200   |             length            | header
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         SSRC of sender                        |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|              NTP timestamp, most significant word             | sender
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ info
|             NTP timestamp, least significant word             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         RTP timestamp                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     sender's packet count                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      sender's octet count                     |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                 SSRC_1 (SSRC of first source)                 | report
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ block
| fraction lost |       cumulative number of packets lost       |   1
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           extended highest sequence number received           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      interarrival jitter                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         last SR (LSR)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   delay since last SR (DLSR)                  |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                 SSRC_2 (SSRC of second source)                | report
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ block
:                               ...                             :   2
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                  profile-specific extensions                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

The sender report packet consists of three sections, possibly followed
by a fourth profile-specific extension section if defined. The first
section, the header, is 8 octets long. The fields have the following
meaning:

version (V): 2 bits
Identifies the version of RTP, which is the same in RTCP packets as in
RTP data packets. The version defined by this specification is two
(2).

padding (P): 1 bit
If the padding bit is set, this RTCP packet contains some additional
padding octets at the end which are not part of the control
information. The last octet of the padding is a count of how many
padding octets should be ignored. Padding may be needed by some
encryption algorithms with fixed block sizes. In a compound RTCP
packet, padding should only be required on the last individual packet
because the compound packet is encrypted as a whole.

reception report count (RC): 5 bits The number of reception report
blocks contained in this packet. A value of zero is valid.

packet type (PT): 8 bits
Contains the constant 200 to identify this as an RTCP SR packet.

length: 16 bits
The length of this RTCP packet in 32-bit words minus one, including
the header and any padding. (The offset of one makes zero a valid
length and avoids a possible infinite loop in scanning a compound RTCP
packet, while counting 32-bit words avoids a validity check for a
multiple of 4.)

SSRC: 32 bits
The synchronization source identifier for the originator of this SR
packet.  The second section, the sender information, is 20 octets long
and is present in every sender report packet. It summarizes the data
transmissions from this sender. The fields have the following meaning:

NTP timestamp: 64 bits
Indicates the wallclock time when this report was sent so that it may
be used in combination with timestamps returned in reception reports
from other receivers to measure round-trip propagation to those
receivers. Receivers should expect that the measurement accuracy of
the timestamp may be limited to far less than the resolution of the
NTP timestamp. The measurement uncertainty of the timestamp is not
indicated as it may not be known. A sender that can keep track of
elapsed time but has no notion of wallclock time may use the elapsed
time since joining the session instead. This is assumed to be less
than 68 years, so the high bit will be zero. It is permissible to use
the sampling clock to estimate elapsed wallclock time. A sender that
has no notion of wallclock or elapsed time may set the NTP timestamp
to zero.

RTP timestamp: 32 bits
Corresponds to the same time as the NTP timestamp (above), but in the
same units and with the same random offset as the RTP timestamps in
data packets. This correspondence may be used for intra- and
inter-media synchronization for sources whose NTP timestamps are
synchronized, and may be used by media- independent receivers to
estimate the nominal RTP clock frequency. Note that in most cases this
timestamp will not be equal to the RTP timestamp in any adjacent data
packet. Rather, it is calculated from the corresponding NTP timestamp
using the relationship between the RTP timestamp counter and real time
as maintained by periodically checking the wallclock time at a
sampling instant.

sender's packet count: 32 bits
The total number of RTP data packets transmitted by the sender since
starting transmission up until the time this SR packet was
generated. The count is reset if the sender changes its SSRC
identifier.

sender's octet count: 32 bits
The total number of payload octets (i.e., not including header or
padding) transmitted in RTP data packets by the sender since starting
transmission up until the time this SR packet was generated. The count
is reset if the sender changes its SSRC identifier. This field can be
used to estimate the average payload data rate.  The third section
contains zero or more reception report blocks depending on the number
of other sources heard by this sender since the last report. Each
reception report block conveys statistics on the reception of RTP
packets from a single synchronization source. Receivers do not carry
over statistics when a source changes its SSRC identifier due to a
collision. These statistics are:

SSRC_n (source identifier): 32 bits
The SSRC identifier of the source to which the information in this
reception report block pertains.

fraction lost: 8 bits
The fraction of RTP data packets from source SSRC_n lost since the
previous SR or RR packet was sent, expressed as a fixed point number
with the binary point at the left edge of the field. (That is
equivalent to taking the integer part after multiplying the loss
fraction by 256.) This fraction is defined to be the number of packets
lost divided by the number of packets expected, as defined in the next
paragraph. An implementation is shown in Appendix A.3. If the loss is
negative due to duplicates, the fraction lost is set to zero. Note
that a receiver cannot tell whether any packets were lost after the
last one received, and that there will be no reception report block
issued for a source if all packets from that source sent during the
last reporting interval have been lost.

cumulative number of packets lost: 24 bits
The total number of RTP data packets from source SSRC_n that have been
lost since the beginning of reception. This number is defined to be
the number of packets expected less the number of packets actually
received, where the number of packets received includes any which are
late or duplicates. Thus packets that arrive late are not counted as
lost, and the loss may be negative if there are duplicates. The number
of packets expected is defined to be the extended last sequence number
received, as defined next, less the initial sequence number
received. This may be calculated as shown in Appendix A.3.

extended highest sequence number received: 32 bits
The low 16 bits contain the highest sequence number received in an RTP
data packet from source SSRC_n, and the most significant 16 bits
extend that sequence number with the corresponding count of sequence
number cycles, which may be maintained according to the algorithm in
Appendix A.1. Note that different receivers within the same session
will generate different extensions to the sequence number if their
start times differ significantly.

interarrival jitter: 32 bits
An estimate of the statistical variance of the RTP data packet
interarrival time, measured in timestamp units and expressed as an
unsigned integer. The interarrival jitter J is defined to be the mean
deviation (smoothed absolute value) of the difference D in packet
spacing at the receiver compared to the sender for a pair of
packets. As shown in the equation below, this is equivalent to the
difference in the "relative transit time" for the two packets; the
relative transit time is the difference between a packet's RTP
timestamp and the receiver's clock at the time of arrival, measured in
the same units.  If Si is the RTP timestamp from packet i, and Ri is
the time of arrival in RTP timestamp units for packet i, then for two
packets i and j, D may be expressed as

                 D(i,j)=(Rj-Ri)-(Sj-Si)=(Rj-Sj)-(Ri-Si)

The interarrival jitter is calculated continuously as each data packet
i is received from source SSRC_n, using this difference D for that
packet and the previous packet i-1 in order of arrival (not
necessarily in sequence), according to the formula

                    J=J+(|D(i-1,i)|-J)/16

Whenever a reception report is issued, the current value of J is
sampled.

The jitter calculation is prescribed here to allow profile-
independent monitors to make valid interpretations of reports coming
from different implementations. This algorithm is the optimal first-
order estimator and the gain parameter 1/16 gives a good noise
reduction ratio while maintaining a reasonable rate of convergence
[11]. A sample implementation is shown in Appendix A.8.

last SR timestamp (LSR): 32 bits
The middle 32 bits out of 64 in the NTP timestamp (as explained in
Section 4) received as part of the most recent RTCP sender report (SR)
packet from source SSRC_n. If no SR has been received yet, the field
is set to zero.

delay since last SR (DLSR): 32 bits
The delay, expressed in units of 1/65536 seconds, between receiving
the last SR packet from source SSRC_n and sending this reception
report block. If no SR packet has been received yet from SSRC_n, the
DLSR field is set to zero.  Let SSRC_r denote the receiver issuing
this receiver report. Source SSRC_n can compute the round propagation
delay to SSRC_r by recording the time A when this reception report
block is received. It calculates the total round-trip time A-LSR using
the last SR timestamp (LSR) field, and then subtracting this field to
leave the round-trip propagation delay as (A- LSR - DLSR). This is
illustrated in Fig. 2.

This may be used as an approximate measure of distance to cluster
receivers, although some links have very asymmetric delays.


6.3.2 RR: Receiver report RTCP packet

   [10 Nov 1995 11:33:25.125]           [10 Nov 1995 11:33:36.5]
   n                 SR(n)              A=b710:8000 (46864.500 s)
   ---------------------------------------------------------------->
                      v                 ^
   ntp_sec =0xb44db705 v               ^ dlsr=0x0005.4000 (    5.250s)
   ntp_frac=0x20000000  v             ^  lsr =0xb705:2000 (46853.125s)
     (3024992016.125 s)  v           ^
   r                      v         ^ RR(n)
   ---------------------------------------------------------------->
                          |<-DLSR->|
                           (5.250 s)

   A     0xb710:8000 (46864.500 s)
   DLSR -0x0005:4000 (    5.250 s)
   LSR  -0xb705:2000 (46853.125 s)
   -------------------------------
   delay 0x   6:2000 (    6.125 s)

           Figure 2: Example for round-trip time computation

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|    RC   |   PT=RR=201   |             length            | header
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     SSRC of packet sender                     |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                 SSRC_1 (SSRC of first source)                 | report
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ block
| fraction lost |       cumulative number of packets lost       |   1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           extended highest sequence number received           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      interarrival jitter                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         last SR (LSR)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   delay since last SR (DLSR)                  |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                 SSRC_2 (SSRC of second source)                | report
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ block
:                               ...                             :   2
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                  profile-specific extensions                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

The format of the receiver report (RR) packet is the same as that of
the SR packet except that the packet type field contains the constant
201 and the five words of sender information are omitted (these are
the NTP and RTP timestamps and sender's packet and octet counts). The
remaining fields have the same meaning as for the SR packet.

An empty RR packet (RC = 0) is put at the head of a compound RTCP
packet when there is no data transmission or reception to report.


6.4 SDES: Source description RTCP packet

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|    SC   |  PT=SDES=202  |             length            | header
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                          SSRC/CSRC_1                          | chunk
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   1
|                           SDES items                          |
|                              ...                              |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                          SSRC/CSRC_2                          | chunk
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   2
|                           SDES items                          |
|                              ...                              |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+

The SDES packet is a three-level structure composed of a header and
zero or more chunks, each of of which is composed of items describing
the source identified in that chunk. The items are described
individually in subsequent sections.

version (V), padding (P), length:
As described for the SR packet (see Section 6.3.1).

packet type (PT): 8 bits
Contains the constant 202 to identify this as an RTCP SDES packet.

source count (SC): 5 bits
The number of SSRC/CSRC chunks contained in this SDES packet. A value
of zero is valid but useless.  Each chunk consists of an SSRC/CSRC
identifier followed by a list of zero or more items, which carry
information about the SSRC/CSRC. Each chunk starts on a 32-bit
boundary. Each item consists of an 8-bit type field, an 8-bit octet
count describing the length of the text (thus, not including this
two-octet header), and the text itself. Note that the text can be no
longer than 255 octets, but this is consistent with the need to limit
RTCP bandwidth consumption.

The text is encoded according to the UTF-2 encoding specified in Annex
F of ISO standard 10646 [12,13]. This encoding is also known as UTF-8
or UTF-FSS. It is described in "File System Safe UCS Transformation
Format (FSS_UTF)", X/Open Preliminary Specification, Document Number
P316 and Unicode Technical Report #4. US-ASCII is a subset of this
encoding and requires no additional encoding. The presence of
multi-octet encodings is indicated by setting the most significant bit
of a character to a value of one.

Items are contiguous, i.e., items are not individually padded to a
32-bit boundary. Text is not null terminated because some multi-octet
encodings include null octets. The list of items in each chunk is
terminated by one or more null octets, the first of which is
interpreted as an item type of zero to denote the end of the list, and
the remainder as needed to pad until the next 32-bit boundary. A chunk
with zero items (four null octets) is valid but useless.

End systems send one SDES packet containing their own source
identifier (the same as the SSRC in the fixed RTP header). A mixer
sends one SDES packet containing a chunk for each contributing source
from which it is receiving SDES information, or multiple complete SDES
packets in the format above if there are more than 31 such sources
(see Section 7).

The SDES items currently defined are described in the next
sections. Only the CNAME item is mandatory. Some items shown here may
be useful only for particular profiles, but the item types are all
assigned from one common space to promote shared use and to simplify
profile- independent applications. Additional items may be defined in
a profile by registering the type numbers with IANA.

6.5 BYE: Goodbye RTCP packet

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P|    SC   |   PT=BYE=203  |             length            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           SSRC/CSRC                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   :                              ...                              :
   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
   |     length    |               reason for leaving             ... (opt)
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

The BYE packet indicates that one or more sources are no longer
active.

version (V), padding (P), length:
As described for the SR packet (see Section 6.3.1).

packet type (PT): 8 bits
Contains the constant 203 to identify this as an RTCP BYE packet.

source count (SC): 5 bits
The number of SSRC/CSRC identifiers included in this BYE packet. A
count value of zero is valid, but useless.  If a BYE packet is
received by a mixer, the mixer forwards the BYE packet with the
SSRC/CSRC identifier(s) unchanged. If a mixer shuts down, it should
send a BYE packet listing all contributing sources it handles, as well
as its own SSRC identifier. Optionally, the BYE packet may include an
8-bit octet count followed by that many octets of text indicating the
reason for leaving, e.g., "camera malfunction" or "RTP loop
detected". The string has the same encoding as that described for
SDES. If the string fills the packet to the next 32-bit boundary, the
string is not null terminated. If not, the BYE packet is padded with
null octets.

6.6 APP: Application-defined RTCP packet

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P| subtype |   PT=APP=204  |             length            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           SSRC/CSRC                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                          name (ASCII)                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                   application-dependent data                 ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

The APP packet is intended for experimental use as new applications
and new features are developed, without requiring packet type value
registration. APP packets with unrecognized names should be
ignored. After testing and if wider use is justified, it is
recommended that each APP packet be redefined without the subtype and
name fields and registered with the Internet Assigned Numbers
Authority using an RTCP packet type.

version (V), padding (P), length:
As described for the SR packet (see Section 6.3.1).

subtype: 5 bits
May be used as a subtype to allow a set of APP packets to be defined
under one unique name, or for any application-dependent data.

packet type (PT): 8 bits
Contains the constant 204 to identify this as an RTCP APP packet.

name: 4 octets
A name chosen by the person defining the set of APP packets to be
unique with respect to other APP packets this application might
receive. The application creator might choose to use the application
name, and then coordinate the allocation of subtype values to others
who want to define new packet types for the
application. Alternatively, it is recommended that others choose a
name based on the entity they represent, then coordinate the use of
the name within that entity. The name is interpreted as a sequence of
four ASCII characters, with uppercase and lowercase characters treated
as distinct.

application-dependent data: variable length
Application-dependent data may or may not appear in an APP packet. It
is interpreted by the application and not RTP itself. It must be a
multiple of 32 bits long.

*/

#endif // _RTCP_PACKET_H_
