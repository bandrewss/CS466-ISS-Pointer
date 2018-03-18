// Ben Andrews | Jared Dupont
// CS466 Final Project: ISS Pointer
// 2018-3-16

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h> // https://arduinojson.org/

// #define ETHERNET_VERBOSE

// a made up MAC address...
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// fall back IP if DHCP fail
// IPAddress ip(192, 168, 0, 122);  removed to save memory

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

    initializeEthernet();
    
    getMyLatLong();

    Serial.println(my_coords.lat);
    Serial.println(my_coords.lon);
    Serial.println();
}

void loop() 
{
    getIssLatLong();

    Serial.print("Latitude: ");
    Serial.println(iss_coords.lat, 2.4);

    Serial.print("Longitude: ");
    Serial.println(iss_coords.lon, 2.4);
    Serial.println();

    delay(1000);
}

int initializeEthernet()
{
#ifdef ETHERNET_VERBOSE
    Serial.print("Initializing Ethernet...");
#endif

    Ethernet.begin(mac);
    /*  removed to save memory
    if(Ethernet.begin(mac) == 0) 
    {
        Serial.println("DHCP failed, trying default address");
        Ethernet.begin(mac, ip);
    }
    */


    // let the sheild initialize
    delay(1000);

#ifdef ETHERNET_VERBOSE
    Serial.println("done.");

    Serial.print("IP Address: ");
#endif

    Serial.print(Ethernet.localIP());
    Serial.println();   
}


// get the current location of the Arduino based on its public IP
int getMyLatLong()
{
    // disconnect if connected to something
    client.stop();
    
#ifdef ETHERNET_VERBOSE
    Serial.print("Connecting to \"ipinfo.io\"...");
#endif
    
    if(client.connect("ipinfo.io", 80))
    {
        // send HTTP GET request
        client.println("GET /json HTTP/1.1");
        client.println("Host: ipinfo.io");
        client.println("User-Agent: arduino-ethernet");
        client.println("Connection: close");
        client.println();
#ifdef ETHERNET_VERBOSE
        Serial.println("successful.");
#endif
    }
    else
    {
#ifdef ETHERNET_VERBOSE
        Serial.println("failed.");
#endif
        return 1;
    }  

    // not checking HTTP status code... assuming everything is OK
    
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) 
    {
#ifdef ETHERNET_VERBOSE
        Serial.println(F("Invalid response"));
#endif

        return 2;
    }

#ifdef ETHERNET_VERBOSE
    Serial.print("Parsing JSON object...");
#endif

    // allocate JSONBuffer
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 256);
    JsonObject &root = jsonBuffer.parseObject(client);
    if (!root.success()) 
    {
#ifdef ETHERNET_VERBOSE
        Serial.println(F("parse failed."));
#endif
        return 3;
    }

#ifdef ETHERNET_VERBOSE
    Serial.println("successful");
    
    Serial.print("Current Location: ");
    Serial.println(root["loc"].as<char *>());
#endif

    // lat comes right out, long needs some convincing 
    my_coords.lat = root["loc"].as<float>();

    // find the comma seperating lat and long
    byte comma_loc = 0;
    while( (char) *(root["loc"].as<char *>() + comma_loc) != ',')
        comma_loc++;

    // use the comma location to get the long out of the JSON object
    root["loc"] = root["loc"].as<char *>() +comma_loc +1;
    my_coords.lon = root["loc"].as<float>();

    client.stop();
    
    return 0;
}


// puts the current lat and long of the ISS in *co
int getIssLatLong()
{
    // disconnect if still connected
    client.stop();

#ifdef ETHERNET_VERBOSE
    Serial.print("Connecting to \"api.open-notify.org\"...");
#endif
    
    if(client.connect("api.open-notify.org", 80))
    {
        // send HTTP GET request
        client.println("GET http://api.open-notify.org/iss-now.json");
        client.println("Host: api.open-notify.org");
        client.println("arduino-ethernet");
        client.println("Connection: keep-alive");
        client.println();

#ifdef ETHERNET_VERBOSE
        Serial.println("successful.");
#endif
    }
    else
    {
#ifdef ETHERNET_VERBOSE
        Serial.println("failed.");
#endif

        return 1;
    }

#ifdef ETHERNET_VERBOSE
    Serial.print("Parsing JSON object...");
#endif

    // allocate JSONBuffer
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 128);
    JsonObject &root = jsonBuffer.parseObject(client);
    if (!root.success()) 
    {
#ifdef ETHERNET_VERBOSE
        Serial.println(F("parse failed."));
#endif

        return 3;
    }

#ifdef ETHERNET_VERBOSE
    Serial.println("successful");
#endif
    
    // Extract values
    iss_coords.lat = root["iss_position"]["latitude"].as<float>();
    iss_coords.lon = root["iss_position"]["longitude"].as<float>();

    client.stop();

    return 0;
}
