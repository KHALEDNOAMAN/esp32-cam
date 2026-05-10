//
// Created by admin on 10.05.2026.
//
#include "components/config/config.h"
#include "components/camera/camera.h"
#include "esp_http_server.h"
#include <stdio.h>

esp_err_t index_handler(httpd_req_t *req) {
    static char html[2048];

    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html><head>"
        "<meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>ESP32-CAM</title>"
        "<style>"
        "body{font-family:monospace;background:#111;color:#0f0;padding:16px}"
        "img{max-width:640px;border:2px solid #0f0;display:block;margin:8px 0}"
        "button{background:#0f0;color:#111;border:0;padding:8px 16px;"
               "cursor:pointer;margin:4px;border-radius:4px}"
        "</style></head><body>"
        "<h2>ESP32-CAM Stream</h2>"
        "<img id='s' src='' alt='Loading stream...'/>"
        "<br>"
        "<button onclick=\"document.getElementById('s').src="
            "'http://'+location.hostname+':%d/stream?t='+Date.now()\">"
            "▶ Start Stream</button>"
        "<button onclick=\"fetch('/capture',{method:'POST'})"
            ".then(r=>r.json()).then(d=>alert('Saved: '+d.file))\">📷 Capture</button>"
        "<button onclick=\"fetch('/status').then(r=>r.json())"
            ".then(d=>document.getElementById('info').textContent="
            "JSON.stringify(d,null,2))\">📊 Status</button>"
        "<pre id='info'></pre>"
        "<script>"
        "document.getElementById('s').src="
            "'http://'+location.hostname+':%d/stream';"
        "</script>"
        "</body></html>",
    HTTP_SERVER_PORT, HTTP_SERVER_PORT);
    httpd_resp_set_type(req, "text/html");

    return httpd_resp_sendstr(req, html);
}