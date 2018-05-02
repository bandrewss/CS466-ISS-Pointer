#define TO_RADIAN(A) (PI * (A) / 180)
#define TO_DEGREE(R) ((R) * (180/PI))

#define MY_RADIUS (6371)
#define ISS_RADIUS (6371 + 408)

class Vec {
    
    public:
    
        union {
            float data[3];
            struct {
                float x;
                float y;
                float z;
            };
        };

        // Constructors

        // Vectors default to 0, 0, 0.
        Vec() {
            x = 0;
            y = 0;
            z = 0;
        }

        // Construct with values, 3D
        Vec(float ax, float ay, float az) {
            x = ax;
            y = ay;
            z = az;
        }

        // Construct with values, 2D
        Vec(float ax, float ay) {
            x = ax;
            y = ay;
            z = 0;
        }

        // Copy constructor
        Vec(const Vec& o) {
            x = o.x;
            y = o.y;
            z = o.z;
        }

        // Addition
        
        Vec operator+(const Vec& o) {
            return Vec(x + o.x, y + o.y, z + o.z);
        }

        Vec& operator+=(const Vec& o) {
            x += o.x;
            y += o.y;
            z += o.z;
            return *this;
        }

        // Subtraction

        Vec operator-() {
            return Vec(-x, -y, -z);
        }

        Vec operator-(const Vec o) {
            return Vec(x - o.x, y - o.y, z - o.z);
        }

        Vec& operator-=(const Vec o) {
            x -= o.x;
            y -= o.y;
            z -= o.z;
            return *this;
        }

        // Multiplication by scalars

        Vec operator*(const float s) {
            return Vec(x * s, y * s, z * s);
        }

        Vec& operator*=(const float s) {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }

        // Division by scalars

        Vec operator/(const float s) {
            return Vec(x / s, y / s, z / s);
        }

        Vec& operator/=(const float s) {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }
        
        // Dot product

        float operator*(const Vec o) {
            return (x * o.x) + (y * o.y) + (z * o.z);
        }

        // An in-place dot product does not exist because
        // the result is not a vector.

        // Cross product

        Vec operator^(const Vec o) {
            float nx = y * o.z - o.y * z;
            float ny = z * o.x - o.z * x;
            float nz = x * o.y - o.x * y;
            return Vec(nx, ny, nz);
        }

        Vec& operator^=(const Vec o) {
            float nx = y * o.z - o.y * z;
            float ny = z * o.x - o.z * x;
            float nz = x * o.y - o.x * y;
            x = nx;
            y = ny;
            z = nz;
            return *this;
        }

        // Other functions
        
        // Length of vector
        float magnitude() {
            return sqrt(magnitude_sqr());
        }

        // Length of vector squared
        float magnitude_sqr() {
            return (x * x) + (y * y) + (z * z);
        }

        float angleBetween(Vec b){
        	return acos( (*this * b) / (this->magnitude() * b.magnitude() ) );
        }

        // Returns a normalised copy of the vector
        // Will break if it's length is 0
        Vec normalised() {
            return Vec(*this) / magnitude();
        }

        // Modified the vector so it becomes normalised
        Vec& normalise() {
            (*this) /= magnitude();
            return *this;
        }
        
};

float myLat = 44;
float myLon = -91.4;

float issLat = 0;
float issLon = 0;
bool gotInput = false;
void setup() {
	Serial.begin(9600);
	Vec v = {.x = 0, .y = 0, .z = 0};
}

void loop() {
	if (Serial.available() > 0) {
		issLat = Serial.parseFloat();
		issLon = Serial.parseFloat();

		// say what you got:
		Serial.print("Input: ");
		Serial.print(issLat);
		Serial.print(" ");
		Serial.println(issLon);
		gotInput = true;
	}

	if(gotInput){

		float myTheta = TO_RADIAN(90.0 - myLat);
		float myPhi = TO_RADIAN(myLon + 180.0);

		float issTheta = TO_RADIAN(90.0 - issLat);
		float issPhi = TO_RADIAN(issLon + 180.0);

		Vec myVector = {
			.x = MY_RADIUS * sin(myTheta) * cos(myPhi), 
			.y = MY_RADIUS * sin(myTheta) * sin(myPhi), 
			.z = MY_RADIUS * cos(myTheta)
			};
		Vec issVector = {
			.x = ISS_RADIUS * sin(issTheta) * cos(issPhi), 
			.y = ISS_RADIUS * sin(issTheta) * sin(issPhi), 
			.z = ISS_RADIUS * cos(issTheta)
			};
		Vec northVector = {
			.x = MY_RADIUS * sin(myTheta - PI/2) * cos(myPhi), 
			.y = MY_RADIUS * sin(myTheta - PI/2) * sin(myPhi), 
			.z = MY_RADIUS * cos(myTheta - PI/2)
			};
		northVector.normalise();

		Vec toIssVector = issVector - myVector;

		float pitch = TO_DEGREE(toIssVector.angleBetween(myVector));
		Serial.println(pitch);

		Vec normalVector = Vec(myVector);
		Vec directionVector = Vec(toIssVector);
		Vec subtractionVector = Vec(myVector);
		normalVector.normalise();
		directionVector.normalise();
		subtractionVector.normalise();

		// This is why I hate overriding operators
		// This line means "Scale subtractionVector by the dot product of normalVector and directionVector"
		subtractionVector *= normalVector * directionVector;

		Vec p = Vec(directionVector);
		p -= subtractionVector;

		float heading = TO_DEGREE(northVector.angleBetween(p));
		if(abs(myPhi - issPhi) > TO_RADIAN(180)) {
			heading = 360 - heading;
		}
		Serial.println(heading);
		gotInput = false;
	}
}


