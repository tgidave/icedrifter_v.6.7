/*                                                                              
 *  rockblockqueue.cpp                                                 
 *                                                                               
 *  Queue processing for sending messages through the rockblock to the
 *  user.                     
 *                                                                               
 *  Introduction                                              
 *  
 *  This code manages a queue of messages to be sent through the rockblock
 *  and on to the user through the iridium messaging system.
 *                                                                               
 *  Author                                                       
 *                                                                               
 *  Uncle Dave                                                  
 *                                                                               
 *  License                                                     
 *                                                                               
 *  Unknown (Talk to Cy)                                                        
 *                                                                               
 *  HISTORY                                                             
 *                                                                               
 *  v0.0   - First release
 */

#include <Arduino.h>
#include <string.h>

#include "icedrifter.h"
#include "rockblock.h"
#include "rockblockqueue.h"

rbqEntry rbqQueue[RBQ_SIZE];

dataChunk dcChunk;


int rbqProcessDataHumanReadable(icedrifterData *msgPtr, uint16_t msgLen, uint8_t msgType);
int rbqProcessChunks(uint8_t *msgPtr, uint16_t msgLen, uint8_t msgType);

int rbqFindEmptyEntry(void) {

  int i;

  for (i = 0; i < RBQ_SIZE; i++) {
    if (rbqQueue[i].msgType = RBQ_MSG_TYPE_FREE) {
      return(i);
    }
  }
  return(-1);
}


int rbqInit(void) {

  int i;

  for (i = 0; i < RBQ_SIZE; i++) {
    rbqQueue[i].msgType = RBQ_MSG_TYPE_FREE;   
    rbqQueue[i].msgLen = 0;
    rbqQueue[i].msg[0] = 0;
  }

  return (RBQ_GOOD);
}
/*
int rbqReserveEntry(void) { 
  for (i = 0; i < RBQ_SIZE; i++) {

    if (rbqQueue[i].msg[0] == 0) {
      rbqQueue[i].msg = (void *)0xffffffff;
      rbqQueue[i].msgLen = 0;
      rbqQueue[i].msgType = MSG_TYPE_RESERVED;
      return(i);
    }
  }
  return(RBQ_QUEUE_FULL);
}

void rbqReleaseEntry(int entNbr) {
  if (rbqQueue[entNbr].msgPtr != (void*)0xffffffff) {
    return (RBQ_ENTRY_NOT_RSVD);
  }

  rbqQueue[entNbr].msgPtr = (void *)NULL;
  rbqQueue[entNbr].msgLen = 0;
  rbqQueue[i].msgType = MSG_TYPE_RESERVED;
    
*/
int rbqAddMessage(uint8_t *msgPtr, uint16_t msgLen, uint8_t msgType) {

  int i;

  if (msgPtr == NULL) {
    return (RBQ_NO_MSG_ADDR);
  }

  if (msgType == RBQ_MSG_TYPE_CHAR) {
    if (strlen(msgPtr) >= MESSAGE_BUFF_SIZE) {
      return (RBQ_CHAR_TOO_LONG);
    }

    if (strchr(msgPtr, '\r') == NULL) {
      return(RBQ_NO_CR_FOUND);
    }

    if (i = rbqFindEmptyEntry() == -1) {
      return(RBQ_QUEUE_FULL);
    }

    rbqQueue[i].msgType = msgType;
    rbqQueue[i].msgLen = msgLen;
    memmove(rbqQueue[i].msg, msgPtr, msgLen);
    return (RBQ_GOOD);

#ifdef HUMAN_READABLE_DISPLAY
  } else if (msgType == RBQ_MSG_TYPE_SENSOR && msgLen == BASE_RECORD_LENGTH) {
      return(rbqProcessDataHumanReadable((icedrifterData *)msgPtr, msgLen, msgType));
#endif
  } else if (msgType == RBQ_MSG_TYPE_SENSOR || msgType == RBQ_MSG_TYPE_BIN ) {

    if (msgLen > MESSAGE_BUFF_SIZE) {
      return(rbqProcessChunks(msgPtr, msgLen, msgType));
    }

    if (i = rbqFindEmptyEntry() == -1) {
      return(RBQ_QUEUE_FULL);
    }

      rbqQueue[i].msgType = msgType;
      rbqQueue[i].msgLen = msgLen;
      memmove(rbqQueue[i].msg, msgPtr, msgLen);
      return (RBQ_GOOD);

    } else {

    return(RBQ_INVALID_TYPE);

  }
}

int rbqTransmitQueueData(void) {

  int i;
  int msgCount;

  msgCount = 0;

  rbInit();

  for (i = 0; i < RBQ_SIZE; i++) {
    if (rbqQueue[i].msgType != RBQ_MSG_TYPE_FREE) {
      rbTransmit(rbqQueue[i].msg, rbqQueue[i].msgLen, rbqQueue[i].msgType);
      rbqQueue[i].msgType = RBQ_MSG_TYPE_FREE;
      rbqQueue[i].msgLen = 0;
      rbqQueue[i].msg[0] = 0;
      ++msgCount;
    }
  }

  rbShutdown();
  return (msgCount);
}

int rbqProcessDataHumanReadable(icedrifterData *idPtr, uint16_t msgLen, uint8_t msgType) {

  struct tm *timeInfo;
  int i;

  uint8_t oBuff[MESSAGE_BUFF_SIZE];
  uint8_t *buffPtr;
  uint8_t buff[128];
  uint8_t rc;

  char errorString[] = "rbqAddMessage returned a bad return code ";

  if (msgLen != BASE_RECORD_LENGTH) {

#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print("processDataHumanReadable - ERROR!!!\n");
    DEBUG_SERIAL.print("dataLen not = BASE_RECORD_LENGTH!!!\n");
#endif // SERIAL_DEBUG

    return(RBQ_HR_WRONG_SIZE);
  }

  oBuff[0] = 0;
  strcat(oBuff, "\nGMT=");
  timeInfo = gmtime(idPtr->idGPSTime);
  buffPtr = asctime(timeInfo);
  strcat(oBuff, buffPtr);

//#define SERIAL_DEBUG_GMT

#ifdef SERIAL_DEBUG_GMT
    DEBUG_SERIAL.print("\nGMT debug=");
    DEBUG_SERIAL.print(buffPtr);
    DEBUG_SERIAL.print("\n");
#endif // SERIAL_DEBUG_ROCKBLOCK

    strcat(oBuff, "\nLBT=");
    timeInfo = gmtime(idPtr->idLastBootTime);
    buffPtr = asctime(timeInfo);
    strcat(oBuff, buffPtr);
    strcat(oBuff, "\nLat=");
    buffPtr = dtostrf(idPtr->idLatitude, 4, 6, buff);
    strcat(oBuff, buffPtr);
    strcat(oBuff, "\nLon=");
    buffPtr = dtostrf(idPtr->idLongitude, 4, 6, buff);
    strcat(oBuff, buffPtr);
//    strcat(oBuff, "\nTmp=");
//    buffPtr = dtostrf(idPtr->idTemperature, 4, 2, buff);
//    strcat(oBuff, buffPtr);
    strcat(oBuff, "\nBP=");
    buffPtr = dtostrf(idPtr->idPressure, 6, 2, buff);
    strcat(oBuff, buffPtr);
    strcat(oBuff, " hPa\nTs=");
    buffPtr = dtostrf(idPtr->idRemoteTemp, 4, 2, buff);
    strcat(oBuff, buffPtr);
    strcat(oBuff, " C = ");
    buffPtr = dtostrf(((idPtr->idRemoteTemp * 1.8) + 32), 4, 2, buff);
    strcat(oBuff, buffPtr);
    strcat(oBuff, " F\n");
    strcat(oBuff, "\nIcedrifter H/V ");
    strcat(oBuff, HARDWARE_VERSION);
    strcat(oBuff, " S/V ");
    strcat(oBuff, SOFTWARE_VERSION);

#ifdef SERIAL_DEBUG
    strcat(oBuff, "\n*** Debug is ON ***\n");
    DEBUG_SERIAL.print((char *)&oBuff);
    delay(1000);  
#endif // SERIAL_DEBUG
    strcat(oBuff, "\r");

    if (i = rbqFindEmptyEntry() == -1) {
      return(RBQ_QUEUE_FULL);
    }

      rbqQueue[i].msgType = msgType;
      rbqQueue[i].msgLen = msgLen;
      memmove(rbqQueue[i].msg, idPtr, msgLen);
      return (RBQ_GOOD);
//    rc = rbqAddMessage(oBuff, strlen(oBuff), RBQ_MSG_TYPE_CHAR);
/*
    if (rc != RBQ_GOOD) {
      oBuff[0] = 0;
      strcat(oBuff, errorString);
      sprintf(buff, "%d", rc);
      strcat(obuff, buff);
      strcat(obuff, "\n\r");

      if(rc = rbInit() != ISBD_SUCCESS) {
#ifdef SERIAL_DEBUG
        DEBUG_SERIAL.print("processData - bad return code from rbInit = ");
        DEBUG_SERIAL.print(rc);
        DEBUG_SERIAL.print("\n");
        delay(1000);  
#endif // SERIAL_DEBUG
        return(rc);
      }

      if (rc = rbTransmit(oBuff, 0, RBQ_MSG_TYPE_CHAR) != ISBD_SUCCESS) {
#ifdef SERIAL_DEBUG
        DEBUG_SERIAL.print("processData.cpp - bad return code from rbTransmit = ");
        DEBUG_SERIAL.print(rc);
        DEBUG_SERIAL.print("\n");
        delay(1000);  
#endif // SERIAL_DEBUG
        return(rc);
      }
      rbShutdown();
    }
*/
  return(rc);
}

int rbqProcessChunks(icedrifterData *idPtr, uint16_t msgLen, uint8_t msgType) {

  int recCount;
  uint8_t *dataPtr;
  uint8_t *chunkPtr;
//  uint8_t *
  uint16_t chunkLen;
  uint16_t wkLen;
  uint8_t wkPtr;  
  int rc;

  recCount = 0;
  dataPtr = (uint8_t *)idPtr;
  chunkPtr = (uint8_t *)&dcChunk.dcBuffer;
  chunkLen = msgLen;

//#ifdef HUMAN_READABLE_DISPLAY
//  if (dataLen == BASE_RECORD_LENGTH) {
//    rbqProcessDataHumanReadable(idPtr, dataLen, dataType);
//#ifdef
//  if (dataLen <= MESSAGE_BUFF_SIZE) {
//    rc = rbqAddMessage(idPtr, dataLen, dataType);
//#endif
//  } else {

  while (chunkLen > 0) {

    if (rbqFindEmptyEntry() == -1) {
      if (recCount == 0) {
        return(RBQ_QUEUE_FULL_0);
      } else if (recCount == 1) {
        return(RBQ_QUEUE_FULL_1);
      } else {
        return(RBQ_QUEUE_FULL_2);
      }
    }

    dcChunk.dcSendTime = idPtr->idGPSTime;
    dcChunk.dcRecordType[0] = 'I';
    dcChunk.dcRecordType[1] = 'D';
    dcChunk.dcRecordNumber = recCount;

    if (chunkLen > MAX_CHUNK_DATA_LENGTH) {
      wkLen = MAX_CHUNK_LENGTH;
      chunkLen -= MAX_CHUNK_DATA_LENGTH;
    } else {
      wkLen = (wkLen + CHUNK_HEADER_SIZE);
      chunkLen = 0;
    }

    memmove(chunkPtr, wkPtr, chunkLen);
    wkPtr += MAX_CHUNK_DATA_LENGTH;
    ++recCount;

#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.print(F("Chunk address="));
    DEBUG_SERIAL.print((long)chunkPtr, HEX);
    DEBUG_SERIAL.print(F(" Chunk length="));
    DEBUG_SERIAL.print(chunkLen);
    DEBUG_SERIAL.print(F("\n"));
    wkPtr = (uint8_t *)&idcChunk;

    for (i = 0; i < chunkLen; i++) {
      rbprintHexChar((uint8_t)*wkPtr);
      ++wkPtr;
    }

    DEBUG_SERIAL.print(F("\n"));
#endif // SERIAL_DEBUG

    rc = rbqFindEmptyEntry();

    if (rc != RBQ_GOOD) {
      return(rc);
    }

    rbqQueue[i].msgLen = msgLen;
    rbqQueue[i].msgType = msgType;
    memmove(rbqQueue[i].msg, msgPtr, msgLen);
    }
  }
  return(RBQ_GOOD);
}


