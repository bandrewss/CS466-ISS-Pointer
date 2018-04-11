// Ben Andrews | Jared Dupont
// CS466 Final Project: ISS Pointer
// 2018-3-16

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h> // https://arduinojson.org/
#include <Servo.h>
#include <math.h>

// #define ETHERNET_VERBOSE

// a made up MAC address...
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// fall back IP if DHCP fail
// IPAddress ip(192, 168, 0, 122);  removed to save memory

EthernetClient client;

Servo servo;
#define SERVO_PIN 6

#define ISS_ALT (390)
#define EARTH_CENTER (6370)

#define TO_RADIAN(A) (PI * (A) / 180)
#define TO_DEGREE(R) ((R) * (180/PI))
#define TAU (2 * PI)

#define STEP_DELAY (1000)
#define STEP_PIN (3)
#define DIR_PIN (4)
#define MAX_STEP (370)
#define DEGREES_TO_STEP(D) (D * 1.02777)
int current_step = 0;


struct Coordinates
{
    float lat;
    float lon;
};

Coordinates iss_coords, iss_coords_rad, my_coords, my_coords_rad;

void setup() 
{
    Serial.begin(9600);
    while(!Serial)
        ;

    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);

    pinMode(SERVO_PIN, OUTPUT);
    servo.attach(SERVO_PIN);

    initializeEthernet();

    getMyLatLong();

    Serial.println(my_coords.lat);
    Serial.println(my_coords.lon);
    Serial.println();
}

float bearing;
void loop() 
{
    getIssLatLong();

    Serial.print("Latitude: ");
    Serial.println(iss_coords.lat, 2.4);

    Serial.print("Longitude: ");
    Serial.println(iss_coords.lon, 2.4);

    bearing = getISSBearing();

    stepToBearing(bearing);
    
    Serial.print("Bearing: ");
    Serial.println(bearing);

    adjustServo();
    
    Serial.println();
    //delay(10000);
    delay(1500);
}


// startup the Ethernet sheild 
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

   return 0;
}


// get the current location of the Arduino based on its public IP
// fills 'my_coords' and 'my_coords_rad'
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
        //client.println("User-Agent: arduino-ethernet");
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

    my_coords_rad.lat = TO_RADIAN(my_coords.lat);
    my_coords_rad.lon = TO_RADIAN(my_coords.lon);
    
    return 0;
}


// gets the current lat and long of the ISS
// fills 'iss_coords' and 'iss_coords_rad'
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
        //client.println("arduino-ethernet");
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

    iss_coords_rad.lat = TO_RADIAN(iss_coords.lat);
    iss_coords_rad.lon = TO_RADIAN(iss_coords.lon);

    return 0;
}


// calculate and return the bearing from us to the ISS
float getISSBearing()
{
    // thanks to 'deya tri' for direction algorithm
    // https://stackoverflow.com/a/45929654

    float d_lat = log(tan((iss_coords_rad.lat /2.0) + (PI/4.0)) / tan((my_coords_rad.lat /2.0) + (PI/4.0)));
    float d_lon = fabs(my_coords_rad.lon - iss_coords_rad.lon);
    
    if(d_lon > TAU)
        d_lon = fmod(d_lon, TAU);

    float theta = atan2(d_lon, d_lat);
    float theta_d = TO_DEGREE(theta);

    if(fabs(my_coords.lon + 180) < fabs(iss_coords.lon))
        theta_d = 360 - theta_d;

    return theta_d;
}


//
void stepToBearing(float bearing)
{
    // which step should we be on
    int target_step = DEGREES_TO_STEP(bearing);

    Serial.print("target_step: ");
    Serial.println(target_step);

    Serial.print("current_step: ");
    Serial.println(current_step);

    int distance_r, distance_l;

    // these formulas can probably be reduced
    if(target_step == current_step)
    {
        // no need to move
        return;
    }
    else if(target_step < current_step)
    {
        distance_r = (MAX_STEP - current_step) + target_step;
        distance_l = current_step - target_step;
    }
    else
    {
        distance_r = target_step - current_step;
        distance_l = current_step + (MAX_STEP - target_step);
    }

    // call step function in correct direction
    int dir = distance_l < distance_r;
    stepMotor(dir ? distance_l : distance_r, dir);
    
}


//
void stepMotor(int steps, int dir)
{
    Serial.print("stepping: ");
    if(dir)
        Serial.print("left ");
    else
        Serial.print("right ");
    
    Serial.print(" ");
    Serial.println(steps);
    
    digitalWrite(DIR_PIN, dir);

    for(int i = 0; i < steps; ++i)
    {
        digitalWrite(STEP_PIN, HIGH); 
        delayMicroseconds(STEP_DELAY); 
        digitalWrite(STEP_PIN, LOW); 
        delayMicroseconds(STEP_DELAY);
    }

    current_step += (dir ? -steps : steps);

    if(current_step < 0)
        current_step = current_step + MAX_STEP;
    
    current_step %= MAX_STEP;
}


//
void adjustServo()
{

    // find the angle created in the center of the earth
    float angle_b;

    if(my_coords.lon > iss_coords.lon)
        angle_b = fabs(my_coords.lon) + fabs(iss_coords.lon);
    else
        angle_b = fabs(iss_coords.lon) - fabs(my_coords.lon);


    // find the length of the last remaining side using the law of cosines
    const float side_a2 = (long)EARTH_CENTER * (long)EARTH_CENTER;
    const float side_c2 = (long)ISS_ALT * (long)ISS_ALT;

    float side_b = sqrt( side_a2 + side_c2 - (2 * (long)EARTH_CENTER * (long)ISS_ALT * cos(TO_RADIAN(angle_b))));

    Serial.print("side_b: ");
    Serial.println(side_b);

    // find the target angle using the law of sines
    float angle_c = asin(sin(TO_RADIAN(angle_b)) * EARTH_CENTER / side_b);

    Serial.print("angle_c: ");
    Serial.println(TO_DEGREE(angle_c));

    servo.write( abs((int) TO_DEGREE(angle_c)));

    //analogWrite(SERVO_PIN, map(abs(TO_DEGREE(angle_c)), 0, 180, 0, 255));

   
}










