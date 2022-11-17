# Hot plate heating drop off

A key part of my work is heating up my optically contacted samples with a hot plate. Idealy, the heat will strengthen the bond, for reasons I can elaborate on another time. However, if the samples are heated too fast, the heating causes differential expansion. In other words, the bottom half of the bonded sample heats up far quicker than the top, and because heat causes the material to expand slightly, the bottom half expands quicker than the top half, breaking the bond. The hot plate set up is as follows:

picture

The samples are placed between two copper sheets to protect them. Then they are sandwiched between two aluminum plates which each have thermocouples attached to them (not shown in that picture). We want measure the temperature above and below the samples then take the average, which gives us the most accurate measurement exactly at the samples. It also helps us understand how quickly the heat permeates the materials.

To control how fast the hot plate heats up, I am using Pulse Width Modulation (PWM) with an Arduino. Effectively, the Arduino repeatedly turns the AC power to the hot plate on and off, slowing the heating process. By varying how long the hot plate is left on or off, I can control the speed of the heating. Determine what length of on/off corresponds to what speed meant a lot of trial and error. One issue that I am still dealing with is that, if I keep the on/off cycle constant, the hot plate flat lines at a certain tempearture, as seen below:

picture

The hope is to find a way to algorithmically increase the on/off rate as the temperature rises so that the actual speed of the temperature's increase stays constant.
