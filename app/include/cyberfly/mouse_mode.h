#ifndef CYBERFLY_MOUSE_MODE_H
#define CYBERFLY_MOUSE_MODE_H

enum cyberfly_mouse_mode {
    CYBERFLY_MOUSE_OFF = 0,
    CYBERFLY_MOUSE_M1,   /* Accelerometer tilt-to-velocity */
    CYBERFLY_MOUSE_M2,   /* Gyroscope with acceleration curve */
    CYBERFLY_MOUSE_MODE_COUNT,
};

#endif
