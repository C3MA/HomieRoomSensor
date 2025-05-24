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
#include "VictronTexts.h"

namespace victron {

    static const char *const TAG = "victron";

    VictronComponent::~VictronComponent()
    {

    }

    VictronComponent::VictronComponent(int initialstate)
    {
        this->state_ = initialstate;
    }

    VictronComponent::VictronComponent(int initialstate, debug_serialcommunication debugFunction)
    {
        this->state_ = initialstate;
        this->fdebugSerial = debugFunction;
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

            if (fdebugSerial) /* debugging enabled */
            {
                /* always store the incoming data */
                complete_line_.push_back(c);
            }

            if (state_ == 0) {
            if (c == '\r' || c == '\n') {
                if ( (fdebugSerial) /* debugging enabled */ && (complete_line_.length() > 0) )
                {
                    fdebugSerial(complete_line_);
                    complete_line_.clear();
                }
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
            if (state_ == 2)
            {
              if (label_ == "Checksum") {
                state_ = 0;
                // The checksum is used as end of frame indicator
                if ((now - this->last_publish_) >= VICTRON_THROTTLE) {
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

    String VictronComponent::toJson(void)
    {
        String buffer;
        if (this->last_publish_ <= 0)
        {
            buffer += "{ ";
            buffer += "\"mode\": \"nodata\",\n";
            buffer += "\"state\":" + String(state_) + ",\n";
            buffer += "\"transmission\":" + String(last_transmission_) + ",\n";
            buffer += "\"publish\":" + String(last_publish_) + "\n";
            buffer += "}";
            return buffer;
        }
        else
        {
            buffer += "{ ";
            buffer += "\"mode\": \"newdata\",\n";
            buffer += "\"load\":" + String(load_state_binary_sensor_) + ",\n";
            buffer += "\"MaxPower\":{\n";
            buffer += "\"yesterday\":" + String(max_power_yesterday_sensor_) + ",\n";
            buffer += "\"today\":" + String(max_power_today_sensor_) + "\n";
            buffer += "},\n";
            buffer += "\"Yield\":{\n";
            buffer += "\"Total\":" + String(yield_total_sensor_) + ",\n";
            buffer += "\"Yesterday\":" + String(yield_yesterday_sensor_) + ",\n";
            buffer += "\"Today\":" + String(yield_today_sensor_) + "\n";
            buffer += "},\n";
            buffer += "\"Panel\":{\n";
            buffer += "\"Voltage\":" + String(panel_voltage_sensor_) + ",\n";
            buffer += "\"Power\":" + String(panel_power_sensor_) + "\n";
            buffer += "},\n";
            buffer += "\"Bat\":{\n";
            buffer += "\"Voltage\":" + String(battery_voltage_sensor_) + ",\n";
            buffer += "\"Current\":" + String(battery_current_sensor_) + "\n";
            buffer += "},\n";
            buffer += "\"LoadCurrent\":" + String(load_current_sensor_) + ",\n";
            buffer += "\"DayNumber\":" + String(day_number_sensor_) + ",\n";
            buffer += "\"ChargingModeID\":" + String(charging_mode_id_sensor_) + ",\n";
            buffer += "\"ErrorCode\":" + String(error_code_sensor_) + ",\n";
            buffer += "\"TrackingModeID\":" + String(tracking_mode_id_sensor_) + ",\n";
            buffer += "\"ErrorText\": \"" + String(error_code_text(error_code_sensor_).c_str()) + "\",\n";
            buffer += "\"TrackingMode\": \"" + String(tracking_mode_text(tracking_mode_id_sensor_).c_str()) + "\",\n";
            buffer += "\"ChargingMode\": \"" + String(charging_mode_text(charging_mode_id_sensor_).c_str()) + "\",\n";
            buffer += "\"FirmwareVersion\": \"" + String(firmware_version_text_sensor_.c_str()) + "\",\n";
            buffer += "\"DeviceType\": \"" + String(device_type_text(device_type_text_sensor_).c_str() ) + "\",\n";
            buffer += "\"AlarmConditionActive\": \"" + String(alarm_condition_active_text_sensor_.c_str()) + "\"\n";
            buffer += "}";
            return buffer;
        }
    }
}
