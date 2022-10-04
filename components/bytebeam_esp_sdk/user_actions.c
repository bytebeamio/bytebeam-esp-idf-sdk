#include "bytebeam_sdk.h"
#include "user_actions.h"
#include "bytebeam_actions.h"

int toggle_led_cmd=0;

int toggle_led(bytebeam_client *bb_obj, char *args, char *action_id)
{
    toggle_led_cmd=1;
    publish_action_completed(bb_obj,action_id);
    return 0;
}

void create_new_action_handler(int (*func_ptr)(bytebeam_client*, char*, char*),char* func_name)
{
    static int function_handler_index=1;
    action_funcs[function_handler_index].func=func_ptr;
    action_funcs[function_handler_index].name=func_name;
    function_handler_index=function_handler_index+1;
}

void create_action_handlers()
{
    create_new_action_handler(toggle_led,"toggle_board_led");
}


