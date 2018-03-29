#include <unistd.h>
#include <errno.h>
#include "common_def.h"
#include "list.h"
#include "stream.h"
#include "trace.h"

static void
stream_handle_tx_event(stream_t* stream)
{
  uint8_t   buffer[128];
  int       data_size,
            ret,
            len;

  if(circ_buffer_is_empty(&stream->tx_buf))
  {
    TRACE(STREAM, "disabling TX event. circ buffer empty now\n");
    watcher_no_watch_tx(&stream->watcher);
    return;
  }

  // XXX
  // looping would be more efficient but
  // on the other hand, this has better time sharing feature
  //
  data_size = circ_buffer_get_data_size(&stream->tx_buf);
  len = data_size < 128 ? data_size : 128;
  circ_buffer_peek(&stream->tx_buf, buffer, len);

  ret = write(stream->watcher.fd, buffer, len);
  if(ret <= 0)
  {
    // definitely stream got into a trouble
    watcher_no_watch_tx(&stream->watcher);
    TRACE(STREAM, "XXXXXXXX this should not happen\n");
    CRASH();
    return;
  }

  circ_buffer_advance(&stream->tx_buf, ret);

  if(circ_buffer_is_empty(&stream->tx_buf))
  {
    watcher_no_watch_tx(&stream->watcher);
  }

  return;
}

static void
stream_watcher_callback(watcher_t* watcher, watcher_event_t evt)
{
  int             ret;
  stream_t*       stream = container_of(watcher, stream_t, watcher);
  stream_event_t  sevt;

  //log_write("watch event: %d\n", evt);
  switch(evt)
  {
  case watcher_event_rx:
    ret = read(watcher->fd, stream->rx_buf, stream->rx_buf_size);
    if(ret > 0)
    {
      sevt = stream_event_rx;
      stream->rx_data_len = ret;
    }
    else if(ret == 0)
    {
      sevt = stream_event_eof;
    }
    else
    {
      sevt = stream_event_err;
    }
    break;

  case watcher_event_tx:
    stream_handle_tx_event(stream);
    return;

  case watcher_event_error:
    sevt = stream_event_err;
    break;
  }

  stream->cb(stream, sevt);
}


void
stream_init_with_fd(stream_t* stream, int fd, uint8_t* rx_buf, int rx_buf_size, int tx_buf_size)
{
  // XXX return value check
  circ_buffer_init(&stream->tx_buf, tx_buf_size);

  stream->rx_buf      = rx_buf;
  stream->rx_buf_size = rx_buf_size;

  watcher_init_with_fd(&stream->watcher, fd, stream_watcher_callback);

  watcher_watch_rx(&stream->watcher);
  watcher_watch_err(&stream->watcher);
}

void
stream_deinit(stream_t* stream)
{
  circ_buffer_deinit(&stream->tx_buf);
  stream_stop(stream);
  close(stream->watcher.fd);
}

void
stream_start(stream_t* stream)
{
  watcher_start(&stream->watcher);
}

void
stream_stop(stream_t* stream)
{
  watcher_stop(&stream->watcher);
}

bool
stream_write(stream_t* stream, uint8_t* data, int len)
{
  int ret;

  if(circ_buffer_is_empty(&stream->tx_buf) == FALSE)
  {
    // something got queued up due to socket buffer full
    // tx watcher is alreadt started in this case
    return circ_buffer_put(&stream->tx_buf, data, len) == 0;
  }
  else
  {
    ret = write(stream->watcher.fd, data, len);
    if(ret == len)
    {
      return TRUE;
    }

    if(ret < 0)
    {
      if(!(errno == EWOULDBLOCK || errno == EAGAIN))
      {
        TRACE(STREAM, "stream tx error other than EWOULDBLOCK\n");
        return FALSE;
      }
      ret = 0;
    }

    // start tx watcher and queue remaining data
    if(circ_buffer_put(&stream->tx_buf, &data[ret], len - ret) == FALSE)
    {
      TRACE(STREAM, "can't TX.. circ buffer overflow\n");
      return FALSE;
    }

    TRACE(STREAM, "enabling TX event\n");
    watcher_watch_tx(&stream->watcher);
    return TRUE;
  }
}
