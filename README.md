# Fire Effect

Just a simple little app that does the classic fire effect on the GPU using OpenGL and Qt.

# Requirements

Built against Qt 6.3 community. Tested with MSVC as the compiler and Windows 10 as the OS. Should compile just fine under Clang and other operating systems, as only Qt modules/plugins and the standard library have been used.

# Implementation

If you're unfamiliar with the theory, [this is a good rundown](https://fabiensanglard.net/doom_fire_psx/).

This particular implementation avoids the palette hacks and renders scalar values between 0.0 and 1.0 to a texture. This texture is then read in the final scene composite and mapped to a colour ramp.

I also do an interpolation between both the source and target render textures for the final composite to allow for low update rates. You don't really see the effect at high update rates, and likewise your monitor's refresh rate will affect at what update rate you actually see the lerp happening.

# Notes

Used the opportunity to get a look at Qt, after many people had told me I should consider it as an alternative to WPF. It can certainly replicate the WPF UI implemented in one of my other projects, but the designer studio is really not quite as smooth as WPF/XAML.

Had to go digging in to the Qt source to get the render textures working correctly. Decided to see just how good Qt's GL wrappers were. The framebuffer object can create render textures, but only if you set it to 0 samples. Can't see that in documentation anywhere. Oh well, with that out of the way it's all standard GL stuff but with helper functions.

I _should_ write a uniform wrapper really, but I don't plan on using OpenGL in the future so it's just writing code for the sake of writing code. Ended up quantizing more than I expected to get sliders to work nice, so a way to automate that and be friendly for the UI would go far if I use Qt again.

Also considered writing a software implementation of the fire simulation. Rolling in a SIMD vector wouldn't be a big issue, but I'd also want to look at ways to do SIMT on the CPU. Probably end up realising there's not a great many options and just slicing up the workload as I normally do anyway (like in [Rum and Raisin Doom]((https://github.com/GooberMan/rum-and-raisin-doom/)).