#include <sh1106.h>

I2C_HandleTypeDef *HI2C;
uint8_t DEVICE_ADDRESS;

uint8_t POS_X;
uint8_t POS_Y;

const size_t LINE_WRAP_INTERVAL = 18;

uint8_t screenBuffer[SH1106_WIDTH * (SH1106_HEIGHT / SH1106_NUM_PAGES)];

uint8_t dynamicInitDataArray[] = {
  SH1106_COMMAND, DISPLAY_OFF,
  SH1106_COMMAND, SET_MULTIPLEX_RATIO, 0x3F,                     // Reset Value: 0x3F / xx11 1111
  SH1106_COMMAND, SET_DISPLAY_OFFSET, 0x00,                      // No Offset
  SH1106_COMMAND, SET_DISPLAY_START_LINE,
  SH1106_COMMAND, SET_SEGMENT_REMAP_SEG0_131,
  SH1106_COMMAND, SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL_MODE,
  SH1106_COMMAND, SET_COM_PINS_HARDWARE_CONFIGURATION, 0x12,     // 0x02 or 0x12 (POR)

  SH1106_COMMAND, SET_CONTRAST_CONTROL, 0x7F,
  SH1106_COMMAND, SET_DISPLAY_CLOCK_DIVIDE_RATIO, 0xA0,          // Suggested Ratio: 0x80
  SH1106_COMMAND, CHARGE_PUMP_SETTING, 0x14,                     // Enable: 0x14, Disable: 0x10

  SH1106_COMMAND, SET_MEMORY_ADDRESSING_MODE, 0x00,              // Horizontal: 0x00, Vertical: 0x01, Page: 0x02
  SH1106_COMMAND, SET_PRECHARGE_PERIOD, 0x22,
  SH1106_COMMAND, SET_VCOMH_DESELECT_LEVEL, 0x20,
  SH1106_COMMAND, SET_DISPLAY_NORMAL,
  SH1106_COMMAND, DISPLAY_ON_RAM_CONTENT,
  SH1106_COMMAND, DEACTIVATE_SCROLL
};

/**
  * @brief This function replaces the HAL_I2C function for easier handling of the i2c pointer and the device address.
  * @param *pData:        pointer to byte array
  * @param size:          size of pData
  * Qparam timeout:       timeout in ms
  */
void SH1106_CALL_HAL_I2C_Transmit(uint8_t *pData, uint16_t size, uint32_t timeout) {
  HAL_I2C_Master_Transmit(HI2C, DEVICE_ADDRESS, pData, size, timeout);
}

/**
  * @brief This function initializes the display.
  * @param *hi2c:         i2c-handle pointer
  * @param deviceAddress: i2c device address
  */
void SH1106_Init(I2C_HandleTypeDef *hi2c, uint8_t deviceAddress) {
  HI2C = hi2c;
  DEVICE_ADDRESS = deviceAddress;
  memset(screenBuffer, 0, sizeof(screenBuffer));
  size_t totalBytesToTransmit = sizeof(dynamicInitDataArray) / sizeof(dynamicInitDataArray[0]);
  SH1106_CALL_HAL_I2C_Transmit(dynamicInitDataArray, totalBytesToTransmit, 1000);
}

/**
  * @brief This function turns the display on and displays the RAM content.
  */
void SH1106_Display_On() {
  uint8_t data[4] = {
    SH1106_COMMAND, DISPLAY_ON_RAM_CONTENT,
    SH1106_COMMAND, DISPLAY_ON_NORMAL_MODE
  };
  SH1106_CALL_HAL_I2C_Transmit(data, 4, 1000);
}

/**
  * @brief This function turns the display on and displays all white pixels.
  */
void SH1106_Display_All_On() {
  uint8_t data[4] = {
    SH1106_COMMAND, DISPLAY_ON_RAM_IGNORE,
    SH1106_COMMAND, DISPLAY_ON_NORMAL_MODE
  };
  SH1106_CALL_HAL_I2C_Transmit(data, 4, 1000);
}

/**
  * @brief This function turns the display off.
  */
void SH1106_Display_Off() {
  uint8_t data[2] = {SH1106_COMMAND, DISPLAY_OFF};
  SH1106_CALL_HAL_I2C_Transmit(data, 2, 1000);
}

/**
  * @brief This function clears the display ram to all zeros.
  */
void SH1106_Display_Clear() {
  uint8_t pixelOff[SH1106_WIDTH + 1];
  pixelOff[0] = SH1106_DATA;
  memset(&pixelOff[1], 0x00, SH1106_WIDTH);

  for(uint8_t page = 0; page < 8; page++){
    SH1106_Set_Page_Address(page);
    SH1106_Set_Column_Address(0);

    SH1106_CALL_HAL_I2C_Transmit(pixelOff, sizeof(pixelOff), 1000);
  }
}

/**
  * @brief This function sets the cursor to a position
  * @param page:          y-position (0-7)
  * @param column:        x-position (0-127)
  */
void SH1106_Set_Cursor(uint8_t page, uint8_t column) {
  POS_Y = page;
  POS_X = column;
  SH1106_Set_Page_And_Column_Address(POS_Y, POS_X);
}

/**
  * @brief This function sets the page.
  * @param page:          page number from 0-7
  */
void SH1106_Set_Page_Address(uint8_t page) {
  POS_Y = page;
  if(page > 7) {
    return; // Check if page is in bounds.
  }
  uint8_t setPageAddress[2] = {SH1106_COMMAND, SET_PAGE_ADDRESS_FOR_PAGE_ADDRESSING_MODE + page};
  SH1106_CALL_HAL_I2C_Transmit(setPageAddress, 2, 1000);
}

/**
  * @brief This function sets the column.
  * @param column:        lower-column number from 0-131
  */
void SH1106_Set_Column_Address(uint8_t column) {
  POS_X = column;
  column = column + 2; // Add 2 to account for bytes in ram without pixels.
  uint8_t lowerNibble  = column & 0x0F; // Mask the lower nibble
  uint8_t higherNibble = (column >> 4) & 0x0F; // Shift and mask the higher nibble
  uint8_t setColumnLowerAddress[2]  = {SH1106_COMMAND, 0x00 + lowerNibble};
  uint8_t setColumnHigherAddress[2] = {SH1106_COMMAND, 0x10 + higherNibble};
  SH1106_CALL_HAL_I2C_Transmit(setColumnLowerAddress, 2, 1000);
  SH1106_CALL_HAL_I2C_Transmit(setColumnHigherAddress, 2, 1000);
}

/**
  * @brief This function sets the page and the column address.
  * @param page:          page number from 0-7
  * @param column:        lower-column number from 0-131
  */
void SH1106_Set_Page_And_Column_Address(uint8_t page, uint8_t column) {
  POS_Y = page;
  POS_X = column;
  SH1106_Set_Page_Address(page);
  SH1106_Set_Column_Address(column);
}

/**
  * @brief This function sends one byte of data to the display.
  * @param data:          data to transmit
  */
void SH1106_Transmit_Data(uint8_t data) {
  uint8_t buffer[2];
    buffer[0] = SH1106_DATA;
    buffer[1] = data;
    SH1106_CALL_HAL_I2C_Transmit(buffer, 2, 1000);
}

/**
  * @brief This function sends a command to the display.
  * @param data:          command to transmit
  */
void SH1106_Transmit_Command(uint8_t command) {
  uint8_t buffer[2];
    buffer[0] = SH1106_COMMAND;
    buffer[1] = command;
    SH1106_CALL_HAL_I2C_Transmit(buffer, 2, 1000);
}


/**
  * @brief This function sends a single character to the display.
  * @param character:     character to transmit
  */
void SH1106_Display_Character(uint8_t character) {
  if(character >= 257) {
    return; // Do nothing when character is out of bound.
  }
  for(int i = 0; i < 7; i++) {
    SH1106_Transmit_Data(minecraft_condensed[character][i]);
    HAL_Delay(1); // Small delay for processing time.
  }
}

/**
  * @brief This function send a string to the display if it fits into one line.
  * @param string:        string to transmit
  */
void SH1106_Print_Line(char* string) {
  size_t stringLength = strlen(string);
  if(stringLength > 18) {
    return;
  }
  for(uint8_t i = 0; i < stringLength; i++){
    SH1106_Display_Character(string[i]);
  }
}

/**
  * @brief This function sends multiple characters to the display.
  * @param page:          page to start writing
  * @param string:        string to transmit
  */
void SH1106_Display_String(char* string) {
  SH1106_Set_Page_And_Column_Address(POS_Y, 0);
  size_t stringLength = strlen(string);
  size_t lineWrapInterval = 18;
  for(uint8_t i = 0; i < stringLength; i++){
    if(i % lineWrapInterval == 0 && i != 0) {
      if(POS_Y > 0) {
        POS_Y--;
      } else {
        POS_Y = 7;
      }
      SH1106_Set_Page_And_Column_Address(POS_Y, 0);
    }
    SH1106_Display_Character(string[i]);
  }
}

/**
  * @brief This function sends multiple characters to the display and wraps to the next line if the last word won't fit into the current line.
  * @param page:          page to start writing
  * @param string:        string to transmit
  */
void SH1106_Display_Text(char* string) {
  SH1106_Set_Page_And_Column_Address(POS_Y, POS_X);

  const char delim[2] = " "; // Delimeter, can also use multiple delimeters " ,;:"
  char buffer[strlen(string) + 1];
  strncpy(buffer, string, sizeof(buffer) - 1);
  buffer[sizeof(buffer)] = '\0';
  char *token = strtok(buffer, delim);
  size_t position = 0;

  while(token != NULL) {
    size_t tokenLength = strlen(token);
    if(position + tokenLength > LINE_WRAP_INTERVAL) {
      if(POS_Y > 0) {
        POS_Y--;
      } else {
        POS_Y = 7;
      }
      SH1106_Set_Page_And_Column_Address(POS_Y, 0);
      position = 0;
    }

    for(uint8_t i = 0; i < tokenLength; i++){
      SH1106_Display_Character(token[i]);
      position++;
    }
    SH1106_Display_Character(' ');
    position++;
    token = strtok(NULL, delim);
  }
}

/**
  * @brief This function sets a pixel at specific coordinates. CAREFUL: function overwrites the whole byte to set one bit.
  * @param x: x-position (0-127)
  * @param y: y-position (0-63)
  */
void SH1106_Set_Pixel(uint8_t x, uint8_t y) {
  if(x >= SH1106_WIDTH || y >= SH1106_HEIGHT) {
    return;
  }

  uint8_t page = y / 8;
  uint8_t bit = (1 << (y % 8));

  uint8_t column = x;

  SH1106_Set_Cursor(page, column);
  SH1106_Transmit_Data(bit);
}

/**
  * @brief This function draws a line between two points. CAREFUL: function overwrites the whole byte to set one bit.
  * @param xA: xA-position (0-127)
  * @param yA: yA-position (0-63)
  * @param xB: xB-position (0-127)
  * @param yB: yB-position (0-63)
  */
void SH1106_Draw_Line(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY) {
  if(startX >= SH1106_WIDTH || endX >= SH1106_WIDTH || startY >= SH1106_HEIGHT || endY >= SH1106_HEIGHT) {
    return;
  }

  int16_t deltaX = abs(endX - startX);
  int16_t deltaY = abs(endY - startY);
  int16_t stepX = (startX < endX) ? 1 : -1;
  int16_t stepY = (startY < endY) ? 1 : -1;
  int16_t error = deltaX - deltaY;

  while(1) {
    // Set pixel at (xA, xY)
    uint8_t page = startY / 8;
    uint8_t bit = (1 << (startY % 8));
    uint8_t column = startX;
    SH1106_Set_Cursor(page, column);
    SH1106_Transmit_Data(bit);

    // Break when reaching endpoint
    if(startX == endX && startY == endY) {
      break;
    }

    int16_t doubleError = 2 * error;
    if(doubleError > deltaY) {
      error -= deltaY;
      startX += stepX;
    }
    if(doubleError < deltaX) {
      error += deltaX;
      startY += stepY;
    }

  }
}
