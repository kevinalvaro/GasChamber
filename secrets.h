#include <pgmspace.h>

#define SECRET

const char WIFI_SSID[] = "WIFI NAME";               //Ganti dengan SSID WiFi Anda
const char WIFI_PASSWORD[] = "WIFI PASSWORD";           //Ganti dengan kata sandi WiFi Anda

#define THINGNAME "Gas_Chamber"

int8_t TIME_ZONE = +7; //NYC(USA): -5 UTC

const char AWS_IOT_ENDPOINT[] = "XXXXXXXXXXXXXXXXX.iot.us-east-1.amazonaws.com"; // Ganti dengan AWS IoT endpoint Anda

// Copy contents from AWS Root CA certificate here
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
-----END CERTIFICATE-----
)EOF";

// Copy contents from device certificate here
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
-----END CERTIFICATE-----
)KEY";

// Copy contents from device private key here
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
-----END RSA PRIVATE KEY-----
)KEY";
