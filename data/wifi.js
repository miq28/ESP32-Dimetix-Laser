var ws = null;

window.onload = function () {
    // getConfigValues("status/network", reqListenerStatusNetwork);
    getConfigValues("config/network", reqListenerConfigNetwork);
    // setInterval(getConfigValues, 10000, "status/network", reqListenerStatusNetwork);
    // startSocket();
    //startEvents();

    // scanWifi();

    // load("bulma-ui.js", "js", function () {
    //   // Do something after load...
    // });

    // document.getElementById("btnScanWifi").disabled = true;
}
var notifTimeout = null;
function showNotification(b) {
    // var a = $("#notification");
    var a = document.getElementById("notification");
    a.innerHTML = b;
    a.removeAttribute("hidden");
    if (notifTimeout != null) {
        clearTimeout(notifTimeout)
    } notifTimout = setTimeout(function () {
        a.setAttribute("hidden", "");
        notifTimout = null
    }, 4000)
}

function startSocket() {
    ws = new WebSocket('ws://' + document.location.host + '/ws', ['arduino']);
    ws.binaryType = "arraybuffer";
    ws.onopen = function (evt) {
        var obj = new Object();
        obj.url = window.location.pathname;
        var myJSON = JSON.stringify(obj);
        // ws.send(myJSON);
        ws.send(obj.url);
    };
    ws.onclose = function (evt) {
        console.log("WebSocket close. Code: " + evt.code + ", Reason: " + evt.reason);
        // clearInterval(sendPingVar);
        console.log(clearInterval.name);
        // setMsg("active", "Disconnected");
        if (ws.readyState == 3) {
            // showDisconnected();
            setTimeout(function () { showDisconnected(); }, 1000);
        }
    };
    ws.onerror = function (evt) {
        console.log("ws error", evt);
    };
    ws.onmessage = function (evt) {
        var msg = "";
        if (evt.data instanceof ArrayBuffer) {
            msg = "BIN:";
            var bytes = new Uint8Array(evt.data);
            for (var i = 0; i < bytes.length; i++) {
                msg += String.fromCharCode(bytes[i]);
            }
        } else {
            msg = "TXT:" + evt.data;
            console.log(msg);
            if (isJSON(evt.data)) {
                var json = JSON.parse(evt.data);
            }
        }
    };
}

function hideShowKey() {
    // document.getElementsByName('hiddenapikey')[0].style.display = "none";

    // var str = document.getElementById('btnHideShow').textContent;
    // console.log(str);
    // if (str === "Show") {
    //   // document.getElementsByName('province')[0].style.display = "inline";
    //   document.getElementById('apikey').style.display = "block";
    //   document.getElementById('hiddenapikey').style.display = "none";
    //   document.getElementById('btnHideShow').textContent = "Hide";
    // }
    // else if (str === "Hide") {
    //   document.getElementById('apikey').style.display = "none";
    //   document.getElementById('hiddenapikey').style.display = "block";
    //   document.getElementById('btnHideShow').textContent = "Show";
    // }

    var x = document.getElementById("password");
    if (x.type === "password") {
        x.type = "text";
        document.getElementById("showpassword").innerHTML = "Hide";
    } else {
        x.type = "password";
        document.getElementById("showpassword").innerHTML = "Show";
    }
}

// https://stackoverflow.com/questions/3710204/how-to-check-if-a-string-is-a-valid-json-string-in-javascript-without-using-try
function isJSON(str) {
    try {
        return (JSON.parse(str) && !!str);
    } catch (e) {
        return false;
    }
}
function securityStr(security) {
    if (security == 0) {
        return 'Open';
    }
    else if (security == 1) {
        return 'WEP';
    }
    else if (security == 2) {
        return 'WPA';
    }
    else if (security == 3) {
        return 'WPA2';
    }
    else if (security == 4) {
        return 'WPA_WPA2';
    }
    else if (security == 5) {
        return 'WPA2_ENTERPRISE';
    }
    else if (security == 6) {
        return 'WPA3';
    }
    else if (security == 7) {
        return 'WPA2_WPA3';
    }
    else if (security == 8) {
        return 'WAPI';
    }
}

function scanWifi() {
    console.log("scanWifi()");
    document.getElementById("spin").style.display = "inline-flex";
    document.getElementById("numNets").innerHTML = "Scanning WiFi... ";
    document.getElementById("btnScanWifi").disabled = true;
    document.getElementById("notification").hidden = true;
    document.getElementById("btnScanWifi").setAttribute("aria-busy", true);
    var table = document.getElementById("tableAP");
    // table.classList.toggle('is-active');
    table.style.display = "none";

    request = new XMLHttpRequest();
    if (request) {
        // request.open("GET", "/scan", true);
        request.open("GET", "/scan", function () {
            showNotification("Wifi scan started");
            // window.setTimeout(scanResult, 1000)
            console.log("Wifi scan started1");
        });
        request.addEventListener("load", processScanResult);

        request.timeout = 10000;
        console.log("wait for 10000 ms");
        request.ontimeout = function (e) {
            console.log("timeout");
            request.abort();
        };

        request.send();
    }
}

function processScanResult(res) {
    console.log("processScanResult()");
    let scanJson = res.target.responseText;
    // console.log(scanJson);
    if (scanJson == '[]') {
        setTimeout(function () { scanWifi(); }, 1000);
        // console.log(res.target.responseText);
        return;
    }
    // console.log(res.target.responseText);
    var array;
    array = JSON.parse(scanJson);
    console.log(array);
    array.sort(function (a, b) { return a.rssi - b.rssi });
    array.reverse();
    document.getElementById("numNets").innerHTML = array.length + " networks found";
    showNotification(array.length + " networks found");


    // classList.toggle('is-active');
    var table = document.getElementById("tableAP");
    table.setAttribute("role", "grid");
    // table.classList.toggle('is-active');
    table.style.display = "inline-table";
    var body = document.getElementById("foundAP");
    body.innerHTML = "";
    for (var i = 0; i < array.length; i++) {
        var row = document.createElement("tr");
        // row.innerHTML = "<td><input id=\"radio\" type=\"radio\" name=\"answer\"></td>" + "<td><label for=\"radio1\" class=\"radio\">Yes</label></td>";
        row.innerHTML = "<td><input id=\"radio" + i + "\"" + " type=\"radio\" name=\"selected-ssid\" value=\"" + array[i].ssid + "\"></td>" + "<td><label for=\"radio" + i + "\"" + " class=\"radio\">" + array[i].ssid + "</label></td>" + "<td>" + array[i].rssi + "</td>" + "<td>" + securityStr(array[i].secure) + "</td>";
        body.appendChild(row);
    }

    var sel_ssid = document.forms['formA'].elements['selected-ssid'];
    // console.log(sel_ssid);

    var len = sel_ssid.length;
    // loop through list
    for (var i = 0; i < len; i++) {
        sel_ssid[i].onclick = function () { // assign onclick handler function to each
            // put clicked radio button's value in total field
            // this.form.elements.total.value = this.value;
            document.getElementById("ssid").value = this.value;
            document.getElementById("password").value = "";
        };
    }

    document.getElementById("btnScanWifi").disabled = false;
    document.getElementById("btnScanWifi").setAttribute("aria-busy", false);
    document.getElementById("spin").style.display = "none";

}


// var form = document.querySelector("form");
var form = document.getElementById("formA");
var log = document.querySelector("#log");

form.addEventListener("submit", function (event) {
    var data = new FormData(form);
    var output = "";
    for (const entry of data) {
        output = entry[0] + "=" + entry[1] + "\r";
    };
    log.innerText = output;
    event.preventDefault();
}, false);


function scanAPs() {
    if (blockScan) {
        scanTimeout = window.setTimeout(scanAPs, 1000);
        return
    } scanTimeout = null;
    scanReqCnt = 0;
    ajaxReq("POST", "scan", function (a) {
        showNotification("Wifi scan started");
        window.setTimeout(scanResult, 1000)
        console.log("Wifi scan started2");
    }, function (b, a) {
        if (b == 400) {
            showWarning("Cannot scan in AP mode");
            $("#aps").innerHTML = 'Switch to <a href="#" onclick="changeWifiMode(3)">STA+AP mode</a> to scan.'
        } else {
            showWarning("Failed to scan: " + a)
        }
    })
}

function selssid(value) {
    document.getElementById("ssid").value = value;
}
function getConfigValues(url, fCallback) {
    console.log("getConfigValues()");
    var req = new XMLHttpRequest();
    if (req) {
        req.open("GET", url, true);
        req.addEventListener("load", fCallback);
        req.timeout = 500;
        req.ontimeout = function (e) {
            console.log("getConfigValues timeout");
            req.abort();
        };
        req.send();
    }
}

var dhcp = document.getElementsByName("dhcp");

var static_ip = document.getElementsByName("static_ip");

var len = dhcp.length;
for (var i = 0; i < len; i++) {
    dhcp[i].onclick = function () { // assign onclick handler function to each
        // put clicked radio button's value in total field
        // this.form.elements.total.value = this.value;
        var len_s = static_ip.length;
        for (var i = 0; i < len_s; i++) {
            if (document.getElementsByName("dhcp")[0].checked == true) {
                static_ip[i].style.display = "none";
            }
            else {
                static_ip[i].style.display = "block";
            }

        }
    };
}

function reqListenerConfigNetwork() {
    console.log("reqListenerConfigNetwork()");
    // console.log(this.responseText);
    let json = JSON.parse(this.responseText);
    console.log(json);
    document.getElementById("hostname").value = json.hostname;
    document.getElementById("ssid").value = json.ssid;
    document.getElementById("password").value = json.password;

    // DHCP
    // var dhcp = document.forms['formA'].elements['selected-ssid'];
    var dhcp_settings = document.getElementsByName("dhcp");
    // console.log(dhcp_settings);

    // var len = dhcp_settings.length;
    // // loop through list
    // for (var i=0; i<len; i++)
    // {
    //     dhcp_settings[i].onclick = function() { // assign onclick handler function to each
    //         // put clicked radio button's value in total field
    //         // this.form.elements.total.value = this.value;
    //         document.getElementById("ssid").value = this.value;
    //     };
    // }

    if (json.dhcp) {
        document.getElementsByName("dhcp")[0].checked = true;

        var static_ip = document.getElementsByName("static_ip");

        var len = static_ip.length;
        for (var i = 0; i < len; i++) {
            static_ip[i].style.display = "none";
        }
    }
    else {
        document.getElementsByName("dhcp")[1].checked = true;
        var static_ip = document.getElementsByName("static_ip");

        var len = static_ip.length;
        for (var i = 0; i < len; i++) {
            static_ip[i].style.display = "block";
        }

        // static_ip[i].onclick = function() { // assign onclick handler function to each
        //     // put clicked radio button's value in total field
        //     // this.form.elements.total.value = this.value;
        //     document.getElementById("ssid").value = this.value;
        // };
    }
    // document.getElementsByName("dhcp")[0] = json.dhcp;
    document.getElementById("static_ip").value = json.static_ip;
    document.getElementById("netmask").value = json.netmask;
    document.getElementById("gateway").value = json.gateway;
    document.getElementById("dns0").value = json.dns0;
    document.getElementById("dns1").value = json.dns1;

    // if (document.getElementById("dhcp").checked == true) {
    //   document.getElementById('static_ip').disabled = true;
    //   document.getElementById('netmask').disabled = true;
    //   document.getElementById('gateway').disabled = true;
    //   document.getElementById('dns0').disabled = true;
    //   document.getElementById('dns1').disabled = true;
    // }
}
function reqListenerStatusNetwork() {
    console.log(this.responseText);
    let json = JSON.parse(this.responseText);
    console.log(json);
    document.getElementById("s_chipid").textContent = json.chipid;
    document.getElementById("s_hostname").textContent = json.hostname;
    document.getElementById("s_status").textContent = json.status;
    document.getElementById("s_mode").textContent = json.mode;
    document.getElementById("s_phymode").textContent = json.phymode;
    document.getElementById("s_channel").textContent = json.channel;
    document.getElementById("s_ssid").textContent = json.ssid;
    document.getElementById("s_password").textContent = json.password;
    document.getElementById("s_encryption").textContent = json.encryption;
    document.getElementById("s_isconnected").textContent = json.isconnected;
    document.getElementById("s_autoconnect").textContent = json.autoconnect;
    document.getElementById("s_persistent").textContent = json.persistent;
    document.getElementById("s_bssid").textContent = json.bssid;
    document.getElementById("s_rssi").textContent = json.rssi;
    document.getElementById("s_sta_ip").textContent = json.sta_ip;
    document.getElementById("s_sta_mac").textContent = json.sta_mac;
    document.getElementById("s_ap_ip").textContent = json.ap_ip;
    document.getElementById("s_ap_mac").textContent = json.ap_mac;
    document.getElementById("s_gateway").textContent = json.gateway;
    document.getElementById("s_netmask").textContent = json.netmask;
    document.getElementById("s_dns0").textContent = json.dns0;
    document.getElementById("s_dns1").textContent = json.dns1;
}
function something() {
    var dhcpVal = document.getElementById('dhcp').checked;
    console.log(dhcpVal);
    if (dhcpVal) {
        document.getElementById('static_ip').disabled = true;
        document.getElementById('netmask').disabled = true;
        document.getElementById('gateway').disabled = true;
        document.getElementById('dns0').disabled = true;
        document.getElementById('dns1').disabled = true;
    } else {
        document.getElementById('static_ip').disabled = false;
        document.getElementById('netmask').disabled = false;
        document.getElementById('gateway').disabled = false;
        document.getElementById('dns0').disabled = false;
        document.getElementById('dns1').disabled = false;
    }
}
// function sendConfig(element) {
//   let json = new Object();
//   // json.saveconfig = window.location.pathname;
//   json.ssid = document.getElementById("ssid").value;
//   json.password = document.getElementById("password").value;
//   json.dhcp = document.getElementById("dhcp").checked;
//   json.static_ip = document.getElementById("static_ip").value;
//   json.netmask = document.getElementById("netmask").value;
//   json.gateway = document.getElementById("gateway").value;
//   json.dns0 = document.getElementById("dns0").value;
//   json.dns1 = document.getElementById("dns1").value;
//   var myJSON = JSON.stringify(json);
//   ws.send(myJSON);
//   console.log("Data sent: ", myJSON);
// }
function prepareConfigJSON() {
    let json = new Object();
    // json.saveconfig = window.location.pathname;
    json.hostname = document.getElementById("hostname").value;
    json.ssid = document.getElementById("ssid").value;
    if (json.ssid === "") {
        document.getElementById("ssid").className += " is-danger";
        alert("WiFi SSID cannot be empty");
        return false;
    }

    json.password = document.getElementById("password").value;

    if (json.password === "") {
        var answer = confirm("Connect without password?\r\nESP will reboot to apply the new settings");
        if (!answer) return false;
    } else {
        var answer = confirm("Confirm to proceed?\r\nESP will reboot to apply the new settings");
        if (!answer) return false;
    }

    // json.mode = document.getElementById("mode").value;
    json.dhcp = document.getElementsByName("dhcp")[0].checked;
    json.static_ip = document.getElementById("static_ip").value;
    json.netmask = document.getElementById("netmask").value;
    json.gateway = document.getElementById("gateway").value;
    json.dns0 = document.getElementById("dns0").value;
    json.dns1 = document.getElementById("dns1").value;

    return (JSON.stringify(json));
}
function sendConfig() {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", '/confignetwork', true);

    //Send the proper header information along with the request
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    // xhr.setRequestHeader("Content-Type", "text/plain");
    // xhr.setRequestHeader("Content-Type", "multipart/form-data");

    xhr.onreadystatechange = function () {//Call a function when the state changes.
        if (this.readyState == XMLHttpRequest.DONE && this.status == 200) {
            // Request finished. Do processing here.
        }
    }

    var configJSON = prepareConfigJSON();
    if (!configJSON) {
        return;
    }
    var config_str = "saveconfig=" + configJSON;
    console.log(config_str);

    xhr.timeout = 1000;
    xhr.ontimeout = function (e) {
        console.log("timeout");
        xhr.abort();
    };

    xhr.send(config_str);
}