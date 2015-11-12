#ifndef SESSION_COMMANDS_H
#define SESSION_COMMANDS_H

#include "session.h"

#define CMD_DONE 0
#define CMD_DONE_QUIT 1
#define CMD_ERR 2
#define CMD_ERR_QUIT 3
#define CMD_NOT_IMPLEMENTED 4

typedef int (*command_handler)(session*, const char*);

command_handler command_parse(char* cmd, char** arg);

int command_abor(psession session, const char* arg);
int command_acct(psession session, const char* arg);
int command_allo(psession session, const char* arg);
int command_appe(psession session, const char* arg);
int command_cwd (psession session, const char* arg);
int command_pwd (psession session, const char* arg);
int command_dele(psession session, const char* arg);
int command_help(psession session, const char* arg);
int command_list(psession session, const char* arg);
int command_mode(psession session, const char* arg);
int command_nlst(psession session, const char* arg);
int command_noop(psession session, const char* arg);
int command_pass(psession session, const char* arg);
int command_pasv(psession session, const char* arg);
int command_port(psession session, const char* arg);
int command_quit(psession session, const char* arg);
int command_rein(psession session, const char* arg);
int command_rest(psession session, const char* arg);
int command_retr(psession session, const char* arg);
int command_rnfr(psession session, const char* arg);
int command_rnto(psession session, const char* arg);
int command_site(psession session, const char* arg);
int command_stat(psession session, const char* arg);
int command_stor(psession session, const char* arg);
int command_stru(psession session, const char* arg);
int command_syst(psession session, const char* arg);
int command_type(psession session, const char* arg);
int command_user(psession session, const char* arg);
int command_feat(psession session, const char* arg);
int command_opts(psession session, const char* arg);
int command_size(psession session, const char* arg);
int command_cdup(psession session, const char* arg);

int command_not_implemented(psession session, const char* arg);

#endif
