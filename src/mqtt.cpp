#include "mqtt.h"
#include "display.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed.h"
#include "platform.h"
#include <MQTTClientMbedOs.h>
#include <cstring>
//#include "wifi_helper.h"

#define LEDON 0
#define LEDOFF 1
#define THING_NAME "ASR_Thang"  // change this to a unique IOT device name for the MQTT Broker
#define LIGHT_SET_TOPIC "/lightSet"
#define LIGHT_LEVEL_SET_TOPIC "ASR_Thang/lightSet"
#define LIGHT_LEVEL_TOPIC "/lightLevel"
#define ANNOUNCE_TOPIC "announce"
#define TEMP_SET_TOPIC "/tempSet"
#define TEMPERATURE_SET_TOPIC "ASR_Thang/tempSet"
#define TEMPERATURE_TOPIC "/temp"
#define MQTTClient_QOS2 1

char topic[80]; //max size of a topic name/subname hierarchy
extern struct dataSet myData;
uint32_t rxCount;
mbed::DigitalOut rxLed(P8_0);

#if MBED_CONF_APP_USE_TLS_SOCKET
#include "root_ca_cert.h"

#ifndef DEVICE_TRNG
#error "mbed-os-example-tls-socket requires a device which supports TRNG"
#endif
#endif // MBED_CONF_APP_USE_TLS_SOCKET

void messageLightSetArrived(MQTT::MessageData &md) {
  MQTT::Message &message = md.message;
  uint32_t len = md.message.payloadlen;
  char rxed[len + 1];

  strncpy(&rxed[0], (char *)(&md.message.payload)[0], len);
  myData.lightSet = atoi(rxed);
  rxCount++;
  rxLed = !rxLed;
  myData.updateDisplay = true;
}
void messageTempSetArrived(MQTT::MessageData &md) {
  MQTT::Message &message = md.message;
  uint32_t len = md.message.payloadlen;
  char rxed[len + 1];

  strncpy(&rxed[0], (char *)(&md.message.payload)[0], len);
  myData.tempSet = atoi(rxed);
  rxCount++;
  rxLed = !rxLed;
  myData.updateDisplay = true;
}

class mqttTask {
  static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
  static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

#if MBED_CONF_APP_USE_TLS_SOCKET
  static constexpr size_t REMOTE_PORT = 443; // tls port
#else
  static constexpr size_t REMOTE_PORT = 1883; // standard HTTP port
#endif // MBED_CONF_APP_USE_TLS_SOCKET

public:
  mqttTask() : _net(NetworkInterface::get_default_instance()) {}

  ~mqttTask() {
    if (_net) {
      _net->disconnect();
    }
  }

  void run() {
    if (!_net) {
      displayText("Error! No network interface found.", 1, 6);
      return;
    }

    /* if we're using a wifi interface run a quick scan */
    if (_net->wifiInterface()) {

      /* in this example we use credentials configured at compile time which are
       * used by NetworkInterface::connect() but it's possible to do this at
       * runtime by using the WiFiInterface::connect() which takes these
       * parameters as arguments */
    }

    /* connect will perform the action appropriate to the interface type to
     * connect to the network */
    char buffer[80];

    displayText("Connected to wifi", 1, 1);

    nsapi_size_or_error_t result = _net->connect();
    if (result != 0) {
      sprintf(buffer, "Error! _net->connect() returned: %d\r\n", result);
      displayText(buffer, 1, 6);
      return;
    } else {
      myData.wifiStatus = true;
    }

    // print_network_info();

    /* opening the socket only allocates resources */
    result = _socket.open(_net);
    if (result != 0) {
      sprintf(buffer, "Error! _socket.open() returned: %d\r\n", result);
      displayText(buffer, 1, 7);
      return;
    }

#if MBED_CONF_APP_USE_TLS_SOCKET
    result = _socket.set_root_ca_cert(root_ca_cert);
    if (result != NSAPI_ERROR_OK) {
      printf("Error: _socket.set_root_ca_cert() returned %d\n", result);
      return;
    }
    _socket.set_hostname(MBED_CONF_APP_HOSTNAME);
#endif // MBED_CONF_APP_USE_TLS_SOCKET

    /* now we have to find where to connect */

    SocketAddress address;

    if (!resolve_hostname(address)) {
      return;
    }

    address.set_port(REMOTE_PORT);

    /* we are connected to the network but since we're using a connection
     * oriented protocol we still need to open a connection on the socket */

    // printf("\033[9;1HOpening connection to remote port %d\r\n", REMOTE_PORT);
    result = _socket.connect(address);
    if (result != 0) {
      sprintf(buffer, "Error! _socket.connect() returned: %d\r\n", result);
      displayText(buffer, 1, 9);
      return;
    }
    MQTTClient client(&_socket);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.clientID.cstring = (char *)THING_NAME;
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.username.cstring = (char *)"";
    data.password.cstring = (char *)"";
    result = client.connect(data);
    if (result == 0) {
      sprintf(buffer, "Succesful connection of %s to broker",
              data.clientID.cstring);
      displayText(buffer, 1, 1);
//      myData.mqttStatus = true;
    } else {
      displayText("Client connection failed", 1, 1);
      return;
    }
    MQTT::Message message{};
    sprintf(buffer, "Hello World! from %s", THING_NAME);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void *)buffer;
    message.payloadlen = strlen(buffer) + 1;
//    strcpy(topic, THING_NAME);
    strcpy(topic, ANNOUNCE_TOPIC);
    result = client.publish(topic, message);
    if (result == 0) {
      displayText("Publish Announce Success", 1, 11);
    } else {
      sprintf(buffer, "publish announce failed %d", result);
      displayText(buffer, 1, 11);
    }
    strcpy(topic, THING_NAME);
    strcat(topic, LIGHT_SET_TOPIC); // this method fails to set up Callback correctly
    result = client.subscribe(LIGHT_LEVEL_SET_TOPIC, MQTT::QOS0,
                              messageLightSetArrived);
    if (result != 0)
      sprintf(buffer, "Subscription Error %d", result);
    else
      sprintf(buffer, "Subscribed to %s", topic);
    displayText(buffer, 1, 5);
    strcpy(topic, THING_NAME);
    strcat(topic, TEMP_SET_TOPIC); // this method fails to set up Callback correctly
    result = client.subscribe(TEMPERATURE_SET_TOPIC, MQTT::QOS0,
                              messageTempSetArrived);
    if (result != 0)
      sprintf(buffer, "Subscription Error %d", result);
    else
      sprintf(buffer, "Subscribed to %s", topic);
    displayText(buffer, 1, 6);

    int i = 0;
    displayText("MQTT Looping", 1, 15);
    myData.updateDisplay = true;
    while (1) {
      i++;
      client.yield(10);
      rtos::ThisThread::sleep_for(10ms);
      if ((i & 0x7ff) == 0) {
          sprintf(buffer, "%2.2f  ", myData.temperature);
          message.payload = (void *)buffer;
          message.payloadlen = strlen(buffer) + 1;
          strcpy(topic, THING_NAME);
          strcat(topic, TEMPERATURE_TOPIC);

          result = client.publish(topic, message);
          if (result == 0) {
            strcat(buffer, topic);
            strcat(buffer, "\033[K");
            displayText(buffer, 1, 13);
          } 
          else {
            sprintf(buffer, "publish temperature reading failed %d", result);
            displayText(buffer, 1, 13);
          }
      }
      if ((i & 0x7ff) == 0x200) {
          sprintf(buffer, "%3.1f  ", myData.lightLevel);
          message.payload = (void *)buffer;
          message.payloadlen = strlen(buffer) + 1;
          strcpy(topic, THING_NAME);
          strcat(topic, LIGHT_LEVEL_TOPIC);

          result = client.publish(topic, message);
          if (result == 0) {
            strcat(buffer, topic);
            strcat(buffer, "\033[K");
            displayText(buffer, 1, 13);
          } 
          else {
            sprintf(buffer, "publish light level failed %d", result);
            displayText(buffer, 1, 13);
          }
      }
    }
  }

private:
  bool resolve_hostname(SocketAddress &address) {
    const char hostname[] = MBED_CONF_APP_HOSTNAME;
    char buffer[80];

    /* get the host address */
    printf("\033[6;1HResolve hostname %s\r\n", hostname);
    nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);
    if (result != 0) {
      sprintf(buffer, "Error! gethostbyname(%s) returned: %d", hostname,
              result);
      displayText(buffer, 1, 7);
      return false;
    }

    sprintf(buffer, "\033[7;1H%s address is %s\r\n", hostname,
            (address.get_ip_address() ? address.get_ip_address() : "None"));
    displayText(buffer, 1, 7);

    return true;
  }

  void print_network_info() {
    /* print the network info */
    SocketAddress a;
    _net->get_ip_address(&a);
    printf("\033[8;1HIP address: %s\r\n",
           a.get_ip_address() ? a.get_ip_address() : "None");
    _net->get_netmask(&a);
    printf("\033[8;32HNetmask: %s\r\n",
           a.get_ip_address() ? a.get_ip_address() : "None");
    _net->get_gateway(&a);
    printf("\033[8:64HGateway: %s\r\n",
           a.get_ip_address() ? a.get_ip_address() : "None");
  }

private:
  NetworkInterface *_net;

#if MBED_CONF_APP_USE_TLS_SOCKET
  TLSSocket _socket;
#else
  TCPSocket _socket;
#endif // MBED_CONF_APP_USE_TLS_SOCKET
};

void mqttThread() {

#ifdef MBED_CONF_MBED_TRACE_ENABLE
  mbed_trace_init();
#endif
  mqttTask *mqttStart = new mqttTask();
  MBED_ASSERT(mqttStart);
  mqttStart->run();
  displayText("MQTT Stopped", 1, 15);

  //    return 0;
}
