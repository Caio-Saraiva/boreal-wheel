#ifndef BOREAL_WHEEL_BRIDGE_H
#define BOREAL_WHEEL_BRIDGE_H
#include <stdint.h>
#define BW_BUTTONS 16
#define BW_AXES 6
#define BW_SOURCES 28
/* Sources: LX, LY, RX, RY, RT, LT; buttons 6..21; raw axes 22..27.
 * Outputs: LX, LY, RX, RY, RT, LT. Bits 10/11 are reserved. */
typedef struct { uint16_t buttons; uint8_t lt, rt; int16_t lx, ly, rx, ry; } BWReport;
typedef struct {
    float steering, throttle, brake, ly, rx, ry;
    float axes[BW_AXES];
    uint16_t buttons;
    int connected, usb_present, combined_pedals;
    char name[160], backend[48], detail[512];
} BWInput;
typedef struct {
    int source, negative, invert;
    float deadzone, limit, gamma, center;
} BWAxisConfig;
typedef struct { int source, direction; float threshold; } BWButtonConfig;
typedef struct {
    int poll_hz, slot, directinput, steering_axis, pedal_axis;
    BWAxisConfig axis[BW_AXES];
    BWButtonConfig button[BW_BUTTONS];
    int button_map[BW_BUTTONS]; /* Legacy input-to-output profile compatibility. */
} BWConfig;
void bw_defaults(BWConfig *c);
int bw_config_valid(const BWConfig *c);
float bw_source(const BWInput *in, int source);
float bw_axis_input(const BWInput *in, const BWConfig *c, int axis);
float bw_axis_map(float value, const BWAxisConfig *axis);
float bw_steering(float value, const BWConfig *c);
void bw_map(const BWInput *in, const BWConfig *c, BWReport *out);
int bw_setting(BWConfig *c, const char *key, const char *value);
int bw_set_language(const char *language);
const char *bw_text(const char *english);
double bw_now(void);
void bw_sleep(double seconds);
void bw_input_read(BWInput *in, const BWConfig *c);
void bw_input_close(void);
int bw_output(const BWReport *report, int enabled, char *status, int capacity);
void bw_output_close(void);
#endif
