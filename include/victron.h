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

namespace victron {

class VictronComponent {
 public:
  VictronComponent(int initialstate);
  VictronComponent();
  ~VictronComponent();
  void loop(void);
  uint32_t getPanelVoltageSensor(void);
  uint32_t getPanelPowerSensor(void);
  uint32_t getBatteryVoltageSensor(void);
  uint32_t getBatteryVoltage2Sensor(void);
  uint32_t getBatteryVoltage3Sensor(void);
  uint32_t getBatteryCurrentSensor(void);
  uint32_t getBatteryCurrent_2Sensor(void);
  uint32_t getBatteryCurrent_3Sensor(void);
  uint32_t getLoadCurrentSensor(void);
  
  std::string getFirmwareVersion(void);
  std::string getFirmware_version_24bit_textSensor(void);
  std::string getSerialNumber(void);

 protected:
  void handle_value_();

private:
  bool publishing_;
  int state_;
  std::string label_;
  std::string value_;
  uint32_t last_transmission_;
  uint32_t last_publish_;
  uint32_t throttle_;

  void log(String tag, String message);


  /* All Settings */

  int max_power_yesterday_sensor_ = 0;
  int max_power_today_sensor_ = 0;
  float yield_total_sensor_ = 0;
  float yield_yesterday_sensor_ = 0;
  float yield_today_sensor_ = 0;
  int panel_voltage_sensor_ = 0;
  int panel_power_sensor_ = 0;
  int battery_voltage_sensor_ = 0;
  int battery_voltage_2_sensor_ = 0;
  int battery_voltage_3_sensor_ = 0;
  int auxiliary_battery_voltage_sensor_ = 0;
  int midpoint_voltage_of_the_battery_bank_sensor_ = 0;
  int midpoint_deviation_of_the_battery_bank_sensor_ = 0;
  int battery_current_sensor_ = 0;
  int battery_current_2_sensor_ = 0;
  int battery_current_3_sensor_ = 0;
  int ac_out_voltage_sensor_ = 0;
  int ac_out_current_sensor_ = 0;
  int ac_out_apparent_power_sensor_ = 0;
  int load_current_sensor_ = 0;
  int day_number_sensor_ = 0;
  int device_mode_sensor_ = 0;
  int charging_mode_id_sensor_ = 0;
  int error_code_sensor_ = 0;
  int warning_code_sensor_ = 0;
  int tracking_mode_id_sensor_ = 0;
  int device_mode_id_sensor_ = 0;
  int dc_monitor_mode_id_sensor_ = 0;
  int off_reason_bitmask_sensor_ = 0;


  bool load_state_binary_sensor_;

  std::string alarm_condition_active_text_sensor_;

  std::string charging_mode_text_sensor_;
  std::string error_text_sensor_;
  std::string warning_text_sensor_;
  std::string tracking_mode_text_sensor_;
  std::string device_mode_text_sensor_;
  std::string firmware_version_text_sensor_;
  std::string firmware_version_24bit_text_sensor_;
  std::string device_type_text_sensor_;
  std::string serial_number_text_sensor_;
  std::string hardware_revision_text_sensor_;
  std::string dc_monitor_mode_text_sensor_;
  std::string off_reason_text_sensor_;

};

}  // namespace victron

#endif /* End of VICTRON_MPPT */