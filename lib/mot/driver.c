#include <avr/io.h>
#include "driver.h"
#include "com.h"

#include <math.h>

int n_cycles=0;
//error check : DONE
//Check Control Register : DONE
//Motor Driver : DONE

//mot_cang = abs_angle

//mot_angle is supposed to be the requested angle for the cylinder sendt from master I2C



// Direction control code: DONE
// mot_cr & (1<<mot_cr_dir)

//
//


void mot_drive(){
//need new registor for cycles of the motor shaft
    //data processing

    //needed to find angle of cylinder
    //n_cycles - need to make a registor for how many cycles the motor shafts has rotated and have the number loop back upon reaching 36

    int period = 360;
    float g_red = 36.0f;

    float c_current = (MOT_ANG+(n_cycles*360))/36; //needs to be global or register in master controller.

    float add = (g_red*MOT_ANG)-((MOT_CANG+period*n_cycles));
    float angle = MOT_CANG+add;
    //-1 since full rotations don't start with the first cycle of modulo
    int cycles = (angle/period) - 1;
    //fmod is kind of a modulo for float values, since %(standard modulo) only works for integral(int) value.
    float remainder = fmod(angle, period);

  
    if (!(0<(MOT_SER & (1<<MOT_STAT_ERROR))))
    {
    //Motor controls 3-step protocol 
    //rotating the motor until it reaches 0 degrees.

        if(angle>=360){
            while(MOT_CANG < period){
                if(0<(MOT_CR & (1<<MOT_CR_DIR))){
                    OCR1A=127;
                } else{
                    OCR1B=127;
                }
            }
            OCR1A=0;
            OCR1B=0;

            n_cycles++;
            n_cycles=n_cycles%36; 
        }

        //For full rotations of cycles
        if(cycles>0){
            for (int i = 0; i < cycles;)
            {
                while(MOT_CANG<(float)period){ // MIGHT NEED TO REWRITE SINCE THIS CONFIGURATION MIGHT JUST KEEP SPINNING SINCE abs_angle can't reach a value of 360
                    if(0<(MOT_CR & (1<<MOT_CR_DIR))){
                        OCR1A=127;
                    } else {
                        OCR1B=127;
                    }
                }
                OCR1A=0;
                OCR1B=0;
                i++;
                //needed for the calculating the angle of the cylinder
                n_cycles++;
                n_cycles=n_cycles%36;
            }
        }

        //rotating the motor to the remainder degrees
        if(remainder>0){
            while(MOT_CANG<remainder){
                if(0<(MOT_CR & (1<<MOT_CR_DIR))){
                    OCR1A=127;
                } else{
                    OCR1B=127;
                }
            }
            OCR1A=0;
            OCR1B=0;
        }
    }
}

void pwm_init(void){
    //PWM Setup - for D9(PB5), D10(PB6)
    DDRB |= (1<<PB5)|(1<<PB6);
    //Sets OCR1A and OCR1B to compare match
    TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM10);
    //Sets WGM12 which in addition to the WGM10 sets mode to 5 which is 8Bit Fast PWM
    //Sets the prescaler 1
    TCCR1B = (1<<WGM12)|(1<<CS10);
    OCR1A=0;
    OCR1B=0;
}