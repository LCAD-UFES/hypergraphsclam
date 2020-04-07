cmake_minimum_required(VERSION 3.14)

project(hypergraphsclam LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# brew install libpng
find_package(PNG REQUIRED)

# brew install pcl
find_package(PCL 1.9 REQUIRED COMPONENTS common io registration features search filters visualization kdtree)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

# git clone https://github.com/RainerKuemmerle/g2o.git
# mkdir build && cd build && cmake .. && make && make install
find_package(g2o REQUIRED)

include(FetchContent)

FetchContent_Declare(viso2
        GIT_REPOSITORY https://github.com/aforechi/libviso2.git
        GIT_TAG        master
        )
FetchContent_MakeAvailable(viso2)

include_directories(${viso2_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/carmen)
include_directories(${CMAKE_SOURCE_DIR}/CustomElements)
include_directories(${CMAKE_SOURCE_DIR}/Helpers)
include_directories(${CMAKE_SOURCE_DIR}/Messages)

set(carmen_SRC_FILES
        carmen/Gdc_Coord_3d.hpp
        carmen/Gdc_Coord_3d.cpp
        carmen/Utm_Coord_3d.hpp
        carmen/Utm_Coord_3d.cpp
        carmen/Gdc_To_Utm_Converter.hpp
        carmen/Gdc_To_Utm_Converter.cpp
        carmen/Utm_To_Gdc_Converter.hpp
        carmen/Utm_To_Gdc_Converter.cpp)

set(CustomElements_SRC_FILES
        CustomElements/VertexOdomAckermanParams.hpp
        CustomElements/CustomVertexSE2.hpp
        CustomElements/EdgeBumblebeeCalibration.hpp
        CustomElements/EdgeFakeGPSOrientation.hpp
        CustomElements/EdgeGPS.hpp
        CustomElements/EdgeLidarCalibration.hpp
        CustomElements/EdgeSE2OdomAckermanCalibration.hpp
        CustomElements/EdgeSickCalibration.hpp
        CustomElements/EdgeVelodyne6DCalibration.hpp
        CustomElements/EdgeVelodyneCalibration.hpp
        CustomElements/EdgeXSENS.hpp)

set(Helpers_SRC_FILES
        Helpers/SimpleLidarSegmentation.cpp
        Helpers/SimpleLidarSegmentation.hpp
        Helpers/StampedMessageType.hpp
        Helpers/StringHelper.cpp
        Helpers/StringHelper.hpp
        Helpers/Wrap2pi.hpp)

set(Messages_SRC_FILES
        Messages/StampedLidar.hpp
        Messages/StampedBumblebee.cpp
        Messages/StampedBumblebee.hpp
        Messages/StampedGPSOrientation.cpp
        Messages/StampedGPSOrientation.hpp
        Messages/StampedGPSPose.cpp
        Messages/StampedGPSPose.hpp
        Messages/StampedLidar.cpp
        Messages/StampedMessage.hpp
        Messages/StampedOdometry.cpp
        Messages/StampedOdometry.hpp
        Messages/StampedSICK.cpp
        Messages/StampedSICK.hpp
        Messages/StampedVelodyne.cpp
        Messages/StampedVelodyne.hpp
        Messages/StampedXSENS.cpp
        Messages/StampedXSENS.hpp)

LIST(APPEND G2O_LIBRARIES
        csparse
        cxsparse
        g2o_solver_csparse
        g2o_csparse_extension
        cholmod
        g2o_solver_cholmod
        g2o_core
        g2o_types_slam2d
        g2o_stuff
        )

add_executable(parser parser.cpp
        src/VehicleModel.cpp
        src/VehicleModel.hpp
        src/GrabData.cpp
        src/GrabData.hpp
        Helpers/StringHelper.cpp
        Helpers/StringHelper.hpp
        Helpers/SimpleLidarSegmentation.cpp
        Helpers/SimpleLidarSegmentation.hpp
        ${carmen_SRC_FILES}
        ${Messages_SRC_FILES})
target_link_libraries(parser viso2 PNG::PNG ${PCL_LIBRARIES})

add_executable(slam hypergraphsclam.cpp
        src/VehicleModel.cpp
        src/VehicleModel.hpp
        src/HyperGraphSclamOptimizer.cpp
        src/HyperGraphSclamOptimizer.hpp
        Helpers/StringHelper.cpp
        Helpers/StringHelper.hpp)
target_link_libraries(slam viso2 PNG::PNG ${PCL_LIBRARIES} ${G2O_LIBRARIES})
