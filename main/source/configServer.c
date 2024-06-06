#include "configServer.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "config.h"
#include "htmlPage.h"
#include "configParser.tab.h"

extern void* yy_scan_bytes ( const char *bytes, int len);

#define TAG "Config Server"
ESP_EVENT_DEFINE_BASE(CONFIG_EVENT);

//Redirect to login portal
static void redirect(httpd_req_t* req){
    httpd_resp_set_status(req, "302");
    httpd_resp_set_hdr(req, "Location", AP_IP);
    httpd_resp_send(req, NULL, 0);
}

static esp_err_t onGet(httpd_req_t* req){
    // If ESP_OK is not returned the underlying socket will be closed
    ESP_LOGI(TAG, "Received get request for URI: %s", req->uri);
    httpd_resp_send(req, htmlPage, strlen(htmlPage));
    
    return ESP_OK;
}

static esp_err_t onPOST(httpd_req_t* req){
    // If ESP_OK is not returned the underlying socket will be closed
    ESP_LOGI(TAG, "Received POST request for URI: %s", req->uri);
    #define BUF_LEN 200
    char buffer[BUF_LEN];
    buffer[BUF_LEN] = 0;
    int read = httpd_req_recv(req, buffer, BUF_LEN);
    

    if(read <= 0)
        return read;

    ESP_LOGI(TAG, "Received %s on post", buffer);

    yy_scan_bytes(buffer, read);
    yyparse(req->user_ctx);

    httpd_resp_send(req, NULL, 0);    
    
    //Launch different event to stop the server. Otherwise the stopping of the server will wait 
    //for this event to finish, which in turn waits for the stopping to finish
    esp_event_post(CONFIG_EVENT, HTTP_SERVER_EVENT_STOP, &req->handle, sizeof(void*), 300);
    return ESP_OK;
}

//Use this event handler to stop the server, in order for the server to not block itself when trying to stop
static void stopHandler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    SemaphoreHandle_t sem = (SemaphoreHandle_t) event_handler_arg;
    httpd_handle_t server = *(httpd_handle_t*)event_data;
    stopConfigServer(server, sem);
}

httpd_handle_t startConfigServer(SemaphoreHandle_t finishedSignal, configData_t* data){
    httpd_handle_t server = NULL;
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &cfg));

    httpd_uri_t getHandler = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = onGet,
    };

    httpd_uri_t postHandler = {
        .uri = "/",
        .method = HTTP_POST,
        .handler = onPOST,
        .user_ctx = data,
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &postHandler));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &getHandler));

    //Used when stopping the server
    ESP_ERROR_CHECK(
        esp_event_handler_register(CONFIG_EVENT, HTTP_SERVER_EVENT_STOP, stopHandler, finishedSignal)
    );

    ESP_LOGI(TAG, "Server started");
    return server;
}

void stopConfigServer(httpd_handle_t server, SemaphoreHandle_t finSignal){
    if(server == NULL)
        return;
    ESP_LOGI(TAG, "Stopping server...");
    ESP_ERROR_CHECK(httpd_stop(server));
    ESP_LOGI(TAG, "Stopped server");
    
    xSemaphoreGive(finSignal);
}