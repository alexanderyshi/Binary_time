#include <pebble.h>
// #include <cstdlib.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_graphics_layer;
static int num_hour = 0;
static int num_min = 0;
static int num_second = 0;
static int debug_hide_time = 0;
//TODO: make this into a .js configured option
static char show_debug_time = 1; /* flag to allow debug time to show */
#define IC_Colour ((uint8_t)0b11000001)
#define PM_Light_Colour  ((uint8_t)0b11001101)
#define Second_Light_Colour ((uint8_t)0b11001001)
#define Min_Light_Colour ((uint8_t)0b11110010)
#define Hour_Light_Colour ((uint8_t)0b11100011)
#define Pin_Colour ((uint8_t)0b11101010)
#define Text_Colour ((uint8_t)0b11111111)
#define Night_Colour ((uint8_t)0b11000110)
#define Dawn_Colour ((uint8_t)0b11100110)
#define Day_Colour ((uint8_t)0b11001011)
#define Noon_Colour ((uint8_t)0b11101111)
#define SCREEN_HEIGHT 168
#define SCREEN_WIDTH 144

//TODO: change all hard-coded operations in function calls into #define macros
//TODO: global binary time struct?
//TODO: daytime colour transition based on time of year, get month and day of year 
		//on setup (or every time midnight hour is reached?) and then load array full of time diff values

	//function takes 2-char string and converts into an int
static int int_from_string(char buffer[]) {
  return 10*(buffer[0]-'0') + (buffer[1]-'0');
}

static int is_PM(){
	return num_hour >= 12;
}

//on shaking, the debug time will be shown for 5 seconds
static void tap_handler(AccelAxisType axis, int32_t direction) {
  if (show_debug_time){
  //show debug time
  layer_set_hidden((Layer*)s_time_layer, false);
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  static char second_string[] = "00";
  strftime(second_string, sizeof("00"), "%S", tick_time);
  debug_hide_time = (int_from_string(second_string)+5)%60;
    
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
					(GRect){.origin = (GPoint){.x = 144/2-20,.y = 168/2-55},
								.size = (GSize){.w = 40,.h = 110}
						    },
					2,
					GCornersAll);
  
  //Draw IC pins and binary lights
	for (int i = 0; i<6; i++)	{
    int x_ = 144/2-40;
    int y_ = 168/2-55+i*20;
		graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);

		//draw minute pins
		if (num_min >> (5-i) & 1)
		{
		  graphics_context_set_fill_color(ctx, (GColor)Min_Light_Colour);
		  
		}else{
      graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
    }
		graphics_fill_rect(ctx, 
							(GRect){.origin = (GPoint){.x = x_ + 60,.y = y_},
									  .size = (GSize){.w = 20,.h = 10}
									},
						  1,
						  GCornersAll);
    
		//draw hour pins
		if (i >0){
			if (num_hour >> (5-i) & 1){
        graphics_context_set_fill_color(ctx, (GColor)Hour_Light_Colour);
      }
      else{
        graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);  
      }
      graphics_fill_rect(ctx, 
                         (GRect){.origin = (GPoint){.x = x_,.y = y_},
                           .size = (GSize){.w = 20,.h = 10}
                                },
                         1,
                         GCornersAll);
			
		}
		else{
		  //draw AM/PM pin
			if (is_PM()){
        graphics_context_set_fill_color(ctx, (GColor)PM_Light_Colour);
      }else{
        graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
      }
      graphics_fill_rect(ctx, 
                         (GRect){.origin = (GPoint){.x = x_,.y = y_},
                           .size = (GSize){.w = 20,.h = 10}
                                },
                         1,
                         GCornersAll);
		}
	}// end for loop
	
  //draw IC body horizontally on the top
  graphics_context_set_fill_color(ctx, (GColor)IC_Colour);
  //rectangle gets drawn past the bottom limit of the screen to prevent the bottom from rounding
  graphics_fill_rect(ctx, 
                     (GRect){.origin = (GPoint){.x = 144/2-55,.y = 168-5},
                       .size = (GSize){.w = 110,.h = 7}
                            },
                     2,
                     GCornersAll);

  //draw seconds pins
  graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);

  for (int i = 0; i<6; i++)	
  {
    int x_ = 17 + 20*i;
    int y_ = 148;

    //draw second pins
    if (num_second >> (5-i) & 1)
    {
      graphics_context_set_fill_color(ctx, (GColor)Second_Light_Colour);

    }else{
      graphics_context_set_fill_color(ctx, (GColor)Pin_Colour);
    }
    graphics_fill_rect(ctx, 
                       (GRect){.origin = (GPoint){.x = x_,.y = y_},
                         .size = (GSize){.w = 10,.h = 15}
                              },
                       1,
                       GCornersAll);

  }
  
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
  
  //hide the debug time string if it has been visible for 5 seconds

  if (int_from_string(second_string)>debug_hide_time && !layer_get_hidden((Layer*)s_time_layer))
  {
    layer_set_hidden((Layer*)s_time_layer, true);
  }
      
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, debug_string);
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
}

static void deinit() {
    accel_tap_service_unsubscribe();
  }

int main(void) {
  init();
  app_event_loop();
  deinit();
  
}
