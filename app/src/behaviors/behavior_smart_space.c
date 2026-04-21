/*
 * CyberFly: Smart space behavior.
 * Mouse OFF → always Space.
 * Mouse ON  → param 0: left click, 1: space, 2: right click.
 */

#define DT_DRV_COMPAT cyberfly_behavior_smart_space

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/input/input.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>
#include <cyberfly/mouse_mode.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

extern enum cyberfly_mouse_mode cyberfly_mouse_get_mode(void);

#define ROLE_LCLICK 0
#define ROLE_SPACE  1
#define ROLE_RCLICK 2

static bool is_mouse_click(uint32_t role, enum cyberfly_mouse_mode mode) {
    return mode != CYBERFLY_MOUSE_OFF && role != ROLE_SPACE;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    uint32_t role = binding->param1;
    enum cyberfly_mouse_mode mode = cyberfly_mouse_get_mode();

    if (!is_mouse_click(role, mode)) {
        return raise_zmk_keycode_state_changed_from_encoded(SPACE, true,
                                                            event.timestamp);
    }

    int btn = (role == ROLE_LCLICK) ? INPUT_BTN_0 : INPUT_BTN_1;
    input_report_key(zmk_behavior_get_binding(binding->behavior_dev),
                     btn, 1, true, K_FOREVER);
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    uint32_t role = binding->param1;
    enum cyberfly_mouse_mode mode = cyberfly_mouse_get_mode();

    if (!is_mouse_click(role, mode)) {
        return raise_zmk_keycode_state_changed_from_encoded(SPACE, false,
                                                            event.timestamp);
    }

    int btn = (role == ROLE_LCLICK) ? INPUT_BTN_0 : INPUT_BTN_1;
    input_report_key(zmk_behavior_get_binding(binding->behavior_dev),
                     btn, 0, true, K_FOREVER);
    return 0;
}

static const struct behavior_driver_api behavior_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_EVENT_SOURCE,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define INST(n)                                                                \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,            \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_api);

DT_INST_FOREACH_STATUS_OKAY(INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
