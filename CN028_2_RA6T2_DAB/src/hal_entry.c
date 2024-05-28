#include "hal_data.h"
#include "ra6t2.h"
#include "dcdc_dab.h"
#include "ntc.h"
#include "console.h"
#include "err_handler.h"
#include "can_fd.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

void heart_beat(void);
void key_scan(void);
uint32_t hb_cntr = 0;
uint32_t key_cntr = 0;
volatile uint32_t g_key_signal = 0;

/*******************************************************************************************************************//**
 * main() is generated by the RA Configuration editor and is used to generate threads if an RTOS is used.  This function
 * is called by main() when no RTOS is used.
 **********************************************************************************************************************/
void hal_entry(void)
{
    /* TODO: add your own code here */
    PORT_OUTPUT_FAN = 1;    /* Turn on the FAN */
    hardware_init();
    dab_para_init();
    serial_para_init();

    Delay_s(1);
    startup();

    while (1)
    {
        burst_function();
        uart_operation();
        canfd_operation();
        key_scan();
        dab_routine();
        heart_beat();
    }

#if BSP_TZ_SECURE_BUILD
    /* Enter non-secure code */
    R_BSP_NonSecureEnter();
#endif
}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and
         * C runtime initialization, should negate the need for a delay since the initialization will typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open (&g_ioport_ctrl, g_ioport.p_cfg);
    }
}

#if BSP_TZ_SECURE_BUILD

BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ();

/* Trustzone Secure Projects require at least one nonsecure callable function in order to build (Remove this if it is not required to build). */
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ()
{

}
#endif

void g_agt1_10kHz_callback(timer_callback_args_t *p_args)
{
    UNUSED(p_args);
    key_cntr++;
    hb_cntr++;
    //if (con.f.uart_connect)
    con.f.uart_rx_timeout++;
    if (con.f.can_connect)
    {
        con.f.can_tx_time++;
        if (con.f.can_tx_time >= 1000)
        {
            con.f.can_tx_time = 0;
            con.f.can_tx_en = true;
        }
    }
}

/* 33kHz */
void adc_b_adi0_isr_user(void)
{
    static bool zero_offset_en = true;
    static uint8_t adc_cont = 0;
    static uint32_t adc_sum0 = 0, adc_sum1 = 0;

    g_dab_para.adc_Isec_raw = R_ADC_B->ADDR[ADC_CHANNEL_12];
    g_dab_para.adc_Vsec_raw = R_ADC_B->ADDR[ADC_CHANNEL_14];
    g_dab_para.adc_Vbus_raw = R_ADC_B->ADDR[ADC_CHANNEL_15];
    g_dab_para.adc_Ibus_raw = R_ADC_B->ADDR[ADC_CHANNEL_13];

    g_dab_para.adc_Vbus_raw = g_dab_para.adc_Vbus_raw & 0x7fffffff;
    g_dab_para.adc_Vsec_raw = g_dab_para.adc_Vsec_raw & 0x7fffffff;
    g_dab_para.adc_Isec_raw = g_dab_para.adc_Isec_raw & 0x7fffffff;
    g_dab_para.adc_Ibus_raw = g_dab_para.adc_Ibus_raw & 0x7fffffff;
    //零偏
    if (g_dab_sys.state == STANDBY_STATE && zero_offset_en)
    {
        adc_sum0 += g_dab_para.adc_Isec_raw;
        adc_sum1 += g_dab_para.adc_Ibus_raw;
        adc_cont++;
        if (adc_cont >= 100)
        {
            g_dab_para.adc_Isec_offset = (int32_t)((int32_t)(adc_sum0 / adc_cont) - 2048);
            if (g_dab_para.adc_Isec_offset > 85 || g_dab_para.adc_Isec_offset < -85)
            {
                g_dab_err.bit.INNER_ERR = 1;
                g_dab_para.adc_Isec_offset = 0;
            }
            g_dab_para.adc_Ibus_offset = (int32_t)((int32_t)(adc_sum1 / adc_cont) - 2000);
            if (g_dab_para.adc_Ibus_offset > 85 || g_dab_para.adc_Ibus_offset < -85)
            {
                g_dab_err.bit.INNER_ERR = 1;
                g_dab_para.adc_Ibus_offset = 0;
            }
            zero_offset_en = false;
        }
    }
    g_dab_para.adc_Isec_raw = (uint32_t)((int32_t)g_dab_para.adc_Isec_raw - g_dab_para.adc_Isec_offset);
    g_dab_para.adc_Ibus_raw = (uint32_t)((int32_t)g_dab_para.adc_Ibus_raw - g_dab_para.adc_Ibus_offset);

    g_dab_para.adc_Isec = (float)g_dab_para.adc_Isec_raw * 0.12656 - 259.1325;
    g_dab_para.adc_Vbus = (float)g_dab_para.adc_Vbus_raw * 0.12625;
    g_dab_para.adc_Vsec = (float)g_dab_para.adc_Vsec_raw * 0.01692;
    g_dab_para.adc_Ibus = ((float)g_dab_para.adc_Ibus_raw * 0.00080586 - 1.61211529) * 31.015151515;

    g_dab_para.adc_Isec_flt = g_dab_para.adc_Isec * 0.0332 + g_dab_para.adc_Isec_old * 0.9668;
    g_dab_para.adc_Isec_old = g_dab_para.adc_Isec_flt;
    g_dab_para.adc_Vsec_flt = g_dab_para.adc_Vsec * 0.332 + g_dab_para.adc_Vsec_old * 0.668;
    g_dab_para.adc_Vsec_old = g_dab_para.adc_Vsec_flt;
    g_dab_para.adc_Vbus_flt = g_dab_para.adc_Vbus * 0.332 + g_dab_para.adc_Vbus_old * 0.668;
    g_dab_para.adc_Vbus_old = g_dab_para.adc_Vbus_flt;
    g_dab_para.adc_Ibus_flt = g_dab_para.adc_Ibus * 0.0332 + g_dab_para.adc_Ibus_old * 0.9668;
    g_dab_para.adc_Ibus_old = g_dab_para.adc_Ibus_flt;

    if (g_dab_sys.state == DAB_RUN_STATE)
    {
        dab_high_fre_routine();
    }

    dab_high_protection();

    R_ADC_B->ADSCANENDSCR = ADC_GROUP_MASK_0;
    R_BSP_IrqStatusClear(VECTOR_NUMBER_ADC12_ADI0);
}

/* 10kHz */
void adc_b_adi1_isr_user(void)
{
    g_dab_para.adc_Temp_raw = R_ADC_B->ADDR[ADC_CHANNEL_4];
    g_dab_para.adc_Temp_HV_raw = R_ADC_B->ADDR[ADC_CHANNEL_0];
    g_dab_para.adc_Temp_raw = g_dab_para.adc_Temp_raw & 0x7fffffff;
    g_dab_para.adc_Temp_HV_raw = g_dab_para.adc_Temp_HV_raw & 0x7fffffff;
    g_dab_para.adc_Temp = ntc_table[(uint16_t)((g_dab_para.adc_Temp_raw - 900) / 10)];
    g_dab_para.adc_Temp_HV = ntc_table[(uint16_t)((g_dab_para.adc_Temp_HV_raw - 900) / 10)];

    if (g_dab_sys.app == APP_P2S_TargetS)
    {
        g_dab_para.Pdc = g_dab_para.adc_Isec_flt * g_dab_para.adc_Vsec_flt;
    }
    else
    {
        g_dab_para.Pdc = g_dab_para.adc_Ibus_flt * g_dab_para.adc_Vbus_flt;
    }

    dab_low_protection();

    R_ADC_B->ADSCANENDSCR = ADC_GROUP_MASK_1;
    R_BSP_IrqStatusClear(VECTOR_NUMBER_ADC12_ADI1);
}

void g_adc_b_callback(adc_callback_args_t *p_args)
{
    UNUSED(p_args);
}

void key_scan(void)
{
    static uint16_t press_cont[2], key_time;
    static bool f_key_released = true, f_key_enable = true;

    if (key_cntr >= 10)
    {
        key_cntr = 0;
        if (f_key_enable)
        {
            if (KEY_ENB == 0)
            {
                press_cont[0]++;
                if (press_cont[0] > 200 && f_key_released == true)
                {
                    g_key_signal = KEY_ON;
                    f_key_released = false;
                    f_key_enable = false;
                }
            }
            else if (KEY_RST == 0)
            {
                press_cont[1]++;
                if (press_cont[1] > 200 && f_key_released == true)
                {
                    g_key_signal = KEY_ERR_RST;
                    f_key_released = false;
                    f_key_enable = false;
                }
            }
            else
            {
                press_cont[0] = 0;
                press_cont[1] = 0;
                f_key_released = true;
            }
        }
        else
        {
            key_time++;
            if (key_time >= 500)
            {
                f_key_enable = true;
                key_time = 0;
            }
        }
    }
}

void heart_beat(void)
{
    if (hb_cntr > 20000)
    {
        hb_cntr = 0;
        LED_BLUE ^= 1;

        switch (g_dab_sys.state)
        {
            case FAULT_STATE:
                LED_RED = 0;
                break;
            case STANDBY_STATE:
                LED_RED = 1;
                break;
            default:
                LED_RED ^= 1;
                break;
        }
    }
}