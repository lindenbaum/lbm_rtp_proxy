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

#ifndef _RTP_PACKET_H_
#define _RTP_PACKET_H_

struct rtp_packet {
#if defined(__BIG_ENDIAN_BITFIELD)
  uint8_t V:2;
  uint8_t P:1;
  uint8_t X:1;
  uint8_t CC:4;

  uint8_t M:1;
  uint8_t PT:7;
#elif defined(__LITTLE_ENDIAN_BITFIELD)
  uint8_t CC:4;
  uint8_t X:1;
  uint8_t P:1;
  uint8_t V:2;

  uint8_t PT:7;
  uint8_t M:1;
#else
# error "this endianness is not supported"
#endif

  __be16  SN;

  __be32  TS;

  __be32  SSRC;

  // __be32  CSRC[15]; // actually 0-15, depending on value of CC

  // ...
};

/* [RFC 1889]

5.1 RTP Fixed Header Fields

The RTP header has the following format:


    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           timestamp                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           synchronization source (SSRC) identifier            |
   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
   |            contributing source (CSRC) identifiers             |
   |                             ....                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

The first twelve octets are present in every RTP packet, while the
list of CSRC identifiers is present only when inserted by a mixer. The
fields have the following meaning:

version (V): 2 bits
This field identifies the version of RTP. The version defined by this
specification is two (2). (The value 1 is used by the first draft
version of RTP and the value 0 is used by the protocol initially
implemented in the "vat" audio tool.)  padding (P): 1 bit If the
padding bit is set, the packet contains one or more additional padding
octets at the end which are not part of the payload. The last octet of
the padding contains a count of how many padding octets should be
ignored. Padding may be needed by some encryption algorithms with
fixed block sizes or for carrying several RTP packets in a lower-layer
protocol data unit.

extension (X): 1 bit
If the extension bit is set, the fixed header is followed by exactly
one header extension, with a format defined in Section 5.3.1.

CSRC count (CC): 4 bits
The CSRC count contains the number of CSRC identifiers that follow the
fixed header.

marker (M): 1 bit
The interpretation of the marker is defined by a profile. It is
intended to allow significant events such as frame boundaries to be
marked in the packet stream. A profile may define additional marker
bits or specify that there is no marker bit by changing the number of
bits in the payload type field (see Section 5.3).

payload type (PT): 7 bits
This field identifies the format of the RTP payload and determines its
interpretation by the application. A profile specifies a default
static mapping of payload type codes to payload formats. Additional
payload type codes may be defined dynamically through non-RTP means
(see Section 3). An initial set of default mappings for audio and
video is specified in the companion profile Internet-Draft
draft-ietf-avt-profile, and may be extended in future editions of the
Assigned Numbers RFC [6]. An RTP sender emits a single RTP payload
type at any given time; this field is not intended for multiplexing
separate media streams (see Section 5.2).

sequence number: 16 bits
The sequence number increments by one for each RTP data packet sent,
and may be used by the receiver to detect packet loss and to restore
packet sequence. The initial value of the sequence number is random
(unpredictable) to make known-plaintext attacks on encryption more
difficult, even if the source itself does not encrypt, because the
packets may flow through a translator that does. Techniques for
choosing unpredictable numbers are discussed in [7].

timestamp: 32 bits
The timestamp reflects the sampling instant of the first octet in the
RTP data packet. The sampling instant must be derived from a clock
that increments monotonically and linearly in time to allow
synchronization and jitter calculations (see Section 6.3.1). The
resolution of the clock must be sufficient for the desired
synchronization accuracy and for measuring packet arrival jitter (one
tick per video frame is typically not sufficient). The clock frequency
is dependent on the format of data carried as payload and is specified
statically in the profile or payload format specification that defines
the format, or may be specified dynamically for payload formats
defined through non-RTP means. If RTP packets are generated
periodically, the nominal sampling instant as determined from the
sampling clock is to be used, not a reading of the system clock. As an
example, for fixed-rate audio the timestamp clock would likely
increment by one for each sampling period. If an audio application
reads blocks covering 160 sampling periods from the input device, the
timestamp would be increased by 160 for each such block, regardless of
whether the block is transmitted in a packet or dropped as silent.
The initial value of the timestamp is random, as for the sequence
number. Several consecutive RTP packets may have equal timestamps if
they are (logically) generated at once, e.g., belong to the same video
frame. Consecutive RTP packets may contain timestamps that are not
monotonic if the data is not transmitted in the order it was sampled,
as in the case of MPEG interpolated video frames. (The sequence
numbers of the packets as transmitted will still be monotonic.)

SSRC: 32 bits
The SSRC field identifies the synchronization source. This identifier
is chosen randomly, with the intent that no two synchronization
sources within the same RTP session will have the same SSRC
identifier. An example algorithm for generating a random identifier is
presented in Appendix A.6. Although the probability of multiple
sources choosing the same identifier is low, all RTP implementations
must be prepared to detect and resolve collisions. Section 8 describes
the probability of collision along with a mechanism for resolving
collisions and detecting RTP-level forwarding loops based on the
uniqueness of the SSRC identifier. If a source changes its source
transport address, it must also choose a new SSRC identifier to avoid
being interpreted as a looped source.

CSRC list: 0 to 15 items, 32 bits each
The CSRC list identifies the contributing sources for the payload
contained in this packet. The number of identifiers is given by the CC
field. If there are more than 15 contributing sources, only 15 may be
identified. CSRC identifiers are inserted by mixers, using the SSRC
identifiers of contributing sources. For example, for audio packets
the SSRC identifiers of all sources that were mixed together to create
a packet are listed, allowing correct talker indication at the
receiver.

*/

#endif // _RTP_PACKET_H_
