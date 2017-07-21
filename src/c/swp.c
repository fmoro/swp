#include <pebble.h>

#ifndef MESSAGE_SOURCES
#define MESSAGE_SOURCES
#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1
#endif

#ifndef LOGGING_TAG
#define LOGGING_TAG 0xFEDE1983
#endif

static Window *s_window;

static StatusBarLayer *s_status_bar;

static ActionBarLayer *s_action_bar;
static GBitmap *s_up_bitmap, *s_down_bitmap, *s_check_bitmap;

static TextLayer *s_text_layer;

static void worker_message_handler(uint16_t type, AppWorkerMessage *message) {
  if (type == SOURCE_BACKGROUND) {
    if (message->data0 == 0x0000 && message->data1 == 0x0000 && message->data2 == 0x0000) {
      text_layer_set_text(s_text_layer, "Worker really stoped");
    } else if (message->data0 == 0xFFFF && message->data1 == 0xFFFF && message->data2 == 0xFFFF) {
      text_layer_set_text(s_text_layer, "Worker really started");
    }
  }
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Select");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Up");
  AppWorkerResult result = app_worker_launch();
  if (result == APP_WORKER_RESULT_NO_WORKER) {
    text_layer_set_text(s_text_layer, "No worker found");
  } else if (result == APP_WORKER_RESULT_ASKING_CONFIRMATION) {
    text_layer_set_text(s_text_layer, "Confirmation needed");
  } else {
    text_layer_set_text(s_text_layer, "Worker running");
  }
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Down");
  AppWorkerResult result = app_worker_kill();
  if (result == APP_WORKER_RESULT_SUCCESS) {
    text_layer_set_text(s_text_layer, "Worker stoped");
  } else if (result == APP_WORKER_RESULT_DIFFERENT_APP) {
    text_layer_set_text(s_text_layer, "Different app worker");
  } else if (result == APP_WORKER_RESULT_NOT_RUNNING){
    text_layer_set_text(s_text_layer, "Worker already stoped");
  }
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

/*static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}*/

static void set_status_bar(Layer *layer) {
  // Create the StatusBarLayer
  s_status_bar = status_bar_layer_create();

  // Set properties
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeNone);

  // Add to Window
  layer_add_child(layer, status_bar_layer_get_layer(s_status_bar));
}

static void set_action_bar_layer(Window *window) {
  // Load icon bitmaps
  s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_START_ICON);
  s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_STOP_ICON);
  s_check_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PAUSE_ICON);

  // Create ActionBarLayer
  s_action_bar = action_bar_layer_create();
  action_bar_layer_set_click_config_provider(s_action_bar, prv_click_config_provider);

  // Set the icons
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_up_bitmap);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_check_bitmap);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_down_bitmap);


  // Add to Window
  action_bar_layer_add_to_window(s_action_bar, window);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  set_status_bar(window_layer);

  set_action_bar_layer(window);

  GRect bounds = layer_get_bounds(window_layer);

  //s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w - ACTION_BAR_WIDTH, 20));
  const GEdgeInsets label_insets = {.top = bounds.size.h / 2, .right = ACTION_BAR_WIDTH, .left = ACTION_BAR_WIDTH / 2};
  s_text_layer = text_layer_create(grect_inset(bounds, label_insets));

  if (app_worker_is_running()) {
    text_layer_set_text(s_text_layer, "Worker is running");
  } else {
    text_layer_set_text(s_text_layer, "Press a button");
  }
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  // Destroy the TextLayer
  text_layer_destroy(s_text_layer);

  // Destroy the StatusBarLayer
  status_bar_layer_destroy(s_status_bar);

  // Desctroy de ActinoBarLayer
  action_bar_layer_destroy(s_action_bar);
}

static void prv_init(void) {
  s_window = window_create();
  //window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
  app_worker_message_subscribe(worker_message_handler);

  /*if (!app_worker_is_running()) {
    // Data logging
	  data_logging_finish(data_logging_create(LOGGING_TAG, DATA_LOGGING_BYTE_ARRAY, sizeof(AccelData), false));
  }*/
}

static void prv_deinit(void) {
  window_destroy(s_window);
  app_worker_message_unsubscribe();
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
