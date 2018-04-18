#ifndef WEBPAGE_H
#define WEBPAGE_H

static const char PROGMEM INDEX_HTML[] = R"rawliteral(



<!DOCTYPE html>
<html>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>ESP8266 WebSocket Demo</title>
<style>
"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
</style>
<script>
var websock;
function start() {
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  websock.onopen = function(evt) { console.log('websock open'); };
  websock.onclose = function(evt) { console.log('websock close'); };
  websock.onerror = function(evt) { console.log(evt); };
  websock.onmessage = function(evt) {
    console.log(evt);
    var e = document.getElementById('ledstatus');
    if (evt.data === 'lightson') {
      e.style.color = 'red';
    }
    else if (evt.data === 'lightsoff') {
      e.style.color = 'black';
    }
    else {
      console.log('unknown event');
    }
  };
}
function buttonclick(e) {
  websock.send(e.id);
}
</script>
</head>
<body onload="javascript:start();">
<h1>ESP8266 Infinity Mirror</h1>
<div id="ledstatus"><b>LED</b></div>
<button id="lightson"  type="button" onclick="buttonclick(this);">On</button> 
<button id="lightsoff" type="button" onclick="buttonclick(this);">Off</button>

<div id="animation"><b>Animation</b></div>
<button id="animationprevious" type="button" onclick="buttonclick(this);">Previous</button>
<button id="animationnext"  type="button" onclick="buttonclick(this);">Next</button> 
</body>
</html>


)rawliteral";

#endif
