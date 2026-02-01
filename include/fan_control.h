#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include "pico/time.h"

/** 
 * \file fan_control.h
 * \defgroup util_fan_control util_fan_control
 * \brief Utility for fan PWM slice setup & speed control
*/

/** 
 * \brief Maximum fan speed value
 * \ingroup util_fan_control
 * 
 * Due to the fan speed value being an unsigned integer, this also sets the resolution at this value + 1 (value of 0).
*/
#define MAX_FAN_SPEED_VALUE 100

/** 
 * \brief Fan instance struct
 * \ingroup util_fan_control
 * 
 * This struct hold the set speed, measured RPM, last RPM measurement gate time, minimum speed value, 
 * PWM slice number, PWM channel number, tachometer PWM slice number and channel number.
 * 
 * \note Tachometer measurement is not yet implemented!
*/
typedef struct
{
    uint speed;
    uint rpm;
    absolute_time_t last_rpm_gate_time;
    uint min_speed;
    uint pwm_slice_num;
    uint pwm_chan_num;
    uint tacho_slice_num;
    uint tacho_chan_num;
} fan_inst_t;

/** 
 * \brief Fan curve struct
 * \ingroup util_fan_control
 * 
 * This struct defines a simple min/max bounded line.
*/
typedef struct
{
    uint min_speed;
    uint max_speed;
    float min_input;
    float max_input;
} fan_curve_t;

/** 
 * \brief Initialize the fan instance struct and setup the PWM slice
 * \ingroup util_fan_control
 * 
 * This function asserts if the user tries to initialise a fan on an already used channel. And also asserts when the user 
 * tries to initialise a fan on a slice used for tachometer or a tachometer on an already used slice.
 * 
 * Checks only its internal channel usage variable! Conflicts with any other library or user code are NOT detected!
 * 
 * \param inst Fan instance struct
 * \param pwm_pin PWM output pin number
 * \param has_tacho True if the fan's tachometer signal is connected to a PWM slice, otherwise false
 * \param tacho_pin Tachometer input pin number
 * \param min_speed Minimum fan speed. If required should be set to the minimum PWM value the fan will start and run at
*/
void fan_init(fan_inst_t *inst, uint pwm_pin, bool has_tacho, uint tacho_pin, uint min_speed);

/** 
 * \brief Set fan speed manually
 * \ingroup util_fan_control
 * 
 * \param inst Fan instance struct
 * \param speed Fan speed to set
 * \return Set speed after clamping
*/
uint fan_set_speed_man(fan_inst_t *inst, uint speed);

/** 
 * \brief Set fan speed from an input using a fan curve
 * \ingroup util_fan_control
 * 
 * Sets fan speed from an input (most commonly temperature or power) using the supplied fan curve.
 * 
 * \param inst Fan instance struct
 * \param curve Fan curve struct
 * \param input Input to use
 * \return Set speed
*/
uint fan_set_speed_curve(fan_inst_t *inst, fan_curve_t *curve, float input);

#endif