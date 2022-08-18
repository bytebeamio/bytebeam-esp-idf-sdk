#include "user_actions.h"

extern device_config test_device_config;
extern char* ota_action_id;

int handle_ota(bytebeam_client *bb_obj, char *args, char *action_id);

struct action_functions_map { const char *name; int (*func)(bytebeam_client *bb_obj, char *args, char *action_id); };

void publish_action_status(device_config device_cfg,char* action_id, int percentage, esp_mqtt_client_handle_t client, char* status, char* error_message);

extern struct action_functions_map action_funcs[];
