
%{
    #include "esp_log.h"
    #include "configServer.h"
    #include "string.h"
    #define BUF_SIZE 100
    extern int yyerror(configData_t* configData, char const *msg);
    extern int yylex();

    int inputIndex = 0;
    char inputBuffer[BUF_SIZE];

    int tokenValue;
    #define clear() memset(inputBuffer, 0, BUF_SIZE)
    #define setData(x, len) memcpy(x, inputBuffer, len); clear(); inputIndex = 0;
%}

%parse-param {void* configData}

%token SSID PASS URL PORT EQ AND CHAR NUM
%start config
%%

config : entry AND entry AND entry AND entry 
    ;

entry : ssid | pass | url | port 
    ;

ssid : SSID EQ input {setData(((configData_t*)configData)->ssid, SSID_MAX_LEN+1); }
    ;

pass : PASS EQ input {setData(((configData_t*)configData)->password, WIFI_PASS_MAX_LEN+1); }
    ;

url : URL EQ input {setData(((configData_t*)configData)->mqttURL, URL_MAX_LEN+1); }
    ;

port : PORT EQ NUM {((configData_t*)configData)->mqttPort = (uint16_t)tokenValue;}
    ;


input : input CHAR {inputBuffer[inputIndex++] = (char)tokenValue; inputBuffer[inputIndex] = 0;}
    | //Empty
    ;


%%

int yyerror(configData_t* configData, char const *msg) {
    ESP_LOGE("PARSE", "Error: %s", msg);
    //YYABORT;
    return 0;
}

int yywrap(){
    return 1;
}
