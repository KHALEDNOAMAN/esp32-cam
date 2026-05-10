file(STRINGS ${CMAKE_CURRENT_LIST_DIR}/../version.txt PROJECT_VER LIMIT_COUNT 1)
string(STRIP "${PROJECT_VER}" PROJECT_VER)

idf_component_register(
        SRCS
        "esp_cam.c"
        "components/config/config.h"
        "components/wifi/wifi.h"
        "components/wifi/wifi.c"
        "components/camera/camera.h"
        "components/camera/camera.c"
        "components/sdcard/sdcard.h"
        "components/sdcard/sdcard.c"
        "components/http_server/http_server.h"
        "components/http_server/http_server.c"
        "components/utils/utils.h"
        "components/dns/dns.h"
        "components/dns/dns.c"
        "components/nvs/nvs.h"
        "components/nvs/nvs.c"
        "components/http_server/actions/actions.h"
        "components/http_server/actions/index_action.c"
        "components/http_server/actions/status_action.c"
        "components/http_server/actions/stream_action.c"
        "components/http_server/actions/capture_action.c"
        "components/http_server/actions/save_image_action.c"
        INCLUDE_DIRS
        "."
        "components/config"
        REQUIRES
        esp_wifi
        esp_http_server
        esp_http_client
        fatfs
        sdmmc
        driver
        esp_timer
        nvs_flash
        esp_event
        esp_netif
        espressif__esp32-camera
        espressif__esp_jpeg
        log
        freertos
        esp_https_ota
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE -DPROJECT_VER="${PROJECT_VER}")