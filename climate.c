/**
 * climate.c
 *
 * Performs analysis on climate data provided by the
 * National Oceanic and Atmospheric Administration (NOAA).
 *
 * Input:    Tab-delimited file(s) to analyze.
 * Output:   Summary information about the data.
 *
 * Compile:  run make
 *
 * Example Run:      ./climate data_tn.tdv data_wa.tdv
 *
 *
 * Opening file: data_tn.tdv
 * Opening file: data_wa.tdv
 * States found: TN WA
 * -- State: TN --
 * Number of Records: 17097
 * Average Humidity: 49.4%
 * Average Temperature: 58.3F
 * Max Temperature: 110.4F
 * Max Temperatuer on: Mon Aug  3 11:00:00 2015
 * Min Temperature: -11.1F
 * Min Temperature on: Fri Feb 20 04:00:00 2015
 * Lightning Strikes: 781
 * Records with Snow Cover: 107
 * Average Cloud Cover: 53.0%
 * -- State: WA --
 * Number of Records: 48357
 * Average Humidity: 61.3%
 * Average Temperature: 52.9F
 * Max Temperature: 125.7F
 * Max Temperature on: Sun Jun 28 17:00:00 2015
 * Min Temperature: -18.7F
 * Min Temperature on: Wed Dec 30 04:00:00 2015
 * Lightning Strikes: 1190
 * Records with Snow Cover: 1383
 * Average Cloud Cover: 54.5%
 *
 * TDV format:
 *
 * CA» 1428300000000»  9prcjqk3yc80»   93.0»   0.0»100.0»  0.0»95644.0»277.58716
 * CA» 1430308800000»  9prc9sgwvw80»   4.0»0.0»100.0»  0.0»99226.0»282.63037
 * CA» 1428559200000»  9prrremmdqxb»   61.0»   0.0»0.0»0.0»102112.0»   285.07513
 * CA» 1428192000000»  9prkzkcdypgz»   57.0»   0.0»100.0»  0.0»101765.0» 285.21332
 * CA» 1428170400000»  9prdd41tbzeb»   73.0»   0.0»22.0»   0.0»102074.0» 285.10425
 * CA» 1429768800000»  9pr60tz83r2p»   38.0»   0.0»0.0»0.0»101679.0»   283.9342
 * CA» 1428127200000»  9prj93myxe80»   98.0»   0.0»100.0»  0.0»102343.0» 285.75
 * CA» 1428408000000»  9pr49b49zs7z»   93.0»   0.0»100.0»  0.0»100645.0» 285.82413
 *
 * Each field is separated by a tab character \t and ends with a newline \n.
 *
 * Fields:
 *      state code (e.g., CA, TX, etc),
 *      timestamp (time of observation as a UNIX timestamp),
 *      geolocation (geohash string),
 *      humidity (0 - 100%),
 *      snow (1 = snow present, 0 = no snow),
 *      cloud cover (0 - 100%),
 *      lightning strikes (1 = lightning strike, 0 = no lightning),
 *      pressure (Pa),
 *      surface temperature (Kelvin)
 */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_STATES 50

/**
 * Climate info is a struct that contains a summary of all the data entries analyzed per state
 */
struct climate_info {
    char code[3]; //State Code
    unsigned long long num_records; ///Number of data entries per state summary
    double totalTemp;
    double totalHumidity;
    double maxTemp;
    char maxTempDate[27];
    double minTemp;
    char minTempDate[27];
    int lightningStrikeCount;
    int snowCoverCount;
    double totalCloudCover;
};

/**
 * @param file - Data file that the method will be analyzing
 * @param states - Array containing climate info structs for all 50 states
 * @param num_states - the number of states in the US- 50
 */
void analyze_file(FILE *file, struct climate_info *states[], int num_states);

void print_report(struct climate_info *states[], int num_states);

/**
 *
 * @param states- Array containing climate info structs for all 50 states
 * @param stateCode- State Code of the state we are looking for
 * @return Index of the state in the states array. If states doesn't contain the state code the method returns the
 * negative index of the next null struct in the states array
 */
int indexOfState(struct climate_info **states, char *stateCode) {
    int i; //index
    //Look for the first null struct
    for (i = 0; states[i] != NULL; i++) {
        //If we find thtat the state is already initialized return its index
        if (strcmp(states[i]->code, stateCode) == 0) return i;
    }
    //If the state is not found we return the index of the first null spot in the array and use the negative sign
    //as a flag to initialize a new struct.
    //Zero is a special case because it has no sign
    if (i == 0) {
        return -50;
    } else return -i;
}

int main(int argc, char *argv[]) {

    if (argc < 2) { //Check for at least one data file
        printf("Usage: %s tdv_file1 tdv_file2 ... tdv_fileN \n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Let's create an array to store our state data in. As we know, there are
     * 50 US states. */
    struct climate_info *states[NUM_STATES] = {NULL};

    FILE *file;
    for (int i = 1; i < argc; i++) {
        file = fopen(argv[i], "r"); //Open the file for reading
        printf("Opening file: %s\n", argv[i]);

        if (file != NULL) {
            //If the file exists analyze it
            analyze_file(file, states, NUM_STATES);
            fclose(file);
        } else {
            //If not print an error
            printf("Error File # %d doesn't exist!\n", i);
        }
    }

    /* Now that we have recorded data for each file, we'll summarize them: */
    print_report(states, NUM_STATES);

    return 0;
}

void analyze_file(FILE *file, struct climate_info **states, int num_states) {
    //Size of each line we are splitting
    const int line_sz = 100;
    //Line we are splitting
    char line[line_sz];
    //Using tab as the delimiter
    char delim[2] = "\t";
    //Current split will be stored in token, so we can handle the data
    char *token;

    //While there is a file to get -> get it
    while (fgets(line, line_sz, file) != NULL) {
        //First token is the state code
        token = strtok(line, delim);
        //Get the index of the state code or the index of the next free space in the states array
        int index = indexOfState(states, token);
        //Declare our struct to edit
        struct climate_info *ci;
        //if index is negative then it's a new state, so we must allocate memory
        if (index < 0) {
            //Allocate the memory
            ci = (struct climate_info*)malloc(sizeof(struct climate_info));
            //Set the code to state code which we currently have stored in our token
            strcpy(ci->code, token);

            //Set Base Values for sum/incrementing
            ci->num_records = 0;
            ci->totalHumidity = 0;
            ci->totalCloudCover = 0;
            ci->lightningStrikeCount = 0;
            ci->snowCoverCount = 0;
            ci->totalTemp = 0;
            //Set both cases to extremes to act as sudo infinity
            ci->maxTemp = -1000;
            ci->minTemp = 1000;

            //Our index function cannot return a negative when the index is zero, so we use a special case of -50
            //to trip the negative flag. We handle this special case by just setting it back to zero.
            //For all other cases we flip the sign
            if (index == -50) {
                index = 0;
            } else index = -index;
            //Once we know the index we can put the pointer into our states array
            states[index] = ci;
        }
        //We grab the state we are going to edit based on the index
        ci = states[index];

        //For every record we increment its states record count
        ci->num_records++;

        //Second token is the Timestamp
        //We store this token for later in case it's needed for the max/min temp
        token = strtok(NULL, delim);
        const unsigned long currentTS = atol(token) / 1000;

        //Third token is the GeoLocation- This is irrelevant to our data output, so we ignore it
        token = strtok(NULL, delim);

        //4th token is avg humidity
        token = strtok(NULL, delim);
        ci->totalHumidity += atof(token);

        //5th token
        token = strtok(NULL, delim);
        if (*token == '1') {
            ci->snowCoverCount++;
        }

        //6th token is cloud cover- Same as avg humid
        token = strtok(NULL, delim);
        ci->totalCloudCover += atof(token);

        //7th token is lightning strikes
        token = strtok(NULL, delim);
        if (*token == '1') {
            ci->lightningStrikeCount++;
        }

        //8- Pressure - Not used
        token = strtok(NULL, delim);

        //9- Temp
        token = strtok(NULL, delim);
        double temp = atof(token) * 1.8 - 459.67;
        ci->totalTemp += temp;
        //Max
        if (temp > ci->maxTemp) {
            ci->maxTemp = temp;
            strcpy(ci->maxTempDate, ctime(&currentTS));


        }
        //Min
        if (temp < ci->minTemp) {
            ci->minTemp = temp;
            strcpy(ci->minTempDate, ctime(&currentTS));
        }
    }
}

void print_report(struct climate_info *states[], int num_states) {
    printf("States found:\n");
    int i;
    for (i = 0; i < num_states; ++i) {
        if (states[i] != NULL) {
            struct climate_info *info = states[i];
            printf("%s ", info->code);
        }
    }
    printf("\n");

    for (i = 0; i < num_states; i++) {
        if (states[i] != NULL) {
            struct climate_info *info = states[i];
            printf("-- State: %s --\n", info->code);
            printf("Number of Records: %llu\n", info->num_records);
            printf("Average Humidity: %.1f%%\n", info->totalHumidity / info->num_records);
            printf("Average Temperature: %.1fF\n", info->totalTemp / info->num_records);
            printf("Max Temperature: %.1fF\n", info->maxTemp);
            printf("Max Temperature on: %s", info->maxTempDate);
            printf("Min Temperature: %.1fF\n", info->minTemp);
            printf("Min Temperature on: %s", info->minTempDate);
            printf("Lightning Strikes: %d\n", info->lightningStrikeCount);
            printf("Records with Snow Cover: %d\n", info->snowCoverCount);
            printf("Average Cloud Cover: %.1f%%\n", info->totalCloudCover / info->num_records);
        }
    }
    printf("\n");
}

