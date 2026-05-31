add_executable(mapreduce MapReduce.cpp)

set_target_properties(
        mapreduce
        PROPERTIES 
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )
