#include <Arduino.h>
#include "SPI.h"
#include "TFT_eSPI.h"
#include "..\LVGL\src\lvgl.h"
#include "GxFT5436.h"
#include <SoftwareSerial.h>


#define MY_DISP_HOR_RES 480
#define MY_DISP_VER_RES 320

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[MY_DISP_HOR_RES * MY_DISP_VER_RES / 10];

TFT_eSPI tft = TFT_eSPI(MY_DISP_HOR_RES, MY_DISP_VER_RES); /* TFT instance */
GxFT5436 touch(/*SDA=*/PB9, /*SCL=*/PB8, /*RST=*/TFT_RST); // HW I2C works

SoftwareSerial DEBUG(10, 5); // RX, TX




uint8_t touchpad_pressed;
uint16_t touchpad_x, touchpad_y;


static void event_cb(lv_event_t* e) {
	lv_obj_t* obj = lv_event_get_current_target(e);
	LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj));
}

void lv_example_msgbox_1(void) {
	static const char* btns[] = { "Apply", "Close", "" };

	lv_obj_t* mbox1 = lv_msgbox_create(NULL, "Hello", "This is a message box with two buttons.", btns, true);
	lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_center(mbox1);
}

static void btn_event_cb(lv_event_t* e) {
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* btn = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED) {
		static uint8_t cnt = 0;
		cnt++;
		/*Get the first child of the button which is the label and change its text*/
		lv_obj_t* label = lv_obj_get_child(btn, 0);
		lv_label_set_text_fmt(label, "Button: %d", cnt);
	}
}


void my_disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
	uint32_t w = (area->x2 - area->x1 + 1);
	uint32_t h = (area->y2 - area->y1 + 1);

	tft.startWrite();
	tft.setAddrWindow(area->x1, area->y1, w, h);
	tft.pushColors(&color_p->full, w * h, true);
	tft.endWrite();

	lv_disp_flush_ready(disp);
}


void my_touchpad_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {

	if (touch.newSingleTouch(touchpad_x, touchpad_y)) {
		data->point.x = touchpad_x;
		data->point.y = touchpad_y;
		data->state = LV_INDEV_STATE_PRESSED;
		DEBUG.print(" ("); DEBUG.print(touchpad_x); DEBUG.print(", "); DEBUG.print(touchpad_y); DEBUG.println(")");
	}

	else
		data->state = LV_INDEV_STATE_RELEASED;
}

void setup() {
	DEBUG.begin(115200);
	delay(5000);

	DEBUG.println("\nGxFT5436 Test");
	delay(500);
	DEBUG.println("\nGxFT5436 Test");
	delay(500);

	Wire.begin();
	touch.init(&DEBUG);

	tft.begin();          /* TFT init */
	tft.setRotation(1); /* Landscape orientation, flipped */

	lv_init();
	/*Declare a buffer for 1/10 screen size*/
	lv_disp_draw_buf_init(&draw_buf, buf1, NULL, MY_DISP_HOR_RES * MY_DISP_VER_RES / 10);
	/*Initialize the display buffer.*/


	static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
	lv_disp_drv_init(&disp_drv); /*Basic initialization*/
	disp_drv.flush_cb = my_disp_flush; /*Set your driver function*/
	disp_drv.draw_buf = &draw_buf; /*Assign the buffer to the display*/
	disp_drv.hor_res = MY_DISP_HOR_RES; /*Set the horizontal resolution of the display*/
	disp_drv.ver_res = MY_DISP_VER_RES; /*Set the vertical resolution of the display*/
	lv_disp_drv_register(&disp_drv); /*Finally register the driver*/


	static lv_indev_drv_t indev_drv; /*Descriptor of a input device driver*/
	lv_indev_drv_init(&indev_drv); /*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
	indev_drv.read_cb = my_touchpad_read; /*Set your driver function*/
	lv_indev_drv_register(&indev_drv); /*Finally register the driver*/


	/* Create simple label */
	lv_obj_t* label = lv_label_create(lv_scr_act());
	lv_label_set_text(label, "Hello World!");

	lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);


	/* Create a button with a label and react on click event.*/
	lv_obj_t* btn = lv_btn_create(lv_scr_act()); /*Add a button the current screen*/
	lv_obj_set_pos(btn, 10, 10); /*Set its position*/
	lv_obj_set_size(btn, 120, 50); /*Set its size*/
	lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/
	lv_obj_t* label2 = lv_label_create(btn); /*Add a label to the button*/
	lv_label_set_text(label2, "Button"); /*Set the labels text*/
	lv_obj_center(label2);

}

void loop() {
	lv_timer_handler(); /* let the GUI do its work */
	delay(5);
}