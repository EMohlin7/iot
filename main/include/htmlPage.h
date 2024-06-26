static const char* htmlPage = "<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>WiFi and MQTT Configuration</title>\n"
"    <style>\n"
"        body {\n"
"            font-family: Arial, sans-serif;\n"
"            background-color: #f0f0f0;\n"
"            margin: 0;\n"
"            padding: 0;\n"
"            display: flex;\n"
"            justify-content: center;\n"
"            align-items: center;\n"
"            height: 100vh;\n"
"        }\n"
"        .container {\n"
"            background-color: #fff;\n"
"            padding: 20px;\n"
"            border-radius: 10px;\n"
"            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);\n"
"        }\n"
"        h2 {\n"
"            margin-top: 0;\n"
"        }\n"
"        .form-group {\n"
"            margin-bottom: 15px;\n"
"        }\n"
"        .form-group label {\n"
"            display: block;\n"
"            margin-bottom: 5px;\n"
"        }\n"
"        .form-group input {\n"
"            width: 100%;\n"
"            padding: 8px;\n"
"            box-sizing: border-box;\n"
"            border: 1px solid #ccc;\n"
"            border-radius: 5px;\n"
"        }\n"
"        .form-group input[type=\"submit\"] {\n"
"            background-color: #4CAF50;\n"
"            color: white;\n"
"            border: none;\n"
"            cursor: pointer;\n"
"        }\n"
"        .form-group input[type=\"submit\"]:hover {\n"
"            background-color: #45a049;\n"
"        }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"\n"
"<div class=\"container\">\n"
"    <h2>WiFi and MQTT Configuration</h2>\n"
"    <form action=\"/\" method=\"post\">\n"
"        <div class=\"form-group\">\n"
"            <label for=\"ssid\">SSID:</label>\n"
"            <input type=\"text\" id=\"ssid\" name=\"ssid\" required>\n"
"        </div>\n"
"        <div class=\"form-group\">\n"
"            <label for=\"password\">Password:</label>\n"
"            <input type=\"password\" id=\"password\" name=\"password\">\n"
"        </div>\n"
"        <div class=\"form-group\">\n"
"            <label for=\"mqtt-url\">MQTT Broker URL:</label>\n"
"            <input type=\"text\" id=\"mqtt-url\" name=\"mqtt-url\" required>\n"
"        </div>\n"
"        <div class=\"form-group\">\n"
"            <label for=\"mqtt-port\">MQTT Broker Port:</label>\n"
"            <input type=\"number\" id=\"mqtt-port\" name=\"mqtt-port\" required>\n"
"        </div>\n"
"        <div class=\"form-group\">\n"
"            <input type=\"submit\" value=\"Submit\">\n"
"        </div>\n"
"    </form>\n"
"</div>\n"
"\n"
"</body>\n"
"</html>\n";