# Popsicle-Stick-Bridge-Tester
This is a basic Popsicle Stick Bridge Tester designed for use in a classroom environment.

# Back Story
This project was created to easily test popsicle stick bridges that students created as a part of the curriculum. This allows a safer and more accurate testing method compared to other more traditional methods.

#Project Notes
Because this is a one-off project little effort was put into refining the project. Certain aspects of the project need manual modification to allow for working functionality.
  1. The onboard regulator for the -5V rail only can produce -3.3V due to being fed 3.3v and a misunderstanding in the datasheet. The fix for this was to simply change the resistors to set the output voltage to -3V. Because the -5V rail is used to excite the load cell the output voltage is slightly lower making it more difficult to measure.
  2. ADC 16x gain input impedance. Due to the small voltage needed to be measured the internal PGA of the ADC is used to help achieve more stable readings from the load cells. This causes the input impedance of the ADC to drop off rapidly resulting in skewed measurements. To fix this additional oversampling (ADC-based) was applied as well as a rolling average(Program based) while using a lower gain. This kept the visually present noise down to an acceptable level. The preferred solution would be to either use a dedicated buffer or general-purpose amp before the ADC to drive it even at the lower input impedances adequately. Or make use of a purpose-built load cell measurement IC. 

Make sure to check out the User Manual for additional information and operation instructions.
