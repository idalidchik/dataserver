call setupenv.bat

cd %BUILD_DIR%
mkdir %TOOLCHAINTAG%
cd %TOOLCHAINTAG%

rem set SDL_BOOST_INCLUDES=C:\boost\boost_1_64_0
rem set SDL_BOOST_LIBS=C:\boost\boost_1_64_0\stage\lib
rem %CMAKE% %CMAKEGENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DSDL_BOOST_INCLUDES=%SDL_BOOST_INCLUDES% -DSDL_BOOST_LIBS=%SDL_BOOST_LIBS% %PROJECT_ROOT_DIR%
%CMAKE% %CMAKEGENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %PROJECT_ROOT_DIR%
%CMAKE% --build . --config %BUILD_TYPE%
cd %SCRIPTS_DIR%