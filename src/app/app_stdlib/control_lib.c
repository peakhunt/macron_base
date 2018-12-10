#include "control_lib.h"
#include "time_util.h"

#ifdef CONTROL_LIB_TEST
extern unsigned long time_now_in_ms(void);
extern unsigned long time_delta_in_ms(unsigned long start);

#else
#define time_now_in_ms      time_util_get_sys_clock_in_ms
#define time_delta_in_ms    time_util_get_sys_clock_elapsed_in_ms
#endif

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
      rs->q = TRUE;
    }
    else
    {
      rs->q = FALSE;
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
    ton->start_time   = time_now_in_ms();
    ton->tmr_running  = TRUE;
  }
  else if(ton->prev_in == TRUE && in == FALSE)
  {
    // falling edge
    ton->tmr_running  = FALSE;
    ton->q            = FALSE;
  }

  ton->prev_in = in;

  if(ton->tmr_running)
  {
    if(time_delta_in_ms(ton->start_time) >= pt)
    {
      // timed out
      ton->q = TRUE;
      ton->tmr_running = FALSE;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// TOFF
//
////////////////////////////////////////////////////////////////////////////////
void
toff_init(toff_t* toff)
{
  toff->q            = TRUE;
  toff->prev_in      = FALSE;
  toff->tmr_running  = FALSE;
  toff->start_time   = 0;
}

void
toff(toff_t* toff, bool in, uint32_t pt)
{
  if(toff->prev_in == TRUE && in == FALSE)
  {
    // falling edge
    toff->start_time  = time_now_in_ms();
    toff->tmr_running = TRUE;
  }
  else if(toff->prev_in == FALSE && in == TRUE)
  {
    // rising edge
    toff->tmr_running = FALSE;
    toff->q           = TRUE;
  }

  toff->prev_in = in;

  if(toff->tmr_running)
  {
    if(time_delta_in_ms(toff->start_time) >= pt)
    {
      // timed out
      toff->q             = FALSE;
      toff->tmr_running   = FALSE;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// TP
//
////////////////////////////////////////////////////////////////////////////////
void
tp_init(tp_t* tp)
{
  tp->q           = FALSE;
  tp->prev_in     = FALSE;
  tp->start_time  = 0;
}

void
tp(tp_t* tp, bool in, uint32_t pt)
{
  if(tp->q == FALSE)
  {
    if(tp->prev_in == FALSE && in == TRUE)
    {
      tp->q = TRUE;
      tp->start_time  = time_now_in_ms();
    }
  }
  else
  {
    if(time_delta_in_ms(tp->start_time) >= pt)
    {
      tp->q = FALSE;
    }
  }

  tp->prev_in = in;
}

////////////////////////////////////////////////////////////////////////////////
//
// blink
//
////////////////////////////////////////////////////////////////////////////////
void
blink_init(blink_t* bl)
{
  bl->q       = FALSE;
  bl->running = FALSE;
}

void
blink(blink_t* bl, bool run, uint32_t cycle)
{
  if(run == TRUE && bl->running == FALSE)
  {
    bl->running     = TRUE;
    bl->start_time  = time_now_in_ms();
    bl->q           = FALSE;
  }
  else if(run == FALSE)
  {
    bl->running = FALSE;
    bl->q       = FALSE;
  }

  if(bl->running)
  {
    if(time_delta_in_ms(bl->start_time) >= (cycle/2))
    {
      bl->q   = !bl->q;
    }
  }
}
