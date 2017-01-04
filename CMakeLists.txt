cmake_minimum_required(VERSION 2.8)

project (mqtt_c)

#Add related projects


#Add include directories
include_directories("inc")


#Add sources
set(srcs src/PubSubClient.c)


#Add Library
add_library(mqtt_c SHARED ${srcs})

#######################################

#if (TEST)
#  set(testsrcs test/test.c)
#  add_executable(fifo_test ${testsrcs})
#  target_link_libraries(fifo_test FIFO)
#endif()


#target_sources(fifo_test)
