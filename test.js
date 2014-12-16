"use strict";

var newCurl = require('./curl.so');

var curl = newCurl();

curl.setopt("url", "http://luvit.io");
// luvit.io is redirected, so we tell libcurl to follow redirection
curl.setopt("followlocation", true);

// Set a useragent to make some servers happy
curl.setopt("useragent", "libcurl-agent/1.0");

// Intercept the write stream to handle in JavaScript
curl.setopt("writefunction", function (data) {
  print("DATA", data.length);
  return data.length;
});

// Perform the request.
curl.perform();

// read some info parameters
[
  "effective-url",
  "total-time",
  "namelookup-time",
  "connect-time",
  "appconnect-time",
  "pretransfer-time",
  "starttransfer-time",
  "redirect-time",
  "content-type",
  "response-code",
  "primary-ip",
  "primary-port",
  "local-ip",
  "local-port",
].forEach(function (name) {
  print(name, curl.getinfo(name));
});

// TODO: get results

