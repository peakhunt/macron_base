#include "generic_starter.h"

#define GENERIC_START_ALERT_TIMEOUT     2000

void
generic_starter_init(generic_starter_t* gs)
{
  //
  // not necessary to initialize input but
  // just to make sure
  //
  gs->in_running      = FALSE;
  gs->in_stop_IL      = FALSE;
  gs->in_starting     = FALSE;
  gs->in_stopped      = FALSE;
  gs->in_start_req    = FALSE;
  gs->in_stop_req     = FALSE;
  gs->in_start_IL     = FALSE;
  gs->in_stop_IL      = FALSE;

  gs->out_start             = FALSE;
  gs->out_stop              = FALSE;
  gs->out_failed_to_start   = FALSE;
  gs->out_failed_to_stop    = FALSE;
  gs->out_is_starting       = FALSE;
  gs->out_is_stopping       = FALSE;
  gs->out_is_running        = FALSE;
  gs->out_is_stopped        = FALSE;
  gs->out_ux_start          = FALSE;
  gs->out_ux_stop           = FALSE;

  gs->status_stopg          = FALSE;
  gs->status_strtg          = FALSE;
  gs->status_stopd          = FALSE;
  gs->status_rung           = FALSE;
  gs->status_unx_strt       = FALSE;
  gs->status_unx_stop       = FALSE;
  gs->status_ft_strt        = FALSE;
  gs->status_ft_stop        = FALSE;

  gs->state = generic_starter_state_stopped;

  TINIT(&gs->ftr_tmr);
  TINIT(&gs->alert_tmr);
}

void
generic_starter_run(generic_starter_t* gs)
{
  // check if machinery is running
  if(gs->in_running == TRUE && gs->in_stopped == FALSE)
  {
    gs->state   = generic_starter_state_running;

    // unexpected start
    if(gs->status_stopd)
    {
      gs->status_unx_strt = TRUE;
    }
  }

  // check if machinery is stopped
  if(gs->in_stopped == TRUE && gs->in_running == FALSE)
  {
    gs->state = generic_starter_state_stopped;

    // unexpected stop
    if(gs->status_rung == TRUE)
    {
      gs->status_unx_stop = TRUE;
    }
  }

  // override feedback signals if we were starting or stopping
  if(gs->status_strtg == TRUE)
  {
    gs->state = generic_starter_state_starting;
  }

  if(gs->status_stopd == TRUE)
  {
    gs->state = generic_starter_state_stopping;
  }

  switch(gs->state)
  {
  case generic_starter_state_stopped:
    gs->status_strtg    = FALSE;
    gs->status_stopg    = FALSE;
    gs->status_stopd    = TRUE;
    gs->status_rung     = FALSE;

    // start request
    if(gs->in_start_req == TRUE && gs->in_start_IL == TRUE)
    {
      gs->state = generic_starter_state_starting;
      gs->status_strtg  = TRUE;
      gs->status_stopg  = FALSE;
      gs->status_stopd  = FALSE;
      gs->status_rung   = FALSE;

      gs->out_start     = TRUE;

      TSTART(&gs->ftr_tmr);
    }
    break;

  case generic_starter_state_stopping:
    gs->status_strtg    = FALSE;
    gs->status_stopg    = TRUE;
    gs->status_stopd    = FALSE;
    gs->status_rung     = FALSE;

    if(gs->in_stopped == TRUE)
    {
      // stopped
      gs->state = generic_starter_state_stopped;
      gs->status_strtg    = FALSE;
      gs->status_stopg    = FALSE;
      gs->status_stopd    = TRUE;
      gs->status_rung     = FALSE;

      gs->out_stop        = FALSE;

      TSTOP(&gs->ftr_tmr);
    }
    else if(TELAPSED(&gs->ftr_tmr) > gs->FTR_TMR)
    {
      // failed to stopped within timeout
      gs->state = generic_starter_state_running;
      gs->status_strtg    = FALSE;
      gs->status_stopg    = FALSE;
      gs->status_stopd    = FALSE;
      gs->status_rung     = TRUE;
      gs->status_ft_stop  = TRUE;

      gs->out_stop        = FALSE;

      TSTOP(&gs->ftr_tmr);
    }
    else if(gs->in_start_req == TRUE)
    {
      // start request during stop sequence
      gs->state = generic_starter_state_starting;

      gs->status_strtg    = TRUE;
      gs->status_stopg    = FALSE;
      gs->status_stopd    = FALSE;
      gs->status_rung     = FALSE;

      gs->out_stop        = FALSE;
      gs->out_start       = TRUE;
    }
    break;

  case generic_starter_state_running:
    gs->status_strtg      = FALSE;
    gs->status_stopg      = FALSE;
    gs->status_stopd      = FALSE;
    gs->status_rung       = TRUE;

    // stop request
    if(gs->in_stop_req == TRUE && gs->in_stop_IL == TRUE)
    {
      gs->state = generic_starter_state_stopping;

      gs->status_strtg    = FALSE;
      gs->status_stopg    = TRUE;
      gs->status_stopd    = FALSE;
      gs->status_rung     = FALSE;

      gs->out_stop        = TRUE;

      TSTART(&gs->ftr_tmr);
    }
    break;

  case generic_starter_state_starting:
    gs->status_strtg    = TRUE;
    gs->status_stopg    = FALSE;
    gs->status_stopd    = FALSE;
    gs->status_rung     = FALSE;

    if(gs->in_running == TRUE)
    {
      // started
      gs->state = generic_starter_state_running;

      gs->status_strtg  = FALSE;
      gs->status_stopg  = FALSE;
      gs->status_stopd  = FALSE;
      gs->status_rung   = TRUE;

      gs->out_start     = FALSE;

      TSTART(&gs->ftr_tmr);
    }
    else if(TELAPSED(&gs->ftr_tmr) > gs->FTR_TMR)
    {
      // failed to start
      gs->state = generic_starter_state_stopped;

      gs->status_strtg    = FALSE;
      gs->status_stopg    = FALSE;
      gs->status_stopd    = TRUE;
      gs->status_rung     = FALSE;
      gs->status_ft_strt  = TRUE;

      gs->out_start       = FALSE;

    }
    else if(gs->in_stop_req)
    {
      // stopping request while starting
      gs->state = generic_starter_state_stopping;

      gs->status_strtg    = FALSE;
      gs->status_stopg    = TRUE;
      gs->status_stopd    = FALSE;
      gs->status_rung     = FALSE;

      gs->out_stop        = TRUE;
      gs->out_start       = FALSE;
    }
    break;
  }

  // clear unexpected and failed to respond alerts
  if((gs->status_unx_strt == TRUE || gs->status_unx_stop) ||
     (gs->status_ft_strt || gs->status_ft_stop))
  {
    TSTART(&gs->alert_tmr);

    if(TELAPSED(&gs->alert_tmr) > GENERIC_START_ALERT_TIMEOUT)
    {
      gs->status_unx_strt   = FALSE;
      gs->status_unx_stop   = FALSE;
      gs->status_ft_strt    = FALSE;
      gs->status_ft_stop    = FALSE;

      TSTOP(&gs->alert_tmr);
    }
  }

  //
  // assert output signals
  //
  gs->out_is_stopped      = gs->status_stopd;
  gs->out_is_stopping     = gs->status_stopg;
  gs->out_is_running      = gs->status_rung;
  gs->out_is_starting     = gs->status_strtg;
  gs->out_ux_start        = gs->status_unx_strt;
  gs->out_ux_stop         = gs->status_unx_stop;
  gs->out_failed_to_start = gs->status_ft_strt;
  gs->out_failed_to_stop  = gs->status_ft_stop;
}
