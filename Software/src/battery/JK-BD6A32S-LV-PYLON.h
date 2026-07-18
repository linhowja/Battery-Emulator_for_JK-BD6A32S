#ifndef JK_BD6A32S_LV_PYLON_H
#define JK_BD6A32S_LV_PYLON_H

#include "../datalayer/datalayer.h"
#include "CanBattery.h"

extern uint16_t user_selected_pylon_baudrate;

class JkBd6A32sLvPylonBattery : public CanBattery {
 public:
  JkBd6A32sLvPylonBattery(DATALAYER_BATTERY_TYPE* datalayer_ptr, bool* contactor_closing_allowed_ptr, CAN_Interface targetCan)
      : CanBattery(targetCan,
                   user_selected_pylon_baudrate == 500 ? CAN_Speed::CAN_SPEED_500KBPS : CAN_Speed::CAN_SPEED_250KBPS) {
    datalayer_battery = datalayer_ptr;
    contactor_closing_allowed = contactor_closing_allowed_ptr;
    allows_contactor_closing = nullptr;
  }

  JkBd6A32sLvPylonBattery()
      : CanBattery(user_selected_pylon_baudrate == 500 ? CAN_Speed::CAN_SPEED_500KBPS : CAN_Speed::CAN_SPEED_250KBPS) {
    datalayer_battery = &datalayer.battery;
    allows_contactor_closing = &datalayer.system.status.battery_allows_contactor_closing;
    contactor_closing_allowed = nullptr;
  }

  virtual void setup(void);
  virtual void handle_incoming_can_frame(CAN_frame rx_frame);
  virtual void update_values();
  virtual void transmit_can(unsigned long currentMillis);
  static constexpr const char* Name = "JK BD6A32S (Pylon LV)";

 private:
  static const int MAX_CELL_DEVIATION_MV = 150;
  static const int MAX_CELLS = 192;

  DATALAYER_BATTERY_TYPE* datalayer_battery;
  bool* allows_contactor_closing;
  bool* contactor_closing_allowed;

  unsigned long previousMillis1000 = 0;
  unsigned long previousMillis5000 = 0;

  CAN_frame PYLON_3010 = {.FD = false, .ext_ID = true, .DLC = 8, .ID = 0x3010, .data = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
  CAN_frame PYLON_8200 = {.FD = false, .ext_ID = true, .DLC = 8, .ID = 0x8200, .data = {0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
  CAN_frame PYLON_8210 = {.FD = false, .ext_ID = true, .DLC = 8, .ID = 0x8210, .data = {0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
  CAN_frame PYLON_4200 = {.FD = false, .ext_ID = true, .DLC = 8, .ID = 0x4200, .data = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
  CAN_frame EMUS_CELL_VOLTAGE_REQUEST = {.FD = false, .ext_ID = true, .DLC = 1, .ID = 0x19B50100, .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
  CAN_frame EMUS_CELL_BALANCING_REQUEST = {.FD = false, .ext_ID = true, .DLC = 1, .ID = 0x19B50300, .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

  int16_t celltemperature_max_dC = 0;
  int16_t celltemperature_min_dC = 0;
  int16_t current_dA = 0;
  uint16_t total_capacity_Wh = 0;
  uint16_t remaining_capacity_Wh = 0;
  uint16_t voltage_dV = 0;
  uint16_t cellvoltage_max_mV = 3300;
  uint16_t cellvoltage_min_mV = 3300;
  uint16_t charge_cutoff_voltage = 0;
  uint16_t discharge_cutoff_voltage = 0;
  int16_t max_charge_current_dA = 0;
  int16_t max_discharge_current_dA = 0;
  int16_t BMS_temperature_dC = 0;
  uint8_t battery_module_quantity = 0;
  uint8_t battery_modules_in_series = 0;
  uint8_t cell_quantity_in_module = 0;
  uint8_t voltage_level = 0;
  uint8_t ah_number = 0;
  uint16_t SOC = 50;
  uint16_t SOH = 100;
  uint8_t charge_forbidden = 0;
  uint8_t discharge_forbidden = 0;
  uint8_t manufacturer_name[16] = {0};
  uint8_t mux = 0;
  uint8_t hardware_version = 0;
  uint8_t hardware_version_V = 0;
  uint8_t hardware_version_R = 0;
  uint8_t software_version_major = 0;
  uint8_t software_version_minor = 0;
  uint8_t actual_cell_count = 0;

  uint16_t pack_voltage_10mV = 0;
  uint8_t raw_370[8] = {0};
  uint8_t raw_371[8] = {0};
};

#endif
