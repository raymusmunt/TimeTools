/**
 * add timeTools.setup()   arduino setup
 */
#ifndef TIME_TOOLS_H
#define TIME_TOOLS_H

#include <ESP8266WiFi.h>
#include <WifiUdp.h>
#include <TimeLib.h>

class TimeTools {

  public:
    void printTime();
    void printTimeln();
    void setup();

  private:
    static unsigned int _UdpPort;    // Listen port
    static IPAddress _timeServerIP; // IP will be updated from ntpServer below
    static const char* _ntpServerName;
    static const int _NTP_PACKET_SIZE; // NTP time stamp is in the first 48 bytes of the message
    static byte _packetBuffer[]; //buffer to hold incoming and outgoing packets
    static WiFiUDP _udp;

    static unsigned long sendNTPpacket(IPAddress& address);
    static time_t getNtpTime();

    String printDigits(int digits);
};

extern TimeTools timeTools;

#endif