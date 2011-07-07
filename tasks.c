#include "main.h"
#include "firmware.h"

#include "display.h"
#include "languages.h"
#include "menu_shortcuts.h"
#include "presets.h"
#include "settings.h"
#include "utils.h"

#include "tasks.h"

void set_intermediate_iso();

void start_up() {
	// Wait for camera to settle down
	SleepTask(1000);

	// Read settings from file
	settings_read();

	if (settings.debug_on_poweron)
		start_debug_mode();

	SleepTask(100);

	// enable IR remote
	// i'm not sure where to call this? perhaps this isnt the right place.
	if (settings.remote_enable) {
		eventproc_RemOn();
	}

	// Set current language
	lang_pack_config();

	// Enable (hidden) CFn.8 for ISO H
	send_to_intercom(IC_SET_CF_EXTEND_ISO, 1, 1);

	// Enable realtime ISO change
	send_to_intercom(IC_SET_REALTIME_ISO_0, 0, 0);
	send_to_intercom(IC_SET_REALTIME_ISO_1, 0, 0);

	// Read presets from file
	presets_read();

	// turn off the blue led after it was lighten by our my_task_MainCtrl()
	eventproc_EdLedOff();

	// We are no longer booting up
	status.booting = FALSE;
}

void dp_action() {
	if (settings.shortcuts_menu || cameraMode->ae > 6) {
		menu_shortcuts_start();
	} else {
		set_intermediate_iso();
		display_refresh();
	}
}

void set_metering_spot() {
	press_button(IC_BUTTON_SET);
	send_to_intercom(IC_SET_METERING, 1, METERING_MODE_SPOT);
	print_icu_info();

	beep();
}

void set_whitebalance_colortemp() {
	press_button(IC_BUTTON_SET);
	send_to_intercom(IC_SET_WB, 1, WB_MODE_COLORTEMP);
	print_icu_info();

	beep();
}

void set_iso_high() {
	press_button(IC_BUTTON_SET);
	send_to_intercom(IC_SET_ISO, 2, 0x6F);
	print_icu_info();

	beep();
}

void toggle_raw_jpeg() {
	switch (cameraMode->ae) {
		case AE_MODE_P:
		case AE_MODE_TV:
		case AE_MODE_AV:
		case AE_MODE_ADEP:
		case AE_MODE_M:
		case AE_MODE_AUTO:
			switch (cameraMode->img_format & 0x03) {
				case 0x01:
					cameraMode->img_format = 0x02;
					break;
				case 0x02:
					cameraMode->img_format = 0x03;
					break;
				case 0x03:
				default:
					cameraMode->img_format = 0x01;
					break;
			}
			send_to_intercom(IC_SET_IMG_FORMAT, 1, cameraMode->img_format);
			break;
		default:
			break;
	}
}

void toggle_CfMLU() {
	send_to_intercom(IC_SET_CF_MIRROR_UP_LOCK, 1, cameraMode->cf_mirror_up_lock ^ 0x01);
}

void toggle_CfFlashSyncRear() {
	send_to_intercom(IC_SET_CF_FLASH_SYNC_REAR, 1, cameraMode->cf_flash_sync_rear ^ 0x01);
}

void set_intermediate_iso() {
	if (cameraMode->ae < 6)
		send_to_intercom(IC_SET_ISO, 2, iso_roll(cameraMode->iso));
}

void restore_iso() {
	int iso;

	if (cameraMode->iso >= 0x68) {
		iso = 0x68;
	} else if (cameraMode->iso >= 0x60) {
		iso = 0x60;
	} else if (cameraMode->iso >= 0x58) {
		iso = 0x58;
	} else if (cameraMode->iso >= 0x50) {
		iso = 0x50;
	} else {
		iso = 0x48;
	}

	send_to_intercom(IC_SET_ISO, 2, iso);
}

void restore_wb() {
	if (cameraMode->wb == WB_MODE_COLORTEMP)
		send_to_intercom(IC_SET_WB, 1, WB_MODE_AUTO);
}

void restore_metering() {
	if (cameraMode->metering == METERING_MODE_SPOT)
		send_to_intercom(IC_SET_METERING, 1, METERING_MODE_EVAL);
}
