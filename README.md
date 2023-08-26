# "Starry night" by Van Gogh animation

This code makes an infinite animation of Van Gogh image "Starry night" and saves gif image. Two waves in the sky flow inside each other infinitely, without image quality degradation. As an input the program requires sparse motion map and motion mask, which were prepared in Gimp software. Also I have here a mask for star, with blurry edges, which serves as a alpha-channel to cut out stars for rotating animation, and make seamless insertion of rotating star into the sky.
It is easy to make rotation infinite, and not deteriorate image quality. For the flowing waves on the sky it is not so obvious!    
There is also a C++ implementation.

![](animation.gif)