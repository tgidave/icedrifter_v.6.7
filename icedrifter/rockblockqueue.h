/*                                                                              
 *  rockblockqueue.h                                                  
 *                                                                               
 *  Header file for implementing the rockblcokqueue code.                     
 */

#ifndef _ROCKBLOCKQUEUE_H
  #define _ROCKBLOCKQUEUE_H

#include "icedrifter.h"

typedef struct rbqEntry {
  uint32_t msgLen;
  uint32_t msgType;   
  uint8_t msg[MESSAGE_BUFF_SIZE];
} rbqEntry;

#define RBQ_GOOD            0
#define RBQ_NO_MSG_ADDR     1
#define RBQ_TOO_LONG        2
#define RBQ_INVALID_TYPE    3
#define RBQ_CHAR_TOO_LONG   4
#define RBQ_NO_CR_FOUND     5
#define RBQ_QUEUE_EMPTY     6
#define RBQ_HR_WRONG_SIZE   7
#define RBC_RESERVED_8      8
#define RBQ_QUEUE_FULL      9
#define RBQ_QUEUE_FULL_0    10
#define RBQ_QUEUE_FULL_1    11
#define RBQ_QUEUE_FULL_2    12


#define RBQ_MSG_TYPE_FREE   0
#define RBQ_MSG_TYPE_SENSOR 1 
#define RBQ_MSG_TYPE_BIN    2
#define RBQ_MSG_TYPE_CHAR   3

//#define RBQ_MSG_PRIORITY_NORMAL 0 // Add message to queue and call processData and empty the queue.
//#define RBQ_MSG_PRIORITY_LOW    1 // Only add the message to the queue.
//#define RBQ_MSG_PRIORITY_high   2 // This option is only available for char messaes.  
                                    // Just call the rockblock code and send message out.

#define RBQ_SIZE 10

#define MAX_CHUNK_LENGTH  MESSAGE_BUFF_SIZE
#define CHUNK_HEADER_SIZE 8
#define MAX_CHUNK_DATA_LENGTH (MAX_CHUNK_LENGTH - CHUNK_HEADER_SIZE)

typedef struct dataChunk {
#ifdef ARDUINO
  time_t dcSendTime;
#else
  uint32_t dcSendTime;
#endif
  char dcRecordType[2];
  uint16_t dcRecordNumber;
  uint8_t dcBuffer[MAX_CHUNK_LENGTH];
} dataChunk; 

int rbqInit(void);

int rbqAddMessage(uint8_t *msgPtr, uint16_t msgLen, uint8_t msgType);

int rbqGetMessage(rbqEntry *msgEntryOut);


#endif // _ROCKBLOCKQUEUE_H
