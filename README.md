This program processes real county demographics data collected by the CORGIS project.

## Data Loading:
  The program loads county demographics data from a CSV file, converting numeric fields to appropriate data types and validating entries.
## Command-Line Arguments: 
  The program accepts two command-line arguments: the demographics data file and the operations file.
## Data Analysis Operations:
  The program supports various operations specified in the operations file, including filtering data, displaying county information, calculating population totals, and computing percentages of sub-populations.
## Error Handling: 
  The program handles malformed entries in both the demographics and operations files, reporting errors and continuing processing.

# Key Features
## Loading and Validating Data:
Load county demographics data from a CSV file.
Skip column headers and handle double-quote characters in fields.
Validate and convert numeric fields to appropriate data types.
Print the number of valid entries loaded and report malformed entries.

## Command-Line Interface:
Accept two arguments: demographics data file and operations file.
Validate the existence and accessibility of these files.

## Operations on Data:

display: Print county information for each entry.
filter-state:<state abbreviation>: Filter entries by state abbreviation.
filter:<field>:<ge/le>:<number>: Filter entries based on numeric field values.
population-total: Calculate the total 2014 population across all entries.
population:<field>: Calculate the total sub-population for specified fields.
percent:<field>: Calculate the percentage of the sub-population within the total population.

## Sample Operations:

Determine the percentage of the population below the poverty level in counties with specific education or ethnicity percentages.
Create operations files to test and verify these calculations.
