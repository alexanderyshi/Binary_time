#include <pebble.h>
// #include <cstdlib.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_graphics_layer;
static int num_month = 0;
static int num_day = 0;
static int num_hour = 0;
static int num_min = 0;
static int num_second = 0;
static int debug_hide_time = -1;
//TODO: make this into a .js configured option
static char show_debug_time = 1; /* flag to allow debug time to show (see DESIGN:)*/
static char show_weather_ic = 1;
//callback constants
#define KEY_SHOW_DEBUG_TIME 0
#define KEY_SHOW_WEATHER 1
#define KEY_TEMPERATURE 2
#define IC_Colour ((uint8_t)0b11000001)
#define Text_Colour ((uint8_t)0b11111111)
#define MMDD_Light_Colour  ((uint8_t)0b11001101)
#define Second_Light_Colour ((uint8_t)0b11010110)
#define Min_Light_Colour ((uint8_t)0b11110010)
#define Hour_Light_Colour ((uint8_t)0b11100011)
#define Day_Light_Colour ((uint8_t)0b11000011)
#define Month_Light_Colour ((uint8_t)0b11001001)
#define Pin_Colour ((uint8_t)0b11101010)
#define Night_Colour ((uint8_t)0b11000110)
#define Dawn_Colour ((uint8_t)0b11100110)
#define Day_Colour ((uint8_t)0b11001011)
#define Noon_Colour ((uint8_t)0b11101111)
#define SCREEN_HEIGHT 168
#define SCREEN_WIDTH 144
#define IC_WIDTH 40
#define IC_HEIGHT 110
#define IC_CORNER_RADIUS 2
#define PIN_CORNER_RADIUS 2
#define PIN_OFFSET 20
#define PIN_WIDTH 20
#define PIN_HEIGHT 10
#define PIN_SCALE (3.0/4)
  
  

//TODO: 
//create config accessor .js file
  //show debug time y/n
  //static silverscreen style background or time of day colour
  //include weather on shake
//daytime colour transition based on time of year from web sunrise/sunset time
	//load an array with time offset values on setup + every time midnight hour is reached 

//DESIGN:
  //keep debug time as an easy to read feature for when the watch is prone to shake, i.e. running
  

	//function takes 2-char string and converts into an int
static int int_from_string(char buffer[]) {
  return 10*(buffer[0]-'0') + (buffer[1]-'0');
}

//on shaking, the debug time will be shown for 5 seconds
static void tap_handler(AccelAxisType axis, int32_t direction) {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  static char second_string[] = "00";
  strftime(second_string, sizeof("00"), "%S", tick_time);
  debug_hide_time = (int_from_string(second_string)+5)%60;
  if (show_debug_time){
    //show debug time
    layer_set_hidden((Layer*)s_time_layer, false);
  }
}

static void graphics_update_proc(Layer *this_layer, GContext *ctx) {
  //TODO: find std GNU C lib and add to src
  //http://www.gnu.org/software/libc/download.html
  
  
  //update background colour
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
  
  
  //Draw IC body
  graphics_context_set_fill_color(ctx, (GColor)IC_Colour);
  graphics_fill_rect(ctx, 
					(GRect){.origin = (GPoint){.x = SCREEN_WIDTH/2-IC_WIDTH/2,.y = SCREEN_HEIGHT/2-IC_HEIGHT/2},
								.size = (GSize){.w = IC_WIDTH,.h = IC_HEIGHT}
						    },
					IC_CORNER_RADIUS,
					GCornersAll);
  	
  //draw seconds IC body horizontally on the bottom
  graphics_context_set_fill_color(ctx, (GColor)IC_Colour);
  //rectangle gets drawn past the bottom limit of the screen to prevent the bottom from rounding
  graphics_fill_rect(ctx, 
                     (GRect){.origin = (GPoint){.x = SCREEN_WIDTH/2-IC_HEIGHT/2,.y = SCREEN_HEIGHT-IC_WIDTH/8},
                       .size = (GSize){.w = IC_HEIGHT,.h = IC_WIDTH/8+IC_CORNER_RADIUS}
                            },
                     IC_CORNER_RADIUS,
                     GCornersAll);
  
  int right_pins = debug_hide_time > 0 ? num_day : num_min;
  int left_pins = debug_hide_time > 0 ? num_month : num_hour;
  //Draw IC pins and binary lights
	for (int i = 0; i<6; i++)	{
    int x_ = SCREEN_WIDTH/2-IC_WIDTH/2-PIN_WIDTH;
    int y_ = SCREEN_HEIGHT/2-IC_HEIGHT/2+i*PIN_OFFSET;
    
    		//draw hour/month pins
		if (i >0){
			if (left_pins >> (5-i) & 1){
        //TODO: figure out how to make this into the ternary:
        //      graphics_context_set_fill_color(ctx, (GColor)(debug_hide_time == -1 ? Hour_Light_Colour:Month_Light_Colour));
        if (debug_hide_time == -1)
        {
          graphics_context_set_fill_color(ctx, (GColor)Hour_Light_Colour);  
        }
        else{
          graphics_context_set_fill_color(ctx, (GColor)Month_Light_Colour);  
        }
        }
      else{
        graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);  
      }
      graphics_fill_rect(ctx, 
                         (GRect){.origin = (GPoint){.x = x_,.y = y_},
                           .size = (GSize){.w = PIN_WIDTH,.h = PIN_HEIGHT}
                                },
                         PIN_CORNER_RADIUS,
                         GCornersAll);
			
		}
		else{
		  //draw MMDD pin
			if (debug_hide_time > 0){
        graphics_context_set_fill_color(ctx, (GColor)MMDD_Light_Colour);
      }else{
        graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
      }
      graphics_fill_rect(ctx, 
                         (GRect){.origin = (GPoint){.x = x_,.y = y_},
                           .size = (GSize){.w = PIN_WIDTH,.h = PIN_HEIGHT}
                                },
                         1,
                         GCornersAll);
		}
		//draw minute/day pins
		if (right_pins >> (5-i) & 1)
		{
      //TODO: figure out how to make this into the ternary:
      // 		  graphics_context_set_fill_color(ctx, (GColor)(debug_hide_time == -1 ? Min_Light_Colour:Day_Light_Colour));
      if (debug_hide_time == -1)
      {
        graphics_context_set_fill_color(ctx, (GColor)Min_Light_Colour);  
      }
      else{
        graphics_context_set_fill_color(ctx, (GColor)Day_Light_Colour);  
      }
		  
		}else{
      graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
    }
		graphics_fill_rect(ctx, 
							(GRect){.origin = (GPoint){.x = x_ + IC_WIDTH + PIN_WIDTH,.y = y_},
									  .size = (GSize){.w = PIN_WIDTH,.h = PIN_HEIGHT}
									},
						  PIN_CORNER_RADIUS,
						  GCornersAll);
  //draw seconds pins
  graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);

    x_ = SCREEN_WIDTH/2 - IC_HEIGHT/2 + PIN_OFFSET*i;
    y_ = SCREEN_HEIGHT-IC_WIDTH/8-PIN_WIDTH*PIN_SCALE;

    //draw second pins
    if (num_second >> (5-i) & 1)
    {
      graphics_context_set_fill_color(ctx, (GColor)Second_Light_Colour);

    }else{
      graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
    }
    graphics_fill_rect(ctx, 
                       (GRect){.origin = (GPoint){.x = x_,.y = y_},
                         .size = (GSize){.w = PIN_HEIGHT,.h = PIN_WIDTH*PIN_SCALE}
                              },
                       PIN_CORNER_RADIUS,
                       GCornersAll);

  
    
	}// end for loop

  
	//TODO: if on debug time, show month/day
	//! if the watch is updated to never show debug time, this criteria will need to be replaced by a flag
	if (!layer_get_hidden((Layer*)s_time_layer)){
	}
      
  
	
}//end graphics_update_proc

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

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
  
  if(show_debug_time){
    //hide the debug time string if it has been visible for 5 seconds
    if (num_second>debug_hide_time && debug_hide_time >0)
    {
      debug_hide_time = -1;
      layer_set_hidden((Layer*)s_time_layer, true);
    }
  }
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, debug_string);
}

static void main_window_load(Window *window) {
  // Graphics layer
  s_graphics_layer = layer_create(GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
  layer_set_update_proc(s_graphics_layer, graphics_update_proc);
  
  //TODO: investigate how the GRect exactly defines this text layer boundary
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 80, 144, 88));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, (GColor)Text_Colour);

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  update_time();

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), s_graphics_layer);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    // Destroy Layers
    text_layer_destroy(s_time_layer);
    layer_destroy(s_graphics_layer);
    s_time_layer = NULL;
    s_graphics_layer = NULL;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_SHOW_DEBUG_TIME:
      show_debug_time = t->value->uint32;  
      break;
    case KEY_SHOW_WEATHER:
	  show_weather_ic = t->value->uint32;
      break;
    case KEY_TEMPERATURE:

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
  window_set_background_color(s_main_window, (GColor)Noon_Colour);
  
  // activate  tap handler
  accel_tap_service_subscribe(tap_handler);
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  update_time();
  
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
