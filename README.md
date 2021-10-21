# cine-moonlite

A moonlite focuser clone for use with telephoto cine lenses. Based on 28BYJ-48 stepper and arduino. This is used to focus a telephoto lens used for astronomical imaging. The focuser is compatible with INDI and ASCOM based systems.

The driver includes backlash compensation to reduce the effects of the cheap mechanicals of the 28BYJ-48 stepper. You may need to tweak the value of `BACKLASHSTEPS` in the code of the firmware depending on the quality of your steppe.

<img src="/resources/onlens.png" alt="parts" width="600"/>

# Parts

* Arduino nano
* 28BYJ-48 stepper and driver
* MakerDoIt 32P 5mm Pinion Gear (RC car spur gear 5mm bore M0.8pitch)
* Project box

Flash the arduino with the firmware, assemble the parts in a project box. Attach to your camera/lens using a bracket (not shown)

<img src="/resources/parts.png" alt="parts" width="600"/>


<img src="/resources/assembled.png" alt="parts" width="600"/>

