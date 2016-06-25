#include <pebble.h>

//static members
static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_graphics_layer;
static Layer *s_seconds_layer;
static Layer *s_battery_layer;
static Layer *s_weather_layer;

static int num_month = 0;
static int num_day = 0;
static int num_hour = 0;
static int num_min = 0;
static int num_second = 0;
static int battery_level = 0;
static int temperature = 0;

// flags
static int debug_hide_time = -1;
static int hour_changed = 0;
static int show_debug_time = 1; 
static int show_weather_ic = 0;
static int show_seconds_ic = 0;
static int show_battery_ic = 0;
static int show_fancy_background = 0;
//config callback constants
#define KEY_SHOW_DEBUG_TIME           0
#define KEY_SHOW_WEATHER              1
#define KEY_SHOW_SECONDS              2
#define KEY_SHOW_BATTERY              3
#define KEY_SHOW_FANCY_BACKGROUND     4
//colours
#define IC_Colour               ((uint8_t)0b11000001)
#define Text_Colour             ((uint8_t)0b11111111)

#define Pin_Colour              ((uint8_t)0b11101010)
#define MMDD_Pin_Colour       ((uint8_t)0b11001101)
#define Second_Pin_Colour     ((uint8_t)0b11010110)
#define Min_Pin_Colour        ((uint8_t)0b11110010)
#define Hour_Pin_Colour       ((uint8_t)0b11100011)
#define Day_Pin_Colour        ((uint8_t)0b11000011)
#define Month_Pin_Colour      ((uint8_t)0b11001001)

#define Night_Colour            ((uint8_t)0b11000110)
#define Dawn_Colour             ((uint8_t)0b11100110)
#define Day_Colour              ((uint8_t)0b11001011)
#define Noon_Colour             ((uint8_t)0b11101111)
//dimensions
#define SCREEN_HEIGHT           168
#define SCREEN_WIDTH            144
#define IC_WIDTH                40
#define IC_HEIGHT               110
#define IC_CORNER_RADIUS        2
#define PIN_CORNER_RADIUS       2
#define PIN_OFFSET              20
#define PIN_WIDTH               20
#define PIN_HEIGHT              10
//scaling factors
#define PIN_SCALE               (3.0/4) //used for seconds pins to shrink vertical dimension
  
  

//TODO: 
//hide seconds config should actually change the tick handler
  // !!! changing seconds tick handler behaviour will break debug time, changing of day values, etc. 

//use seperate layer and update proc for seconds IC (make bg clear)
  //keep the main time IC layer as the main layer that has a background color filled in
//display 10 values on 6 pins more efficiently
//hide battery config should actually change the battery service

//weather on shake
//daytime colour transition based on time of year from web sunrise/sunset time
  //load an array with time offset values on setup + every time midnight hour is reached 

//static silverscreen style background or time of day colour

//DESIGN:
  //keep debug time as an easy to read feature for when the watch is prone to shake, i.e. running
    //if it counts more than "x" shakes in "y" seconds, it will assume that I am running and enter a different type of mode

//WATCHFACES AND APPS
//make a cycling oriented watchface! 
//make a cute weightlifting app for tracking PRs, with activity tracking
  

  //function takes 2-char string and converts into an int
static int int_from_string(char buffer[]) {
  return 10*(buffer[0]-'0') + (buffer[1]-'0');
}

static void _draw_vertical_pins(Layer *this_layer, GContext *ctx, int pins, int hor_coord, int vert_coord, char orientation, uint8_t colour, double scale){
  //choose x position - by default draw pins above, if orientation is 'b' for below, draw below
  int x_;
  if (orientation == 'l') {
     x_ = hor_coord - IC_WIDTH/2-PIN_WIDTH*scale;
  }
  else if (orientation == 'r'){
     x_ = hor_coord + IC_WIDTH/2;
  }
  else{
    return;
  }
  //draw pins
  for (int i = 0; i<6; i++) {
    int y_ = vert_coord-IC_HEIGHT/2+i*PIN_OFFSET;
    if (pins >> (5-i) & 1)
    {
      graphics_context_set_fill_color(ctx, (GColor)colour);

    }else{
      graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
    }
    graphics_fill_rect(ctx, 
               (GRect){.origin = (GPoint){.x = x_,.y = y_},
               .size = (GSize){.w = PIN_WIDTH*scale,.h = PIN_HEIGHT}
                  },
               PIN_CORNER_RADIUS,
               GCornersAll);

  }// end for loop
}

static void _draw_horizontal_pins(Layer *this_layer, GContext *ctx, int pins, int hor_coord, int vert_coord, char orientation, uint8_t colour, double scale){
  //choose y position - by default draw pins above, if orientation is 'b' for below, draw below
  int y_;
  if (orientation == 'a') {
     y_ = vert_coord - IC_WIDTH/2-PIN_WIDTH*scale;
  }
  else if (orientation == 'b'){
     y_ = vert_coord + IC_WIDTH/2;
  }
  else{
    return;
  }

  //draw pins
  for (int i = 0; i<6; i++) {
    int x_ = hor_coord - IC_HEIGHT/2 + PIN_OFFSET*i;
    if (pins >> (5-i) & 1)
    {
      graphics_context_set_fill_color(ctx, (GColor)colour);

    }else{
      graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
    }
    graphics_fill_rect(ctx, 
               (GRect){.origin = (GPoint){.x = x_,.y = y_},
               .size = (GSize){.w = PIN_HEIGHT,.h = PIN_WIDTH*scale}
                  },
               PIN_CORNER_RADIUS,
               GCornersAll);

  }// end for loop
};

static void draw_seconds_IC(Layer *this_layer, GContext *ctx, int hor_coord, int vert_coord) {
  //rectangle gets drawn past the bottom limit of the screen to prevent the bottom from rounding
  graphics_context_set_fill_color(ctx, (GColor)IC_Colour);
  graphics_fill_rect(ctx, 
                     (GRect){.origin = (GPoint){.x = hor_coord-IC_HEIGHT/2,.y = vert_coord-IC_WIDTH/2},
                       .size = (GSize){.w = IC_HEIGHT,.h = IC_WIDTH}
                            },
                     IC_CORNER_RADIUS,
                     GCornersAll);
  //draw seconds pins
  _draw_horizontal_pins(this_layer, ctx, num_second, hor_coord, vert_coord, 'a', Second_Pin_Colour, PIN_SCALE);
}

static void draw_battery_IC(Layer *this_layer, GContext *ctx, int hor_coord, int vert_coord) {
  //rectangle gets drawn above the top of the screen to prevent the bottom from rounding
  graphics_context_set_fill_color(ctx, (GColor)IC_Colour);
  graphics_fill_rect(ctx, 
                     (GRect){.origin = (GPoint){.x = hor_coord-IC_HEIGHT/2,.y = vert_coord-IC_WIDTH/2},
                       .size = (GSize){.w = IC_HEIGHT,.h = IC_WIDTH}
                            },
                     IC_CORNER_RADIUS,
                     GCornersAll);
  //draw battery pins - 
  _draw_horizontal_pins(this_layer, ctx, battery_level/2, hor_coord, vert_coord, 'b', Second_Pin_Colour, PIN_SCALE);
}

static void draw_time_IC(Layer *this_layer, GContext *ctx, int hor_coord, int vert_coord){
    //draw IC 
    graphics_context_set_fill_color(ctx, (GColor)IC_Colour);
    graphics_fill_rect(ctx, 
          (GRect){.origin = (GPoint){.x = hor_coord-IC_WIDTH/2,.y = vert_coord-IC_HEIGHT/2},
                .size = (GSize){.w = IC_WIDTH,.h = IC_HEIGHT}
                },
          IC_CORNER_RADIUS,
          GCornersAll);
           
  //choose colours according to debug mode
  int right_pins = debug_hide_time > 0 ? num_day : num_min;
  uint8_t right_colour = debug_hide_time > 0 ? Day_Pin_Colour : Min_Pin_Colour;
  int left_pins = debug_hide_time > 0 ? num_month : num_hour;
  uint8_t left_colour = debug_hide_time > 0 ? Month_Pin_Colour : Hour_Pin_Colour;
  uint8_t top_left_colour = debug_hide_time > 0 ? MMDD_Pin_Colour : Second_Pin_Colour;

  _draw_vertical_pins(this_layer, ctx, right_pins, hor_coord, vert_coord, 'r', right_colour, 1);
  _draw_vertical_pins(this_layer, ctx, left_pins, hor_coord, vert_coord, 'l', left_colour, 1);

  graphics_context_set_fill_color(ctx, (GColor)top_left_colour);
  //fill in remaining 6th pin
  graphics_fill_rect(ctx, 
          (GRect){.origin = (GPoint){.x = hor_coord-IC_WIDTH/2-PIN_WIDTH,.y = vert_coord-IC_HEIGHT/2},
                .size = (GSize){.w = PIN_WIDTH,.h = PIN_HEIGHT}
                },
          IC_CORNER_RADIUS,
          GCornersAll);
}

static void update_background_color(Layer *this_layer, GContext *ctx) {
  //choose a background colour value once an hour in the tick handler, catching tick timer service subscription to updates in terms of minutes

    //TODO: update background colour
  //1300 treated as peak of day, sunrise/sunset is roughly 0600-2100 in summer
  
  //TODO: change time diff to account for minutes as well so that daylight hours can be more accurately tracked
  unsigned int time_diff = (unsigned int)(num_hour-13>=0 ? num_hour-13 : 13-num_hour);
  if (time_diff >= 8)
  {
    window_set_background_color(s_main_window, (GColor)Night_Colour);   
  }else if (time_diff >= 6)
  {
    window_set_background_color(s_main_window, (GColor)Dawn_Colour);   
  }else if (time_diff >= 2)
  {
    window_set_background_color(s_main_window, (GColor)Day_Colour);  
  }else{
    window_set_background_color(s_main_window, (GColor)Noon_Colour);  
  }

};

//on shaking, the debug time will be shown for 5 seconds if config is set
static void tap_handler(AccelAxisType axis, int32_t direction) {
    //set disable time to 5 seconds from now
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    static char second_string[] = "00";
    strftime(second_string, sizeof("00"), "%S", tick_time);
    debug_hide_time = (int_from_string(second_string)+5)%60;

    //show debug timelayer
  if (show_debug_time){
    layer_set_hidden((Layer*)s_time_layer, false);
  }
}

static void graphics_update_proc(Layer *this_layer, GContext *ctx) {
  //TODO: find std GNU C lib and add to src
  //http://www.gnu.org/software/libc/download.html

  if(hour_changed)
  {
    update_background_color(this_layer, ctx);  
  }
  draw_time_IC(this_layer, ctx, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
}//end graphics_update_proc

static void battery_update_proc(Layer *this_layer, GContext *ctx)
{
  if (show_battery_ic ==1 && debug_hide_time > 0)
  {
    draw_battery_IC(this_layer, ctx, SCREEN_WIDTH/2, 0-IC_WIDTH/4);
  }
}

static void seconds_update_proc(Layer *this_layer, GContext *ctx)
{
  int aux_layer_depth = IC_WIDTH/4+PIN_WIDTH*PIN_SCALE;
  if(show_seconds_ic == 1){
    draw_seconds_IC(this_layer, ctx, SCREEN_WIDTH/2, aux_layer_depth + IC_WIDTH/4);
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  int old_hour = num_hour;

  // Create a long-lived buffer
  static char hour_string[] = "00";
  static char min_string[] = "00";
  static char second_string[] = "00";
  static char debug_string[] = "00:00";
  // Write the current hours and minutes into the buffer
  strftime(hour_string, sizeof("00"), "%H", tick_time);
  strftime(min_string, sizeof("00"), "%M", tick_time);
  strftime(debug_string, sizeof("00:00"), "%H%M", tick_time);
  strftime(second_string, sizeof("00"), "%S", tick_time);
  
  //convert timestamp into decimal value
  num_hour = int_from_string(hour_string);
  num_min = int_from_string(min_string);
  num_second = int_from_string(second_string);
  
  //check to see if the hour changed
  if (num_hour != old_hour)
  {
    hour_changed = 1;
  }
  
  //update month and day at midnight
  //if all three are zero, then update month/day int
  if (!(num_hour || num_min || num_second)){
    static char month_string[] = "00";
    static char day_string[] = "00";
    strftime(month_string, sizeof("00"), "%m", tick_time);
    strftime(day_string, sizeof("00"), "%d", tick_time);
    num_month = int_from_string(month_string);
    num_day = int_from_string(day_string);
  }
  
    //hide the debug time string if it has been visible for 5 seconds
    if (num_second>debug_hide_time && debug_hide_time >0)
    {
      debug_hide_time = -1;
      layer_set_hidden((Layer*)s_time_layer, true);
    }
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, debug_string);
}

static void main_window_load(Window *window) {
  int aux_layer_depth = IC_WIDTH/4+PIN_WIDTH*PIN_SCALE;
  // Graphics layer
  s_graphics_layer = layer_create(GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
  layer_set_update_proc(s_graphics_layer, graphics_update_proc);
  
  s_battery_layer = layer_create(GRect(0,0, SCREEN_WIDTH, aux_layer_depth));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  s_seconds_layer = layer_create(GRect(0,SCREEN_HEIGHT - aux_layer_depth, SCREEN_WIDTH, aux_layer_depth));
  layer_set_update_proc(s_seconds_layer, seconds_update_proc);

  //TODO: investigate how the GRect exactly defines this text layer boundary
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 80, 144, 88));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, (GColor)Text_Colour);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  update_time();

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), s_graphics_layer);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  layer_add_child(window_get_root_layer(window), s_seconds_layer);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    // Destroy Layers
    text_layer_destroy(s_time_layer);
    layer_destroy(s_graphics_layer);
    layer_destroy(s_battery_layer);
    s_time_layer = NULL;
    s_graphics_layer = NULL;
    s_battery_layer = NULL;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void battery_callback(BatteryChargeState state) {
  battery_level = state.charge_percent*64/100;
  layer_mark_dirty(s_battery_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_SHOW_DEBUG_TIME:
        show_debug_time = t->value->int32;  
        break;
      case KEY_SHOW_WEATHER:
        show_weather_ic = t->value->int32;
        break;
      case KEY_SHOW_SECONDS:
        show_seconds_ic = t->value->int32;
        break;
      case KEY_SHOW_BATTERY:
        show_battery_ic = t->value->int32;
        break;
      case KEY_SHOW_FANCY_BACKGROUND:
        show_fancy_background = t->value->int32;
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }
    // Look for next item
    t = dict_read_next(iterator);

  }
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  hour_changed =1;
  // window_set_background_color(s_main_window, (GColor)Noon_Colour);
  
  // activate  tap handler
  accel_tap_service_subscribe(tap_handler);
  //subscribe to battery state handler service
  battery_state_service_subscribe(battery_callback);
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  //get initial values
  update_time();
  battery_callback(battery_state_service_peek());

  layer_set_hidden((Layer*)s_time_layer, true);
  
  //register for callbacks 
  app_message_register_inbox_received(inbox_received_callback);
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  
  //first time month and day
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  //update month and day 
  static char month_string[] = "00";
  static char day_string[] = "00";
  strftime(month_string, sizeof("00"), "%m", tick_time);
  strftime(day_string, sizeof("00"), "%d", tick_time);
  num_month = int_from_string(month_string);
  num_day = int_from_string(day_string);
}

static void deinit() {
    accel_tap_service_unsubscribe();
  }

int main(void) {
  init();
  app_event_loop();
  deinit();
  
}