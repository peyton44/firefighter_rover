/*
 * json_parser.h
 *
 *  Created on: Oct 15, 2019
 *      Author: aalmz
 */

#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_
#include "uart_term.h"
#include "client_cbs.h"

//#include <string.h>


void parse_string(const char *payload, size_t payload_len);
void create_gen_status_json(char *json, char* rover_dir ,int speed);
void  create_stats_json(char *json, int pub_attempts, int pub_success);


#endif /* JSON_PARSER_H_ */
