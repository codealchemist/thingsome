#include <Arduino.h>

String RAW(String message, String status = "200 OK") {
  String output = ""
  "HTTP/1.1 " + status + " \r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "Content-Type: text/html\r\n\r\n";

  output += message;
  return output;
}
