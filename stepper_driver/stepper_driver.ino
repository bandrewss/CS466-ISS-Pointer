// Ben Andrews | Jared DuPont
// CS466 Final Project: ISS Pointer
// 2018-3-25

#define PIN_1 (4)
#define PIN_2 (5)
#define PIN_3 (6)
#define PIN_4 (7)

#define STEP_SEED (0b1010000)

#define STEP_1 (0b00110000)
#define STEP_2 (0b11000000)  

#define CLEAR_STEPPER (0b00001111)

#define STEPPER_DELAY (5)

#define step

void setup() 
{
    pinMode(PIN_1, OUTPUT);
    pinMode(PIN_2, OUTPUT);
    pinMode(PIN_3, OUTPUT);
    pinMode(PIN_4, OUTPUT);

    PORTD = STEP_SEED;
}

void loop() 
{
    step_cwise();
    delay(1000);

    step_cwise();
    delay(1000);

    step_cwise();
    delay(1000);
    
    step_ccwise();
    delay(1000);

    step_ccwise();
    delay(1000);
}


byte step_num = 0;
void step_cwise()
{
    if(step_num)
        PORTD ^= STEP_1;
    else
        PORTD ^= STEP_2;

    step_num = !step_num;

    delay(STEPPER_DELAY);
}

void step_ccwise()
{
    if(step_num)
        PORTD ^= STEP_2;
    else
        PORTD ^= STEP_1;

    step_num = !step_num;

    delay(STEPPER_DELAY);
}












