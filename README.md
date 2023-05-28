# ESRRS
Energy-saving refresh rate switch

A small util for laptops and such. Automatically switches between two refresh rates based on if the power cable is connected or not.

## Usage
Even though it's usable, the util may have a lot of bugs so i don't take any resposobility for but if u think it's good enough:  
1. Download the code  
2. Alter the variables for battery(batteryRR) and ac-powered(lineRR) refresh rates (in hertz) in App.cpp
3. Compile
4. Launch the app and add a shortcut to it to your startup directory for it to always run in the background

## Roadmap
-Refactor the code to make it more future-proof  
-Add a settings window  
-Add battery-level based refresh rate changes
