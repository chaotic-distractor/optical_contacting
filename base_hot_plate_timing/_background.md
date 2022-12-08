# Modeling base cycles for hot plate

The hot plate has 4 settings: off, low, medium, and high. I turned the hot plate from off to low, recorded what happened for 20ish minutes, turned it from low to med, and repeated until I had 3 sets of data. The goal of this data was 1) to figure out how the hot plate's built-in pulse width modulation (PWM) works 2) to find a correlation between how long the hot plate is on/off and how quickly it heats up/cools down. This preliminary data served as a basis for the Arduino PWM code.

I have not had the chance to entirely read through [the link I shared with you](https://allendowney.github.io/ModSimPy/chap15.html), but if you could find a way to model this heating (i.e. replicate the graphs I made with the data) using the techniques described there or elsewhere, I think this would be a good start towards eventually modeling the system with the optically contacted sample. I'm going to try modeling it as well, just to check that it's a reasonable and feasible task.

I've described and linked the related files below. If something doesn't make sense, don't hesitate to reach out over email!

## Files:

The .txt files are data I took over the summer. The 3 columns are the time (s), whether the hot plate is on/off, and the temperature (deg C):

[off_to_low.txt](https://github.com/jhritzCaltech/optical_contacting/blob/main/base_hot_plate_timing/off_to_low.txt)

[low_to_med.txt](https://github.com/jhritzCaltech/optical_contacting/blob/main/base_hot_plate_timing/low_to_med.txt)

[med_to_high.txt](https://github.com/jhritzCaltech/optical_contacting/blob/main/base_hot_plate_timing/med_to_high.txt)

I plotted them with some (hastily thrown together) code:

[graphing_heat_data.ipynb](https://github.com/jhritzCaltech/optical_contacting/blob/main/base_hot_plate_timing/graphing_heat_data_v2.ipynb)

Resulting graphs:

[time-temp-off-low.pdf](https://github.com/jhritzCaltech/optical_contacting/blob/main/base_hot_plate_timing/time-temp-off-low.pdf)

[time-temp-low-med.pdf](https://github.com/jhritzCaltech/optical_contacting/blob/main/base_hot_plate_timing/time-temp-low-med.pdf)

[time-temp-med-high.pdf](https://github.com/jhritzCaltech/optical_contacting/blob/main/base_hot_plate_timing/time-temp-med-high.pdf)
