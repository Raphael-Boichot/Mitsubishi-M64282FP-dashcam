## Two versions that does about the same thing

- the 250MHz version is more responsive but drains battery like crazy due to overclocking (behaves more like a digital regular camera, my prefer version to point and shoot);
- the 133MHz version is slower but battery last for eternity (my prefered version for long timelapses).

In any case the internal clock to bitbang the sensor is always forced to 1 MHz maximum, recommended frequency by Mitsubishi. It is possible to downclock the sensor (this is the principle of the "nigth mode" feature), but not overclocking it (this produces images with dark halo in the upper side as the photon collection collapses with the pixel reading). Of course this is written nowhere in the datasheet.

The image writing to SD is the bottleneck of the device, it cannot be improved by overclocking, at least with my current knowledge. This locks the writing speed to more or less to 4 fps. It would be possible to compress data with the Game Boy Tile Format to increase writing speed but this would be possible only with dithering mode on, so a complicated feature to implement for very little added value.
