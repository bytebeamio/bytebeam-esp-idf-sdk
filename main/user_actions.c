#include "bytebeam_sdk.h"
#include "user_actions.h"
#include "bytebeam_actions.h"

int blink_led_cmd=0;

int toggle_led(bytebeam_client *bb_obj, char *args, char *action_id)
{
    blink_led_cmd=1;
    publish_action_status(bb_obj->device_cfg,action_id,100,bb_obj->client,"Completed", "LED_Toggled");
    return 0;
}