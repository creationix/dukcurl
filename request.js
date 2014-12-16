"use strict";
var newCurl = require('./curl.so');
var p = require('./modules/utils.js').prettyPrint;


// Shared curl agent for keepalive and shared connections.
var curl = newCurl();

function get(url, headers) {
  curl.reset();
  curl.setopt("useragent", "DukLuv libcurl bindings");
  curl.setopt("httpget", true);
  curl.setopt("url", url);
  var chunks = [];
  var outHeaders = {};
  var length = 0;
  if (headers) {
    curl.setopt("httpheader", headers);
  }
  curl.setopt("followlocation", true);
  var end = false;
  curl.perform(function (chunk) {
    chunks.push(chunk);
    length += chunk.length;
    return chunk.length;
  }, function (header) {
    var length = header.length;
    header = header.toString();
    if (end) {
      // Only remember headers for last response.
      end = false;
      outHeaders = {};
    }
    if (header === "\r\n") {
      end = true;
    }
    var match = header.toString().match(/^([^:]+): *([^\r]+)/);
    if (match) {
      outHeaders[match[1].toLowerCase()] = match[2];
    }
    return length;
  });
  return {
    code: curl.getinfo("response-code"),
    headers: outHeaders,
    body: JSON.parse(chunks.join(""))
  };
}

// Replace with your own github token
var headers = [
  "Authorization: token 25d25503a512e22994507cbcc16d92d52e563622"
];
var res = get("https://api.github.com/repos/creationix/dukcurl/git/refs/heads/master", headers);
p(res);
res = get(res.body.object.url, headers);
p(res);
res = get(res.body.tree.url, headers);
p(res);
