/* 
 * File:   M10GTop.cpp
 * Author: Viproz
 * Used code from rs1729
 * Created on December 13, 2018, 4:39 PM
 */

/*
#define stdFLEN        0x64  // pos[0]=0x64
#define pos_GPSTOW     0x0A  // 4 byte
#define pos_GPSlat     0x0E  // 4 byte
#define pos_GPSlon     0x12  // 4 byte
#define pos_GPSalt     0x16  // 4 byte
#define pos_GPSweek    0x20  // 2 byte
//Velocity East-North-Up (ENU)
#define pos_GPSvE      0x04  // 2 byte
#define pos_GPSvN      0x06  // 2 byte
#define pos_GPSvU      0x08  // 2 byte
#define pos_SN         0x5D  // 2+3 byte
#define pos_Check     (stdFLEN-1)  // 2 byte*/

#include "M10PtuParser.h"

M10PtuParser::M10PtuParser() {
}

M10PtuParser::~M10PtuParser() {
}

void M10PtuParser::changeData(std::array<unsigned char, DATA_LENGTH> data, bool good) {
    correctCRC = good;
    frame_bytes = data;

    int i;
    unsigned byte;
    unsigned short gpsweek_bytes[2];

    //Number of weeks
    for (i = 0; i < 2; i++) {
        byte = frame_bytes[0x20 + i];
        gpsweek_bytes[i] = byte;
    }

    week = (gpsweek_bytes[0] << 8) + gpsweek_bytes[1];

    //Time in ms
    unsigned short gpstime_bytes[4];

    for (i = 0; i < 4; i++) {
        byte = frame_bytes[0x0A + i];
        gpstime_bytes[i] = byte;
    }

    time = 0;
    for (i = 0; i < 4; i++) {
        time |= gpstime_bytes[i] << (8 * (3 - i));
    }

    gps2Date(week, time / 1000, &year, &month, &day);
}

double M10PtuParser::getLatitude() {
    int i;
    unsigned byte;
    unsigned short gpslat_bytes[4];
    int gpslat;
    double B60B60 = 0xB60B60;

    for (i = 0; i < 4; i++) {
        byte = frame_bytes[0x0E + i];
        gpslat_bytes[i] = byte;
    }

    gpslat = 0;
    for (i = 0; i < 4; i++) {
        gpslat |= gpslat_bytes[i] << (8 * (3 - i));
    }
    return gpslat / B60B60;
}

double M10PtuParser::getLongitude() {
    int i;
    unsigned byte;
    unsigned short gpslon_bytes[4];
    int gpslon;
    double B60B60 = 0xB60B60;

    for (i = 0; i < 4; i++) {
        byte = frame_bytes[0x12 + i];
        gpslon_bytes[i] = byte;
    }

    gpslon = 0;
    for (i = 0; i < 4; i++) {
        gpslon |= gpslon_bytes[i] << (8 * (3 - i));
    }
    return gpslon / B60B60;
}

double M10PtuParser::getAltitude() {
    int i;
    unsigned byte;
    unsigned short gpsalt_bytes[4];
    int gpsalt;

    for (i = 0; i < 4; i++) {
        byte = frame_bytes[0x16 + i];
        gpsalt_bytes[i] = byte;
    }

    gpsalt = 0;
    for (i = 0; i < 4; i++) {
        gpsalt |= gpsalt_bytes[i] << (8 * (3 - i));
    }
    return gpsalt / 1000.0;
}

int M10PtuParser::getDay() {
    return day;
}

int M10PtuParser::getMonth() {
    return month;
}

int M10PtuParser::getYear() {
    return year;
}

int M10PtuParser::getHours() {
    return (time / 1000 % (24 * 3600)) / 3600;
}

int M10PtuParser::getMinutes() {
    return ((time / 1000 % (24 * 3600)) % 3600) / 60;
}

int M10PtuParser::getSeconds() {
    return (time / 1000 % (24 * 3600)) % 60;
}

double M10PtuParser::getVerticalSpeed() {
    int i;
    unsigned byte;
    unsigned short gpsVel_bytes[2];
    short vel16;
    const double ms2kn100 = 2e2; // m/s -> knots: 1 m/s = 3.6/1.852 kn = 1.94 kn

    for (i = 0; i < 2; i++) {
        byte = frame_bytes[0x08 + i];
        gpsVel_bytes[i] = byte;
    }
    vel16 = gpsVel_bytes[0] << 8 | gpsVel_bytes[1];
    return vel16 / ms2kn100;
}

double M10PtuParser::getHorizontalSpeed() {
    int i;
    unsigned byte;
    unsigned short gpsVel_bytes[2];
    short vel16;
    double vx, vy;
    const double ms2kn100 = 2e2; // m/s -> knots: 1 m/s = 3.6/1.852 kn = 1.94 kn

    for (i = 0; i < 2; i++) {
        byte = frame_bytes[0x04 + i];
        gpsVel_bytes[i] = byte;
    }
    vel16 = gpsVel_bytes[0] << 8 | gpsVel_bytes[1];
    vx = vel16 / ms2kn100; // est

    for (i = 0; i < 2; i++) {
        byte = frame_bytes[0x06 + i];
        gpsVel_bytes[i] = byte;
    }
    vel16 = gpsVel_bytes[0] << 8 | gpsVel_bytes[1];
    vy = vel16 / ms2kn100; // nord

    return sqrt(vx * vx + vy * vy);
}

double M10PtuParser::getDirection() {
    int i;
    unsigned byte;
    unsigned short gpsVel_bytes[2];
    short vel16;
    double vx, vy, dir;
    const double ms2kn100 = 2e2; // m/s -> knots: 1 m/s = 3.6/1.852 kn = 1.94 kn

    for (i = 0; i < 2; i++) {
        byte = frame_bytes[0x04 + i];
        gpsVel_bytes[i] = byte;
    }
    vel16 = gpsVel_bytes[0] << 8 | gpsVel_bytes[1];
    vx = vel16 / ms2kn100; // est

    for (i = 0; i < 2; i++) {
        byte = frame_bytes[0x06 + i];
        gpsVel_bytes[i] = byte;
    }
    vel16 = gpsVel_bytes[0] << 8 | gpsVel_bytes[1];
    vy = vel16 / ms2kn100; // nord

    ///*
    dir = atan2(vx, vy)*180 / M_PI;
    if (dir < 0) dir += 360;
    return dir;
}

double M10PtuParser::getTemperature() {
    // NTC-Thermistor Shibaura PB5-41E
    // T00 = 273.15 +  0.0 , R00 = 15e3
    // T25 = 273.15 + 25.0 , R25 = 5.369e3
    // B00 = 3450.0 Kelvin // 0C..100C, poor fit low temps
    // [  T/C  , R/1e3 ] ( [P__-43]/2.0 ):
    // [ -50.0 , 204.0 ]
    // [ -45.0 , 150.7 ]
    // [ -40.0 , 112.6 ]
    // [ -35.0 , 84.90 ]
    // [ -30.0 , 64.65 ]
    // [ -25.0 , 49.66 ]
    // [ -20.0 , 38.48 ]
    // [ -15.0 , 30.06 ]
    // [ -10.0 , 23.67 ]
    // [  -5.0 , 18.78 ]
    // [   0.0 , 15.00 ]
    // [   5.0 , 12.06 ]
    // [  10.0 , 9.765 ]
    // [  15.0 , 7.955 ]
    // [  20.0 , 6.515 ]
    // [  25.0 , 5.370 ]
    // [  30.0 , 4.448 ]
    // [  35.0 , 3.704 ]
    // [  40.0 , 3.100 ]
    // -> Steinhart–Hart coefficients (polyfit):
    float p0 = 1.07303516e-03,
            p1 = 2.41296733e-04,
            p2 = 2.26744154e-06,
            p3 = 6.52855181e-08;
    // T/K = 1/( p0 + p1*ln(R) + p2*ln(R)^2 + p3*ln(R)^3 )

    // range/scale 0, 1, 2:                        // M10-pcb
    float Rs[3] = {12.1e3, 36.5e3, 475.0e3}; // bias/series
    float Rp[3] = {1e20, 330.0e3, 3000.0e3}; // parallel, Rp[0]=inf

    unsigned char scT; // {0,1,2}, range/scale voltage divider
    unsigned short ADC_RT; // ADC12 P6.7(A7) , adr_0377h,adr_0376h
    unsigned short Tcal[2]; // adr_1000h[scT*4]

    float adc_max = 4095.0; // ADC12
    float x, R;
    float T = 0; // T/Kelvin

    scT = frame_bytes[0x3E]; // adr_0455h
    ADC_RT = (frame_bytes[0x40] << 8) | frame_bytes[0x3F];
    ADC_RT -= 0xA000;
    Tcal[0] = (frame_bytes[0x42] << 8) | frame_bytes[0x41];
    Tcal[1] = (frame_bytes[0x44] << 8) | frame_bytes[0x43];

    x = (adc_max - ADC_RT) / ADC_RT; // (Vcc-Vout)/Vout
    if (scT < 3)
        R = Rs[scT] / (x - Rs[scT] / Rp[scT]);
    else R = -1;

    if (R > 0)
        T = 1 / (p0 + p1 * log(R) + p2 * log(R) * log(R) + p3 * log(R) * log(R) * log(R));


    /*if (1) { // on-chip temperature
        unsigned short ADC_Ti_raw = (frame_bytes[0x49] << 8) | frame_bytes[0x48]; // int.temp.diode, ref: 4095->1.5V
        float vti, ti;
        // INCH1A (temp.diode), slau144
        vti = ADC_Ti_raw / 4095.0 * 1.5; // V_REF+ = 1.5V, no calibration
        ti = (vti - 0.986) / 0.00355; // 0.986/0.00355=277.75, 1.5/4095/0.00355=0.1032
        fprintf(stdout, "  (Ti:%.1fC)\n", ti);
        // SegmentA-Calibration:
        //ui16_t T30 = adr_10e2h; // CAL_ADC_15T30
        //ui16_t T85 = adr_10e4h; // CAL_ADC_15T85
        //float  tic = (ADC_Ti_raw-T30)*(85.0-30.0)/(T85-T30) + 30.0;
        //fprintf(stdout, "  (Tic:%.1fC)", tic);
    }//*/

    return T - 273.15; // Celsius
}

double M10PtuParser::getHumidity() {
    return 0;
}

double M10PtuParser::getDp() {
    return 0;
}

std::string M10PtuParser::getSerialNumber() {
    int i;
    unsigned byte;
    unsigned short sn_bytes[5];
    //char SN[12];

    /*for (i = 0; i < 11; i++)
        SN[i] = ' ';
    SN[11] = '\0';*/

    for (i = 0; i < 5; i++) {
        byte = frame_bytes[0x5D + i];
        sn_bytes[i] = byte;
    }

    // More meaningfull way
    /*byte = sn_bytes[2];
    sprintf(SN, "%1X%02u", (byte >> 4)&0xF, byte & 0xF);
    byte = sn_bytes[3] | (sn_bytes[4] << 8);
    sprintf(SN + 3, " %1X %1u%04u", sn_bytes[0]&0xF, (byte >> 13)&0x7, byte & 0x1FFF);*/

    // The way used by dxlARPS used for compatibility.
    uint32_t id;
    char ids[9];
     
    id = (uint32_t) (((uint32_t) ((uint32_t) (uint8_t)
            sn_bytes[4] + 256UL * (uint32_t) (uint8_t)
            sn_bytes[3] + 65536UL * (uint32_t) (uint8_t)
            sn_bytes[2])^(uint32_t) ((uint32_t) (uint8_t)
            sn_bytes[0] / 16UL + 16UL * (uint32_t) (uint8_t)
            sn_bytes[1] + 4096UL * (uint32_t) (uint8_t)
            sn_bytes[2]))&0xFFFFFUL);
    i = 8UL;
    ids[8U] = 0;
    --i;
    do {
        ids[i] = (char) (id % 10UL + 48UL);
        id = id / 10UL;
        --i;
    } while (i != 1UL);
    ids[i] = 'E';
    --i;
    ids[i] = 'M';

    return ids;
}

void M10PtuParser::printFrame() {
    setenv("TZ", "", 1); // Set local timezone to UTC
    time_t frame = 0;
    struct tm timeinfo;

    timeinfo.tm_hour = getHours();
    timeinfo.tm_min = getMinutes();
    timeinfo.tm_sec = getSeconds();
    timeinfo.tm_mday = getDay();
    timeinfo.tm_mon = getMonth() - 1;
    timeinfo.tm_year = getYear() - 1900;
    timeinfo.tm_isdst = 0;

    frame = mktime(&timeinfo);

    // Decoder sensible to comma at the end, strict json
    printf("{ "
            "\"sub_type\": \"%s\", "
            "\"frame\": %ld, "
            "\"id\": \"%s\", "
            "\"datetime\": \"%04d-%02d-%02dT%02d:%02d:%02dZ\", "
            "\"lat\": %.5f, "
            "\"lon\": %.5f, "
            "\"alt\": %.2f, "
            "\"vel_h\": %.5f, "
            "\"heading\": %.5f, "
            "\"vel_v\": %.2f, "
            "\"temp\": %.1f, "
            "\"crc\": %d "
            "}\n",
            "Ptu", frame, getSerialNumber().c_str(), getYear(), getMonth(), getDay(), getHours(), getMinutes(), getSeconds(), getLatitude(), getLongitude(),
            getAltitude(), getHorizontalSpeed(), getDirection(), getVerticalSpeed(), getTemperature(), correctCRC);
}

/*
 * Convert GPS Week and Seconds to Modified Julian Day.
 * - Adapted from sci.astro FAQ.
 * - Ignores UTC leap seconds.
 */
void M10PtuParser::gps2Date(long GpsWeek, long GpsSeconds, int *Year, int *Month, int *Day) {

    long GpsDays, Mjd;
    long J, C, Y, M;

    GpsDays = GpsWeek * 7 + (GpsSeconds / 86400);
    Mjd = 44244 + GpsDays;

    J = Mjd + 2468570;
    C = 4 * J / 146097;
    J = J - (146097 * C + 3) / 4;
    Y = 4000 * (J + 1) / 1461001;
    J = J - 1461 * Y / 4 + 31;
    M = 80 * J / 2447;
    *Day = J - 2447 * M / 80;
    J = M / 11;
    *Month = M + 2 - (12 * J);
    *Year = 100 * (C - 49) + Y + J;
}