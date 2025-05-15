// ###################           Mini wither station with electronic ink display 2.9 Inch | nRF52            ############### //
//                                                                                                                           //
//        @filename   :   EFEKTA_THPEINK290_1.ino                                                                         //
//        @brief en   :   Wireless, battery-operated temperature,humidity and pressure sensor(SHT20, SI7020, HTU21D, BME280) //
//                        with electronic ink display(Good Display GDEH029A1). The extended version adds the MAX44009 light  //
//                        sensor, an active bizzer Works on nRF52.                                                           //
//        @brief ru   :   Беcпроводной, батарейный датчик температуры, влажности и давления(SHT20, SI7020, HTU21D, BME280)   //
//                        с дисплеем на электронных чернилах(Good Display GDEH029A1). В расширенной версии добавлен          //
//                        датчик света MAX44009, активный биззер. Работает на nRF52832, nRF52840.                            //
//        @author     :   Andrew Lamchenko aka Berk                                                                          //
//                                                                                                                           //
//        Copyright (C) EFEKTALAB 2020                                                                                       //
//        Copyright (c) 2014-2015 Arduino LLC.  All right reserved.                                                          //
//        Copyright (c) 2016 Arduino Srl.  All right reserved.                                                               //
//        Copyright (c) 2017 Sensnology AB. All right reserved.                                                              //
//        Copyright (C) Waveshare     August 10 2017//                                                                       //
//                                                                                                                           //
// ######################################################################################################################### //


#include <stdlib.h>
#include "eink290.h"

Epd::~Epd() {
};

Epd::Epd() {
  reset_pin = RST_PIN;
  dc_pin = DC_PIN;
  cs_pin = CS_PIN;
  busy_pin = BUSY_PIN;
  width = EPD_WIDTH;
  height = EPD_HEIGHT;
};

int Epd::InitV1(const unsigned char* lut) {
  /* this calls the peripheral hardware interface, see epdif */
  if (IfInit() != 0) {
    return -1;
  }
  /* EPD hardware init start */
  this->lut = lut;
  Reset();
  SendCommand(DRIVER_OUTPUT_CONTROL);
  SendData((EPD_HEIGHT - 1) & 0xFF);
  SendData(((EPD_HEIGHT - 1) >> 8) & 0xFF);
  SendData(0x00);                     // GD = 0; SM = 0; TB = 0;
  SendCommand(BOOSTER_SOFT_START_CONTROL);
  SendData(0xD7);
  SendData(0xD6);
  SendData(0x9D);
  SendCommand(WRITE_VCOM_REGISTER);
  //SendData(0xA8);                     // VCOM 7C
  SendData(0x9A);
  SendCommand(SET_DUMMY_LINE_PERIOD);
  SendData(0x1A);                     // 4 dummy lines per gate
  SendCommand(SET_GATE_TIME);
  SendData(0x08);                     // 2us per line
  SendCommand(DATA_ENTRY_MODE_SETTING);
  SendData(0x03);                     // X increment; Y increment
  SetLut(this->lut);
  /* EPD hardware init end */
  return 0;
}


int Epd::InitV2() {
  /* this calls the peripheral hardware interface, see epdif */
  if (IfInit() != 0) {
    return -1;
  }

  Reset();

  /* EPD hardware init start */
  WaitUntilIdle();
  SendCommand(0x12);  //SWRESET
  WaitUntilIdle();

  SendCommand(0x01); //Driver output control
  SendData(0x27);
  SendData(0x01);
  SendData(0x00);

  SendCommand(0x11); //data entry mode
  SendData(0x03);

  SetMemoryArea(0, 0, width - 1, height - 1);

  SendCommand(0x3C); //BorderWavefrom
  SendData(0x05);

  SendCommand(0x21); //  Display update control
  SendData(0x00);
  SendData(0x80);

  SendCommand(0x18); //Read built-in temperature sensor
  SendData(0x80);

  SetMemoryPointer(0, 0);
  WaitUntilIdle();
  /* EPD hardware init end */
  return 0;
}


/**
    @brief: basic function for sending commands
*/
void Epd::SendCommand(unsigned char command) {
  DigitalWrite(dc_pin, LOW);
  DigitalWrite(cs_pin, LOW);
  SpiTransfer(command);
  DigitalWrite(cs_pin, HIGH);
}

/**
    @brief: basic function for sending data
*/
void Epd::SendData(unsigned char data) {
  DigitalWrite(dc_pin, HIGH);
  DigitalWrite(cs_pin, LOW);
  SpiTransfer(data);
  DigitalWrite(cs_pin, HIGH);
}

/**
    @brief: Wait until the busy_pin goes LOW
*/
/*
  void Epd::WaitUntilIdle(void) {
  while (DigitalRead(busy_pin) == HIGH) {     //LOW: idle, HIGH: busy
    DelayMs(20);
  }
  }
*/
void Epd::WaitUntilIdle(void) {
  while (1) {  //=1 BUSY
    if (DigitalRead(busy_pin) == LOW)
      break;
    DelayMs(5);
  }
  DelayMs(20);
}

/**
    @brief: module reset.
            often used to awaken the module in deep sleep,
            see Epd::Sleep();
*/
void Epd::Reset(void) {
  DigitalWrite(reset_pin, LOW);                //module reset
  DelayMs(20);
  DigitalWrite(reset_pin, HIGH);
  DelayMs(20);
}

/**
    @brief: set the look-up table register
*/
#ifdef EINK_V1
void Epd::SetLut(const unsigned char* lut) {
  this->lut = lut;
  SendCommand(WRITE_LUT_REGISTER);
  /* the length of look-up table is 30 bytes */
  for (int i = 0; i < 30; i++) {
    SendData(this->lut[i]);
  }
}
#endif

/**
    @brief: put an image buffer to the frame memory.
            this won't update the display.
*/
#ifdef EINK_V1
void Epd::SetFrameMemory(
  const unsigned char* image_buffer,
  int x,
  int y,
  int image_width,
  int image_height
) {
  int x_end;
  int y_end;

  if (
    image_buffer == NULL ||
    x < 0 || image_width < 0 ||
    y < 0 || image_height < 0
  ) {
    return;
  }
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  x &= 0xF8;
  image_width &= 0xF8;
  if (x + image_width >= this->width) {
    x_end = this->width - 1;
  } else {
    x_end = x + image_width - 1;
  }
  if (y + image_height >= this->height) {
    y_end = this->height - 1;
  } else {
    y_end = y + image_height - 1;
  }
  SetMemoryArea(x, y, x_end, y_end);
  SetMemoryPointer(x, y);
  SendCommand(WRITE_RAM);
  /* send the image data */
  for (int j = 0; j < y_end - y + 1; j++) {
    for (int i = 0; i < (x_end - x + 1) / 8; i++) {
      SendData(image_buffer[i + j * (image_width / 8)]);
    }
  }
}
#else

unsigned char WF_PARTIAL_2IN9[159] =
{
0x0,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x80,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x40,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0A,0x0,0x0,0x0,0x0,0x0,0x2,  
0x1,0x0,0x0,0x0,0x0,0x0,0x0,
0x1,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x22,0x22,0x22,0x22,0x22,0x22,0x0,0x0,0x0,
0x22,0x17,0x41,0xB0,0x32,0x36,
};

void Epd::SetFrameMemory(
    const unsigned char* image_buffer,
    int x,
    int y,
    int image_width,
    int image_height
) {
    int x_end;
    int y_end;

    if (
        image_buffer == NULL ||
        x < 0 || image_width < 0 ||
        y < 0 || image_height < 0
    ) {
        return;
    }
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    x &= 0xF8;
    image_width &= 0xF8;
    if (x + image_width >= this->width) {
        x_end = this->width - 1;
    } else {
        x_end = x + image_width - 1;
    }
    if (y + image_height >= this->height) {
        y_end = this->height - 1;
    } else {
        y_end = y + image_height - 1;
    }

    DigitalWrite(reset_pin, LOW);
    DelayMs(5);
    DigitalWrite(reset_pin, HIGH);
    DelayMs(10);
  
  SetLut2();
  SendCommand(0x37); 
  SendData(0x00);  
  SendData(0x00);  
  SendData(0x00);  
  SendData(0x00); 
  SendData(0x00);   
  SendData(0x40);  
  SendData(0x00);  
  SendData(0x00);   
  SendData(0x00);  
  SendData(0x00);

  SendCommand(0x3C); //BorderWavefrom
  SendData(0x80); 

  SendCommand(0x22); 
  SendData(0xC0);   
  SendCommand(0x20); 
  WaitUntilIdle();  
  
    SetMemoryArea(x, y, x_end, y_end);
    SetMemoryPointer(x, y);
    SendCommand(0x24);
    /* send the image data */
    for (int j = 0; j < y_end - y + 1; j++) {
        for (int i = 0; i < (x_end - x + 1) / 8; i++) {
            SendData(image_buffer[i + j * (image_width / 8)]);
        }
    }
}

void Epd::SetFrameMemoryFull(
  const unsigned char* image_buffer,
  int x,
  int y,
  int image_width,
  int image_height
) {
  int x_end;
  int y_end;

  if (
    image_buffer == NULL ||
    x < 0 || image_width < 0 ||
    y < 0 || image_height < 0
  ) {
    return;
  }
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  x &= 0xF8;
  image_width &= 0xF8;
  if (x + image_width >= this->width) {
    x_end = this->width - 1;
  } else {
    x_end = x + image_width - 1;
  }
  if (y + image_height >= this->height) {
    y_end = this->height - 1;
  } else {
    y_end = y + image_height - 1;
  }
  SetMemoryArea(x, y, x_end, y_end);
  SetMemoryPointer(x, y);
  SendCommand(0x24);
  /* send the image data */
  for (int j = 0; j < y_end - y + 1; j++) {
    for (int i = 0; i < (x_end - x + 1) / 8; i++) {
      SendData(image_buffer[i + j * (image_width / 8)]);
    }
  }
}
#endif


/**
    @brief: put an image buffer to the frame memory.
            this won't update the display.

            Question: When do you use this function instead of
            void SetFrameMemory(
                const unsigned char* image_buffer,
                int x,
                int y,
                int image_width,
                int image_height
            );
            Answer: SetFrameMemory with parameters only reads image data
            from the RAM but not from the flash in AVR chips (for AVR chips,
            you have to use the function pgm_read_byte to read buffers
            from the flash).
*/

#ifdef EINK_V1
void Epd::SetFrameMemory(const unsigned char* image_buffer) {
  SetMemoryArea(0, 0, this->width - 1, this->height - 1);
  SetMemoryPointer(0, 0);
  SendCommand(WRITE_RAM);
  /* send the image data */
  for (int i = 0; i < this->width / 8 * this->height; i++) {
    SendData(pgm_read_byte(&image_buffer[i]));
  }
}
#else
void Epd::SetFrameMemory(const unsigned char* image_buffer) {

  SetMemoryArea(0, 0, this->width - 1, this->height - 1);
  SetMemoryPointer(0, 0);
  SendCommand(WRITE_RAM);
  /* send the image data */
  for (int i = 0; i < this->width / 8 * this->height; i++) {
    SendData(pgm_read_byte(&image_buffer[i]));
  }
}


void Epd::SetLut2(void) {       
  unsigned char count;
  SendCommand(0x32);
  for(count=0; count<153; count++) 
    SendData(WF_PARTIAL_2IN9[count]); 
  WaitUntilIdle();
}



void Epd::SetFrameMemory_Base(const unsigned char* image_buffer) {
  SetMemoryArea(0, 0, this->width - 1, this->height - 1);
  SetMemoryPointer(0, 0);
  SendCommand(WRITE_RAM);
  /* send the image data */
  for (int i = 0; i < this->width / 8 * this->height; i++) {
    SendData(pgm_read_byte(&image_buffer[i]));
  }
  SendCommand(WRITE_RAM2);
  /* send the image data */
  for (int i = 0; i < this->width / 8 * this->height; i++) {
    SendData(pgm_read_byte(&image_buffer[i]));
  }
}
#endif
/**
    @brief: clear the frame memory with the specified color.
            this won't update the display.
*/
void Epd::ClearFrameMemory(unsigned char color) {
  SetMemoryArea(0, 0, this->width - 1, this->height - 1);
  SetMemoryPointer(0, 0);
  SendCommand(WRITE_RAM);
  /* send the color data */
  for (int i = 0; i < this->width / 8 * this->height; i++) {
    SendData(color);
  }
}

/**
    @brief: update the display
            there are 2 memory areas embedded in the e-paper display
            but once this function is called,
            the the next action of SetFrameMemory or ClearFrame will
            set the other memory area.
*/
#ifdef EINK_V1
void Epd::DisplayFrame(void) {
  SendCommand(DISPLAY_UPDATE_CONTROL_2);
  SendData(0xC4);
  SendCommand(MASTER_ACTIVATION);
  SendCommand(TERMINATE_FRAME_READ_WRITE);
  WaitUntilIdle();
}
#else
void Epd::DisplayFrame(void) {
  SendCommand(DISPLAY_UPDATE_CONTROL_2);
  SendData(0xFF);
  SendCommand(MASTER_ACTIVATION);
  WaitUntilIdle();
}

void Epd::DisplayFrameFull(void) {
  SendCommand(0x22);
  SendData(0xF7);
  SendCommand(0x20);
  WaitUntilIdle();
}
#endif

/**
    @brief: private function to specify the memory area for data R/W
*/
void Epd::SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
  SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  SendData((x_start >> 3) & 0xFF);
  SendData((x_end >> 3) & 0xFF);
  SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
  SendData(y_start & 0xFF);
  SendData((y_start >> 8) & 0xFF);
  SendData(y_end & 0xFF);
  SendData((y_end >> 8) & 0xFF);
}

/**
    @brief: private function to specify the start point for data R/W
*/
void Epd::SetMemoryPointer(int x, int y) {
  SendCommand(SET_RAM_X_ADDRESS_COUNTER);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  SendData((x >> 3) & 0xFF);
  SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
  SendData(y & 0xFF);
  SendData((y >> 8) & 0xFF);
  WaitUntilIdle();
}

/**
    @brief: After this command is transmitted, the chip would enter the
            deep-sleep mode to save power.
            The deep sleep mode would return to standby by hardware reset.
            You can use Epd::Init() to awaken
*/
void Epd::Sleep() {
  SendCommand(0x10);
  SendData(0x01);
  //DelayMs(50);
}

const unsigned char lut_full_update[] = {
  0x50, 0xAA, 0x55, 0xAA, 0x11, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char lut_partial_update[] = {
  0x10, 0x18, 0x18, 0x08, 0x18, 0x18,
  0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/* END OF FILE */
