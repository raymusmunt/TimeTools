#include "TimeTools.h"

unsigned int TimeTools::_UdpPort = 2390;
const int TimeTools::_NTP_PACKET_SIZE = 48;
byte TimeTools::_packetBuffer[_NTP_PACKET_SIZE];
IPAddress TimeTools::_timeServerIP;
const char* TimeTools::_ntpServerName = "uk.pool.ntp.org";
WiFiUDP TimeTools::_udp;

unsigned long TimeTools::sendNTPpacket(IPAddress &address)
{
    Serial.println("\nTIME: sending NTP packet...");
    // set all bytes in the buffer to 0
    memset(_packetBuffer, 0, _NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    _packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    _packetBuffer[1] = 0;     // Stratum, or type of clock
    _packetBuffer[2] = 6;     // Polling Interval
    _packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    _packetBuffer[12]  = 49;
    _packetBuffer[13]  = 0x4E;
    _packetBuffer[14]  = 49;
    _packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    _udp.beginPacket(address, 123); //NTP requests are to port 123
    _udp.write(_packetBuffer, _NTP_PACKET_SIZE);
    _udp.endPacket();
}

time_t TimeTools::getNtpTime()
{
    unsigned long ntpTimeOut = millis();
    while (millis() - ntpTimeOut < 10000) {
        //get a random server from the pool
        WiFi.hostByName(_ntpServerName, _timeServerIP);

        sendNTPpacket(_timeServerIP); // send an NTP packet to a time server
        // wait to see if a reply is available
        delay(1000);

        int cb = _udp.parsePacket();
        if (!cb) {
            Serial.println("TIME: no packet yet");
        } else {
            Serial.printf("TIME: packet received, length=%d\n", cb);
            // We've received a packet, read the data from it
            _udp.read(_packetBuffer, _NTP_PACKET_SIZE); // read the packet into the buffer

            //the timestamp starts at byte 40 of the received packet and is four bytes,
            // or two words, long. First, esxtract the two words:

            unsigned long highWord = word(_packetBuffer[40], _packetBuffer[41]);
            unsigned long lowWord = word(_packetBuffer[42], _packetBuffer[43]);
            // combine the four bytes (two words) into a long integer
            // this is NTP time (seconds since Jan 1 1900):
            time_t unixTime = (highWord << 16 | lowWord) - 2208988800;
            if ( month( unixTime ) >= 3 && month( unixTime ) < 11 ) {
                unixTime += 3600;
                Serial.println("TIME: Adjusted +1hr for DST");
            }
            setTime(unixTime);
            return unixTime;
        }
    }
    Serial.println("TIME: Failed to get time");
    return 0;
}

void TimeTools::setup()
{
    _udp.begin(_UdpPort);
    Serial.printf("TIME: UDP server started port : %d\n", _udp.localPort());

    setSyncProvider(getNtpTime);
    setSyncInterval(86400);

    if (timeStatus() == timeNotSet) {
        Serial.println("TIME: Was not set :( \n\n");
    } else {
        printTimeln();
    }
}

void TimeTools::printTime()
{
    if (timeStatus() == timeNotSet)   return;

    Serial.printf("TIME: %s:%s:%s - %d/%d/%d",
        printDigits(hour()).c_str(),
        printDigits(minute()).c_str(),
        printDigits(second()).c_str(),
        day(), month(), year());

    // Serial.print( "TIME: ",printDigits(hour()),printDigits(minute()),printDigits(second()));
    // Serial.printf(" - %d/%d/%d", day(), month(), year());
}

void TimeTools::printTimeln()
{
    printTime();
    Serial.print("\n");
}

String TimeTools::printDigits(int digits)
{
    String digitBuffer = "";
    // utility for digital clock display: prints preceding colon and leading 0
    if (digits < 10)
        digitBuffer +="0";
    digitBuffer += String(digits);

    return digitBuffer;
}

TimeTools timeTools;

