/**
 * @file victron.h
 * @author Icefest
 * @brief Wrapper for Victron MPPT
 * @version 0.1
 *
 * Inspired by:
 * https://github.com/KinDR007/VictronMPPT-ESPHOME/blob/main/components/victron/victron.h
 */

#ifndef VICTRON_MPPT
#define VICTRON_MPPT

#include <stdint.h>
#include <Homie.h>

namespace victron
{

    class VictronComponent 
    {
    public:
        VictronComponent(int initialstate);
        VictronComponent();
        ~VictronComponent();
        void loop(void);
        void dump_config(void);
        String toJson(void);

        int getBatteryVoltage() {
            return battery_voltage_sensor_;
        }

    private:
        void handle_value_();
        void logTextSensor(String tag, String message, std::string text);
        void logBinarySensor(String tag, String message, bool flag);
        void logSensor(String tag, String message, int number);

        /* States during serial parsing */
        bool publishing_;
        int state_;
        std::string label_;
        std::string value_;
        uint32_t last_transmission_;
        uint32_t last_publish_;
        uint32_t throttle_;


        /* All Settings */
        int max_power_yesterday_sensor_ = 0;
        int max_power_today_sensor_ = 0;
        float yield_total_sensor_ = 0;
        float yield_yesterday_sensor_ = 0;
        float yield_today_sensor_ = 0;
        int panel_voltage_sensor_ = 0;
        int panel_power_sensor_ = 0;
        int battery_voltage_sensor_ = 0;
        int battery_current_sensor_ = 0;
        int load_current_sensor_ = 0;
        int day_number_sensor_ = 0;
        int charging_mode_id_sensor_ = 0;
        int error_code_sensor_ = 0;
        int tracking_mode_id_sensor_ = 0;


        bool load_state_binary_sensor_;

        std::string alarm_condition_active_text_sensor_;

        std::string firmware_version_text_sensor_;
        long device_type_text_sensor_;
        std::string serial_number_text_sensor_;

    };

}  // namespace victron

#endif /* End of VICTRON_MPPT */