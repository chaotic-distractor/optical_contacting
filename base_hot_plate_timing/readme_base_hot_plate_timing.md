WIP

# Base cycles for hot plate

The hot plate has 4 settings: off, low, medium, and high. The goal of this data was 1) to figure out how the hot plate's built-in pulse width modulation (PWM) works 2) to find a correlation between how long the hot plate is on/off and how quickly it heats up/cools down. This preliminary data served as a basis for the Arduino PWM code.

## Files:

data blah blah 3 columns are the time (s), whether the hot plate is on/off, and the temperature (deg C)

low_to_med.txt

med_to_high.txt

off_to_low.txt


graphing_heat_data.ipynb - plots the afformentioned txt files


time-temp-off-low.pdf

time-temp-low-med.pdf

time-temp-med-high.pdf


