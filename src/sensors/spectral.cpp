#include <AS726X.h>

#include "sensors/spectral.h"

AS726X sensor;
// byte GAIN = 0;
// byte MEASUREMENT_MODE = 0;

bool spectral_init() {
    return sensor.begin();
    // sensor.begin(Wire, GAIN, MEASUREMENT_MODE); //Initializes the sensor with non default values
}

SpectralData get_spectrum() {
    sensor.takeMeasurements();
    return (SpectralData){
        .violet = sensor.getCalibratedViolet(),
        .blue = sensor.getCalibratedBlue(),
        .green = sensor.getCalibratedGreen(),
        .yellow = sensor.getCalibratedYellow(),
        .orange = sensor.getCalibratedOrange(),
        .red = sensor.getCalibratedRed(),
    };
}
