#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

// #################################################################################################################################
// ################################################    KALMAN    ###################################################################
// #################################################################################################################################

class Kalman {
public:
    Kalman() {
        /* We will set the variables like so, these can also be tuned by the user */
        Q_angle = 0.001;
        Q_bias = 0.003;
        R_measure = 0.03;

        angle = 0; // Reset the angle
        bias = 0; // Reset bias

        P[0][0] = 0; // Since we assume that the bias is 0 and we know the starting angle (use setAngle), the error covariance matrix is set like so - see: http://en.wikipedia.org/wiki/Kalman_filter#Example_application.2C_technical
        P[0][1] = 0;
        P[1][0] = 0;
        P[1][1] = 0;
    };
    // The angle should be in degrees and the rate should be in degrees per second and the delta time in seconds
    double getAngle(double newAngle, double newRate, double dt) {
        // KasBot V2  -  Kalman filter module - http://www.x-firm.com/?page_id=145
        // Modified by Kristian Lauszus
        // See my blog post for more information: http://blog.tkjelectronics.dk/2012/09/a-practical-approach-to-kalman-filter-and-how-to-implement-it

        // Discrete Kalman filter time update equations - Time Update ("Predict")
        // Update xhat - Project the state ahead
        /* Step 1 */
        rate = newRate - bias;
        angle += dt * rate;

        // Update estimation error covariance - Project the error covariance ahead
        /* Step 2 */
        P[0][0] += dt * (dt*P[1][1] - P[0][1] - P[1][0] + Q_angle);
        P[0][1] -= dt * P[1][1];
        P[1][0] -= dt * P[1][1];
        P[1][1] += Q_bias * dt;

        // Discrete Kalman filter measurement update equations - Measurement Update ("Correct")
        // Calculate Kalman gain - Compute the Kalman gain
        /* Step 4 */
        S = P[0][0] + R_measure;
        /* Step 5 */
        K[0] = P[0][0] / S;
        K[1] = P[1][0] / S;

        // Calculate angle and bias - Update estimate with measurement zk (newAngle)
        /* Step 3 */
        y = newAngle - angle;
        /* Step 6 */
        angle += K[0] * y;
        bias += K[1] * y;

        // Calculate estimation error covariance - Update the error covariance
        /* Step 7 */
        P[0][0] -= K[0] * P[0][0];
        P[0][1] -= K[0] * P[0][1];
        P[1][0] -= K[1] * P[0][0];
        P[1][1] -= K[1] * P[0][1];

        return angle;
    };
    void setAngle(double newAngle) { angle = newAngle; }; // Used to set angle, this should be set as the starting angle
    double getRate() { return rate; }; // Return the unbiased rate

    /* These are used to tune the Kalman filter */
    void setQangle(double newQ_angle) { Q_angle = newQ_angle; };
    void setQbias(double newQ_bias) { Q_bias = newQ_bias; };
    void setRmeasure(double newR_measure) { R_measure = newR_measure; };

    double getQangle() { return Q_angle; };
    double getQbias() { return Q_bias; };
    double getRmeasure() { return R_measure; };

private:
    /* Kalman filter variables */
    double Q_angle; // Process noise variance for the accelerometer
    double Q_bias; // Process noise variance for the gyro bias
    double R_measure; // Measurement noise variance - this is actually the variance of the measurement noise

    double angle; // The angle calculated by the Kalman filter - part of the 2x1 state vector
    double bias; // The gyro bias calculated by the Kalman filter - part of the 2x1 state vector
    double rate; // Unbiased rate calculated from the rate and the calculated bias - you have to call getAngle to update the rate

    double P[2][2]; // Error covariance matrix - This is a 2x2 matrix
    double K[2]; // Kalman gain - This is a 2x1 vector
    double y; // Angle difference
    double S; // Estimate error
};

// #################################################################################################################################
// ################################################    END KALMAN    ###############################################################
// #################################################################################################################################


// #################################################################################################################################
// ################################################    I2C CODE    #################################################################
// #################################################################################################################################

#define RESTRICT_PITCH // Comment out to restrict roll to ±90deg instead - please read: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf
#define RAD_TO_DEG 57.29578

int mymillis();

int main(){

    Kalman kalmanX; // Create the Kalman instances
    Kalman kalmanY;

    /* IMU Data */
    double accX, accY, accZ;
    double gyroX, gyroY, gyroZ;
    int16_t tempRaw;

    double gyroXangle, gyroYangle; // Angle calculate using the gyro only
    double compAngleX, compAngleY; // Calculated angle using a complementary filter
    double kalAngleX, kalAngleY; // Calculated angle using a Kalman filter

    uint32_t timer;
    uint8_t accXhi, accXlo, accYhi, accYlo, accZhi, accZlo;
    uint8_t gyrXhi, gyrXlo, gyrYhi, gyrYlo, gyrZhi, gyrZlo;
    uint8_t temphi, templo;

    // TODO: Make calibration routine


    // ############################################################################################################################
    // ############################################    SETUP    ###################################################################
    // ############################################################################################################################
    int endeMPU6050 = wiringPiI2CSetup(0x68);
    int endeHMC5883L = wiringPiI2CSetup(0x1e);
    int i = 0;

    wiringPiI2CWriteReg8(endeMPU6050, 0x19, 0x07); // Set the sample rate to 1000Hz - 8kHz/(7+1) = 1000Hz
    wiringPiI2CWriteReg8(endeMPU6050, 0x1a, 0x00); // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
    wiringPiI2CWriteReg8(endeMPU6050, 0x1b, 0x00); // Set Gyro Full Scale Range to ±250deg/s
    wiringPiI2CWriteReg8(endeMPU6050, 0x1c, 0x00); // Set Accelerometer Full Scale Range to ±2g
    wiringPiI2CWriteReg8(endeMPU6050, 0x6b, 0x01); // PLL with X axis gyroscope reference and disable sleep mode
 
    if(wiringPiI2CReadReg8(endeMPU6050, 0x75) != 0x68){
        printf("\nErro na identificação do MPU6050.\n");
        while(1);
    }

    sleep(0.1); // Wait for sensor to stabilize

    /* Set kalman and gyro starting angle */
    accXhi = wiringPiI2CReadReg8(endeMPU6050, 0x3b);
    accXlo = wiringPiI2CReadReg8(endeMPU6050, 0x3c);
    accYhi = wiringPiI2CReadReg8(endeMPU6050, 0x3d);
    accYlo = wiringPiI2CReadReg8(endeMPU6050, 0x3e);
    accZhi = wiringPiI2CReadReg8(endeMPU6050, 0x3f);
    accZlo = wiringPiI2CReadReg8(endeMPU6050, 0x40);
    accX = (accXhi << 8) | accXlo;
    accY = (accYhi << 8) | accYlo;
    accZ = (accZhi << 8) | accZlo;

    // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
    // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
    // It is then converted from radians to degrees
    #ifdef RESTRICT_PITCH // Eq. 25 and 26
        double roll  = atan2(accY, accZ) * RAD_TO_DEG;
        double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
    #else // Eq. 28 and 29
        double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
        double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
    #endif

    kalmanX.setAngle(roll); // Set starting angle
    kalmanY.setAngle(pitch);
    gyroXangle = roll;
    gyroYangle = pitch;
    compAngleX = roll;
    compAngleY = pitch;

    timer = mymillis();

    // ############################################################################################################################
    // ############################################    LOOP    ####################################################################
    // ############################################################################################################################
    while(1){
        /* Update all the values */
        gyrXhi = wiringPiI2CReadReg8(endeMPU6050, 0x43);
        gyrXlo = wiringPiI2CReadReg8(endeMPU6050, 0x44);
        gyrYhi = wiringPiI2CReadReg8(endeMPU6050, 0x45);
        gyrYlo = wiringPiI2CReadReg8(endeMPU6050, 0x46);
        gyrZhi = wiringPiI2CReadReg8(endeMPU6050, 0x47);
        gyrZlo = wiringPiI2CReadReg8(endeMPU6050, 0x48);
        
        accXhi = wiringPiI2CReadReg8(endeMPU6050, 0x3b);
        accXlo = wiringPiI2CReadReg8(endeMPU6050, 0x3c);
        accYhi = wiringPiI2CReadReg8(endeMPU6050, 0x3d);
        accYlo = wiringPiI2CReadReg8(endeMPU6050, 0x3e);
        accZhi = wiringPiI2CReadReg8(endeMPU6050, 0x3f);
        accZlo = wiringPiI2CReadReg8(endeMPU6050, 0x40);

        temphi = wiringPiI2CReadReg8(endeMPU6050, 0x41);
        templo = wiringPiI2CReadReg8(endeMPU6050, 0x42);

        gyroX = (gyrXhi << 8) | gyrXlo;
        gyroY = (gyrYhi << 8) | gyrYlo;
        gyroZ = (gyrZhi << 8) | gyrZlo;
        accX = (accXhi << 8) | accXlo;
        accY = (accYhi << 8) | accYlo;
        accZ = (accZhi << 8) | accZlo;
        tempRaw = (temphi << 8) | templo;

        double dt = (double)(mymillis() - timer) / 1000; // Calculate delta time
        timer = mymillis();

        // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
        // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
        // It is then converted from radians to degrees
        #ifdef RESTRICT_PITCH // Eq. 25 and 26
            double roll  = atan2(accY, accZ) * RAD_TO_DEG;
            double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
        #else // Eq. 28 and 29
            double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
            double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
        #endif

        double gyroXrate = gyroX / 131.0; // Convert to deg/s
        double gyroYrate = gyroY / 131.0; // Convert to deg/s

        #ifdef RESTRICT_PITCH
            // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
            if ((roll < -90 && kalAngleX > 90) || (roll > 90 && kalAngleX < -90)) {
            kalmanX.setAngle(roll);
            compAngleX = roll;
            kalAngleX = roll;
            gyroXangle = roll;
        } else
            kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter

        if (abs(kalAngleX) > 90)
            gyroYrate = -gyroYrate; // Invert rate, so it fits the restriced accelerometer reading
        kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt);
        #else
        // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
        if ((pitch < -90 && kalAngleY > 90) || (pitch > 90 && kalAngleY < -90)) {
            kalmanY.setAngle(pitch);
            compAngleY = pitch;
            kalAngleY = pitch;
            gyroYangle = pitch;
        } else
            kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt); // Calculate the angle using a Kalman filter

        if (abs(kalAngleY) > 90)
            gyroXrate = -gyroXrate; // Invert rate, so it fits the restriced accelerometer reading
        kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter
        #endif

        //gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
        //gyroYangle += gyroYrate * dt;
        gyroXangle += kalmanX.getRate() * dt; // Calculate gyro angle using the unbiased rate
        gyroYangle += kalmanY.getRate() * dt;

        compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter
        compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * pitch;

        // Reset the gyro angle when it has drifted too much
        if (gyroXangle < -180 || gyroXangle > 180)
            gyroXangle = kalAngleX;
        if (gyroYangle < -180 || gyroYangle > 180)
        gyroYangle = kalAngleY;


        // ########################################################################################################################
        // ############################################    PRINT DATA    ##########################################################
        // ########################################################################################################################

        if (i == 10){
        system("clear");
        printf("roll: %f \n", roll);
        printf("gyroXangle: %f \n", gyroXangle);
        printf("compAngleX: %f \n", compAngleX);
        printf("kalAngleX: %f \n\n", kalAngleX);

        printf("pitch: %f \n", pitch);
        printf("gyroYangle: %f \n", gyroYangle);
        printf("compAngleY: %f \n", compAngleY);
        printf("kalAngleY: %f \n", kalAngleY);
        i = 0;    
        } else {
            i++;
        }


    }
}

int mymillis(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

// #################################################################################################################################
// ################################################    END I2C CODE    #############################################################
// #################################################################################################################################


