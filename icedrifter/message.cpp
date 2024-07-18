#include <Arduino.h>
#include <SoftwareSerial.h>

#include "icedrifter.h"
#include "message.h"

#ifdef PROCESS_MESSAGE

SoftwareSerial SerialMsg(MSG_RX, MSG_TX); 

bool processMessage(uint8_t *messageBuffPtr) {

  unsigned int msgLen;
//  uint8_t *wkptr;
  int i;
  int tries;
  bool xferDone;
  bool xferGotData;
  bool xferOverrun;
//  bool xferNoResponse;

  SerialMsg.begin(9600);

  xferDone = false;

  while (SerialMsg.available()) {
    SerialMsg.read();
    // Clear the read buffer.
  }

  SerialMsg.write('?');

#ifdef SERIAL_DEBUG_MESSAGE
  DEBUG_SERIAL.println("? sent...");
#endif

  i = 0;

  while (1) {
    if (SerialMsg.available()) {
      messageBuffPtr[i] = SerialMsg.read();
      xferGotData = true;

      if (messageBuffPtr[i] == '\n') {
        messageBuffPtr[i + 1] = '\0';
        xferDone = true;

#ifdef SERIAL_DEBUG_MESSAGE
        DEBUG_SERIAL.println("Got xferDone");
#endif

        break;
      }

      i++;

      if (i >= MESSAGE_BUFF_SIZE) {
        xferOverrun = true;
        break;
      }
    } else {  // Data not available

      ++tries;

      if (tries > MAX_TRIES) {
        break;
      }

      delay(10);
    }
  }

  if (xferOverrun == true) {
    xferDone = false;
#ifdef SERIAL_DEBUG_MESSAGE
    DEBUG_SERIAL.println("Overrun on message receive!!!");
#endif

  } else {

#ifdef SERIAL_DEBUG_MESSAGE

    if (xferGotData == true) {
      if(xferDone == false) {
      
        DEBUG_SERIAL.println("Timeout receiving message!!!");

      }
    }

#endif

  }

#ifdef SERIAL_DEBUG_MESSAGE

  if (xferDone == true) {

    DEBUG_SERIAL.println("Message received!!!");
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("Done...");
    DEBUG_SERIAL.println((char *)messageBuffPtr);
    DEBUG_SERIAL.println();
  }

#endif 

  SerialMsg.end();
  return(xferDone);
}

#endif  //PROCESS_MESSAGE
