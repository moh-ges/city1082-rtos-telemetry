#include "mqtt.h"
#include "display.h"
#include "mbed.h"
#include <MQTTClientMbedOs.h>
#include <cstring>
#include "mbed-trace/mbed_trace.h"
//#include "wifi_helper.h"

#define LEDON 0
#define LEDOFF 1
#define MQTT_BROKER "192.168.1.176"
#define THING_NAME "ASR_Thang"
#define LIGHT_LEVEL_TOPIC "ASR_Thang/light"
#define ANNOUNCE_TOPIC "mytopic/announce"
#define LIGHT_THRESH_TOPIC "ASR_Thang/lthresh"
#define MQTTClient_QOS2 1

extern struct dataSet myData;

#if MBED_CONF_APP_USE_TLS_SOCKET
#include "root_ca_cert.h"

#ifndef DEVICE_TRNG
#error "mbed-os-example-tls-socket requires a device which supports TRNG"
#endif
#endif // MBED_CONF_APP_USE_TLS_SOCKET

class SocketDemo {
  static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
  static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

#if MBED_CONF_APP_USE_TLS_SOCKET
  static constexpr size_t REMOTE_PORT = 443; // tls port
#else
  static constexpr size_t REMOTE_PORT = 1883; // standard HTTP port
#endif // MBED_CONF_APP_USE_TLS_SOCKET

public:
  SocketDemo() : _net(NetworkInterface::get_default_instance()) {}

  ~SocketDemo() {
    if (_net) {
      _net->disconnect();
    }
  }

  void run() {
    if (!_net) {
      printf("\033[6;1HError! No network interface found.\r\n");
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

    printf("\033[1;1HConnecting to the network...\r\n");

    nsapi_size_or_error_t result = _net->connect();
    if (result != 0) {
      printf("\033[6;1HError! _net->connect() returned: %d\r\n", result);
      return;
    }

    print_network_info();

    /* opening the socket only allocates resources */
    result = _socket.open(_net);
    if (result != 0) {
      printf("\033[7;1HError! _socket.open() returned: %d\r\n", result);
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

    printf("\033[9;1HOpening connection to remote port %d\r\n", REMOTE_PORT);

    result = _socket.connect(address);
    if (result != 0) {
      printf("\033[10;1HError! _socket.connect() returned: %d\r\n", result);
      return;
    }
    MQTTClient client(&_socket);
    char buffer[80];
    uint32_t rc;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.clientID.cstring = (char *)THING_NAME;
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.username.cstring = (char *)"";
    data.password.cstring = (char *)"";
    rc = client.connect(data);
    if (rc == 0) {
        printf("\033[12;1HSuccesful connection of %s to Broker\n", data.clientID.cstring);
    } else {
        printf("\033[12;1HClient connection failed");
    }
    MQTT::Message message{};
    sprintf(buffer, "Hello World! from %s\r\n", THING_NAME);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void *)buffer;
    message.payloadlen = strlen(buffer) + 1;

    rc = client.publish(ANNOUNCE_TOPIC, message);
    if (rc == 0)
      printf("\033[13;1Hpublish announce worked\n");
    else {
      printf("\033[13;1Hpublish announce failed %d\n", rc);
    }
    printf("\033[15;1HDemo concluded successfully \r\n");
    while(1) {
        client.yield(10);
        rtos::ThisThread::sleep_for(1000ms);
        rc = client.publish(ANNOUNCE_TOPIC, message);
        if (rc == 0) {
            printf("\033[13;1Hpublish announce worked\n");
        }
        else {
            printf("\033[13;1Hpublish announce failed %d\n", rc);
        }

    }
  }

private:
  bool resolve_hostname(SocketAddress &address) {
    const char hostname[] = MBED_CONF_APP_HOSTNAME;

    /* get the host address */
    printf("\033[6;1HResolve hostname %s\r\n", hostname);
    nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);
    if (result != 0) {
      printf("\033[7;1HError! gethostbyname(%s) returned: %d\r\n", hostname,
             result);
      return false;
    }

    printf("\033[7;1H%s address is %s\r\n", hostname,
           (address.get_ip_address() ? address.get_ip_address() : "None"));

    return true;
  }

  void print_network_info() {
    /* print the network info */
    SocketAddress a;
    _net->get_ip_address(&a);
    printf("IP address: %s\r\n",
           a.get_ip_address() ? a.get_ip_address() : "None");
    _net->get_netmask(&a);
    printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    _net->get_gateway(&a);
    printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
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
  printf("\033\x63");
  ThisThread::sleep_for(1s);
  printf("\033(A");
  ThisThread::sleep_for(100ms);
  printf("\033[1;40HStarting socket demo\r\n\r\n");

#ifdef MBED_CONF_MBED_TRACE_ENABLE
  mbed_trace_init();
#endif

  SocketDemo *example = new SocketDemo();
  MBED_ASSERT(example);
  example->run();

  //    return 0;
}
