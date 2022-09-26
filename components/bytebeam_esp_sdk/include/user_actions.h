
#define NUMBER_OF_ACTIONS 10

extern int toggle_led_cmd;

int toggle_led(bytebeam_client *bb_obj, char *args, char *action_id);
int test_func1(bytebeam_client *bb_obj, char *args, char *action_id);
int test_func2(bytebeam_client *bb_obj, char *args, char *action_id);
void create_new_action_handler(int (*func_ptr)(bytebeam_client*, char*, char*),char* func_name);
void create_action_handlers();
