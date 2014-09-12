kwfsnpp
=======

A gateway from SNPP clients to the APRS packet network

The Simple Network Paging Protocol is a text-based protocol
originally designed for gatewaying messages from the Internet
to commercial pager networks. This server implements SNPP as a
means to send one-way messages to messaging-capable APRS clients.

For more information about SNPP, see RFC1861.
For more information about APRS, good luck.

SNPP Verbs include:
 * PAGEr N0CALL-NN - Target device for next message
 * MESSage message text - Message contents; <67 characters
 * RESEt - Dump the current transaction and start over
 * SEND - Start sending all messages to all pagers
 * QUIT - Close connection
 * HELP - Print something helpful

Client Examples:

		$ telnet snpp.thelifeofkenneth.com 444
	<--	220 SNPP APRS Gateway Ready
	-->	PAGE W6KWF-5
	<--	250 APRS ID Accepted
	-->	MESS Call Sean on W6BHZ/R
	<--	250 Message OK
	-->	SEND
	<--	250 Message Sent
	-->	QUIT
	<--	221 Goodbye!

