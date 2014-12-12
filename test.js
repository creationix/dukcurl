"use strict";

var newCurl = require('./curl.so');

var curl = newCurl();
curl.setopt("url", "http://luvit.io");
/* luvit.io is redirected, so we tell libcurl to follow redirection */
curl.setopt("followlocation", true);
/* Perform the request, res will get the return code */
curl.perform();

// TODO: get results

