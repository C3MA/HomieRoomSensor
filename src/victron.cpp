/**
 * @file victron.cpp
 * @author Icefest
 * @brief Wrapper for Victron MPPT
 * @version 0.1
 *
 * Inspired by:
 * https://github.com/KinDR007/VictronMPPT-ESPHOME/blob/main/components/victron/victron.cpp
 */

#include "victron.h"
#include "MqttLog.h"

namespace victron {

    static const char *const TAG = "victron";

    VictronComponent::VictronComponent()
    {
        this->state_ = 0;
    }

    VictronComponent::~VictronComponent()
    {

    }

    VictronComponent::VictronComponent(int initialstate)
    {
        this->state_ = initialstate;
    }

    void VictronComponent::logTextSensor(String tag, String message, std::string text)
    {
        String complete = message + " : " +  String(text.c_str());
        log(MQTT_LEVEL_INFO, complete, MQTT_LOG_VICTRON);
    }

    void VictronComponent::logBinarySensor(String tag, String message, bool flag)
    {
        String complete = message + " : " +  String(flag);
        log(MQTT_LEVEL_INFO, complete, MQTT_LOG_VICTRON);
    }

    void VictronComponent::logSensor(String tag, String message, int number)
    {
        String complete = message + " : " +  String(number);
        log(MQTT_LEVEL_INFO, complete, MQTT_LOG_VICTRON);
    }

    void VictronComponent::loop()
    {
        const uint32_t now = millis();
        if ((state_ > 0) && (now - last_transmission_ >= 200)) {
            // last transmission too long ago. Reset RX index.
            log(MQTT_LEVEL_INFO, "Last transmission too long ago", MQTT_LOG_VICTRON);
            state_ = 0;
        }

        if (!Serial.available())
            return;

        last_transmission_ = now;
        while (Serial.available()) {
            uint8_t c;
            c = Serial.read();
            if (state_ == 0) {
            if (c == '\r' || c == '\n') {
                continue;
            }
            label_.clear();
            value_.clear();
            state_ = 1;
            }
            if (state_ == 1) {
            // Start of a ve.direct hex frame
            if (c == ':') {
                state_ = 3;
                continue;
            }
            if (c == '\t') {
                state_ = 2;
            } else {
                label_.push_back(c);
            }
            continue;
            }
            if (state_ == 2) {
            if (label_ == "Checksum") {
                state_ = 0;
                // The checksum is used as end of frame indicator
                if (now - this->last_publish_ >= this->throttle_) {
                this->last_publish_ = now;
                this->publishing_ = true;
                } else {
                this->publishing_ = false;
                }
                continue;
            }
            if (c == '\r' || c == '\n') {
                if (this->publishing_) {
                handle_value_();
                }
                state_ = 0;
            } else {
                value_.push_back(c);
            }
            }
            // Discard ve.direct hex frame
            if (state_ == 3) {
            if (c == '\r' || c == '\n') {
                state_ = 0;
            }
            }
        }
    }

    void VictronComponent::handle_value_()
    {
        int value;

        if (label_ == "V") {
            battery_voltage_sensor_ = atoi(value_.c_str()); /* mV */
            return;
        }

        if (label_ == "VPV") {
            // mV to V
            panel_voltage_sensor_ = atoi(value_.c_str()); /* mV */
            return;
        }

        if (label_ == "PPV") {
            panel_power_sensor_ = atoi(value_.c_str());
            return;
        }

        if (label_ == "I") {
            // mA to A
            battery_current_sensor_ = atoi(value_.c_str()); /* mA */
            return;
        }

        if (label_ == "IL") {
            load_current_sensor_ = atoi(value_.c_str()); /* mA */
            return;
        }

        if (label_ == "LOAD") {
            load_state_binary_sensor_= (value_ == "ON" || value_ == "On");
            return;
        }

        if (label_ == "Alarm") {
            alarm_condition_active_text_sensor_ = value_;
            return;
        }

        if (label_ == "H19") {
            yield_total_sensor_ =  (atoi(value_.c_str()) * 10.0f);  // NOLINT(cert-err34-c)
            return;
        }

        if (label_ == "H20") {
            yield_today_sensor_ = (atoi(value_.c_str()) * 10.0f);  // NOLINT(cert-err34-c)
            return;
        }

        if (label_ == "H21") {
            max_power_today_sensor_ = (atoi(value_.c_str()));  // NOLINT(cert-err34-c)
            return;
        }

        if (label_ == "H22") {
            yield_yesterday_sensor_ = (atoi(value_.c_str()) * 10.0f);  // NOLINT(cert-err34-c)
            return;
        }

        if (label_ == "H23") {
            max_power_yesterday_sensor_ = atoi(value_.c_str());
            return;
        }

        if (label_ == "ERR") {
            value = atoi(value_.c_str());  // NOLINT(cert-err34-c)
            error_code_sensor_ =  value;
            return;
        }

        if (label_ == "CS") {
            value = atoi(value_.c_str());  // NOLINT(cert-err34-c)
            charging_mode_id_sensor_ = value;
            return;
        }


        if (label_ == "FW") {
            firmware_version_text_sensor_ = value_.insert(value_.size() - 2, ".");
            return;
        }


        if (label_ == "PID") {
            device_type_text_sensor_ = strtol(value_.c_str(), nullptr, 0);
            return;
        }

        if (label_ == "SER#") {
            serial_number_text_sensor_ = value_;
            return;
        }

        if (label_ == "HSDS") {
            day_number_sensor_ = atoi(value_.c_str());
            return;
        }
        
        if (label_ == "MPPT") {
            value = atoi(value_.c_str());  // NOLINT(cert-err34-c)
            tracking_mode_id_sensor_ = value;
            return;
        }

        String message= "Unhandled property:" + String(label_.c_str()) + " : " +  String(value_.c_str());
        log(MQTT_LEVEL_ERROR, message, MQTT_LOG_VICTRON);
    }


    void VictronComponent::dump_config(void)
    {
        if (this->last_publish_ <= 0)
        {
            return; /* No data -> no log */
        }

        logBinarySensor("  ", "Load state", load_state_binary_sensor_);
        logSensor("  ", "Max Power Yesterday", max_power_yesterday_sensor_);
        logSensor("  ", "Max Power Today", max_power_today_sensor_);
        logSensor("  ", "Yield Total", yield_total_sensor_);
        logSensor("  ", "Yield Yesterday", yield_yesterday_sensor_);
        logSensor("  ", "Yield Today", yield_today_sensor_);
        logSensor("  ", "Panel Voltage", panel_voltage_sensor_);
        logSensor("  ", "Panel Power", panel_power_sensor_);
        logSensor("  ", "Battery Voltage", battery_voltage_sensor_);
        logSensor("  ", "Battery Current", battery_current_sensor_);
        logSensor("  ", "Load Current", load_current_sensor_);
        logSensor("  ", "Day Number", day_number_sensor_);
        logSensor("  ", "Charging Mode ID", charging_mode_id_sensor_);
        logSensor("  ", "Error Code", error_code_sensor_);
        logSensor("  ", "Tracking Mode ID", tracking_mode_id_sensor_);
        logTextSensor("  ", "Firmware Version", firmware_version_text_sensor_);
        logTextSensor("  ", "Alarm Condition Active", alarm_condition_active_text_sensor_);   
        /** 
         * Linking .pio/build/nodemcuv2/firmware.elf
            .pio/build/nodemcuv2/libFrameworkArduino.a(core_esp8266_postmortem.cpp.o): in function `__wrap_system_restart_local':
            core_esp8266_postmortem.cpp:(.text.__wrap_system_restart_local+0x2): dangerous relocation: j: cannot encode: (.text.postmortem_report+0x88)


        logTextSensor("  ", "Error Text",  error_code_text(error_code_sensor_));
        logTextSensor("  ", "Tracking Mode", tracking_mode_text(tracking_mode_id_sensor_));
        logTextSensor("  ", "Charging Mode", charging_mode_text(charging_mode_id_sensor_));
        logTextSensor("  ", "Device Type", device_type_text(device_type_text_sensor_));
         */
    }

    String VictronComponent::toJson(void)
    {
        String buffer;
        if (this->last_publish_ <= 0)
        {
            return buffer;
        }
        else
        {
            StaticJsonDocument<200> doc;
            //FIXME doc["LoadState"] = load_state_binary_sensor_;
            // doc["MaxPowerYesterday"] = max_power_yesterday_sensor_;
            // doc["MaxPowerToday"] = max_power_today_sensor_;
            // doc["YieldTotal"] = yield_total_sensor_;
            // doc["YieldYesterday"] = yield_yesterday_sensor_;
            // doc["YieldToday"] = yield_today_sensor_;
            // doc["PanelVoltage"] = panel_voltage_sensor_;
            // doc["PanelPower"] = panel_power_sensor_;
            doc["BatVoltage"] = battery_voltage_sensor_;
            // doc["BatCurrent"] = battery_current_sensor_;
            // doc["LoadCurrent"] = load_current_sensor_;
            // doc["DayNumber"] = day_number_sensor_;
            // doc["ChargingModeID"] = charging_mode_id_sensor_;
            // doc["ErrorCode"] = error_code_sensor_;
            // doc["TrackingModeID"] = tracking_mode_id_sensor_;
            //FIXME doc["ErrorText"] =  error_code_text(error_code_sensor_).c_str();
            //FIXME doc["TrackingMode"] = tracking_mode_text(tracking_mode_id_sensor_).c_str();
            //FIXME doc["ChargingMode"] = charging_mode_text(charging_mode_id_sensor_).c_str();
            //FIXME doc["FirmwareVersion"] = firmware_version_text_sensor_.c_str();
            //FIXME doc["DeviceType"] = device_type_text(device_type_text_sensor_).c_str();
            //FIXME doc["AlarmConditionActive"] = alarm_condition_active_text_sensor_.c_str();
            serializeJson(doc, buffer);
            return buffer;
        }
    }
}
