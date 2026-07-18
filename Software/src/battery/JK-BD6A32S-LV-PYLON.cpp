#include "JK-BD6A32S-LV-PYLON.h"
#include <cstring>
#include "../battery/BATTERIES.h"
#include "../communication/can/comm_can.h"
#include "../datalayer/datalayer.h"
#include "../devboard/utils/events.h"

void JkBd6A32sLvPylonBattery::update_values() {
  voltage_dV = (pack_voltage_10mV / 10);

  datalayer_battery->status.real_soc = (SOC * 100);
  datalayer_battery->status.soh_pptt = (SOH * 100);
  datalayer_battery->status.voltage_dV = voltage_dV;
  datalayer_battery->status.current_dA = current_dA;

  datalayer_battery->status.max_charge_power_W = ((max_charge_current_dA / 10) * (voltage_dV / 10));
  datalayer_battery->status.max_discharge_power_W = ((max_discharge_current_dA / 10) * (voltage_dV / 10));

  if (total_capacity_Wh > 0) {
    datalayer_battery->info.total_capacity_Wh = total_capacity_Wh;
  }

  if (remaining_capacity_Wh > 0) {
    datalayer_battery->status.remaining_capacity_Wh = remaining_capacity_Wh;
  } else {
    datalayer_battery->status.remaining_capacity_Wh = static_cast<uint32_t>(
        (static_cast<double>(datalayer_battery->status.real_soc) / 10000) * datalayer_battery->info.total_capacity_Wh);
  }

  if (actual_cell_count > 0) {
    datalayer_battery->info.number_of_cells = actual_cell_count;
  }

  datalayer_battery->status.cell_max_voltage_mV = cellvoltage_max_mV;
  datalayer_battery->status.cell_min_voltage_mV = cellvoltage_min_mV;

  if (actual_cell_count == 0) {
    datalayer_battery->status.cell_voltages_mV[0] = cellvoltage_max_mV;
    datalayer_battery->status.cell_voltages_mV[1] = cellvoltage_min_mV;
  }

  datalayer_battery->status.temperature_min_dC = celltemperature_min_dC;
  datalayer_battery->status.temperature_max_dC = celltemperature_max_dC;

  if (user_selected_max_pack_voltage_dV == 0) {
    datalayer_battery->info.max_design_voltage_dV = charge_cutoff_voltage;
  }
  if (user_selected_min_pack_voltage_dV == 0) {
    datalayer_battery->info.min_design_voltage_dV = discharge_cutoff_voltage;
  }
}

void JkBd6A32sLvPylonBattery::handle_incoming_can_frame(CAN_frame rx_frame) {
  switch (rx_frame.ID) {
    case 0x351: 
      // Evidence C: canlog_20260718_0831.txt (Frame: 36 04 B0 04 FB 04 48 03)
      // Byte0-1: Charge Voltage Limit, Byte2-3: Max Charge Current, Byte4-5: Max Discharge Current
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      charge_cutoff_voltage = ((rx_frame.data.u8[1] << 8) | rx_frame.data.u8[0]);
      max_charge_current_dA = (int16_t)((rx_frame.data.u8[3] << 8) | rx_frame.data.u8[2]);
      max_discharge_current_dA = (int16_t)((rx_frame.data.u8[5] << 8) | rx_frame.data.u8[4]);
      break;

    case 0x355: 
      // Evidence C: canlog_20260718_0831.txt (Frame: 45 00 64 00 00 00 00 00)
      // Byte0-1: SOC, Byte2-3: SOH
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      SOC = ((rx_frame.data.u8[1] << 8) | rx_frame.data.u8[0]);
      SOH = ((rx_frame.data.u8[3] << 8) | rx_frame.data.u8[2]);
      break;

    case 0x356: 
      // Evidence C: canlog_20260718_0831.txt (Frame: F3 26 00 00 28 01 00 00)
      // Byte0-1: Pack Voltage (10mV), Byte2-3: Current (dA), Byte4-5: Temp (0.1C)
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      pack_voltage_10mV = ((rx_frame.data.u8[1] << 8) | rx_frame.data.u8[0]);
      current_dA = (int16_t)((rx_frame.data.u8[3] << 8) | rx_frame.data.u8[2]);
      celltemperature_max_dC = (int16_t)((rx_frame.data.u8[5] << 8) | rx_frame.data.u8[4]);
      celltemperature_min_dC = celltemperature_max_dC; 
      break;

    case 0x359: 
    case 0x35C:
      // Evidence C: canlog_20260718_0831.txt
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      break;

    case 0x35E: 
      // Evidence C: canlog_20260718_0831.txt (Frame: 4A 4B 2D 42 4D 53 00 00)
      // Byte0-7: Manufacturer Name ("JK-BMS")
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      memcpy(manufacturer_name, rx_frame.data.u8, 8);
      break;

    case 0x370: 
      // Evidence C: canlog_20260718_0831.txt (JK Proprietary Extension)
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      memcpy(raw_370, rx_frame.data.u8, 8);
      break;

    case 0x371: 
      // Evidence C: canlog_20260718_0831.txt (JK Proprietary Extension)
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      memcpy(raw_371, rx_frame.data.u8, 8);
      break;

    default:
      break;
  }
}

void JkBd6A32sLvPylonBattery::transmit_can(unsigned long currentMillis) {
  if (currentMillis - previousMillis1000 >= 1000) { 
    previousMillis1000 = currentMillis;

    PYLON_8200.data.u8[0] = 0xAA;
    PYLON_8210.data.u8[0] = 0xAA;
    PYLON_8210.data.u8[1] = 0x00;
    
    // transmit_can_frame(&PYLON_3010);
    // transmit_can_frame(&PYLON_4200);
    // transmit_can_frame(&PYLON_8200);
    // transmit_can_frame(&PYLON_8210);

    mux = (mux + 1) % 3;
    PYLON_4200.data.u8[0] = mux;
  }

  if (currentMillis - previousMillis5000 >= 5000) { 
    previousMillis5000 = currentMillis;
    // transmit_can_frame(&EMUS_CELL_VOLTAGE_REQUEST);
    // transmit_can_frame(&EMUS_CELL_BALANCING_REQUEST);
  }
}

void JkBd6A32sLvPylonBattery::setup(void) {
  strncpy(datalayer.system.info.battery_protocol, Name, 63);
  datalayer.system.info.battery_protocol[63] = '\0';
  datalayer_battery->info.number_of_cells = 2;
  
  if (user_selected_max_pack_voltage_dV > 0) {
    datalayer_battery->info.max_design_voltage_dV = user_selected_max_pack_voltage_dV;
  }
  if (user_selected_min_pack_voltage_dV > 0) {
    datalayer_battery->info.min_design_voltage_dV = user_selected_min_pack_voltage_dV;
  }
  if (user_selected_max_cell_voltage_mV > 0) {
    datalayer_battery->info.max_cell_voltage_mV = user_selected_max_cell_voltage_mV;
  }
  if (user_selected_min_cell_voltage_mV > 0) {
    datalayer_battery->info.min_cell_voltage_mV = user_selected_min_cell_voltage_mV;
  }
  
  datalayer_battery->info.max_cell_voltage_deviation_mV = MAX_CELL_DEVIATION_MV;

  if (allows_contactor_closing) {
    *allows_contactor_closing = true;
  }
}
