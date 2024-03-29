#include "whatsapp_messaging_http.h"

static const char *TAG = "HTTP_CLIENT";

//callmebot api specific values
const char *api_key = CONFIG_API_KEY;
const char *whatsapp_num = CONFIG_WHATSAPP_NUM;
const char *whatsapp_message = CONFIG_WHATSAPP_MESSAGE;

void send_whatsapp_message()
{

    // callmebot api uri without values
    char callmebot_url[] = "https://api.callmebot.com/whatsapp.php?phone=%s&text=%s&apikey=%s";

    // uri with values
    char URL[strlen(callmebot_url)];

    // use sprintf function to set values to URL variable
    sprintf(URL, callmebot_url, whatsapp_num, url_encode(whatsapp_message), api_key);
    ESP_LOGI(TAG, "URL = %s", URL);

    //set URL with values and http request method
    esp_http_client_config_t config = {
        .url = URL,
        .method = HTTP_METHOD_GET,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    //perform http request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200)
        {
            ESP_LOGI(TAG, "Message sent Successfully");
        }
        else
        {
            ESP_LOGI(TAG, "Message sent Failed");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Message sent Failed");
    }
    esp_http_client_cleanup(client);
}