#ifndef _LCD_
#define _LCD_

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_RA8875.h>  //screen

#ifdef _SD_
#include <SD.h>  //SD card
#endif

class LCD
{
private:
	Adafruit_RA8875* screen;
	int _x;
	int _y;
	uint32_t power(uint32_t x, uint32_t y);

#ifdef _SD_
	// Draw bmp img
	uint16_t read16(File f);
	uint32_t read32(File f);
	uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
	byte decToBcd(byte val);
#endif


public:
	LCD(int screen_cs, int screen_reset);
	//~LCD();
	bool screen_init();
	
	void screen_clear();
	void set_cursor(uint32_t x, uint32_t y);
	void increment_row();

	void screen_fill_color(uint8_t);
	void clear_line();
	void write_string(const char*);
	void write_string_no_increment(const char*);
	void write_value_string(const char*);
	void write_value_string_no_increment(const char*);
	void write_value_int(uint32_t);
	void write_value_int_no_increment(uint32_t);
	void write_value_with_dp(uint32_t, uint32_t);
	void write_value_dp_no_increment(uint32_t, uint32_t);
	void write_value_with_dp_custom(uint32_t, uint32_t, uint8_t x_size, uint8_t y_size);
	void write_value_string_colour(const char*, uint16_t);
	void write_value_string_custom(const char* var, uint8_t x_size, uint8_t y_size, uint16_t box_colour, uint16_t text_colour, bool increment);

	
#ifdef _SD_
	void bmpDraw(char *filename, int picx, int picy);
#endif

};

#endif