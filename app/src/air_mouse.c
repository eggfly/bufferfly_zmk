/*
 * CyberFly Air Mouse — QMI8658A IMU tilt/gyro to cursor.
 * M1: accelerometer tilt-to-velocity
 * M2: gyroscope angular velocity with pointer acceleration curve
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <zmk/hid.h>
#include <zmk/endpoints.h>
#include <cyberfly/mouse_mode.h>

LOG_MODULE_REGISTER(air_mouse, LOG_LEVEL_INF);

#define IMU_NODE DT_NODELABEL(qmi8658a)

static const struct device *const imu = DEVICE_DT_GET(IMU_NODE);

extern enum cyberfly_mouse_mode cyberfly_mouse_get_mode(void);

/* M1 accel parameters */
#define POLL_INTERVAL_MS  18
#define ACCEL_DEAD_ZONE   0.15
#define ACCEL_SENSITIVITY 3.0
#define ACCEL_MAX_SPEED   15

/* M2 gyro parameters */
#define GYRO_DEAD_ZONE_DPS   2.0
#define GYRO_BASE_SENS       0.06
#define GYRO_ACCEL_THRESH    30.0
#define GYRO_ACCEL_FACTOR    2.5
#define GYRO_MAX_SPEED       50

#define RAD_TO_DEG  (180.0 / 3.14159265)

static bool imu_ok;
static uint32_t poll_count;

static int16_t accel_to_mouse(double val) {
    if (val > -ACCEL_DEAD_ZONE && val < ACCEL_DEAD_ZONE) {
        return 0;
    }
    int16_t r = (int16_t)(val * ACCEL_SENSITIVITY);
    if (r > ACCEL_MAX_SPEED) r = ACCEL_MAX_SPEED;
    if (r < -ACCEL_MAX_SPEED) r = -ACCEL_MAX_SPEED;
    return r;
}

static int16_t gyro_to_mouse(double gyro_rads) {
    double dps = gyro_rads * RAD_TO_DEG;
    if (dps > -GYRO_DEAD_ZONE_DPS && dps < GYRO_DEAD_ZONE_DPS) {
        return 0;
    }
    double abs_dps = (dps > 0) ? dps : -dps;
    double multiplier = 1.0;
    if (abs_dps > GYRO_ACCEL_THRESH) {
        double ratio = (abs_dps - GYRO_ACCEL_THRESH) / GYRO_ACCEL_THRESH;
        multiplier = 1.0 + ratio * (GYRO_ACCEL_FACTOR - 1.0);
        if (multiplier > GYRO_ACCEL_FACTOR) multiplier = GYRO_ACCEL_FACTOR;
    }
    int16_t r = (int16_t)(dps * multiplier * GYRO_BASE_SENS);
    if (r > GYRO_MAX_SPEED) r = GYRO_MAX_SPEED;
    if (r < -GYRO_MAX_SPEED) r = -GYRO_MAX_SPEED;
    return r;
}

static void air_mouse_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(air_mouse_work, air_mouse_work_handler);

static void air_mouse_work_handler(struct k_work *work) {
    if (!imu_ok) {
        if (!device_is_ready(imu)) {
            LOG_ERR("QMI8658A not ready (retry in 5s)");
            k_work_reschedule(&air_mouse_work, K_SECONDS(5));
            return;
        }
        int rc = sensor_sample_fetch(imu);
        if (rc) {
            LOG_ERR("QMI8658A first fetch failed: %d (retry in 5s)", rc);
            k_work_reschedule(&air_mouse_work, K_SECONDS(5));
            return;
        }
        imu_ok = true;
        LOG_INF("QMI8658A online, air mouse ready");
    }

    k_work_reschedule(&air_mouse_work, K_MSEC(POLL_INTERVAL_MS));

    enum cyberfly_mouse_mode mode = cyberfly_mouse_get_mode();
    if (mode == CYBERFLY_MOUSE_OFF) {
        return;
    }

    int rc = sensor_sample_fetch(imu);
    if (rc) {
        if ((poll_count % 500) == 0) {
            LOG_ERR("fetch failed: %d (count=%u)", rc, poll_count);
        }
        poll_count++;
        return;
    }

    int16_t dx, dy;

    if (mode == CYBERFLY_MOUSE_M1) {
        struct sensor_value accel[3];
        sensor_channel_get(imu, SENSOR_CHAN_ACCEL_XYZ, accel);
        double ax = sensor_value_to_double(&accel[0]);
        double ay = sensor_value_to_double(&accel[1]);
        dx = accel_to_mouse(-ax);
        dy = accel_to_mouse(-ay);
    } else {
        struct sensor_value gyro[3];
        sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, gyro);
        double gx = sensor_value_to_double(&gyro[0]);
        double gy = sensor_value_to_double(&gyro[1]);
        dx = gyro_to_mouse(-gx);
        dy = gyro_to_mouse(-gy);
    }

    poll_count++;

    if (dx == 0 && dy == 0) {
        return;
    }

    zmk_hid_mouse_movement_set(dx, dy);
    zmk_endpoint_send_mouse_report();
    zmk_hid_mouse_clear();
}

static int air_mouse_init(void) {
    LOG_INF("air_mouse init, probe in 3s");
    k_work_reschedule(&air_mouse_work, K_SECONDS(3));
    return 0;
}

SYS_INIT(air_mouse_init, APPLICATION, 99);
