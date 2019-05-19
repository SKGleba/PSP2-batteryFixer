# PSP2-batteryFixer
A small kernel plugin that fixes most of PSVita's battery-related problems.
<br>
Basically the result is the same as hardware-disconnecting the battery or using a syscon keycombo.
<br>
As a bonus it deletes all the temp flags from the battery chip.
# Usage
1) Put the plugin (bicr.skprx) in ur0:tai/ or ux0:tai/
2) Add the plugin path in tai config.txt under '\*KERNEL'
    - i.e: *ur0:tai/bicr.skprx*
3) Reboot, the vita will shutdown itself
4) Turn on the vita. Now you should be asked to set the current date and time
    - if you were asked - the fix worked
5) Remove the plugin path from tai config.txt
    - you may notice that the *bicr.skprx* is no longer present, its normal.
 
