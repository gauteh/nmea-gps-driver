/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Interface to standard NMEA GPS.
 *
 *
 */

# pragma once

# ifndef ONLY_SPEC

# include <stdint.h>
# include "types.h"

class GPS {
# define TELEGRAM_LEN 80

# define GPS_BAUDRATE 4800
# define GPS_Serial Serial1

# define GPS_SYNC_PIN 27 // Should be 5V tolerant

  private:
    char gps_buf [TELEGRAM_LEN + 2];
    int  gps_buf_pos;

    /* Keep track of last sync pulse */
    volatile uint8_t  referencerolled; // Pass debug message from main loop

    void parse ();

  public:
    /* Telegram and data structures {{{ */
    typedef enum _GPS_TELEGRAM {
      UNSPECIFIED = 0,
      UNKNOWN,
      GPRMC,
      GPGGA,
      GPGLL,
      GPGSA,
      GPGSV,
      GPVTG,
    } GPS_TELEGRAM;

    typedef struct _GPS_DATA {
      GPS_TELEGRAM lasttype;
      char    lasttelegram[TELEGRAM_LEN];
      int     received; /* Received telegrams */
      bool    valid;
      int     fixtype;

      int     satellites;
      int     satellites_used[12];
      uint8_t    mode1;
      uint8_t    mode2;

      char    latitude[12];
      bool    north;    /* true = Latitude is north aligned, false = south */
      char    longitude[12];
      bool    east;     /* true = Longitude is east aligned, false = south */

      uint32_t   time;
      int     hour;
      int     minute;
      int     second;
      int     seconds_part;
      int     day;
      int     month;
      int     year;

      char    speedoverground[6];
      char    courseoverground[6]; /* True north */
    } GPS_DATA; // }}}

    GPS_DATA gps_data;

    GPS ();
    void        setup ();
    void        loop  ();

    static void sync_pulse_int ();
    void        sync_pulse ();
    void        enable_sync ();
    void        disable_sync ();

    void        update_second ();
    void        assert_time ();

    /* Timing
     *
     * Synchronization:
     * - Telegrams with UTC time information and validity is received
     *   continuously.
     *
     * - A PPS is received at the time of each second, this _must_ mean
     *   that it is synchronized to the second _after_ the last time fix
     *   received as a telegram.
     *
     * - The telegrams that are sent each second might continue to arrive
     *   during the pulse. There _should_ then already have arrived one
     *   time fix for the previous second.
     *
     * - This means that one series of telegrams might change to the next
     *   second - but _only_ after the pulse!
     *
     * - Any exact reference or timing should only be done in the PPS
     *   interrupt handler.
     *
     */
    bool HAS_TIME;                     // Has valid time from GPS

    volatile bool HAS_SYNC;            // Has PPS synced
    volatile bool HAS_SYNC_REFERENCE;  // Reference is set using PPS

    enum GPS_STATUS {
      NOTHING = 0b0,
      TIME = 0b1,
      SYNC = 0b10,
      SYNC_REFERENCE = 0b100,
      POSITION = 0b1000,
    };
    /* Leap seconds:
     * Are not counted in lastseconds (unix time since epoch).
     *
     * TODO: Handle if receiver is including them in telegrams.
     */


    /* The last unix time calculated from GPS telegram, with timestamp
     * in millis (). Is also incremented by a PPS signal. */
    uint64_t lastsecond;
    uint64_t lastsecond_time;

    /* The latest most reliable reference for picking by AD */
    uint64_t reference;
    uint64_t microdelta;
    uint64_t lastsync;
    uint64_t lastmicros;


    /* Time to wait before manually updating reference (in case of no sync)
     *
     * Included tolerance for millis () drift.
     *
     */
# define REFERENCE_TIMEOUT 60 // [s]

    /* Maple Native Beta Crystal: 535-9721-1-ND from DigiKey */
# define TIMING_PPM  10

# define ULONG_MAX ((2^32) - 1)

/* Overflow handling, the math.. {{{
*
* micros () overflow in about 70 minutes.
* millis () overflow in about 50 days.
*
* m   = micros
* d   = delta
*
* at t0:
* ref = const.
* d   = m0
* t   = ref + (m0 - d)
*
* at t1:
* t   = ref + (m1 - d)
*
* at t2:
* m > ULONG_MAX and is clocked around
*
* g is modulated m
*
* g = m - ULONG_MAX, we _will_ catch the overflow before a second
*                    overflow would have time to occur.
* t = ref + (m - d)
*   = ref + (g + ULONG_MAX - d)
*   = ref + (g + (ULONG_MAX - d))
*
* for d > g, this means we will have to re-calculate ref before g >= d.
*
* The re-calculation should only be based on micros () so as to not loose
* any resolution.
*
* }}} */
};

# endif

/* vim: set filetype=arduino :  */

