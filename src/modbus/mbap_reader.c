#include "trace.h"
#include "mbap_reader.h"

static inline void
mbap_state_machine(mbap_reader_t* mbap, uint8_t data)
{
  switch(mbap->state)
  {
  case mbap_reader_state_header:
    mbap->frame[mbap->ndx++] = data;
    mbap->header_remaining--;

    if(mbap->header_remaining == 0)
    {
      // decode header
      mbap->tid     = (mbap->frame[0] << 8 | mbap->frame[1]);
      mbap->pid     = (mbap->frame[2] << 8 | mbap->frame[3]);
      mbap->length  = (mbap->frame[4] << 8 | mbap->frame[5]);
      mbap->uid     = mbap->frame[6];

      // taking unit ID into account
      mbap->data_remaining = mbap->length - 1;
      mbap->state = mbap_reader_state_pdu;

      TRACE(MBAP, "moving to state pdu: data to read: %d\n", mbap->data_remaining);
    }
    break;

  case mbap_reader_state_pdu:
    if(mbap->ndx >= MB_TCP_PDU_SIZE_MAX)
    {
      TRACE(MBAP, "MBAP rx buffer overflow %d:%d:%d:%d\n",
          mbap->tid, mbap->pid, mbap->length, mbap->uid);
      mbap_reader_reset(mbap);
      return;
    }

    mbap->frame[mbap->ndx++] = data;
    mbap->data_remaining--;

    if(mbap->data_remaining == 0)
    {
      TRACE(MBAP, "got mbap frame:\n");
      mbap->cb(mbap);
      mbap_reader_reset(mbap);
    }
    break;
  }
}

void
mbap_reader_init(mbap_reader_t* mbap, mbap_reader_cb cb)
{
  mbap->cb    = cb;
  mbap_reader_reset(mbap);
}

void
mbap_reader_reset(mbap_reader_t* mbap)
{
  mbap->state   = mbap_reader_state_header;

  mbap->tid     = 0;
  mbap->pid     = 0;
  mbap->length  = 0;
  mbap->uid     = 0;

  mbap->header_remaining    = 7;
  mbap->data_remaining      = 0;
  mbap->ndx                 = 0;
}

void
mbap_reader_feed(mbap_reader_t* mbap, uint8_t* buf, int len)
{
  TRACE(MBAP, "feeding %d bytes\n", len);
  for(int i = 0; i < len; i++)
  {
    mbap_state_machine(mbap, buf[i]);
  }
}
