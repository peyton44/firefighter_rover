/*
 * json_parser.c
 *
 *  Created on: Oct 15, 2019
 *      Author: aalmz
 */
#include "json_parser.h"
#define JSMN_STATIC
#include "jsmn.h"

#include <stdio.h>


static const char *STAT =
    "{\"publishAttempts\": \"%d\", \"publishSuccesses\": %d\"}";

static const char *GEN_STATUS =
    "{\"RoverDirection\": \"%s\", \"Speed\": %d}";

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

void parse_string(const char *payload, size_t payload_len)
{
    jsmn_parser parser;
    int parsed;
    jsmntok_t t[128];
    jsmn_init(&parser);
    parsed = jsmn_parse(&parser, payload, payload_len, t,
                     sizeof(t) / sizeof(t[0]));
    if (parsed < 0) {
        Report("Failed to parse JSON: %d\n", parsed);
       return;
     }

    /* Assume the top-level element is an object */
    if (parsed < 1 || t[0].type != JSMN_OBJECT) {
        Report("Object expected\n");
    }
    int i;
    /* Loop over all keys of the root object */
    for (i = 1; i < parsed; i++) {
      if (jsoneq(payload, &t[i], "ArmStatus") == 0) {
        /* We may use strndup() to fetch string value */
        Report("- ArmStatus: %.*s\n", t[i + 1].end - t[i + 1].start,
               payload + t[i + 1].start);
        i++;
      } else if (jsoneq(payload, &t[i], "RoverDirection") == 0) {
        /* We may additionally check if the value is either "true" or "false" */
          Report("- RoverDirection: %.*s\n", t[i + 1].end - t[i + 1].start,
               payload + t[i + 1].start);
        i++;
      } else if (jsoneq(payload, &t[i], "CameraTemp") == 0) {
        /* We may want to do strtol() here to get numeric value */
          Report("- CameraTemp: %.*s\n", t[i + 1].end - t[i + 1].start,
               payload + t[i + 1].start);
        i++;
      } else if (jsoneq(payload, &t[i], "HeaterTemp") == 0) {
        Report("- HeaterTemp: %.*s\n", t[i + 1].end - t[i + 1].start,
               payload + t[i + 1].start);
        i++;
      } else {
          Report("Unexpected key: %.*s\n", t[i].end - t[i].start,
               payload + t[i].start);
      }
    }

    Report("\n");
}


void create_stats_json(char* json, int pub_attempts, int pub_success)
{
    snprintf(json, 256, STAT, pub_attempts, pub_success);
}


void create_gen_status_json(char *json, char* rover_dir , int speed)
{

    snprintf(json, 256, GEN_STATUS, rover_dir, speed);
    //Report(json);


}


