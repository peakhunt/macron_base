#include "control_lib.h"
#include "time_util.h"

////////////////////////////////////////////////////////////////////////////////
//
// SR Latch
//
////////////////////////////////////////////////////////////////////////////////
void
sr_latch_init(sr_latch_t* sr, bool q)
{
  sr->q     =   q;
}

void
sr_latch(sr_latch_t* sr, bool set, bool reset)
{
  if(set == FALSE)
  {
    if(reset == FALSE)
    {
      // retain previous q
    }
    else
    {
      sr->q = FALSE;
    }
  }
  else
  {
    //
    // if set is TRUE,
    // q is set no matter what
    //
    sr->q = TRUE;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// RS Latch
//
////////////////////////////////////////////////////////////////////////////////
void
rs_latch_init(rs_latch_t* rs, bool q)
{
  rs->q     =   q;
}

void
rs_latch(rs_latch_t* rs, bool set, bool reset)
{
  if(set == FALSE)
  {
    if(reset == FALSE)
    {
      // retain previous q
    }
    else
    {
      rs->q = FALSE;
    }
  }
  else
  {
    // if set, reset is first respected
    if(reset == FALSE)
    {
      rs->q = FALSE;
    }
    else
    {
      rs->q = TRUE;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// R_TRIG
//
////////////////////////////////////////////////////////////////////////////////
void
r_trig_init(r_trig_t* rt)
{
  rt->q         = FALSE;
  rt->prev_clk  = FALSE;
}

void
r_trig(r_trig_t* rt, bool clk)
{
  if(rt->prev_clk == FALSE && clk == TRUE)
  {
    rt->q = TRUE;
  }
  else
  {
    rt->q = FALSE;
  }
  rt->prev_clk = clk;
}

////////////////////////////////////////////////////////////////////////////////
//
// F_TRIG
//
////////////////////////////////////////////////////////////////////////////////
void
f_trig_init(f_trig_t* ft)
{
  ft->q         = FALSE;
  ft->prev_clk  = FALSE;
}

void
f_trig(f_trig_t* ft, bool clk)
{
  if(ft->prev_clk == TRUE && clk == FALSE)
  {
    ft->q = TRUE;
  }
  else
  {
    ft->q = FALSE;
  }
  ft->prev_clk = clk;
}

////////////////////////////////////////////////////////////////////////////////
//
// TON
//
////////////////////////////////////////////////////////////////////////////////
void
ton_init(ton_t* ton)
{
  ton->q            = FALSE;
  ton->prev_in      = FALSE;
  ton->tmr_running  = FALSE;
  ton->start_time   = 0;
}

void
ton(ton_t* ton, bool in, uint32_t pt)
{
  if(ton->prev_in == FALSE && in == TRUE)
  {
    // rising edge
    ton->start_time   = time_util_get_sys_clock_in_ms();
    ton->tmr_running  = TRUE;
  }
  else  if(ton->prev_in == TRUE && in == FALSE)
  {
    // falling edge
    ton->tmr_running  = FALSE;
    ton->q            = FALSE;
  }

  ton->prev_in = in;

  if(ton->tmr_running)
  {
    if(time_util_get_sys_clock_elapsed_in_ms(ton->start_time) >= pt)
    {
      // timed out
      ton->q = TRUE;
      ton->tmr_running = FALSE;
    }
  }
}
