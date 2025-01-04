#ifndef MEASURE_ROUTINE_H
#define MEASURE_ROUTINE_H

// Function to create a JSON payload
void performMeasurementIrOnly(unsigned short int num_samples);
void performMeasurement(unsigned short int num_samples);
void sensorSetup(short int parameter);

#endif // MEASURE_ROUTINE_H