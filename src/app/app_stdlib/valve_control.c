#include "valve_control.h"

#define GENERIC_VALVE_CONTROL_ALERT_TIMEOUT     2000

void
valve_control_init(valve_control_t* v,
    bool* in_opened,
    bool* in_closed,
    bool* in_opening,
    bool* in_closing,
    bool* in_open_req,
    bool* in_close_req,
    bool* in_open_IL,
    bool* in_close_IL,
    
    bool* out_open_op,
    bool* out_close_op,
    bool* out_fail_to_open,
    bool* out_fail_to_close,
    bool* out_is_closing,
    bool* out_is_opening,
    bool* out_is_closed,
    bool* out_is_opened,
    bool* out_ux_opened,
    bool* out_ux_closed,
    bool* out_fault,

    unsigned long FTR_TMR)
{
  v->in_opened    = in_opened;
  v->in_closed    = in_closed;
  v->in_opening   = in_opening;
  v->in_closing   = in_closing;
  v->in_open_req  = in_open_req;
  v->in_close_req = in_close_req;
  v->in_open_IL   = in_open_IL;
  v->in_close_IL  = in_close_IL;

  v->out_open_op        = out_open_op;
  v->out_close_op       = out_close_op;
  v->out_fail_to_open   = out_fail_to_open;
  v->out_fail_to_close  = out_fail_to_close;
  v->out_is_closing     = out_is_closing;
  v->out_is_opening     = out_is_opening;
  v->out_is_closed      = out_is_closed;
  v->out_is_opened      = out_is_opened;
  v->out_ux_opened      = out_ux_opened;
  v->out_ux_closed      = out_ux_closed;
  v->out_fault          = out_fault;

  v->status_opened        = FALSE;
  v->status_closed        = FALSE;
  v->status_opening       = FALSE;
  v->status_closing       = FALSE;
  v->status_unx_open      = FALSE;
  v->status_unx_close     = FALSE;
  v->status_indeterminate = FALSE;
  v->status_fail_to_close = FALSE;
  v->status_fail_to_open  = FALSE;

  v->state  = valve_control_state_indeterminate;

  v->FTR_TMR    = FTR_TMR;
  TINIT(&v->ftr_tmr);
  TINIT(&v->alert_tmr);
}

void
valve_control_run(valve_control_t* v)
{
  // based state
  if(!(*v->in_opened) && !(*v->in_closed) && !(*v->in_opening) && !(*v->in_closing))
  {
    v->state = valve_control_state_indeterminate;
  }

  if(*v->in_opened && *v->in_closed)
  {
    v->state = valve_control_state_indeterminate;
  }

  // opened
  if(*v->in_opened && !(*v->in_closed))
  {
    v->state = valve_control_state_opened;

    // unexpected open
    if(v->status_closed)
    {
      v->status_unx_open = TRUE;
    }
  }

  // closed
  if(*v->in_closed && !(*v->in_opened))
  {
    v->state = valve_control_state_closed;

    // unexpected closed
    if(v->status_opened)
    {
      v->status_unx_close = TRUE;
    }
  }

  // override feedback signal if we were opening or closing
  if(v->status_opening)
  {
    v->state = valve_control_state_opening;
  }

  if(v->status_closing)
  {
    v->state = valve_control_state_closing;
  }

  switch(v->state)
  {
  case valve_control_state_closed:
    v->status_opening       = FALSE;
    v->status_closing       = FALSE;
    v->status_closed        = TRUE;
    v->status_opened        = FALSE;
    v->status_indeterminate = FALSE;

    // open request
    if(*v->in_open_req && *v->in_open_IL)
    {
      v->state = valve_control_state_opening;
      v->status_opening       = TRUE;
      v->status_closing       = FALSE;
      v->status_closed        = FALSE;
      v->status_opened        = FALSE;
      v->status_indeterminate = FALSE;

      *v->out_open_op   = TRUE;
      TSTART(&v->ftr_tmr);
    }
    break;

  case valve_control_state_opened:
    v->status_opening       = FALSE;
    v->status_closing       = FALSE;
    v->status_closed        = FALSE;
    v->status_opened        = TRUE;
    v->status_indeterminate = FALSE;

    // close request
    if(*v->in_close_req && *v->in_close_IL)
    {
      v->state = valve_control_state_closing;
      v->status_opening       = FALSE;
      v->status_closing       = TRUE;
      v->status_closed        = FALSE;
      v->status_opened        = FALSE;
      v->status_indeterminate = FALSE;

      *v->out_close_op  = TRUE;
      TSTART(&v->ftr_tmr);
    }
    break;

  case valve_control_state_closing:
    v->status_opening       = FALSE;
    v->status_closing       = TRUE;
    v->status_closed        = FALSE;
    v->status_opened        = FALSE;
    v->status_indeterminate = FALSE;

    if(*v->in_closed)
    {
      v->state = valve_control_state_closed;
      v->status_opening       = FALSE;
      v->status_closing       = FALSE;
      v->status_closed        = TRUE;
      v->status_opened        = FALSE;
      v->status_indeterminate = FALSE;

      *v->out_close_op  = FALSE;
      TSTOP(&v->ftr_tmr);
    }
    else if((TELAPSED(&v->ftr_tmr) > v->FTR_TMR) || *v->in_open_req)
    {
      // failed to close or open request during closing
      v->status_closing       = FALSE;
      v->status_closed        = FALSE;
      v->status_opened        = FALSE;

      *v->out_close_op        = FALSE;

      if(*v->in_open_req)
      {
        // open request
        v->state = valve_control_state_opening;
        v->status_opening       = TRUE;
        v->status_indeterminate = FALSE;

        *v->out_open_op   = TRUE;

        TSTOP(&v->ftr_tmr);
        TSTART(&v->ftr_tmr);
      }
      else if(TELAPSED(&v->ftr_tmr) > v->FTR_TMR)
      {
        // failed to close
        *v->out_fail_to_close    = TRUE;
        v->status_fail_to_close = TRUE;

        if(!(*v->in_open_IL))
        {
          // can't open again
          v->state = valve_control_state_indeterminate;
          v->status_opening       = FALSE;
          v->status_indeterminate = TRUE;

          *v->out_open_op = FALSE;
          TSTOP(&v->ftr_tmr);
        }
        else
        {
          // failed to close. open again
          v->state = valve_control_state_opening;
          v->status_opening       = TRUE;
          v->status_indeterminate = FALSE;

          *v->out_open_op = TRUE;
          TSTOP(&v->ftr_tmr);
          TSTART(&v->ftr_tmr);
        }
      }
    }
    break;

  case valve_control_state_opening:
    v->status_opening       = TRUE;
    v->status_closing       = FALSE;
    v->status_closed        = FALSE;
    v->status_opened        = FALSE;
    v->status_indeterminate = FALSE;

    if(*v->in_opened)
    {
      v->state = valve_control_state_opened;
      v->status_opening       = FALSE;
      v->status_closing       = FALSE;
      v->status_closed        = FALSE;
      v->status_opened        = TRUE;
      v->status_indeterminate = FALSE;

      *v->out_open_op = FALSE;
      TSTOP(&v->ftr_tmr);
    }
    else if((TELAPSED(&v->ftr_tmr) > v->FTR_TMR) || *v->in_close_req)
    {
      // failed to open or close request during opening
      v->status_opening       = FALSE;
      v->status_closed        = FALSE;
      v->status_opened        = FALSE;

      *v->out_open_op        = FALSE;

      if(*v->in_close_req)
      {
        // close request
        v->state = valve_control_state_closing;
        v->status_closing       = TRUE;
        v->status_indeterminate = FALSE;

        *v->out_close_op        = FALSE;

        TSTOP(&v->ftr_tmr);
        TSTART(&v->ftr_tmr);
      }
      else if(TELAPSED(&v->ftr_tmr) > v->FTR_TMR)
      {
        // failed to open
        *v->out_fail_to_open    = TRUE;
        v->status_fail_to_open  = TRUE;

        if(!(*v->in_close_IL))
        {
          // can't close again
          v->state = valve_control_state_indeterminate;
          v->status_closing       = FALSE;
          v->status_indeterminate = TRUE;

          *v->out_close_op  = FALSE;
          *v->out_open_op   = FALSE;

          TSTOP(&v->ftr_tmr);
        }
        else
        {
          // close again
          v->state = valve_control_state_closing;
          v->status_closing       = TRUE;
          v->status_indeterminate = FALSE;

          *v->out_close_op  = TRUE;
          *v->out_open_op   = FALSE;

          TSTOP(&v->ftr_tmr);
          TSTART(&v->ftr_tmr);
        }
      }
    }
    break;

  case valve_control_state_indeterminate:
    v->status_opening       = FALSE;
    v->status_closing       = FALSE;
    v->status_closed        = FALSE;
    v->status_opened        = FALSE;
    v->status_indeterminate = TRUE;

    // close req
    if(*v->in_close_req && *v->in_close_IL)
    {
      v->state  = valve_control_state_closing;
      v->status_opening   = FALSE;
      v->status_closing   = TRUE;
      v->status_closed    = FALSE;
      v->status_opened    = FALSE;

      *v->out_close_op    = TRUE;

      TSTART(&v->ftr_tmr);
    }

    // open req
    if(*v->in_open_req && *v->in_open_IL)
    {
      v->state  = valve_control_state_opening;
      v->status_opening   = TRUE;
      v->status_closing   = FALSE;
      v->status_closed    = FALSE;
      v->status_opened    = FALSE;

      *v->out_open_op     = TRUE;

      TSTART(&v->ftr_tmr);
    }
    break;
  }

  // control alert signals
  if(v->status_unx_close      ||
     v->status_unx_open       ||
     v->status_fail_to_close  ||
     v->status_fail_to_open   ||
     v->status_indeterminate)
  {
    TSTART(&v->alert_tmr);

    if(TELAPSED(&v->alert_tmr) > GENERIC_VALVE_CONTROL_ALERT_TIMEOUT)
    {
      v->status_unx_close     = FALSE;
      v->status_unx_open      = FALSE;
      v->status_fail_to_open  = FALSE;
      v->status_fail_to_close = FALSE;
      v->status_indeterminate = FALSE;

      *v->out_fail_to_close   = FALSE;
      *v->out_fail_to_open    = FALSE;

      TSTART(&v->alert_tmr);
    }
  }
  else
  {
    TSTOP(&v->alert_tmr);
  }

  // assert out signals
  *v->out_is_closed   = v->status_closed;
  *v->out_is_closing  = v->status_closing;
  *v->out_is_opened   = v->status_opened;
  *v->out_is_opening  = v->status_opening;
  *v->out_ux_opened   = v->status_unx_open;
  *v->out_ux_closed   = v->status_unx_close;
  *v->out_fault       = v->status_indeterminate;
}
