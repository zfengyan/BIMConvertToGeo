# BIMConvertToGeo
Convert a BIM model(IFC file) to a CityJSON file.

<img src="https://user-images.githubusercontent.com/72781910/161611724-60f5be17-fc03-4d14-a406-2ac0c6a38c41.PNG" width="600" height="300">

# HOW TO USE

This project can be cloned at: https://github.com/SEUZFY/BIMConvertToGeo.git.

Compile and run it directly.

# Basic Info
`data` : All input and output files.

`figure` : All relevant figures.

`include` : External dependencies for this project.

`report` : The report of this project.

`src` : All source files.

# Dependencies

`CGAL` - https://www.cgal.org/

`nlohmann json` - https://github.com/nlohmann/json

# WHAT TO EXPECT

`The converted CityJSON file`:

<img src="https://user-images.githubusercontent.com/72781910/161613061-d1625a48-8ef8-4e77-a5c8-8a5c971219c4.PNG" width="300" height="230">

`Internal Structures`:

<img src="https://user-images.githubusercontent.com/72781910/161613508-09cf5cee-66d7-4d4e-9210-0a652e6cb2a6.PNG" width="300" height="230">

Visualized in [ninja](https://ninja.cityjson.org/).

# Special Thanks

Ken Arroyo Ohori - k.ohori@tudelft.nl --> [more info](https://3d.bk.tudelft.nl/ken/en/)

Ken's kindly given [guidance](https://3d.bk.tudelft.nl/courses/geo1004//hw/3/) helps us to develop our code and our questions are always answered quickly and accurately, without which this project would not have been possible to be delivered.


# Contributors

Yitong  - xiayitong0630@gmail.com

As for reading input OBJ file and build convex hulls directly, please refer to: https://github.com/YitongXia/BIM_processing.git

Fengyan - zmocheng@gmail.com

