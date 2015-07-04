#include <pebble.h>
#include <math.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_graphics_layer;
static char bin_buffer[12];
#define IC_Colour ((uint8_t)0b11000001)
#define Light_Colour ((uint8_t)0b11101001)
#define Pin_Colour ((uint8_t)0b11010101)
#define Background_Colour ((uint8_t)0b11000111)
#define Text_Colour ((uint8_t)0b11111111)
  
static void dec_to_bin(char buffer[], int num){
  //index one for AM/PM
  if (num >=1200)
    buffer[0] = '1';
  //reduce to 12h time
  num %= 1200;
  for (int i =11; i>= 1; i--){
    if (num%2)
      buffer[i] = '1';
    else
      buffer[i] = '0';
    num >>= 1;  
  }
}

static int int_from_string(char buffer[]) {
  int num = 0;
  for (int i = 2; i<4; i++){
    buffer[i] = buffer[i+1];
  }
  for (int i = 0; i<4; i++){
      int order = 1;
      for (int j = 3-i; j> 0; j--)
        order *= 10;
      num += (buffer[i]-'0')*order;
  }
  return num;
}

static void graphics_update_proc(Layer *this_layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, (GColor)IC_Colour);
  graphics_fill_rect(ctx, 
					(GRect){.origin = (GPoint){.x = 144/2-20,.y = 168/2-55},
								.size = (GSize){.w = 40,.h = 110}
						    },
					2,
					GCornersAll);
  
	for (int i = 0; i<12; i++)	{
    int x_ = 144/2-40+60*(i/6);
    int y_ = 168/2-55+i%6*20;
    graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
		graphics_fill_rect(ctx, 
                       (GRect){.origin = (GPoint){.x = x_,.y = y_},
								              .size = (GSize){.w = 20,.h = 10}
						                  },
					              1,
					              GCornersAll);
    if (bin_buffer[i] == '1')
    {
      graphics_context_set_fill_color(ctx, (GColor)Light_Colour);
      graphics_fill_circle(ctx, 
                           (GPoint){.x = x_ + 10,.y = y_+5}
                           , (uint16_t) 3);
    }
	}
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  
  //convert timestamp into decimal value
  int num = int_from_string(buffer);
  
  //turn decimal into binary
  strcpy(bin_buffer, "000000000000");
  dec_to_bin(bin_buffer, num);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, bin_buffer);
}

static void main_window_load(Window *window) {
  // Graphics layer
  s_graphics_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(s_graphics_layer, graphics_update_proc);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 0, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, (GColor)Text_Colour);

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  update_time();

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), s_graphics_layer);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    layer_destroy(s_graphics_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_background_color(s_main_window, (GColor)Background_Colour);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  update_time();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  
}