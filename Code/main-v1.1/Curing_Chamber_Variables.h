/****************************************************************************************
 * "Curing_Chamber_Variables.h"
 *
 * Description:
 *
 *   This file contains all of the functions required to handle the menu display
 *   structures. Functions that control the display through a rotary encoder and
 *   menu control logic.
 *
 ****************************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////
//--- HEADER FILES ---//
//////////////////////////////////////////////////////////////////////////////////////////

// Adafruit TFT Libraries
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include "EEPROM.h"          // Flash memory library

// ESP32 TFT I2C initializer
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//////////////////////////////////////////////////////////////////////////////////////////
//--- GLOBAL VARIABLES ---//
//////////////////////////////////////////////////////////////////////////////////////////

bool rotary_A = true, rotary_B = true, button_value = false, update_screen = false, button_press = false;
int menu = 1, option = 1, time_value = 0, temp_value = 0, dial_value = 0, time_min = 1, temp = 1;

/****************************************************************************************
 * startup_screen:
 *
 *   Executes the startup screen routine. Displays the team's name, device name, and
 *   an "Initialization . . ." delay with a scrolling screen refresh.
 *
 * @param   - none
 * @returns - none
 ****************************************************************************************/

void startup_screen() {
  // Sets Up Screen and Text
  tft.fillScreen(0xC01F);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(3);

  // Displays Team Name
  tft.setCursor(15, 12);
  tft.println("UV");
  tft.setCursor(34, 37);
  tft.println("VIKINGS");

  // Displays Device Name
  tft.setTextSize(2);
  tft.setCursor(30, 70);
  tft.println("Curing");
  tft.setCursor(50, 90);
  tft.println("Chamber");

  // "Initializing" Text
  tft.setTextSize(1);
  tft.setCursor(5, 115);
  tft.println("Initializing");
  delay(750);
  tft.setCursor(75, 115);
  tft.println(" .");
  delay(750);
  tft.setCursor(85, 115);
  tft.println(" .");
  delay(750);
  tft.setCursor(95, 115);
  tft.println(" .");
  delay(1250);

  // Screen Refresh for Menu
  for(int i = 0; i <= 260; i++)
    for(int j = 0; j <= 160; j++) {
      tft.drawPixel(j, i, 0xC000 + (50 * i));
      tft.drawPixel(j, i-5, 0xC01F);
    }
}

/****************************************************************************************
 * initialize_menu
 *
 *   This sets up the menu backscreen color and the side arrows.
 *
 * @param   - Nothing.
 * @returns - Nothing.
 ****************************************************************************************/

void intialize_menu() {
  // Reset Menu Field
  tft.fillRect(5, 5, 150, 20, 0xC01F);

  // Format the Text
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);

  // Print Header
  tft.setCursor(15, 5);
  tft.println("SELECT CURE");
  tft.drawLine(15, 21, 145, 21, 0xFFFF);
  tft.drawLine(15, 22, 145, 22, 0xFFFF);
  tft.drawLine(15, 23, 145, 23, 0xFFFF);

  tft.fillTriangle(140, 45, 145, 35, 150, 45, 0xFFFF);
  tft.fillCircle(145, 77, 5, 0xFFFF);
  tft.fillTriangle(140, 110, 145, 120, 150, 110, 0xFFFF);
}

/****************************************************************************************
 * poll_button
 *
 *   This function polls the rotary encoder button. If the button is pressed, the function
 *   returns true, else it returns false.
 *
 * @param   - Noting.
 * @returns - Boolean: True if button is pressed, False if it is not.
 ****************************************************************************************/

bool poll_button () {
  button_press = digitalRead(BUTT_PRESS);
  if(!button_press) {
    while(!button_press)
      button_press = digitalRead(BUTT_PRESS);
    return true;
  }
  return false;
}

/****************************************************************************************
 * poll_rotary_dial
 *
 *   This function polls the rotary encoder dial. If the dial turns clockwise, a '1' is
 *   returned. If the dial turns counter-clockwise, a '-1' is returned. Else, '0' is
 *   returned.
 *
 * @param   - Noting.
 * @returns - Integer: '1' for clockwise, '-1'  for counter-clockwise, '0' otherwise.
 ****************************************************************************************/

int poll_rotary_dial() {
  rotary_A = digitalRead(ROT_A);
  rotary_B = digitalRead(ROT_B);
  if(!rotary_A) {
    if(!rotary_B){
      while (!digitalRead(ROT_B) || !digitalRead(ROT_A))
        delay(1);
      return 1;
    }
    else if(rotary_B) {
      while(!digitalRead(ROT_A) || !digitalRead(ROT_B))
        delay(1);
      return -1;
    }
    return 0;
  }
  return 0;
}

void update_menu_header() {

  // Reset Menu Field
  tft.fillRect(5, 5, 150, 20, 0xC01F);

  // Format the Text
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);

  switch(menu) {
    case 1: tft.setCursor(15, 5);
            tft.println("SELECT CURE");
            tft.drawLine(15, 21, 145, 21, 0xFFFF);
            tft.drawLine(15, 22, 145, 22, 0xFFFF);
            tft.drawLine(15, 23, 145, 23, 0xFFFF);
            break;
    case 2: tft.setCursor(15, 5);
            tft.println("SELECT TEMP");
            tft.drawLine(15, 21, 145, 21, 0xFFFF);
            tft.drawLine(15, 22, 145, 22, 0xFFFF);
            tft.drawLine(15, 23, 145, 23, 0xFFFF);
            break;
    case 3: tft.setCursor(15, 5);
            tft.println("SELECT TIME");
            tft.drawLine(15, 21, 145, 21, 0xFFFF);
            tft.drawLine(15, 22, 145, 22, 0xFFFF);
            tft.drawLine(15, 23, 145, 23, 0xFFFF);
            break;
    case 4: tft.setCursor(35, 5);
            tft.println("SAVE OPT");
            tft.drawLine(35, 21, 125, 21, 0xFFFF);
            tft.drawLine(35, 22, 125, 22, 0xFFFF);
            tft.drawLine(35, 23, 125, 23, 0xFFFF);
            break;
    case 5: tft.setCursor(35, 5);
            tft.println("CONFIRM?");
            tft.drawLine(35, 21, 125, 21, 0xFFFF);
            tft.drawLine(35, 22, 125, 22, 0xFFFF);
            tft.drawLine(35, 23, 125, 23, 0xFFFF);
            break;
    case 6: tft.setCursor(50, 5);
            tft.println("CURING");
            tft.drawLine(50, 21, 105, 21, 0xFFFF);
            tft.drawLine(50, 22, 105, 22, 0xFFFF);
            tft.drawLine(50, 23, 105, 23, 0xFFFF);
            break;
    default:tft.setCursor(50, 5);
            tft.println("PAUSE");
            tft.drawLine(50, 21, 105, 21, 0xFFFF);
            tft.drawLine(50, 22, 105, 22, 0xFFFF);
            tft.drawLine(50, 23, 105, 23, 0xFFFF);
            break;
  }

}


/****************************************************************************************
 * menu_options
 *
 *   This function handles the text fields for
 *
 * @param   - int menu_type: to determine how the fields should be laid out.
 *            int region: to determine which region is being written to.
 *            String heading: the heading bein written to the region.
 *            int time_min: the time in minutes being written to specific text fields.
 *            int temp: the temperature value being written to specific text fields.
 * @returns - Nothing.
 ****************************************************************************************/


void menu_options(int menu_type, int region, String heading, int time_min, int temp) {
  switch (menu_type) { // Allows the user to view various menus with menu-specific text regions

    case 1: // Curing Settings Menu
      switch (region) {
        case 1:
          tft.fillRect(10, 30, 125, 17, 0xC01F);
          tft.setTextColor(0xFFFF);
          tft.setCursor(10, 30);
          tft.println(heading);
          break;
        case 2:
          tft.fillRect(10, 50, 125, 55, 0x8812);
          tft.setTextColor(0xFFFF);
          tft.setCursor(15, 55);
          tft.println(heading);
          tft.setTextSize(1);
          tft.setCursor(50, 75);
          tft.println("Time:");
          tft.setCursor(85, 75);
          tft.println(time_min);
          tft.setCursor(105, 75);
          tft.println("min");
          tft.setCursor(50, 90);
          tft.println("Temp:");
          tft.setCursor(85, 90);
          tft.println(temp);
          tft.setCursor(110, 90);
          tft.println("C");
          tft.setTextSize(2);
          break;
        case 3:
          tft.fillRect(10, 110, 125, 17, 0xC01F);
          tft.setTextColor(0xFFFF);
          tft.setCursor(10, 110);
          tft.println(heading);
          break;
      break;
    }
    break;

    case 2: // Custom/Preset Time Menu
      switch (region) {
        case 1:
          tft.fillRect(10, 30, 125, 17, 0xC01F);
          tft.setTextColor(0xFFFF);
          tft.setCursor(10, 30);
          tft.println("Set Temp:");
          break;
        case 2:
          tft.fillRect(10, 50, 125, 55, 0x8812);
          tft.setTextColor(0xFFFF);
          tft.setCursor(15, 70);
          tft.println(time_min);
          break;
        case 3:
          tft.fillRect(10, 110, 125, 17, 0xC01F);
          break;
        }
        break;

    case 3: // Custom/Preset Temp Menu
      switch (region) {
        case 1:
          tft.fillRect(10, 30, 125, 17, 0xC01F);
          tft.setTextColor(0xFFFF);
          tft.setCursor(10, 30);
          tft.println("Set Time:");
          break;
        case 2:
          tft.fillRect(10, 50, 125, 55, 0x8812);
          tft.setTextColor(0xFFFF);
          tft.setCursor(15, 70);
          tft.println(temp);
          break;
        case 3:
          tft.fillRect(10, 110, 125, 17, 0xC01F);
          break;
        }
        break;

    case 4: // Confirm Settings + Pause Menu
      switch (region) {
        case 1:
          tft.fillRect(10, 30, 125, 17, 0xC01F);
          tft.setTextColor(0xFFFF);
          tft.setCursor(10, 30);
          tft.println(heading);
          break;
        case 2:
          tft.fillRect(10, 50, 125, 55, 0x8812);
          tft.setTextColor(0xFFFF);
          tft.setCursor(15, 70);
          tft.println(heading);
          break;
        case 3:
          tft.fillRect(10, 110, 125, 17, 0xC01F);
          tft.setTextColor(0xFFFF);
          tft.setCursor(10, 110);
          tft.println(heading);
          break;
        }
        break;
        default: return;
    }
    return;
  }


/****************************************************************************************
 * menu_1
 *
 *   This fucntion handles what to display in the 3 menu-option text fields for menu #1.
 *   Menu #1 is the top menu that appears just after boot-up.
 *
 * @param   - int option: menu_1 options.
 * @returns - Nothing.
 ****************************************************************************************/

void menu_1(int option) { // Curing Settings Menu
  switch (option) {
    case 1: menu_options(1, 1, " ", 0, 0);
            menu_options(1, 2, "Regular", regResin[0], regResin[1]);
            menu_options(1, 3, "Tough", 0, 0);
            break;
    case 2: menu_options(1, 1, "Regular", 0, 0);
            menu_options(1, 2, "Tough", toughResin[0], toughResin[1]);
            menu_options(1, 3, "High Temp", 0, 0);
            break;
    case 3: menu_options(1, 1, "Tough", 0, 0);
            menu_options(1, 2, "High Temp", highResin[0], highResin[1]);
            menu_options(1, 3, "Resin 0", 0, 0);
            break;
    case 4: menu_options(1, 1, "High Temp", 0, 0);
            menu_options(1, 2, "Resin 0", colorResin[0], colorResin[1]);
            menu_options(1, 3, "Resin 1", 0, 0);
            break;
    case 5: menu_options(1, 1, "Resin 0", 0, 0);
            menu_options(1, 2, "Resin 1", gpResin[0], gpResin[1]);
            menu_options(1, 3, "Resin 2", 0, 0);
            break;
    case 6: menu_options(1, 1, "Resin 1", 0, 0);
            menu_options(1, 2, "Resin 2", rigidResin[0], rigidResin[1]);
            menu_options(1, 3, "Resin 3", 0, 0);
            break;
    case 7: menu_options(1, 1, "Resin 2", 0, 0);
            menu_options(1, 2, "Resin 3", durResin[0], durResin[1]);
            menu_options(1, 3, "Resin 4", 0, 0);
            break;
    case 8: menu_options(1, 1, "Resin 3", 0, 0);
            menu_options(1, 2, "Resin 4", castResin[0], castResin[1]);
            menu_options(1, 3, "Preset 1", 0, 0);
            break;
    case 9: menu_options(1, 1, "Resin 4", 0, 0);
            menu_options(1, 2, "Preset 1", preset1Resin[0], preset1Resin[1]);
            menu_options(1, 3, "Preset 2", 0, 0);
            break;
    case 10:menu_options(1, 1, "Preset 1", 0, 0);
            menu_options(1, 2, "Preset 2", preset2Resin[0], preset2Resin[1]);
            menu_options(1, 3, "Preset 3", 0, 0);
            break;
    case 11:menu_options(1, 1, "Preset 2", 0, 0);
            menu_options(1, 2, "Preset 3", preset3Resin[0], preset3Resin[1]);
            menu_options(1, 3, "Preset 4", 0, 0);
            break;
    case 12:menu_options(1, 1, "Preset 3", 0, 0);
            menu_options(1, 2, "Preset 4", preset4Resin[0], preset4Resin[1]);
            menu_options(1, 3, "Preset 5", 0, 0);
            break;
    case 13:menu_options(1, 1, "Preset 4", 0, 0);
            menu_options(1, 2, "Preset 5", preset5Resin[0], preset5Resin[1]);
            menu_options(1, 3, "Custom Set", 0, 0);
            break;
    default:menu_options(1, 1, "Preset 5", 0, 0);
            menu_options(1, 2, "Custom Set", 95, 95);
            menu_options(1, 3, " ", 0, 0);
            break;
  }
}

/****************************************************************************************
 * menu_2
 *
 *   This fucntion handles what to display in the menu-option text field for menu #2.
 *   Menu #2 is the set-time menu that appears just after selecting the Custom Set.
 *
 * @param   - int time_min: menu_2 options.
 * @returns - Nothing.
 ****************************************************************************************/

void menu_2(int time_min) { // Custom/Preset Time Menu
  menu_options(2, 1, " ", 0, 0);
  menu_options(2, 2, " ", time_min, 0);
  menu_options(2, 3, " ", 0, 0);
}

/****************************************************************************************
 * menu_3
 *
 *   This fucntion handles what to display in the menu-option text field for menu #3.
 *   Menu #3 is the set-temp menu that appears just after Menu #2.
 *
 * @param   - int temp: menu_3 options.
 * @returns - Nothing.
 ****************************************************************************************/

void menu_3(int temp) { // Custom/Preset Temp Menu
  menu_options(3, 1, " ", 0, 0);
  menu_options(3, 2, " ", 0, temp);
  menu_options(3, 3, " ", 0, 0);
}

/****************************************************************************************
 * menu_4
 *
 *   This fucntion handles what to display in the 3 menu-option text fields for menu #4.
 *   Menu #4 is the "Save Presets" menu that appears just after Menu #3.
 *
 * @param   - int option: menu_4 options.
 * @returns - Nothing.
 ****************************************************************************************/

void menu_4(int option) { // Curing Settings Menu
  switch (option) {
    case 1: menu_options(1, 1, " ", 0, 0);
            menu_options(1, 2, "Preset 1", preset1Resin[0], preset1Resin[1]);
            menu_options(1, 3, "Preset 2", 0, 0);
            break;
    case 2: menu_options(1, 1, "Preset 1", 0, 0);
            menu_options(1, 2, "Preset 2", preset2Resin[0], preset2Resin[1]);
            menu_options(1, 3, "Preset 3", 0, 0);
            break;
    case 3: menu_options(1, 1, "Preset 2", 0, 0);
            menu_options(1, 2, "Preset 3", preset3Resin[0], preset3Resin[1]);
            menu_options(1, 3, "Preset 4", 0, 0);
            break;
    case 4: menu_options(1, 1, "Preset 3", 0, 0);
            menu_options(1, 2, "Preset 4", preset4Resin[0], preset4Resin[1]);
            menu_options(1, 3, "Preset 5", 0, 0);
            break;
    case 5: menu_options(1, 1, "Preset 4", 0, 0);
            menu_options(1, 2, "Preset 5", preset5Resin[0], preset5Resin[1]);
            menu_options(1, 3, "Dont Save", 0, 0);
            break;
    case 6: menu_options(1, 1, "Preset 5", 0, 0);
            menu_options(1, 2, "Dont Save", 0, 0);
            menu_options(1, 3, "Return", 0, 0);
            break;
    default:menu_options(1, 1, "Dont Save", 0, 0);
            menu_options(1, 2, "Return", 0, 0);
            menu_options(1, 3, " ", 0, 0);
            break;
  }
}

/****************************************************************************************
 * menu_5
 *
 *   This fucntion handles what to display in the menu-option text fields for menu #5.
 *   Menu #5 is the top menu that appears just after Menus #1 & #4.
 *
 * @param   - int option: menu_5 options.
 * @returns - Nothing.
 ****************************************************************************************/

void menu_5(int option) { // Confirm Settings Menu

  switch (option) {
    case 1: menu_options(4, 1, " ", 0, 0);
            menu_options(4, 2, "Confirm", 0, 0);
            menu_options(4, 3, "Return", 0, 0);
            break;
    default:menu_options(4, 1, "Confirm", 0, 0);
            menu_options(4, 2, "Return", 0, 0);
            menu_options(4, 3, " ", 0, 0);
            break;
    }
}

/****************************************************************************************
 * curing_screen
 *
 *   This fucntion handles what to display on the curing screen.
 *
 * @param   - int current_time: curing screen current curing time.
 *          - int end_time: when the curing protocol should end.
 * @returns - Nothing.
 ****************************************************************************************/

void curing_screen(int current_time, int current_temp) { // Pause Menu Settings
  menu_options(1, 1, " ", 0, 0);
  menu_options(1, 2, "Current", current_time, current_temp);
  menu_options(1, 3, " ", 0, 0);
}

/****************************************************************************************
 * menu_pause
 *
 *   This fucntion handles what to display in the 3 menu-option text fields for the pause menu.
 *
 * @param   - int option: menu_pause options.
 * @returns - Nothing.
 ****************************************************************************************/

void menu_pause(int option) { // Pause Menu Settings
  switch (option) {
    case 1: menu_options(4, 1, " ", 0, 0);
            menu_options(4, 2, "Resume", 0, 0);
            menu_options(4, 3, "Menu", 0, 0);
            break;
    default:menu_options(4, 1, "Resume", 0, 0);
            menu_options(4, 2, "Menu", 0, 0);
            menu_options(4, 3, " ", 0, 0);
            break;
  }
}

/****************************************************************************************
 * display_menu
 *
 *   Used to decide which menu and which options to display based on updated menu options.
 *
 * @param   - int option: menu_1 options.
 * @returns - Nothing.
 ****************************************************************************************/

void display_menu(int menu_num, int option_num1, float option_num2) {  // FIXME: Changed option_num2 to float from int
  switch (menu_num) {
    case 0: menu_pause(option_num1);
      break;
    case 1: menu_1(option_num1);
      break;
    case 2: menu_2(option_num1);
      break;
    case 3: menu_3(option_num1);
      break;
    case 4: menu_4(option_num1);
      break;
    case 5: menu_5(option_num1);
      break;
    case 6: curing_screen(option_num1, option_num2);
      break;
    default:menu_pause(option_num1);
  }

}

/****************************************************************************************
 * update_menu_options
 *
 *   This function is in charge of managing all of the logic that drives how the menus
 *   navigate from one another.
 *
 * @param   - int dial_value: the value of the recently turned rotary encoder dial.
 *          - int button_value: the value of the recently pushed select-button.
 * @returns - True if a menu navigation was successful, False otherwise.
 ****************************************************************************************/

bool update_menu_options(int dial_value, bool button_value) {
  // Menu 1 = Resin Types
  if(menu == 1) {
    if((option >= 1 && option < 14 && dial_value == 1) || (option > 1 && option <= 14 && dial_value == -1))
      option += dial_value;
    if(button_value) {
      switch(option){
      case 1: cureTemp = regResin[0];
              cureTime = regResin[1];
              menu = 5;
              break;
      case 2: cureTemp = toughResin[0];
              cureTime = toughResin[1];
              menu = 5;
              break;
      case 3: cureTemp = highResin[0];
              cureTime = highResin[1];
              menu = 5;
              break;
      case 4: cureTemp = colorResin[0];
              cureTime = colorResin[1];
              menu = 5;
              break;
      case 5: cureTemp = gpResin[0];
              cureTime = gpResin[1];
              menu = 5;
              break;
      case 6: cureTemp = rigidResin[0];
              cureTime = rigidResin[1];
              menu = 5;
              break;
      case 7: cureTemp = durResin[0];
              cureTime = durResin[1];
              menu = 5;
              break;
      case 8: cureTemp = castResin[0];
              cureTime = castResin[1];
              menu = 5;
              break;
      case 9: cureTemp = preset1Resin[0];
              cureTime = preset1Resin[1];
              menu = 5;
              break;
      case 10:cureTemp = preset2Resin[0];
              cureTime = preset2Resin[1];
              menu = 5;
              break;
      case 11:cureTemp = preset3Resin[0];
              cureTime = preset3Resin[1];
              menu = 5;
              break;
      case 12:cureTemp = preset4Resin[0];
              cureTime = preset4Resin[1];
              menu = 5;
              break;
      case 13:cureTemp = preset5Resin[0];
              cureTime = preset5Resin[1];
              menu = 5;
              break;
     default: menu = 2;
              break;
      }
      option = 1;
      update_menu_header();
    }
    button_value = false;
    dial_value = 0;
    return true;
  }

  // Menu 2 = Time 1 - 100
  if(menu == 2) {
    if((option >= 1 && option < 100 && dial_value == 1) || (option > 1 && option <= 100 && dial_value == -1))
      option += dial_value;
    if(button_value) {
      time_min = option;
      menu = 3;
      option = 1;
      update_menu_header();
    }
    button_value = false;
    dial_value = 0;
    return true;
  }

  // Menu 3 = Temp 1 - 100
  if(menu == 3) {
    if((option >= 1 && option < 100 && dial_value == 1) || (option > 1 && option <= 100 && dial_value == -1))
      option += dial_value;
    if(button_value) {
      temp = option;
      menu = 4;
      option = 1;
      update_menu_header();
    }
    button_value = false;
    dial_value = 0;
    return true;
  }

  // Menu 4 = Save Options
if(menu == 4) {
    if((option >= 1 && option < 7 && dial_value == 1) || (option > 1 && option <= 7 && dial_value == -1))
      option += dial_value;
    if(button_value) {
      switch(option) {
        case 1: // Save Preset 1
                EEPROM.write(P1_TEMP, temp);
                EEPROM.commit();
                EEPROM.write(P1_TIME, time_min);
                EEPROM.commit();
                preset1Resin[0] = EEPROM.read(P1_TEMP);
                preset1Resin[1] = EEPROM.read(P1_TIME);
                cureTemp = preset1Resin[0];
                cureTime = preset1Resin[1];
                /*SAVES PRESET TO FLASH*/
                menu = 5;
                break;
        case 2: // Save Preset 2
                EEPROM.write(P2_TEMP, temp);
                EEPROM.commit();
                EEPROM.write(P2_TIME, time_min);
                EEPROM.commit();
                preset2Resin[0] = EEPROM.read(P2_TEMP);
                preset2Resin[1] = EEPROM.read(P2_TIME);
                cureTemp = preset2Resin[0];
                cureTime = preset2Resin[1];
                /*SAVES PRESET TO FLASH*/
                menu = 5;
                break;
        case 3: // Save Preset 3
                EEPROM.write(P3_TEMP, temp);
                EEPROM.commit();
                EEPROM.write(P3_TIME, time_min);
                EEPROM.commit();
                preset3Resin[0] = EEPROM.read(P3_TEMP);
                preset3Resin[1] = EEPROM.read(P3_TIME);
                cureTemp = preset3Resin[0];
                cureTime = preset3Resin[1];
                /*SAVES PRESET TO FLASH*/
                menu = 5;
                break;
        case 4: // Save Preset 4
                EEPROM.write(P4_TEMP, temp);
                EEPROM.commit();
                EEPROM.write(P4_TIME, time_min);
                EEPROM.commit();
                preset4Resin[0] = EEPROM.read(P4_TEMP);
                preset4Resin[1] = EEPROM.read(P4_TIME);
                cureTemp = preset4Resin[0];
                cureTime = preset4Resin[1];
                /*SAVES PRESET TO FLASH*/
                menu = 5;
                break;
        case 5: // Save Preset 5
                EEPROM.write(P5_TEMP, temp);
                EEPROM.commit();
                EEPROM.write(P5_TIME, time_min);
                EEPROM.commit();
                preset5Resin[0] = EEPROM.read(P5_TEMP);
                preset5Resin[1] = EEPROM.read(P5_TIME);
                cureTemp = preset5Resin[0];
                cureTime = preset5Resin[1];
                /*SAVES PRESET TO FLASH*/
                menu = 5;
                break;
        case 6: // Doesn't save
                cureTemp = temp;
                cureTime = time_min;
                menu = 5;
                break;
       default: // Cancel to top menu
                menu = 1;
                break;
      }
      if(option == 6)
        menu = 5;
      else if (option == 7)
        menu = 1;
      else
        menu = 5;
      option = 1;
      update_menu_header();
    }
    button_value = false;
    dial_value = 0;
    return true;
  }

  // Confirm Screen
  if (menu == 5) {
    if((option == 1 && dial_value == 1) || (option == 2 && dial_value == -1))
      option += dial_value;
    if(option == 1 && button_value == true) {
      menu = 6;
      option = 1;
      cureTrigger = true;
      update_menu_header();
    }
    if(option == 2 && button_value == true) {
      menu = 1;
      option = 1;
      update_menu_header();
    }
    return true;
  }

  // Cure Screen
  if(menu == 6) {
    if (cureComplete) {
      menu = 1;
      option = 1;
      cureComplete = false;
      update_menu_header();
    }
    if(button_value == true) {
      menu = 7;
      option = 1;
      pauseTrigger = true;
      Serial.println("Paused");
      update_menu_header();
    }
    button_value = false;
    dial_value = 0;
    return true;
  }


  // Pause Menu
  if(menu == 7) {
    if((option == 1 && dial_value == 1) || (option == 2 && dial_value == -1))
      option += dial_value;
    if(option == 1 && button_value == true) {
      menu = 6;
      option = 1;
      resumeTrigger = true;
      update_menu_header();
    }
    if(option == 2 && button_value == true) {
      menu = 1;
      option = 1;
      cancelTrigger = true;
      update_menu_header();
    }
    button_value = false;
    dial_value = 0;
    return true;
  }
  return false;
}

/****************************************************************************************
 * update_display
 *
 *   This function polls both the rotary encoder dial and push-button to decide whether
 *   or not to change the state of the display screen. This function needs to be continuously
 *   executed for the menu function to work properly. Additional polling may be later inserted
 *   into this polling protocol to control the device menus.
 *
 * @param   - Nothing.
 * @returns - Nothing.
 ****************************************************************************************/

void update_display() {
  dial_value = poll_rotary_dial();
  button_value = poll_button();
  if((dial_value != 0) || button_value){
    if(update_menu_options(dial_value, button_value)) {
      display_menu(menu, option, 0);
    }
  } else if(cureState && (cureProgress % 5000 <= 100)){                             // FIXME: Chuck added
    display_menu(6, cureProgress/60000, currentTemp);  // FIXME: Chuck Added
  }
}
