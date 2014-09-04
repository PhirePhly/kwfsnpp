// snpp.h
// Kenneth Finnegan, 2014

#define SNPP_HELLO		"220 SNPP APRS Gateway Ready\r\n"
#define SNPP_GOODBYE	"221 Goodbye!\r\n"
#define SNPP_GATEDOWN	"421 Gateway Service Unavailable\r\n"

#define SNPP_ID_OK		"250 APRS ID Accepted\r\n"
#define SNPP_ID_BAD		"550 APRS ID Invalid\r\n"
#define SNPP_MESS_OK	"250 Message OK\r\n"
#define SNPP_MESS_BAD	"550 Message length/contents invalid\r\n"
#define SNPP_MAXENTRY	"552 Maximum Entries Exceeded\r\n"
#define SNPP_RESET_OK	"250 Reset OK\r\n"
#define SNPP_SENT		"250 Message(s) Sent\r\n"

#define SNPP_ERR_TIMEOUT	"421 Idle timeout; Goodbye!\r\n"
#define SNPP_ERR_TOOMANY	"421 Too Many Errors; Goodbye!\r\n"
#define SNPP_ERR_NOTIMPL	"500 Command not implemented\r\n"
#define SNPP_ERR_MISSING	"503 Error: Pager or Message Missing\r\n"

