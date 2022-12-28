# Miscellaneous Graphics Programs

Small projects that I made to learn graphics rendering on the GPU through various graphics APIs. Instructions on how to use the programs are found [here](./INSTRUCTIONS.md). These programs were made by using the following resources:

- **[Modern OpenGL Guide](https://raw.githubusercontent.com/Overv/Open.GL/master/ebook/Modern%20OpenGL%20Guide.pdf)** by Alexander Overvoorde. Contains a quick overview of the various OpenGL interfaces.
- **[Learn OpenGL](https://learnopengl.com/)** by Joey de Vries. This gives a more thorough tutorial to OpenGL.
- **[WebGL tutorial](https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/Tutorial)** by Mozilla. Quick overview and tutorial of the WebGL interfaces.
- **OpenGL Programming Guide, Eighth Edition** by Shreiner et al. General OpenGL reference.
- **Mathematics for 3D Game Programming and Computer Graphics** by Eric Lengyel. Contains succinct explanations of the math involved.
- For the spherical harmonics project, an [external library for handling marching cubes](https://github.com/ilastik/marching_cubes) by [ilastik](https://github.com/ilastik) is used, where the source code for this library is downloaded by the Makefile. The formulas for the spherical harmonics are obtained by using [Sympy](https://www.sympy.org/en/index.html). 
- The textures are retrieved from the images found [here](https://renderman.pixar.com/pixar-one-twenty-eight) and converted to bmp files. These were made by Pixar and published under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
- An image of the earth is retrieved from [here](https://visibleearth.nasa.gov/images/73776/august-blue-marble-next-generation-w-topography-and-bathymetry), which was made by NASA.
- The [Noto Sans Font](https://fonts.google.com/specimen/Noto+Sans#standard-styles) by Google are converted to bmp and json format using [FontBuilder](https://github.com/andryblack/fontbuilder) by [andryblack](https://github.com/andryblack/)
- [This series of Youtube videos](https://www.youtube.com/watch?v=PcA-VAybgIQ&list=PLEXt1-oJUa4BVgjZt9tK2MhV_DW7PVDsg&index=1) by the channel [2etime](https://www.youtube.com/@2etime) was helpful in learning the Metal Shading language

More resources are listed in each of the individual project's subdirectories.