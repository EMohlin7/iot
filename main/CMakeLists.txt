idf_component_register(
    SRCS "main.c" 
    "source/wifiController.c" 
    "source/mqttHandler.c"
    "source/configServer.c"
    "source/lex.yy.c"
    "source/configParser.tab.c"
                    
    
    INCLUDE_DIRS "." "include")


