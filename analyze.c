#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//county structure to hold all important info
typedef struct {
    char county[100];
    char state[3];
    double education_bachelors_or_higher;
    double education_high_school_or_higher;

    double american_indian_and_alaska_native_alone;
    double asian_alone;
    double black_alone;
    double hispanic_or_latino;
    double native_hawaiian_or_other_pacific_islander_alone;
    double two_or_more_races;
    double white_alone;
    double white_alone_not_hispanic_or_latino;

    double income_median_household_income;
    double income_per_capita_income;
    double income_persons_below_poverty_level;
    int population_2014;
} County;

void setCounties(FILE *demFile, County** counties, int* countNum);
void checkOps(FILE* opFile, County* counties, int countNum);
void unquote(char* field);
void display(County *county);
void stateFilter(County** counties, int* countNum, const char* state);
void filter(County** countiesPtr, int* countNum, const char* field, const char* op, double val);
int popTotal(County* counties, int countNum);
double subPop(County* counties, int countNum, const char* field);
void percents(County* counties, int countNum, const char* field);
double getPercent(const County* county, const char* field);

void unquote(char* field) {
    //get rid of double quotes around fields 
    //from is where it actually is in field (index)
    //to is the returned unquoted field 
    char* from = field;
    char* to = field;
    while (*from != '\0') {
        if (*from != '\"') {
            *to++ = *from;
        }
        from++;
    }
    *to = '\0';
}
int main(int argc, char *argv[]) {
    FILE *demFile; //demographics file pointer
    FILE *opFile; //operations file pointer
    County *counties = NULL; //pointer to 1st struct in array of county structs
    int countNum = 0; //count number of counties

    //error if incorrect number of arguments
    if (argc != 3){
        fprintf(stderr, "invalid number of arguments given");
        return 1;
    }

    //read files
    demFile = fopen(argv[1], "r");
    opFile = fopen(argv[2], "r");

    //print error if either file doesn't open
    if (!demFile || !opFile){
        fprintf(stderr, "error opening files\n");
        return 1;
    }

    setCounties(demFile, &counties, &countNum);
    checkOps(opFile, counties, countNum);
    //free counties if not already freed
    if (counties != NULL) {
        free(counties);  
        counties = NULL; 
    }
    return 0;
}


void setCounties(FILE *demFile, County** counties, int* countNum){
    char line[2048]; //2048 for max possible line length
    //allocate memory for array of county structs
    *counties = calloc(5000, sizeof(County));

    //check if malloc worked
    if (*counties == NULL){
        fprintf(stderr, "error with malloc in setCounties");
        return;
    }

    //skip header line
    fgets(line, sizeof(line), demFile);

    while (fgets(line, sizeof(line), demFile) != NULL){
        //set pointer to address of next open spot in array
        County *curCounty = &(*counties)[*countNum];
        int i = 0;
        //split up fields by , delimiter
        char* field = strtok(line, ",");
        while (field != NULL) {
            unquote(field);
            //convert field based on exact column number and type 
            if(i == 0) strncpy(curCounty->county, field, sizeof(curCounty->county) - 1);
            else if(i == 1) strncpy(curCounty->state, field, sizeof(curCounty->state) - 1);
            //convert string to doubles with atof in helper

            else if(i == 5) curCounty->education_bachelors_or_higher = atof(field);
            else if(i == 6) curCounty->education_high_school_or_higher = atof(field);

            else if(i == 11) curCounty->american_indian_and_alaska_native_alone = atof(field);
            else if(i == 12) curCounty->asian_alone = atof(field);
            else if(i == 13) curCounty->black_alone = atof(field);
            else if(i == 14) curCounty->hispanic_or_latino = atof(field);
            else if(i == 15) curCounty->native_hawaiian_or_other_pacific_islander_alone = atof(field);
            else if(i == 16) curCounty->two_or_more_races = atof(field);
            else if(i == 17) curCounty->white_alone = atof(field);
            else if(i == 18) curCounty->white_alone_not_hispanic_or_latino = atof(field);

            else if(i == 25) curCounty->income_median_household_income = atof(field);
            else if(i == 26) curCounty->income_per_capita_income = atof(field);
            else if(i == 27) curCounty->income_persons_below_poverty_level = atof(field);
            else if(i == 38) curCounty->population_2014 = atoi(field);

            field = strtok(NULL, ",");
            i++;
        }
        
        (*countNum)++;
    }
    printf("%d records loaded.\n", *countNum);
    fclose(demFile);
}

void checkOps(FILE* opFile, County* counties, int countNum){
    char line[2048];
    while (fgets(line, sizeof(line), opFile)){
        //find newline and removes it/ sets its to \0
        line[strcspn(line, "\n")] = '\0';

        //check for what the op is asking for based on string comparison
        if (strcmp(line, "display") == 0) {
            for (int i = 0; i < countNum; i++) {
                display(&counties[i]);
            }
        //check if first 13 characters are filter-state: to use that function
        } else if (strncmp(line, "filter-state:", 13) == 0) {
            //initialize to all null chars
            char state[3] = {0};
            //read state abbreviation up to 2 chars 
            sscanf(line, "filter-state:%2s", state);
            stateFilter(&counties, &countNum, state);

        } else if (strncmp(line, "filter:", 7) == 0) {
            //vars for field name and operation (le, ge)
            char field[100], op[3];
            //value being compared against
            double val;
            //scan line at 8th pos up to 99 chars (bc null) long stopping before :
            //and reads op input to 2 chars so space for null
            sscanf(line + 7, "%99[^:]:%2s:%lf", field, op, &val);
            filter(&counties, &countNum, field, op, val); 
        } else if (strncmp(line, "population-total", 16) == 0) {
            popTotal(counties, countNum);
        } else if (strncmp(line, "population:", 11) == 0) {
            char field[100];
            //reads before newline and before null
            sscanf(line + 11, "%99[^\n]", field);
            subPop(counties, countNum, field);
        } else if (strncmp(line, "percent:", 8) == 0){
            char field[100];
            sscanf(line + 8, "%99[^\n]", field); 
            percents(counties, countNum, field);
        }
        
    }
    fclose(opFile);
}

void stateFilter(County** counties, int* countNum, const char* state){
    int index = 0;
    //malloc temp array for correctly filtered matching states
    County* temp = (County*)calloc((*countNum), sizeof(County));
    if (temp == NULL) {
        fprintf(stderr, "malloc in stateFilter failed\n");
        return;
    }
    //look through num of all the counties
    for (int i = 0; i < *countNum; ++i) {
        //if matching state with specific county state, copy
        if (strcmp((*counties)[i].state, state) == 0) {
            temp[index++] = (*counties)[i];
        }
    }
    //update num of counties and switch counties to filtered temp

    //free(*counties);
    *counties = temp;
    *countNum = index;
    printf("Filter: state == %s (%d entries)\n", state, index);
}
void filter(County** countiesPtr, int* countNum, const char* field, const char* op, double val) {
    //County* counties = *countiesPtr;
    int count = 0; //count num of filtered counties

    //malloc temp array for correctly filtered counties
    County* filtered = (County*)malloc(*countNum * sizeof(County));
    if (!filtered) {
        fprintf(stderr, "malloc in filter failed\n");
        return;
    }
    
    //for all the counties, check percent if the field matches
    for (int i = 0; i < *countNum; ++i) {
        //reset percent each time
        double percent = getPercent((*countiesPtr)+i, field);        
        //check operation type and if true add to filtered
        if ((strcmp(op, "ge") == 0 && percent >= val) ||
            (strcmp(op, "le") == 0 && percent <= val)) {
            filtered[count++] = (*countiesPtr)[i];
        }
    }
    //change original counties to new filtered set
    free(countiesPtr);
    *countiesPtr = filtered; 
    //can't free filtere since countiesptr is using it 
    *countNum =  count; //change original count to filtered count

    printf("Filter: %s %s %.2f (%d entries)\n", field, op, val, count);
}
//find the total population
int popTotal(County* counties, int countNum) {
    int total = 0;
    for (int i = 0; i < countNum; i++) {
        total += counties[i].population_2014;
    }
    printf("2014 population: %d\n", total);
    return total;
}
//find the sub population based on the specific field
double subPop(County* counties, int countNum, const char* field) {
    double subTotal = 0;

    for (int i = 0; i < countNum; ++i) {
        //reset percent using helper
        double percent = getPercent(&counties[i], field);        
        subTotal += ((percent / 100.0) * counties[i].population_2014);
    }

    printf("2014 %s population: %f\n", field, subTotal);
    return subTotal;
}
//get the percent of the sub population based on field
void percents(County* counties, int countNum, const char* field) {
    int total = 0;
    double subPopu = 0;

    for (int i = 0; i < countNum; ++i) {
        //reset percent using helper
        double percent = getPercent(&counties[i], field);        
        subPopu += ((percent / 100.0) * counties[i].population_2014);
    }

    for (int i = 0; i < countNum; i++) {
        total += counties[i].population_2014;
    }
    //make sure no division by 0 
    if (total > 0) {
        double perTot = 100.0 * subPopu / total;
        printf("2014 %s percentage: %.2lf%%\n", field, perTot);
    } else {
        printf("Error: Total population for 2014 is zero.\n");
    }
}
//helper used to get specific field 
double getPercent(const County* county, const char* field) {
    if (strcmp(field, "Education.Bachelor's Degree or Higher") == 0) {
        return county->education_bachelors_or_higher;
    } else if (strcmp(field, "Education.High School or Higher") == 0) {
        return county->education_high_school_or_higher;
    } else if (strcmp(field, "Ethnicities.American Indian and Alaska Native Alone") == 0) {
        return county->american_indian_and_alaska_native_alone;
    } else if (strcmp(field, "Ethnicities.Asian Alone") == 0) {
        return county->asian_alone;
    } else if (strcmp(field, "Ethnicities.Black Alone") == 0) {
        return county->black_alone;
    } else if (strcmp(field, "Ethnicities.Hispanic or Latino") == 0) {
        return county->hispanic_or_latino;
    } else if (strcmp(field, "Ethnicities.Native Hawaiian and Other Pacific Islander Alone") == 0) {
        return county->native_hawaiian_or_other_pacific_islander_alone;
    } else if (strcmp(field, "Ethnicities.Two or More Races") == 0) {
        return county->two_or_more_races;
    } else if (strcmp(field, "Ethnicities.White Alone") == 0) {
        return county->white_alone;
    } else if (strcmp(field, "Ethnicities.White Alone, not Hispanic or Latino") == 0) {
        return county->white_alone_not_hispanic_or_latino;
    } else if (strcmp(field, "Income.Persons Below Poverty Level") == 0) {
        return county->income_persons_below_poverty_level;
    }
    return 0.0;
}
void display(County *county) {
    //print based on format in assignment
    printf("%s County, %s\n", county->county, county->state);
    printf("\tPopulation: %d\n", county->population_2014);
    printf("\tEducation\n");
    printf("\t\t>= High School: %.6f%%\n", county->education_high_school_or_higher);
    printf("\t\t>= Bachelor's: %.6f%%\n", county->education_bachelors_or_higher);
    printf("\tEthnicity percents\n");
    printf("\t\tAmerican Indian and Alaska Native: %.6f%%\n", county->american_indian_and_alaska_native_alone);
    printf("\t\tAsian Alone: %.6f%%\n", county->asian_alone);
    printf("\t\tBlack Alone: %.6f%%\n", county->black_alone);
    printf("\t\tHispanic or Latino: %.6f%%\n", county->hispanic_or_latino);
    printf("\t\tNative Hawaiian and Other Pacific Islander Alone: %.6f%%\n", county->native_hawaiian_or_other_pacific_islander_alone);
    printf("\t\tTwo or More Races: %.6f%%\n", county->two_or_more_races);
    printf("\t\tWhite Alone: %.6f%%\n", county->white_alone);
    printf("\t\tWhite Alone, not Hispanic or Latino: %.6f%%\n", county->white_alone_not_hispanic_or_latino);
    printf("\tIncome\n"); 
    printf("\t\tMedian Household: %.2f\n", county->income_median_household_income);
    printf("\t\tPer Capita: %.2f\n", county->income_per_capita_income);
    printf("\t\tBelow Poverty Level: %.6f%%\n", county->income_persons_below_poverty_level);
}
