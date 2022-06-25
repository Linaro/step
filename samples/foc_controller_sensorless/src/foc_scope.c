#include <string.h>
#include <zephyr.h>
#include <device.h>
#include <sys/printk.h>
#include "foc_scope.h"

#define SCOPE_BUFF_SIZE 512

static bool scope_enable = false;
static struct foc_controller_payload scope_buffer[2][SCOPE_BUFF_SIZE];
static bool ping_pong_switch = false;
static uint32_t rd_buff_index = 0;
static uint32_t wr_buff_index = 0;

static void foc_scope_daemon_thread(void *arg);

/* Rotor sensor measurement thread. */
K_THREAD_DEFINE(foc_scope_tid, 8192,
		foc_scope_daemon_thread, NULL, NULL, NULL,
		0, 0, 0);

static void foc_scope_daemon_thread(void *arg)
{
    struct foc_controller_payload *next_sample;
    memset(&scope_buffer, 0, sizeof(scope_buffer));

    while(1) {
        if(scope_enable) {
            next_sample = &scope_buffer[ping_pong_switch][rd_buff_index];
            		printk("%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
                        next_sample->timestamp,
                        next_sample->state.motor_pole_pairs,
                        next_sample->state.rotor_position,
                        next_sample->state.estimated_rotor_position,
                        next_sample->state.i_alpha_beta[0],
                        next_sample->state.i_alpha_beta[1],
                        next_sample->cmd.voltage_q,
                        next_sample->cmd.voltage_d,
                        next_sample->cmd.voltage_alpha,
                        next_sample->cmd.voltage_beta,
                        next_sample->state.i_abc[0],
                        next_sample->state.i_abc[1],
                        next_sample->state.i_abc[2],
                        next_sample->state.dt);
            rd_buff_index++;
            if(rd_buff_index >= SCOPE_BUFF_SIZE) {
                uint32_t key = irq_lock();
                rd_buff_index = 0;
                wr_buff_index = 0;
                ping_pong_switch ^= 1;
                irq_unlock(key);
            }
           
        } else {
            k_sleep(K_MSEC(100));
        }
    }
}

void foc_scope_enable(void)
{
    scope_enable = true;
}

void foc_scope_disable(void)
{
    scope_enable = false;
}

void foc_scope_data_push(struct foc_controller_payload *p)
{
    if(!p || wr_buff_index >= SCOPE_BUFF_SIZE) {
        return;
    }

    uint32_t key = irq_lock();
    memcpy(&scope_buffer[!ping_pong_switch][wr_buff_index],
        p,
        sizeof(*p));
    
    wr_buff_index++;
    irq_unlock(key);
}
