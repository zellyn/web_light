// Copied from
// https://github.com/espressif/esp-idf/blob/master/examples/protocols/http_server/simple/main/main.c

#include <stdlib.h>

#include <esp_event.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_tls.h>
#include <esp_tls_crypto.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include "weblight.h"
#include "leds.h"
#include "protocol_examples_utils.h"

#define HTTP_QUERY_KEY_MAX_LEN  (64)



static const char *form_html = "<html>\
<head><title>Light Control: WebLight &ldquo;%s&rdquo;</title></head>\
<body>\
<h1>WebLight &ldquo;%s&rdquo;</h1>\
<form action='' method='get'>\
\
<label for='stay_ms'>Time between color switches (ms): </label>\
<input id='stay_ms' type='number' name='stay_ms' value='%d' min='0' max='65535'/><br>\
\
<label for='fade_steps'>Number of fade steps (0 for instant switch without fade): </label>\
<input id='fade_steps' type='number' name='fade_steps' value='%d' min='0' max='255'/><br>\
\
<label for='fade_step_ms'>Time between fade steps (ms): </label>\
<input id='fade_step_ms' type='number' name='fade_step_ms' value='%d' min='0' max='65535'/><br>\
\
\
<label for='color1'>color 1: </label><input id='color1' name='color1' type='color' value='#%02x%02x%02x'/> \
<label for='color2'>color 2: </label><input id='color2' name='color2' type='color' value='#%02x%02x%02x'/><br>\
<button>Submit</button>\
</form>\
</html>";

uint16_t clamped_parse(char* str, uint16_t max) {
  long parsed = strtol(str, NULL, 10);
  if (parsed < 0) {
    return 0;
  }
  if (parsed > max) {
    return max;
  }
  return (uint16_t) parsed;
}

/* An HTTP GET handler */
static esp_err_t home_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[HTTP_QUERY_KEY_MAX_LEN], dec_param[HTTP_QUERY_KEY_MAX_LEN] = {0};
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "color1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => color1=%s", param);
                example_uri_decode(dec_param, param, strnlen(param, HTTP_QUERY_KEY_MAX_LEN-1)+1);
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
		parse_hash_rrggbb_hex_color(dec_param, &colors[0]);
            }
            if (httpd_query_key_value(buf, "color2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => color2=%s", param);
                example_uri_decode(dec_param, param, strnlen(param, HTTP_QUERY_KEY_MAX_LEN-1)+1);
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
		parse_hash_rrggbb_hex_color(dec_param, &colors[1]);
            }
            if (httpd_query_key_value(buf, "stay_ms", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => stay_ms=%s", param);
                example_uri_decode(dec_param, param, strnlen(param, HTTP_QUERY_KEY_MAX_LEN-1)+1);
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);

		stay_ms = clamped_parse(dec_param, 65535);
            }
            if (httpd_query_key_value(buf, "fade_step_ms", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => fade_step_ms=%s", param);
                example_uri_decode(dec_param, param, strnlen(param, HTTP_QUERY_KEY_MAX_LEN-1)+1);
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);

		fade_step_ms = clamped_parse(dec_param, 65535);
            }
            if (httpd_query_key_value(buf, "fade_steps", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => fade_steps=%s", param);
                example_uri_decode(dec_param, param, strnlen(param, HTTP_QUERY_KEY_MAX_LEN-1)+1);
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);

		fade_steps = clamped_parse(dec_param, 255);
            }
        }
        free(buf);
    }

    buf_len = strlen(form_html) + 1;
    buf_len += strlen(TAG);
    buf_len += strlen(TAG);
    buf_len += 14; // two CSS colors
    buf_len += 5; // stay_ms
    buf_len += 5; // fade_step_ms
    buf_len += 3; // fade_steps

    buf = malloc(buf_len);

    snprintf(buf, buf_len-1, form_html, TAG, TAG,
	     stay_ms, fade_steps, fade_step_ms,
	     colors[0].r, colors[0].g, colors[0].b,
	     colors[1].r, colors[1].g, colors[1].b);

    /* Send response */
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);

    free(buf);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t home = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = home_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &home);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void server_setup_and_start(void)
{
    static httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    server = start_webserver();
}
