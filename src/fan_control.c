#include "fan_control.h"

#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include <math.h>

typedef enum{
    FAN_CHAN_FREE,
    FAN_CHAN_PWM,
    FAN_CHAN_TACHO
} fan_channel_claim_state_t;

#if NUM_PWM_SLICES > 8
    fan_channel_claim_state_t chan_claim_state[12][2] = {FAN_CHAN_FREE};
#else
    fan_channel_claim_state_t chan_claim_state[8][2] = {FAN_CHAN_FREE};
#endif

void fan_init(fan_inst_t *inst, uint pwm_pin, bool has_tacho, uint tacho_pin, uint min_speed)
{
    uint pwm_slice_num = pwm_gpio_to_slice_num(pwm_pin);
    uint pwm_chan_num = pwm_gpio_to_channel(pwm_pin);

    assert(chan_claim_state[pwm_slice_num][pwm_chan_num] == FAN_CHAN_FREE);
    assert(chan_claim_state[pwm_slice_num][PWM_CHAN_B] != FAN_CHAN_TACHO);

    chan_claim_state[pwm_slice_num][pwm_chan_num] = FAN_CHAN_PWM;

    inst->min_speed = min_speed;
    inst->pwm_slice_num = pwm_slice_num;
    inst->pwm_chan_num = pwm_chan_num;
    inst->speed = 0;

    if(has_tacho)
    {   
        uint tacho_slice_num = pwm_gpio_to_slice_num(tacho_pin);
        uint tacho_chan_num = pwm_gpio_to_channel(tacho_pin);

        assert(tacho_chan_num == PWM_CHAN_B);
        assert(chan_claim_state[tacho_slice_num][PWM_CHAN_A] == FAN_CHAN_FREE);
        assert(chan_claim_state[tacho_slice_num][PWM_CHAN_B] == FAN_CHAN_FREE);

        chan_claim_state[tacho_slice_num][tacho_chan_num] = FAN_CHAN_TACHO;

        inst->tacho_slice_num = tacho_slice_num;
        inst->tacho_chan_num = tacho_chan_num;

        pwm_config c = pwm_get_default_config();
        pwm_config_set_wrap(&c, 50000);
        pwm_config_set_clkdiv_mode(&c, PWM_DIV_B_FALLING);
        pwm_init(tacho_slice_num, &c, true);

        gpio_set_function(tacho_pin, GPIO_FUNC_PWM);
    }

    // Do not initialise the PWM slice if it's already initialised
    if(chan_claim_state[pwm_slice_num][PWM_CHAN_A] == FAN_CHAN_FREE && chan_claim_state[pwm_slice_num][PWM_CHAN_B] == FAN_CHAN_FREE)
    {
        pwm_config c = pwm_get_default_config();
        float div = (float)clock_get_hz(clk_sys) / (25 * KHZ * MAX_FAN_SPEED_VALUE);
        pwm_config_set_clkdiv(&c, div);
        pwm_config_set_wrap(&c, MAX_FAN_SPEED_VALUE - 1); //The PWM slice counts TOP+1 cycles (includes 0) & CC=TOP+1 -> glitchless 100% duty cycle
        pwm_init(pwm_slice_num, &c, true);
    }

    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
}

uint fan_set_speed_man(fan_inst_t *inst, uint speed)
{
    uint speed_clamped = speed < inst->min_speed ? 0 : (speed > MAX_FAN_SPEED_VALUE ? MAX_FAN_SPEED_VALUE : speed);
    pwm_set_chan_level(inst->pwm_slice_num, inst->pwm_chan_num, speed_clamped);
    inst->speed = speed_clamped;
    return inst->speed;
}

uint fan_set_speed_curve(fan_inst_t *inst, fan_curve_t *map, float input)
{
    uint output;
    if(input == NAN || input > map->max_input)
    {
        output = map->max_speed;
    }
    else if(input < map->min_input)
    {
        output = map->min_speed;
    }
    else
    {
        float k = (map->max_speed - map->min_speed) / (map->max_input - map->min_input);
        output = (uint)roundf(k * (input - map->min_input)) + (map->min_speed);
    }

    if(output < inst->min_speed)
    {
        output = 0;
    }

    pwm_set_chan_level(inst->pwm_slice_num, inst->pwm_chan_num, output);
    inst->speed = output;
    return output;
}
