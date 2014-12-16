"use strict";
var newCurl = require('./curl.so');
var p = require('./modules/utils.js').prettyPrint;


// Shared curl agent for keepalive and shared connections.
var curl = newCurl();

function request(url, body, headers) {
  var chunks = [];
  var outHeaders = {};
  var length = 0;
  var end = false;

  curl.setopt("useragent", "DukLuv libcurl bindings");
  curl.setopt("url", url);
  curl.setopt("followlocation", true);

  curl.setopt("writefunction", function (chunk) {
    chunks.push(chunk);
    length += chunk.length;
    return chunk.length;
  });

  curl.setopt("headerfunction", function (header) {
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

  if (body) {
    if ((typeof body) !== "string") {
      body = JSON.stringify(body) + "\n";
      headers = headers || [];
      headers.push("Content-Type: application/json");
    }
    body = Duktape.Buffer(body);
    headers.push("Content-Length: " + body.length);
    curl.setopt("infilesize", body.length);
    curl.setopt("readfunction", function (size) {
      if (!body) { return ""; }
      var chunk;
      if (body.length < size) {
        chunk = body;
        body = "";
        return chunk;
      }
      throw new Error("TODO: handle large file uploads");
    });
  }

  if (headers) {
    curl.setopt("httpheader", headers);
  }

  curl.perform();

  return {
    code: curl.getinfo("response-code"),
    headers: outHeaders,
    body: JSON.parse(chunks.join(""))
  };
}

function get(url, headers) {
  curl.reset();
  curl.setopt("httpget", true);
  return request(url, null, headers);
}

function put(url, body, headers) {
  curl.reset();
  curl.setopt("put", true);
  return request(url, body, headers);
}

function post(url, body, headers) {
  curl.reset();
  curl.setopt("post", true);
  return request(url, body, headers);
}

var res = put("http://httpbin.org/put", {name:"Tim Caswell", age:32});
p(res);
res = post("http://httpbin.org/post", {name:"Tim Caswell", age:32});
p(res);


// Create token at https://github.com/settings/applications#personal-access-tokens
// To increate rate limit to 5000
var headers = [
  "Authorization: token 25d25503a512e22994507cbcc16d92d52e563622"
];
res = get("https://api.github.com/repos/creationix/dukcurl/git/refs/heads/master", headers);
p(res);
if (!res.body.object) { throw new Error(res.body.message); }
res = get(res.body.object.url, headers);
p(res);
if (!res.body.tree) { throw new Error(res.body.message); }
res = get(res.body.tree.url, headers);
p(res);

