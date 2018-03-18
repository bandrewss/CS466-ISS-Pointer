#include <ArduinoJson.h>

// Ben Andrews | Jared Dupont
// CS466 Final Project: ISS Pointer
// 2018-3-16

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h> // https://arduinojson.org/

// a made up MAC address...
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// fall back IP if DHCP fails
IPAddress ip(192, 168, 0, 122);

EthernetClient client;

struct Coordinates
{
    float lat;
    float lon;
};

Coordinates iss_coords, my_coords;

void setup() 
{
    Serial.begin(9600);
    while(!Serial)
        ;

    Serial.print("Initilizing Ethernet...");
    if(Ethernet.begin(mac) == 0) 
    {
        Serial.println("DHCP failed, trying default address");
        Ethernet.begin(mac, ip);
    }

    // let the sheild initialize
    delay(1000);
    Serial.println("done.");

    Serial.print("IP Address: ");
    Serial.print(Ethernet.localIP());
    Serial.println('\0');
    
    // get current location based on IP address
    Serial.print("Connecting to \"ipinfo.io\"...");
    if(client.connect("ipinfo.io", 80))
    {
        // send HTTP GET request
        client.println("GET /json HTTP/1.1");
        client.println("Host: ipinfo.io");
        client.println("User-Agent: arduino-ethernet");
        client.println("Connection: close");
        client.println();
        Serial.println("successful.");
    }
    else
    {
        Serial.println("failed.");
        //return 1;
    }  

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) 
    {
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
        //return;
    }
    
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
        Serial.println(F("Invalid response"));
        //return;
    }

    // allocate JSONBuffer
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 256);
    Serial.print("Parsing JSON object...");
    JsonObject &root = jsonBuffer.parseObject(client);
    if (!root.success()) 
    {
        Serial.println(F("parse failed."));
        //return 2;
    }

    Serial.println("successful");

    Serial.print("Current Location: ");
    Serial.println(root["loc"].as<char *>());
    

    my_coords.lat = root["loc"].as<float>();

    // find the comma seperating lat an long
    byte comma_loc = 0;
    while( (char) *(root["loc"].as<char *>() + comma_loc) != ',')
        comma_loc++;

    // use the comma location to get the long out of the JSON object
    root["loc"] = root["loc"].as<char *>() +comma_loc +1;
    my_coords.lon = root["loc"].as<float>();


    Serial.println(my_coords.lat);
    Serial.println(my_coords.lon);

    
    Serial.println('\0');
}

void loop() 
{
    getLatLong(&iss_coords);

    Serial.print("Latitude: ");
    Serial.println(iss_coords.lat, 2.4);

    Serial.print("Longitude: ");
    Serial.println(iss_coords.lon, 2.4);
    Serial.println('\0');

    delay(1000);
}

// puts the current lat and long of the ISS in *co
int getLatLong(Coordinates *co)
{
    // disconnect if still connected
    client.stop();

    Serial.print("Connecting to \"api.open-notify.org\"...");
    if(client.connect("api.open-notify.org", 80))
    {
        // send HTTP GET request
        client.println("GET http://api.open-notify.org/iss-now.json");
        client.println("Host: api.open-notify.org");
        client.println("arduino-ethernet");
        client.println("Connection: keep-alive");
        client.println();
        Serial.println("successful.");
    }
    else
    {
        Serial.println("failed.");
        return 1;
    }

    // allocate JSONBuffer
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 128);
    
    Serial.print("Parsing JSON object...");
    JsonObject &root = jsonBuffer.parseObject(client);
    if (!root.success()) 
    {
        Serial.println(F("parse failed."));
        return 2;
    }

    Serial.println("successful");
    
    // Extract values
    co->lat = root["iss_position"]["latitude"].as<float>();
    co->lon = root["iss_position"]["longitude"].as<float>();

    client.stop();

    return 0;
}
