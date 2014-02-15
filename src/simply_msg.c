#include "simply_msg.h"

#include "simplyjs.h"

#include <pebble.h>

typedef enum SimplyACmd SimplyACmd;

enum SimplyACmd {
  SimplyACmd_setText = 0,
  SimplyACmd_singleClick,
  SimplyACmd_longClick,
  SimplyACmd_accelTap,
  SimplyACmd_vibe,
  SimplyACmd_setScrollable,
  SimplyACmd_setStyle,
  SimplyACmd_setFullscreen,
};

typedef enum VibeType VibeType;

enum VibeType {
  VibeShort = 0,
  VibeLong = 1,
  VibeDouble = 2,
};

static void handle_set_text(DictionaryIterator *iter, Simply *simply) {
  SimplyUi *ui = simply->ui;
  Tuple *tuple;
  bool clear = false;
  if ((tuple = dict_find(iter, 4))) {
    clear = true;
  }
  if ((tuple = dict_find(iter, 1)) || clear) {
    simply_ui_set_text(ui, &ui->title_text, tuple ? tuple->value->cstring : NULL);
  }
  if ((tuple = dict_find(iter, 2)) || clear) {
    simply_ui_set_text(ui, &ui->subtitle_text, tuple ? tuple->value->cstring : NULL);
  }
  if ((tuple = dict_find(iter, 3)) || clear) {
    simply_ui_set_text(ui, &ui->body_text, tuple ? tuple->value->cstring : NULL);
  }
}

static void handle_vibe(DictionaryIterator *iter, Simply *simply) {
  Tuple *tuple;
  if ((tuple = dict_find(iter, 1))) {
    switch ((VibeType) tuple->value->int32) {
      case VibeShort: vibes_short_pulse(); break;
      case VibeLong: vibes_short_pulse(); break;
      case VibeDouble: vibes_double_pulse(); break;
    }
  }
}

static void handle_set_scrollable(DictionaryIterator *iter, Simply *simply) {
  Tuple *tuple;
  if ((tuple = dict_find(iter, 1))) {
    simply_ui_set_scrollable(simply->ui, tuple->value->int32);
  }
}

static void handle_set_style(DictionaryIterator *iter, Simply *simply) {
  Tuple *tuple;
  if ((tuple = dict_find(iter, 1))) {
    simply_ui_set_style(simply->ui, tuple->value->int32);
  }
}

static void handle_set_fullscreen(DictionaryIterator *iter, Simply *simply) {
  Tuple *tuple;
  if ((tuple = dict_find(iter, 1))) {
    simply_ui_set_fullscreen(simply->ui, tuple->value->int32);
  }
}

static void received_callback(DictionaryIterator *iter, void *context) {
  Tuple *tuple = dict_find(iter, 0);
  if (!tuple) {
    return;
  }

  switch (tuple->value->uint8) {
    case SimplyACmd_setText:
      handle_set_text(iter, context);
      break;
    case SimplyACmd_vibe:
      handle_vibe(iter, context);
      break;
    case SimplyACmd_setScrollable:
      handle_set_scrollable(iter, context);
      break;
    case SimplyACmd_setStyle:
      handle_set_style(iter, context);
      break;
    case SimplyACmd_setFullscreen:
      handle_set_fullscreen(iter, context);
      break;
  }
}

static void dropped_callback(AppMessageResult reason, void *context) {
}

static void sent_callback(DictionaryIterator *iter, void *context) {
}

static void failed_callback(DictionaryIterator *iter, AppMessageResult reason, Simply *simply) {
  SimplyUi *ui = simply->ui;
  if (reason == APP_MSG_NOT_CONNECTED) {
    simply_ui_set_text(ui, &ui->subtitle_text, "Disconnected");
    simply_ui_set_text(ui, &ui->body_text, "Run the Pebble Phone App");
  }
}

void simply_msg_init(Simply *simply) {
  const uint32_t size_inbound = 2048;
  const uint32_t size_outbound = 128;
  app_message_open(size_inbound, size_outbound);

  app_message_set_context(simply);

  app_message_register_inbox_received(received_callback);
  app_message_register_inbox_dropped(dropped_callback);
  app_message_register_outbox_sent(sent_callback);
  app_message_register_outbox_failed((AppMessageOutboxFailed) failed_callback);
}

void simply_msg_deinit() {
  app_message_deregister_callbacks();
}

bool simply_msg_single_click(ButtonId button) {
  DictionaryIterator *iter = NULL;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
    return false;
  }
  dict_write_uint8(iter, 0, SimplyACmd_singleClick);
  dict_write_uint8(iter, 1, button);
  return (app_message_outbox_send() == APP_MSG_OK);
}

bool simply_msg_long_click(ButtonId button) {
  DictionaryIterator *iter = NULL;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
    return false;
  }
  dict_write_uint8(iter, 0, SimplyACmd_longClick);
  dict_write_uint8(iter, 1, button);
  return (app_message_outbox_send() == APP_MSG_OK);
}

bool simply_msg_accel_tap(AccelAxisType axis, int32_t direction) {
  DictionaryIterator *iter = NULL;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
    return false;
  }
  dict_write_uint8(iter, 0, SimplyACmd_accelTap);
  dict_write_uint8(iter, 1, axis);
  dict_write_int8(iter, 2, direction);
  return (app_message_outbox_send() == APP_MSG_OK);
}

