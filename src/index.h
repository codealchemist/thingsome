#include <Arduino.h>

String INDEX(String status) {
  String html = "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/html\r\n\r\n"
  "<!DOCTYPE HTML>"
  "<html>"
   "<head>"
     "<style>"
       "body {"
         "font: normal normal normal 14px arial;"
         "padding: 0 20px;"
       "}"
       ".status {"
         "background-color: #d9edf7;"
         "border-color: #bce8f1;"
         "color: #31708f;"
         "padding: 15px;"
         "margin: 20px 0;"
         "border-radius: 4px;"
       "}"
     "</style>"
   "</head>"
  "<body>"
  "<h1>IOT DEVICE</h1><br>"
  "ID: <b>" + DEVICE_ID + "</b><br>"
  "Name: <b>" + DEVICE_NAME + "</b><br>"
  "Type: <b>" + DEVICE_TYPE + "</b><br>"
  "Description: <em>" + DEVICE_DESCRIPTION + "</em><br>";

  if (status.length()) {
    html += "<div class='status'>" + status + "</div>";
  }

  html += ""
  "</body>"
  "</html>";
  return html;
}
