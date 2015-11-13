#ifndef FTP_H
#define FTP_H

#include "session.h"
#include <errno.h>

int ftp_send_feats(session* session, const char** feats);
int ftp_send_status(session* session, int status, const char* arg);

int ftp_read_cmd(session* session, char* cmd);

int open_data_con(psession session);
int close_data_con(psession session);

int send_record(psession session, const char* record, int record_length);


//int ftp_write_data_header(session* session,
//int ftp_write_data_block(session* session, const char* data, int data_len);

#define MAX_CMD_LEN 255

#define COMMAND_OK              200
#define NOT_IMPL_SUPERFLUOUS    202
#define PATHNAME_CREATED        257
#define SYNTAX_ERROR            500
#define SYNTAX_ERROR_ARG        501
#define FILE_ANAVAILABLE        550

#define ABOUT_TO_OPEN           150
#define ALREADY_OPEN_SENDING    125

#define CANT_OPEN               425
#define CON_CLOSED_XFER_ABORTED 426

#define FILE_ACTION_FILE_ANAVAILABLE 450
#define FILE_ACTION_ERROR       450
#define LOCAL_ERROR             451

#define ACTION_OK_CLOSING       226
#define ENTERING_PASSIVE        227
#define ACTION_OK               250

#define EXT_SUPPORT             211
#define FILE_STAT               213
#define SYSTEM_TYPE             215
#define SERVICE_READY		220
#define CONTROL_CLOSE           221
#define FTP_GREETING 		230
#define USER_LOGGED_IN          200 
#define SERVICE_NOT_READY	421

#define SYNTAX_ERROR		500
#define SYNTAX_ERROR_ARG	501

#define BAD_SEQUENCE		503

#define NOT_IMPLEMENTED		502
#define NOT_IMPLEMENTED_ARG     504


#define IAC 255

#endif

