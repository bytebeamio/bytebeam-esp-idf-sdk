#include "user_actions.h"
extern device_config test_device_config;
extern char* ota_action_id;

int handle_ota(bytebeam_client *bb_obj, char *args, char *action_id);

typedef struct  
{      
    const char *name; 
    int (*func)(bytebeam_client *bb_obj, char *args, char *action_id); 
}action_functions_map;

void initialize_action_handler_array(action_functions_map* action_handler_array);

void publish_action_status(device_config device_cfg,char* action_id, int percentage, bytebeam_client_handle_t client, char* status, char* error_message);

extern action_functions_map action_funcs[NUMBER_OF_ACTIONS];