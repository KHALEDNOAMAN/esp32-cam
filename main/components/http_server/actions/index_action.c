//
// Created by admin on 10.05.2026.
//
#include "components/config/config.h"
#include "components/camera/camera.h"
#include "esp_http_server.h"
#include <stdio.h>

esp_err_t index_handler(httpd_req_t *req) {
    static char html[3048];

    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html><head>"
        "<meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>ESP32-CAM</title>"
        "<style>"
        "body{margin:0;background:#0d0d0d;color:#e0e0e0;font-family:monospace}"
        "h2{color:#00e5ff;margin:12px}"
        "#wrap{display:flex;flex-direction:column;align-items:center;padding:12px}"
        "#stream{max-width:100%%;border:2px solid #00e5ff;border-radius:4px;background:#111;min-height:160px}"
        ".btns{display:flex;flex-wrap:wrap;gap:8px;margin:10px 0}"
        "button{background:#00e5ff;color:#000;border:0;padding:8px 16px;"
               "font-size:13px;font-family:monospace;cursor:pointer;border-radius:4px;font-weight:bold}"
        "button:hover{background:#00b8d4}"
        "button.red{background:#ff5252;color:#fff}"
        "#fps{color:#00e5ff;font-size:12px;margin:4px}"
        "#info{font-size:11px;color:#aaa;white-space:pre;margin-top:8px;"
              "background:#1a1a1a;padding:8px;border-radius:4px;width:100%%;box-sizing:border-box}"
        "</style></head><body>"
        "<div id='wrap'>"
        "<h2>&#128247; ESP32-CAM</h2>"
        "<img id='s' src='/stream' style='max-width:640px'/>"
        "<div id='fps'></div>"
        "<div class='btns'>"
        "<button onclick=\"document.getElementById('s').src='/stream?t='+Date.now()\">&#9654; Stream</button>"
        "<button class='red' onclick=\"document.getElementById('s').src='';document.getElementById('fps').textContent=''\">&#9646; Stop</button>"
        "<button onclick=\"window.open('/capture')\">&#128247; Snapshot</button>"
        "<button onclick=\"fetch('/save',{method:'POST'}).then(r=>r.json()).then(d=>document.getElementById('info').textContent=JSON.stringify(d,null,2))\">&#128190; Save SD</button>"
        "<button onclick=\"fetch('/status').then(r=>r.json()).then(d=>document.getElementById('info').textContent=JSON.stringify(d,null,2))\">&#128202; Status</button>"
        "<button onclick=\"fetch('/flash?v=255')\">💡 Flash ON</button>"
        "<button onclick=\"fetch('/flash?v=128')\">💡 50%%</button>"
        "<button onclick=\"fetch('/flash?v=0')\" class='red'>💡 Flash OFF</button>"
        "</div>"
        "<pre id='info'>Ready.</pre>"
        "</div>"
        "<script>"
        "var s=document.getElementById('s'),fc=0,lt=Date.now();"
        "s.onload=function(){fc++;var n=Date.now();if(n-lt>=1000){document.getElementById('fps').textContent=(fc*1000/(n-lt)).toFixed(1)+' fps';fc=0;lt=n;}};"
        "</script></body></html>");
    httpd_resp_set_type(req, "text/html");

    return httpd_resp_sendstr(req, html);
}