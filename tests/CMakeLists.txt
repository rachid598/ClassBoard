# Tests automatisés de ClassBoard (Qt Test).
# Activés avec -DCLASSBOARD_BUILD_TESTS=ON ; lancés par "ctest".

find_package(Qt6 REQUIRED COMPONENTS Core Test)

add_executable(tst_classboardprofile
    tst_classboardprofile.cpp
    ${CMAKE_SOURCE_DIR}/src/classboard/ClassBoardProfile.cpp
    ${CMAKE_SOURCE_DIR}/src/classboard/ClassBoardDisciplines.cpp
)
target_include_directories(tst_classboardprofile PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_classboardprofile PRIVATE Qt6::Core Qt6::Test)
set_target_properties(tst_classboardprofile PROPERTIES AUTOMOC ON AUTOUIC OFF)
add_test(NAME classboard_profile COMMAND tst_classboardprofile)

# Vérifie que chaque application web livrée possède un config.xml et un
# index.html valides (contrôle léger, sans navigateur).
add_test(NAME classboard_widgets
    COMMAND ${CMAKE_COMMAND} -DLIBRARY_DIR=${CMAKE_SOURCE_DIR}/resources/library/applications
            -P ${CMAKE_CURRENT_SOURCE_DIR}/check_widgets.cmake)
