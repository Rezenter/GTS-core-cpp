#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};
union In{
  char buff[UDP_TX_PACKET_MAX_SIZE];
  uint32_t  value;
};

In in;
EthernetUDP Udp;

void setup() {
  Ethernet.init(10);  // Most Arduino shields
  Ethernet.begin(mac, IPAddress(192, 168, 10, 49));
  //Serial.begin(115200);
  delay(5000);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    //Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1);
    }
  }
  //if (Ethernet.linkStatus() == LinkOFF) {
    //Serial.println("Ethernet cable is not connected.");
  //}
  Udp.begin(8080); // port

  //Serial.println("Sending...");
  Udp.beginPacket(IPAddress(192, 168, 10, 41), 1234); 
  Udp.write({1});
  Udp.endPacket(); // send something anywhere to update routing table?

  analogWriteResolution(12);
  //Serial.println("Setup OK");
}

void loop() {
  if (Udp.parsePacket()) {
    Udp.read(in.buff, UDP_TX_PACKET_MAX_SIZE);
    //Serial.println(in.value);
    analogWrite(DAC0, in.value);
    //Serial.println("Contents:");
    /*
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write({1});
    Udp.endPacket();
    */
  }
}
