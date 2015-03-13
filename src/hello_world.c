#include <pebble.h>
#include <math.h>
  
Window *window;
TextLayer *stretch_label_text_layer;
TextLayer *pause_label_text_layer;
TextLayer *main_label_text_layer;

#define INT_DEFAULT_STRETCH_TIME 30;
#define INT_DEFAULT_PAUSE_TIME 5;
#define INT_INCREMENT_STRETCH 15;
#define INT_INCREMENT_PAUSE 1;

int stretch_time = INT_DEFAULT_STRETCH_TIME;
int stretch_sec = INT_DEFAULT_STRETCH_TIME;
int pause_time = INT_DEFAULT_PAUSE_TIME;
int pause_sec = INT_DEFAULT_PAUSE_TIME;

int timer_delta = 1000;
int long_press_delay = 3000;
AppTimer *stretch_timer;
AppTimer *pause_timer;

char stretch_label[64];
char pause_label[64];
char stretch_timer_label[64];
char pause_timer_label[64];
char main_label[3];

GColor foreground;
GColor background;

bool running;

//*** Interface

void register_stretch_timer(void);
void register_pause_timer(void);
static void send_debug(char message[]);
void stretch_timer_callback(void *data);
void handle_init(void);
void handle_deinit(void);
static void window_load(Window *window);
static void window_unload(Window *window);
void config_provider(Window *window);
void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
void create_labels(void);
void start(void);
void stop(void);
void update_main_label(int secs);
void long_click_reset_stretch_sec(ClickRecognizerRef recognizer, void *context);
void long_click_reset_pause_sec(ClickRecognizerRef recognizer, void *context);
void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
void refresh_stretch_time_label(void);
void refresh_pause_time_label(void);
//*** Implementation
  
static void send_debug(char message[]) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, message);
}

void pause_timer_callback(void *data) {
  if (pause_sec > 0) {
    update_main_label(pause_sec);
    pause_sec--;
    if (pause_sec < 3) {
      vibes_short_pulse();
    }
    register_pause_timer();
  } else {
    update_main_label(pause_sec);
    vibes_long_pulse();
    pause_sec = pause_time;
    register_stretch_timer();
  }
}

void stretch_timer_callback(void *data) {
  if (stretch_sec > 0) {
    update_main_label(stretch_sec);
    stretch_sec--;
    register_stretch_timer();
  } else {
    update_main_label(stretch_sec);
    stretch_sec = stretch_time;
    vibes_long_pulse();
    register_pause_timer();
  }
}

void register_stretch_timer(void) {
  stretch_timer = app_timer_register(timer_delta, (AppTimerCallback) stretch_timer_callback, NULL);
}

void register_pause_timer(void) {
  pause_timer = app_timer_register(timer_delta, (AppTimerCallback) pause_timer_callback, NULL);
}

void create_labels(void) {
  void create_label(TextLayer *layer) {
    text_layer_set_text_alignment(layer, GTextAlignmentCenter);
    text_layer_set_background_color(layer, background);
    text_layer_set_text_color(layer, foreground);
    text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  }
  stretch_label_text_layer = text_layer_create(GRect(0, 0, 144, 16));
  create_label(stretch_label_text_layer);
  
  pause_label_text_layer = text_layer_create(GRect(0, 136, 144, 16));
  create_label(pause_label_text_layer);
  
  main_label_text_layer = text_layer_create(GRect(0, 16, 144, 120));
  text_layer_set_text_alignment(main_label_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(main_label_text_layer, background);
  text_layer_set_text_color(main_label_text_layer, foreground);
  text_layer_set_font(main_label_text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text(main_label_text_layer, "0");
  
}

void update_main_label(int secs) {
  snprintf(main_label, 3, "%d", secs);
  text_layer_set_text(main_label_text_layer, main_label);
}

void handle_init(void) {
  foreground = GColorWhite;
  background = GColorBlack;
  running = false;
  
	// Create a window and text layer
	window = window_create();
  window_set_window_handlers(window, (WindowHandlers) 
  {
    .load = window_load,
    .unload = window_unload
  });
  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);
  window_stack_push(window, true);
}

static void window_load(Window *window) {
  create_labels();

  refresh_stretch_time_label();
  refresh_pause_time_label();

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(stretch_label_text_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(pause_label_text_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(main_label_text_layer));
}

static void window_unload(Window *window) {
  // Destroy the text layer
	text_layer_destroy(stretch_label_text_layer);
	text_layer_destroy(pause_label_text_layer);
}

void handle_deinit(void) {
	
	// Destroy the window
	window_destroy(window);
}

void start(void) {
  running = true;
  register_stretch_timer();
}

void stop(void) {
  running = false;
  app_timer_cancel(stretch_timer);
  app_timer_cancel(pause_timer);
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (running) {
    stop();
  } else {
    start();
  }
}

void long_click_reset_stretch_sec(ClickRecognizerRef recognizer, void *context) {
  stretch_time = INT_DEFAULT_STRETCH_TIME;
  if (stretch_sec > stretch_time) { stretch_sec = stretch_time; }
  refresh_stretch_time_label();
}

void long_click_reset_pause_sec(ClickRecognizerRef recognizer, void *context) {
  pause_time = INT_DEFAULT_PAUSE_TIME;
  if (pause_sec > pause_time) { pause_sec = pause_time; }
  refresh_pause_time_label();
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  stretch_time += INT_INCREMENT_STRETCH;
  stretch_sec += INT_INCREMENT_STRETCH;
  refresh_stretch_time_label();
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  pause_time += INT_INCREMENT_PAUSE;
  pause_sec += INT_INCREMENT_PAUSE;
  refresh_pause_time_label();
}

void refresh_stretch_time_label(void) {
  snprintf(stretch_label, 64, "Stretch Time: %d", stretch_time);
  text_layer_set_text(stretch_label_text_layer, stretch_label);
}

void refresh_pause_time_label(void) {
  snprintf(pause_label, 64, "Pause Time: %d", pause_time);
  text_layer_set_text(pause_label_text_layer, pause_label);
}

void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  window_long_click_subscribe(BUTTON_ID_UP, long_press_delay, long_click_reset_stretch_sec, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, long_press_delay, long_click_reset_pause_sec, NULL);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
