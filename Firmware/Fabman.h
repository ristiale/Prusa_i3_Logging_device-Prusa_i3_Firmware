#include <string.h>
extern const char* lcd_display_message_fullscreen_P();
extern void lcd_set_custom_characters_progress();
extern void lcd_set_custom_characters_degree();
extern void lcd_show_fullscreen_message_and_wait_P(const char *msg);
extern void lcd_FM_login_screen();

extern char *strchr_pointer;
extern int Fabman_mode;
extern bool stoppedInfo;
extern char FM_UserName[18]; //PrusaLab
extern char FM_ID[13]; //PrusaLab
extern char FM_Mode[7]; //Prusalab

void lcd_FM_login();
void serial_FM_login();
void serial_FM_logoff();
void lcd_FM_offline_screen();
void show_user_name_after_print(); //PrusaLab
bool print_successful; //PrusaLab
char FM_filename;

#include "cardreader.h"
#include "messages.h" //PrusaLab
#include "temperature.h" //PrusaLab
#include "language.h" //PrusaLab
#include "lcd.h" //PrusaLab
#include "cmdqueue.h" //PrusaLab
char *starpos = NULL;
char *username = NULL;
bool fileExist = 0;


void lcd_FM_login() {
  SERIAL_PROTOCOLLN("Stage 1 = reached lcd_FM_login()");
  while (Fabman_mode == 1) {
    SERIAL_ECHO("Fabman mode = ");
    SERIAL_ECHOLN(Fabman_mode);
    SERIAL_PROTOCOLLN("Stage 2 = Calling lcd_FM_login_screen()");
    lcd_FM_login_screen();

  }
}

void lcd_FM_offline() {
  SERIAL_PROTOCOLLN("Stage 1 = reached lcd_FM_offline_screen()");
  while (Fabman_mode == 2) {
    SERIAL_ECHO("Fabman mode = ");
    SERIAL_ECHOLN(Fabman_mode);
    SERIAL_PROTOCOLLN("Stage 2 = Calling lcd_FM_offline_screen()");
    lcd_FM_offline_screen();
  }
}


void FM_print_username() {
  starpos = (strchr(strchr_pointer + 5, '*'));
  if (starpos != NULL)
    *(starpos) = '\0';
  username = (strchr_pointer + 5);
  SERIAL_ECHO("UNM: ");
  SERIAL_ECHOLN(username);
}

void filament_used_in_last_print() {
  SERIAL_ECHOLN("stoppedInfo is: " + (stoppedInfo));
  if (stoppedInfo == 1) {
    SERIAL_ECHOLN("stopState:false");
    stoppedInfo = 0;
  } else {
    SERIAL_ECHOLN("stopState:true");
  }
  SERIAL_ECHO("UserName: "); //PrusaLab
  SERIAL_ECHOLN(FM_UserName); //PrusaLab
  SERIAL_ECHO("UserID: "); //PrusaLab
  SERIAL_ECHOLN(FM_ID); //PrusaLab
  SERIAL_ECHO("UserMode: "); //PrusaLab
  SERIAL_ECHOLN(FM_Mode); //PrusaLab
  SERIAL_ECHO("FNM: ");
  SERIAL_ECHOLN(card.longFilename);
  SERIAL_ECHO("TFU: ");
  SERIAL_ECHOLN(total_filament_used);
  SERIAL_ECHO("TTU: ");
  SERIAL_ECHOLN(time_used_in_last_print);


}

#define MAX_FILE_SIZE 1024
#define JSON_BUFFER_SIZE 200

void extractDataFromJSON(uint8_t c) {
  // print whole file
  while (!card.eof()) {
    // filter newlines/carriage returns/tabulators to have message in one line
    if ((c != '\n') && (c != '\r') && (c != '\t')) {
      SERIAL_ECHO(c);
    }
    c = card.getFilteredGcodeChar();
  }
  // end of message
  SERIAL_ECHOLN("");
}

void getConfigFromJSON() {
  card.ls(CardReader::ls_param(code_seen('L'), code_seen('T')));
  if (fileExist != 0) {
    SERIAL_PROTOCOLLN("File found, trying to read it");
    // Converted name of "config.json" file in root of SD card
    card.openFileReadFilteredGcode("CONFI~1.JSO", true);
    // get first character
    uint8_t c = card.getFilteredGcodeChar();
    if (card.eof()) {
      card.openFileReadFilteredGcode("CONFIG~1.JSO", true);
      c = card.getFilteredGcodeChar();
      extractDataFromJSON(c);
    }
    else {
      extractDataFromJSON(c);
    }
  } else {
    SERIAL_PROTOCOLLN("config file doesn't exist, keep going");
  }
}

void lcd_FM_not_allowed() {
  SERIAL_PROTOCOLLN("Stage 1 = reached lcd_FM_not_allowed()");
  while (Fabman_mode == 3) {
    SERIAL_ECHO("Fabman mode = ");
    SERIAL_ECHOLN(Fabman_mode);
    SERIAL_PROTOCOLLN("Stage 2 = Calling lcd_FM_not_allowed_screen()");
  }
}


void serial_FM_login() {
  SERIAL_PROTOCOLLN("Serial FM login");
  Fabman_mode = 0;
  SERIAL_ECHO("Fabman mode = ");
  SERIAL_ECHOLN(Fabman_mode);
}

void serial_FM_logoff() {
  disable_heater();
  finishAndDisableSteppers(); //disable steppers
  SERIAL_PROTOCOLLN("Serial FM logoff");
  Fabman_mode = 1;
  SERIAL_ECHO("Fabman mode = ");
  SERIAL_ECHOLN(Fabman_mode);

  // Clear variables - PrusaLab
  memset(FM_UserName, 0, 18);
  memset(FM_ID, 0, 12);
  memset(FM_Mode, 0, 6);
}

void check_FM_login() {
  /*SERIAL_ECHO("Fabman mode = ");
    SERIAL_ECHOLN(Fabman_mode);*/
  if (Fabman_mode == 1) {
    lcd_FM_login();
  }
  /*else if (Fabman_mode == 2) {
    lcd_FM_offline();
    }*/
  else if (Fabman_mode == 3) {
    lcd_FM_not_allowed();
  }
}

/*PrusaLab*/
void show_user_name_after_print()
{
  disable_heater(); //safety feature
  lcd_display_message_fullscreen_P(_i("Tisk patri:"));
  lcd_puts_at_P(0, 2, PSTR(""));
  lcd_print(FM_UserName);
  lcd_wait_for_click_delay(0);
  lcd_update_enable(true);
  lcd_clear();
  lcd_update(0);
}

//PrusaLab
/*void print_ok()
{
	uint8_t filament_type;
	int enc_dif = 0;
	int cursor_pos = 1;
	int _ret = 0;
	int _t = 0;

	enc_dif = lcd_encoder_diff;
	lcd_clear();

	lcd_set_cursor(0, 0);
	lcd_print("Print ok ?");

	do
	{
		if (abs(enc_dif - lcd_encoder_diff) > 12) {
			if (enc_dif > lcd_encoder_diff) {
				cursor_pos--;
			}

			if (enc_dif < lcd_encoder_diff) {
				cursor_pos++;
			}
			enc_dif = lcd_encoder_diff;
		}

		if (cursor_pos > 2) { cursor_pos = 2; }
		if (cursor_pos < 1) { cursor_pos = 1; }

		lcd_set_cursor(0, 2); lcd_print("          ");
		lcd_set_cursor(0, 3); lcd_print("          ");
		lcd_set_cursor(2, 2);
		lcd_puts_P(_T(MSG_YES));
		lcd_set_cursor(2, 3);
		lcd_puts_P(_T(MSG_NO));
		lcd_set_cursor(0, 1 + cursor_pos);
		lcd_print(">");
		_delay(100);

		_t = _t + 1;
		if (_t>100)
		{
			prusa_statistics(99);
			_t = 0;
		}
		if (lcd_clicked())
		{  
			if (cursor_pos == 1) //yes
			{
        print_successful = true;
        _ret = 1;
			}
			if (cursor_pos == 2) //no
			{
        print_successful = false;
        _ret = 2;
			}
		}

		manage_heater();
		manage_inactivity();
		proc_commands();

	} while (_ret == 0);

}*/
