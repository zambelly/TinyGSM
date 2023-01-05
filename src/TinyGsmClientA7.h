/**
 * @file       TinyGsmClientA7.h
 * @author     Soma Zambelly
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2022 Soma Zambelly
 * @date       Nov 2022
 */

#ifndef SRC_TINYGSMCLIENTA7_H_
#define SRC_TINYGSMCLIENTA7_H_
// #pragma message("TinyGSM:  TinyGsmClientA7")

// #define TINY_GSM_DEBUG Serial

#include "TinyGsmClientA6.h"
#include "TinyGsmGPS.tpp"

class TinyGsmA7 : public TinyGsmA6, public TinyGsmGPS<TinyGsmA7> {
  friend class TinyGsmGPS<TinyGsmA7>;

 public:
  explicit TinyGsmA7(Stream& stream) : TinyGsmA6(stream) {}

  enum GPS_RESPONSE { GPS_TIMEOUT, GPS_SUCCESS, GPS_NO_FIX, GPS_UNKNOWN };

  int8_t getShortGPS(float* lat, float* lon) {
    float  ilat, ilon;
    int8_t retval = GPS_UNKNOWN;

    sendAT(GF("+LOCATION=2"));
    switch (waitResponse(10000L, GF("\r\n"), GF("+LOCATION: "))) {
      case 0:
        // Timeout
        retval = GPS_TIMEOUT;
        break;
      case 1:
        // Coordinates
        ilat = streamGetFloatBefore(',');
        ilon = streamGetFloatBefore('\r');

        if (ilat != -9999.0F && ilon != -9999.0F) {
          if (lat != NULL) *lat = ilat;
          if (lon != NULL) *lon = ilon;
          retval = GPS_SUCCESS;
        }
        break;
      case 2:
        // Error message ("GPS NOT FIX NOW", for example)
        if (stream.readStringUntil('\r') == "GPS NOT FIX NOW")
          retval = GPS_NO_FIX;
        break;
    }

    waitResponse();
    return retval;
  }


 protected:
  /*
   * GPS/GNSS/GLONASS location functions
   */

  // enable GPS
  bool enableGPSImpl() {
    sendAT(GF("+GPS=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    sendAT(GF("+GPS=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  String getGPSrawImpl() {
    sendAT(GF("+LOCATION=2"));
    if (waitResponse(10000L, GF("\n\r\n"), GF("+LOCATION: ")) != 1) {
      return "";
    }

    // Coordinates
    String res = stream.readStringUntil('\r');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS information
  bool getGPSImpl(float* lat, float* lon, float* speed = 0, float* alt = 0,
                  int* vsat = 0, int* usat = 0, float* accuracy = 0,
                  int* year = 0, int* month = 0, int* day = 0, int* hour = 0,
                  int* minute = 0, int* second = 0) {
    return GPS_SUCCESS == getShortGPS(lat, lon);
  }
};

#endif  // SRC_TINYGSMCLIENTA7_H_
