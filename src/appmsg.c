#include <pebble.h>

Window *window;
TextLayer *text_layer;
uint8_t sample_freq = ACCEL_SAMPLING_25HZ;
uint16_t acc_count=0;
uint16_t ack_count=0;
uint16_t fail_count=0;
int16_t *acc_data;
time_t   acc_time;
uint8_t num_samples = 25; 
static DictionaryIterator dict_iter, *iter = &dict_iter;
GFont *font_count;


void request_send_acc(void) {
     	app_message_outbox_begin(&iter);
        Tuplet count_val = TupletInteger(43, acc_count);
        dict_write_tuplet(iter, &count_val);
        Tuplet time_val = TupletInteger(44, acc_time);
        dict_write_tuplet(iter, &time_val);
        Tuplet acc_val = TupletBytes(45, (uint8_t *)acc_data, num_samples*6);
        dict_write_tuplet(iter, &acc_val);
        app_message_outbox_send();
}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
    // Need to be static because they're used by the system later.
    static char count_text[] = "                                                  ";
	
	snprintf(count_text,sizeof(count_text) ," sent: %03d \n ack:  %03d \n faild: %03d", acc_count,ack_count,fail_count);
    text_layer_set_text(text_layer, count_text);

}
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send(%3d)! error: 0x%02X ",++fail_count,reason);
}
static void out_received_handler(DictionaryIterator *iterator, void *context) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "App Message sent");
	ack_count++;
	
}
void accel_data_handler(AccelData *data, uint32_t num_samples) {

	AccelData *d = data; int16_t cnt=0;
	for (uint8_t i = 0; i < num_samples; i++, d++) {
		acc_data[cnt++]= d->x;
		acc_data[cnt++]= d->y;
		acc_data[cnt++]= d->z;
	}
    acc_time=time(NULL);
	request_send_acc();
	acc_count++;
}
void handle_init(void) {
    Layer *window_layer;
    TimeUnits units_changed = SECOND_UNIT;
	window = window_create();
    window_layer = window_get_root_layer(window);
	font_count = fonts_get_system_font(FONT_KEY_GOTHIC_28);

	
	text_layer = text_layer_create(GRect(0, 20, 144, 100));
    text_layer_set_font(text_layer, font_count);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));

    window_stack_push(window, true /* Animated */);
    tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
	accel_service_set_sampling_rate(sample_freq);
	accel_data_service_subscribe(25, &accel_data_handler);
	acc_data= malloc(num_samples * 6);
	
	app_message_register_outbox_failed(out_failed_handler);
	app_message_register_outbox_sent(out_received_handler);
    app_message_open(64, 200);
}

void handle_deinit(void) {
    tick_timer_service_unsubscribe();
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
