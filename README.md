# "Starry night" by Van Gogh animation

This code makes an infinite animation of Van Gogh image "Starry night" and saves gif image. As an input it requires sparse motion map and motion mask, which were prepared in Gimp software. Also I have here a mask for star, with blurry edge, which serves as a alpha-channel to cut out stars for rotating animation.
It easy to make rotation infinite, and not deteriorating image quality. For the wave on the sky it is not so obvious!    
There is also a C++ implementation

![](animation.gif)