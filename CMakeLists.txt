set(CMAKE_C_STANDARD 99)

add_executable(converter src/converter.c src/bmp_handler.c)
add_executable(comparer src/comparer.c src/bmp_handler.c)

if(MSVC)
  target_compile_options(converter PRIVATE /W4 /WX)
  target_compile_options(comparer PRIVATE /W4 /WX)
else()
  target_compile_options(converter PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(comparer PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_executable(comparer src/comparer.c src/bmp_handler.c)