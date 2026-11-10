#include <Wire.h>
#include <string.h>

#define SLAVE_ADDR 0x68

volatile uint8_t active_command = 0xFF;

char name_msg[32] = "Welcome to FastBit EBA\n";

uint8_t get_len_of_data(void)
{
  return (uint8_t)strlen(name_msg);
}

void setup()
{
  Wire.begin(SLAVE_ADDR);

  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

// STM32 master writes command to Arduino slave
void receiveEvent(int bytes)
{
  if (bytes > 0)
  {
    active_command = Wire.read();
  }
}

// STM32 master requests data from Arduino slave
void requestEvent()
{
  if (active_command == 0x51)
  {
    // Send 1 byte: length of string
    uint8_t len = get_len_of_data();

    Wire.write(&len, 1);

    active_command = 0xFF;
  }
  else if (active_command == 0x52)
  {
    // Send the actual string
    Wire.write(
      (uint8_t *)name_msg,
      get_len_of_data()
    );

    active_command = 0xFF;
  }
}

void loop()
{
}
