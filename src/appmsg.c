#include <pebble.h>

uint8_t sample_freq = ACCEL_SAMPLING_50HZ;
Window *window;
TextLayer *text_layer;
uint16_t sample_count=0;
uint16_t acc_count=0;
uint16_t ack_count=0;
uint16_t fail_count=0;
int16_t *acc_data;
time_t   acc_time;
uint8_t num_samples = 25; 
static DictionaryIterator dict_iter, *iter = &dict_iter;
GFont *font_count;
char *xyz_str = "X,Y,Z:                      ";
bool waiting_data = false;
bool msg_run = false;

#define timer_interval 100
AppTimer *timer;

#define KEY_Count	43
#define KEY_Time	44
#define KEY_Data	45
#define KEY_XYZ		46

void request_send_acc(void) {
	

	snprintf(xyz_str,22 ,"X,Y,Z: %d,%d,%d",acc_data[0],acc_data[1],acc_data[2] );
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "%s",xyz_str);
	
	app_message_outbox_begin(&iter);
	Tuplet xyzstr_val = TupletCString(KEY_XYZ, xyz_str);
	dict_write_tuplet(iter, &xyzstr_val);
	Tuplet count_val = TupletInteger(KEY_Count, acc_count);
	dict_write_tuplet(iter, &count_val);
	Tuplet time_val = TupletInteger(KEY_Time, acc_time);
	dict_write_tuplet(iter, &time_val);
	Tuplet acc_val = TupletBytes(KEY_Data, (uint8_t *)acc_data, num_samples*6);
	dict_write_tuplet(iter, &acc_val);
	app_message_outbox_send();
	waiting_data = false;
	msg_run = true;
	acc_count++;
}

void timer_callback (void *data) {
	
	if ((waiting_data) && (msg_run==false))
		request_send_acc(); 
	timer = app_timer_register(timer_interval, timer_callback, NULL);
}
void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
	// Need to be static because they're used by the system later.
	static char count_text[] = "                                                                    ";

	snprintf(count_text,sizeof(count_text) ,"sample:%03d \n   sent:  %03d \n   ack:   %03d \n   faild:  %03d", 
		sample_count, acc_count, ack_count, fail_count);
	text_layer_set_text(text_layer, count_text);
	if (acc_count %100==0)
		APP_LOG(APP_LOG_LEVEL_INFO, "sample:%03d sent: %03d  ack: %03d  faild: %03d", 
			sample_count, acc_count, ack_count, fail_count);

}
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send(%3d)! error: 0x%02X ",++fail_count,reason);
	msg_run = false;

}
static void out_received_handler(DictionaryIterator *iterator, void *context) {
	//APP_LOG(APP_LOG_LEVEL_INFO, "App Message sent");
	ack_count++;
	msg_run = false;

	
}
void accel_data_handler(AccelData *data, uint32_t num_samples) {

	AccelData *d = data; int16_t cnt=0;
	for (uint8_t i = 0; i < num_samples; i++, d++) {
		acc_data[cnt++]= d->x;
		acc_data[cnt++]= d->y;
		acc_data[cnt++]= d->z;
	}
	acc_time=time(NULL);
	waiting_data = true;

	sample_count++;
}
void handle_init(void) {
	Layer *window_layer;
	TimeUnits units_changed = SECOND_UNIT;
	window = window_create();
	window_layer = window_get_root_layer(window);
	font_count = fonts_get_system_font(FONT_KEY_GOTHIC_28);

	
	text_layer = text_layer_create(GRect(0, 5, 144, 140));
	text_layer_set_font(text_layer, font_count);
	layer_add_child(window_layer, text_layer_get_layer(text_layer));

	window_stack_push(window, true /* Animated */);
	tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
	//accel_service_set_sampling_rate(sample_freq); //This is the logicl place
	accel_data_service_subscribe(25, &accel_data_handler);
	accel_service_set_sampling_rate(sample_freq); //This is the place that works
	acc_data= malloc(num_samples * 6);
	
	app_message_register_outbox_failed(out_failed_handler);
	app_message_register_outbox_sent(out_received_handler);
	app_message_open(64, 600);
	timer = app_timer_register(timer_interval, timer_callback, NULL);
	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
	app_message_deregister_callbacks();
	free(acc_data);
	text_layer_destroy(text_layer);
	window_destroy(window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
