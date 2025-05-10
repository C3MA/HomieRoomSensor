#pragma once



    std::string tracking_mode_text(int value) {
        switch (value) {
            case 0:
            return "Off";
            case 1:
            return "Limited";
            case 2:
            return "Active";
            default:
            return "Unknown";
        }
    }
 
    std::string error_code_text(int value) {
        switch (value) {
            case 0:
            return "No error";
            case 2:
            return "Battery voltage too high";
            case 17:
            return "Charger temperature too high";
            case 18:
            return "Charger over current";
            case 19:
            return "Charger current reversed";
            case 20:
            return "Bulk time limit exceeded";
            case 21:
            return "Current sensor issue";
            case 26:
            return "Terminals overheated";
            case 28:
            return "Converter issue";
            case 33:
            return "Input voltage too high (solar panel)";
            case 34:
            return "Input current too high (solar panel)";
            case 38:
            return "Input shutdown (excessive battery voltage)";
            case 39:
            return "Input shutdown (due to current flow during off mode)";
            case 65:
            return "Lost communication with one of devices";
            case 66:
            return "Synchronised charging device configuration issue";
            case 67:
            return "BMS connection lost";
            case 68:
            return "Network misconfigured";
            case 116:
            return "Factory calibration data lost";
            case 117:
            return "Invalid/incompatible firmware";
            case 119:
            return "User settings invalid";
            default:
            return "Unknown";
        }
    }



    std::string charging_mode_text(int value) {
        switch (value) {
            case 0:
            return "Off";
            case 1:
            return "Low power";
            case 2:
            return "Fault";
            case 3:
            return "Bulk";
            case 4:
            return "Absorption";
            case 5:
            return "Float";
            case 6:
            return "Storage";
            case 7:
            return "Equalize (manual)";
            case 9:
            return "Inverting";
            case 11:
            return "Power supply";
            case 245:
            return "Starting-up";
            case 246:
            return "Repeated absorption";
            case 247:
            return "Auto equalize / Recondition";
            case 248:
            return "BatterySafe";
            case 252:
            return "External control";
            default:
            return "Unknown";
        }
    }



    std::string device_type_text(long value)
    {
        switch (value) {
            case 0x203:
            return "BMV-700";
            case 0x204:
            return "BMV-702";
            case 0x205:
            return "BMV-700H";
            case 0x0300:
            return "BlueSolar MPPT 70|15";
            case 0xA040:
            return "BlueSolar MPPT 75|50";
            case 0xA041:
            return "BlueSolar MPPT 150|35";
            case 0xA042:
            return "BlueSolar MPPT 75|15";
            case 0xA043:
            return "BlueSolar MPPT 100|15";
            case 0xA044:
            return "BlueSolar MPPT 100|30";
            case 0xA045:
            return "BlueSolar MPPT 100|50";
            case 0xA046:
            return "BlueSolar MPPT 150|70";
            case 0xA047:
            return "BlueSolar MPPT 150|100";
            case 0xA049:
            return "BlueSolar MPPT 100|50 rev2";
            case 0xA04A:
            return "BlueSolar MPPT 100|30 rev2";
            case 0xA04B:
            return "BlueSolar MPPT 150|35 rev2";
            case 0xA04C:
            return "BlueSolar MPPT 75|10";
            case 0xA04D:
            return "BlueSolar MPPT 150|45";
            case 0xA04E:
            return "BlueSolar MPPT 150|60";
            case 0xA04F:
            return "BlueSolar MPPT 150|85";
            case 0xA050:
            return "SmartSolar MPPT 250|100";
            case 0xA051:
            return "SmartSolar MPPT 150|100";
            case 0xA052:
            return "SmartSolar MPPT 150|85";
            case 0xA053:
            return "SmartSolar MPPT 75|15";
            case 0xA075:
            return "SmartSolar MPPT 75|15 rev2";
            case 0xA054:
            return "SmartSolar MPPT 75|10";
            case 0xA074:
            return "SmartSolar MPPT 75|10 rev2";
            case 0xA055:
            return "SmartSolar MPPT 100|15";
            case 0xA056:
            return "SmartSolar MPPT 100|30";
            case 0xA073:
            return "SmartSolar MPPT 150|45 rev3";
            case 0xA057:
            return "SmartSolar MPPT 100|50";
            case 0xA058:
            return "SmartSolar MPPT 150|35";
            case 0xA059:
            return "SmartSolar MPPT 150|100 rev2";
            case 0xA05A:
            return "SmartSolar MPPT 150|85 rev2";
            case 0xA05B:
            return "SmartSolar MPPT 250|70";
            case 0xA05C:
            return "SmartSolar MPPT 250|85";
            case 0xA05D:
            return "SmartSolar MPPT 250|60";
            case 0xA05E:
            return "SmartSolar MPPT 250|45";
            case 0xA05F:
            return "SmartSolar MPPT 100|20";
            case 0xA060:
            return "SmartSolar MPPT 100|20 48V";
            case 0xA061:
            return "SmartSolar MPPT 150|45";
            case 0xA062:
            return "SmartSolar MPPT 150|60";
            case 0xA063:
            return "SmartSolar MPPT 150|70";
            case 0xA064:
            return "SmartSolar MPPT 250|85 rev2";
            case 0xA065:
            return "SmartSolar MPPT 250|100 rev2";
            case 0xA066:
            return "BlueSolar MPPT 100|20";
            case 0xA067:
            return "BlueSolar MPPT 100|20 48V";
            case 0xA068:
            return "SmartSolar MPPT 250|60 rev2";
            case 0xA069:
            return "SmartSolar MPPT 250|70 rev2";
            case 0xA06A:
            return "SmartSolar MPPT 150|45 rev2";
            case 0xA06B:
            return "SmartSolar MPPT 150|60 rev2";
            case 0xA06C:
            return "SmartSolar MPPT 150|70 rev2";
            case 0xA06D:
            return "SmartSolar MPPT 150|85 rev3";
            case 0xA06E:
            return "SmartSolar MPPT 150|100 rev3";
            case 0xA06F:
            return "BlueSolar MPPT 150|45 rev2";
            case 0xA070:
            return "BlueSolar MPPT 150|60 rev2";
            case 0xA071:
            return "BlueSolar MPPT 150|70 rev2";
            case 0xA07D:
            return "BlueSolar MPPT 75|15 rev3";
            case 0xA102:
            return "SmartSolar MPPT VE.Can 150/70";
            case 0xA103:
            return "SmartSolar MPPT VE.Can 150/45";
            case 0xA104:
            return "SmartSolar MPPT VE.Can 150/60";
            case 0xA105:
            return "SmartSolar MPPT VE.Can 150/85";
            case 0xA106:
            return "SmartSolar MPPT VE.Can 150/100";
            case 0xA107:
            return "SmartSolar MPPT VE.Can 250/45";
            case 0xA108:
            return "SmartSolar MPPT VE.Can 250/60";
            case 0xA109:
            return "SmartSolar MPPT VE.Can 250/70";
            case 0xA10A:
            return "SmartSolar MPPT VE.Can 250/85";
            case 0xA10B:
            return "SmartSolar MPPT VE.Can 250/100";
            case 0xA10C:
            return "SmartSolar MPPT VE.Can 150/70 rev2";
            case 0xA10D:
            return "SmartSolar MPPT VE.Can 150/85 rev2";
            case 0xA10E:
            return "SmartSolar MPPT VE.Can 150/100 rev2";
            case 0xA10F:
            return "BlueSolar MPPT VE.Can 150/100";
            case 0xA112:
            return "BlueSolar MPPT VE.Can 250/70";
            case 0xA113:
            return "BlueSolar MPPT VE.Can 250/100";
            case 0xA114:
            return "SmartSolar MPPT VE.Can 250/70 rev2";
            case 0xA115:
            return "SmartSolar MPPT VE.Can 250/100 rev2";
            case 0xA116:
            return "SmartSolar MPPT VE.Can 250/85 rev2";
            case 0xA201:
            return "Phoenix Inverter 12V 250VA 230V";
            case 0xA202:
            return "Phoenix Inverter 24V 250VA 230V";
            case 0xA204:
            return "Phoenix Inverter 48V 250VA 230V";
            case 0xA211:
            return "Phoenix Inverter 12V 375VA 230V";
            case 0xA212:
            return "Phoenix Inverter 24V 375VA 230V";
            case 0xA214:
            return "Phoenix Inverter 48V 375VA 230V";
            case 0xA221:
            return "Phoenix Inverter 12V 500VA 230V";
            case 0xA222:
            return "Phoenix Inverter 24V 500VA 230V";
            case 0xA224:
            return "Phoenix Inverter 48V 500VA 230V";
            case 0xA231:
            return "Phoenix Inverter 12V 250VA 230V";
            case 0xA232:
            return "Phoenix Inverter 24V 250VA 230V";
            case 0xA234:
            return "Phoenix Inverter 48V 250VA 230V";
            case 0xA239:
            return "Phoenix Inverter 12V 250VA 120V";
            case 0xA23A:
            return "Phoenix Inverter 24V 250VA 120V";
            case 0xA23C:
            return "Phoenix Inverter 48V 250VA 120V";
            case 0xA241:
            return "Phoenix Inverter 12V 375VA 230V";
            case 0xA242:
            return "Phoenix Inverter 24V 375VA 230V";
            case 0xA244:
            return "Phoenix Inverter 48V 375VA 230V";
            case 0xA249:
            return "Phoenix Inverter 12V 375VA 120V";
            case 0xA24A:
            return "Phoenix Inverter 24V 375VA 120V";
            case 0xA24C:
            return "Phoenix Inverter 48V 375VA 120V";
            case 0xA251:
            return "Phoenix Inverter 12V 500VA 230V";
            case 0xA252:
            return "Phoenix Inverter 24V 500VA 230V";
            case 0xA254:
            return "Phoenix Inverter 48V 500VA 230V";
            case 0xA259:
            return "Phoenix Inverter 12V 500VA 120V";
            case 0xA25A:
            return "Phoenix Inverter 24V 500VA 120V";
            case 0xA25C:
            return "Phoenix Inverter 48V 500VA 120V";
            case 0xA261:
            return "Phoenix Inverter 12V 800VA 230V";
            case 0xA262:
            return "Phoenix Inverter 24V 800VA 230V";
            case 0xA264:
            return "Phoenix Inverter 48V 800VA 230V";
            case 0xA269:
            return "Phoenix Inverter 12V 800VA 120V";
            case 0xA26A:
            return "Phoenix Inverter 24V 800VA 120V";
            case 0xA26C:
            return "Phoenix Inverter 48V 800VA 120V";
            case 0xA271:
            return "Phoenix Inverter 12V 1200VA 230V";
            case 0xA272:
            return "Phoenix Inverter 24V 1200VA 230V";
            case 0xA274:
            return "Phoenix Inverter 48V 1200VA 230V";
            case 0xA279:
            case 0xA2F9:
            return "Phoenix Inverter 12V 1200VA 120V";
            case 0xA27A:
            return "Phoenix Inverter 24V 1200VA 120V";
            case 0xA27C:
            return "Phoenix Inverter 48V 1200VA 120V";
            case 0xA281:
            return "Phoenix Inverter 12V 1600VA 230V";
            case 0xA282:
            return "Phoenix Inverter 24V 1600VA 230V";
            case 0xA284:
            return "Phoenix Inverter 48V 1600VA 230V";
            case 0xA291:
            return "Phoenix Inverter 12V 2000VA 230V";
            case 0xA292:
            return "Phoenix Inverter 24V 2000VA 230V";
            case 0xA294:
            return "Phoenix Inverter 48V 2000VA 230V";
            case 0xA2A1:
            return "Phoenix Inverter 12V 3000VA 230V";
            case 0xA2A2:
            return "Phoenix Inverter 24V 3000VA 230V";
            case 0xA2A4:
            return "Phoenix Inverter 48V 3000VA 230V";
            case 0xA30A:
            return "Blue Smart IP65 Charger 12|25";
            case 0xA332:
            return "Blue Smart IP22 Charger 24|8";
            case 0xA334:
            return "Blue Smart IP22 Charger 24|12";
            case 0xA336:
            return "Blue Smart IP22 Charger 24|16";
            case 0xA340:
            return "Phoenix Smart IP43 Charger 12|50 (1+1)";
            case 0xA341:
            return "Phoenix Smart IP43 Charger 12|50 (3)";
            case 0xA342:
            return "Phoenix Smart IP43 Charger 24|25 (1+1)";
            case 0xA343:
            return "Phoenix Smart IP43 Charger 24|25 (3)";
            case 0xA344:
            return "Phoenix Smart IP43 Charger 12|30 (1+1)";
            case 0xA345:
            return "Phoenix Smart IP43 Charger 12|30 (3)";
            case 0xA346:
            return "Phoenix Smart IP43 Charger 24|16 (1+1)";
            case 0xA347:
            return "Phoenix Smart IP43 Charger 24|16 (3)";
            case 0xA381:
            return "BMV-712 Smart";
            case 0xA382:
            return "BMV-710H Smart";
            case 0xA383:
            return "BMV-712 Smart Rev2";
            case 0xA389:
            return "SmartShunt 500A/50mV";
            case 0xA38A:
            return "SmartShunt 1000A/50mV";
            case 0xA38B:
            return "SmartShunt 2000A/50mV";
            case 0xA442:
            return "Multi RS Solar 48V 6000VA 230V";
            default:
            return "Unknown";
        }
    }