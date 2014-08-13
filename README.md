kwfsnpp
=======

A gateway from SNPP clients to the APRS packet network

The Simple Network Paging Protocol is a text-based protocol
originally designed for gatewaying messages from the Internet
to commercial pager networks. This server implements SNPP as a
means to send one-way messages to messaging-capable APRS clients.

For more information about SNPP, see RFC1861.
For more information about APRS, good luck.

Client Examples:
	$ telnet snpp.thelifeofkenneth.com 444
<--	220 SNPP Gateway Ready
-->	PAGE W6KWF-5
<--	250 APRS ID Accepted
-->	MESS Call Sean on W6BHZ/R
<--	250 Message OK
-->	SEND
<--	250 Message Sent
-->	QUIT
<--	221 Goodbye!

