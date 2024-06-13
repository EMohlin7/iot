


#idf_component_register(
#    SRCS "main.c" 
#    "source/wifiController.c" 
#    "source/mqttHandler.c"
#    "source/configServer.c"
#    #"source/lex.yy.c"
#    #"source/configParser.tab.c"
#    "source/sensors.c"
#    "source/connection.c"
#    "source/power.c"
#
#    "dht11/dht11.c"
#                    
#    
#    INCLUDE_DIRS "." "include" "dht11/include"
#    )

idf_component_register(SRC_DIRS "." "source" "dht11" INCLUDE_DIRS "include" "dht11/include")

set(PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../main/source/configParser.tab.c)
set(PARSER_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/../main/include/configParser.tab.h)

set(LEX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../main/source/lex.yy.c)


add_custom_command(
    OUTPUT ${LEX_SRC} 
    COMMAND flex -o ${LEX_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/../configLex.l 
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../configLex.l
)

add_custom_command(
    OUTPUT ${PARSER_SRC} ${PARSER_HEAD} 
    COMMAND bison -o ${PARSER_SRC} --header=${PARSER_HEAD} ${CMAKE_CURRENT_SOURCE_DIR}/../configParser.y
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../configParser.y
)

add_custom_target(prebuild ALL DEPENDS ${LEX_SRC} ${PARSER_SRC})
