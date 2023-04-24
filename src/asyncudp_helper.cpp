#include "asyncudp_helper.h"

AsyncUDP udp;

strAsyncUdp _asyncUdp;

void asyncudp_setup()
{
    if (udp.listenMulticast(_asyncUdp.server_address, _asyncUdp.listen_port))
        DEBUG("AsyncUDP server started, listening on IP: %s\n", _asyncUdp.server_address.toString());
    {

        udp.onPacket([](AsyncUDPPacket packet)
                     {
                  // Send to Serial2
                   Serial2.write(packet.data(), packet.length());
                  //  Serial2.flush();
                  //  const uint8_t* hello = (uint8_t*)"hello";
                  //  udp.writeTo(hello, 6, packet.remoteIP(), packet.remotePort()); 

                   DEBUG("UDP Packet Type: ");
                   DEBUG("%s", packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast"
                                                                                         : "Unicast");
                   DEBUG(", From: ");
                   DEBUG("%s", packet.remoteIP().toString().c_str());
                   DEBUG(":");
                   DEBUG("%i", packet.remotePort());
                   DEBUG(", To: ");
                   DEBUG("%s", packet.localIP().toString().c_str());
                   DEBUG(":");
                   DEBUG("%i", packet.localPort());
                   DEBUG(", Length: ");
                   DEBUG("%i", packet.length());

                   char *msg = (char *)packet.data();
                   msg[packet.length()] = 0; // fill null at the end
                   for (uint8_t i = 0; i < packet.length(); i++)
                   {
                     // Check the decimal value of the element.
                     // If the value is less than 32, then replace it with dot "." or ascii code 46.
                     // The first 32 ASCII codes are unprintable.
                     if ((uint8_t)msg[i] < 32)
                       msg[i] = (char)46;
                   }
                   DEBUG(", Data: %s\n", msg); });
        // Send multicast
        // udp.print("Hello!");
        // udp.printf("Hello! I'm %s, ip: %s\r\n", WiFi.hostname, WiFi.localIP().toString().c_str());
    }
}