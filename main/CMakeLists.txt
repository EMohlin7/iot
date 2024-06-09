idf_component_register(
    SRCS "main.c" 
    "source/wifiController.c" 
    "source/mqttHandler.c"
    "source/configServer.c"
    "source/lex.yy.c"
    "source/configParser.tab.c"
    "source/sensors.c"
    "source/connection.c"

    "dht11/dht11.c"
                    
    
    INCLUDE_DIRS "." "include" "dht11/include")


