#include "WifiCaptive.h"
#include <trmnl_log.h>

void WifiCaptive::setUpDNSServer(DNSServer &dnsServer, const IPAddress &localIP)
{
    dnsServer.setTTL(3600);
    dnsServer.start(53, "*", localIP);
}

void WifiCaptive::setUpWebserver(AsyncWebServer &server, const IPAddress &localIP)
{
    //======================== Webserver ========================
    // WARNING IOS (and maybe macos) WILL NOT POP UP IF IT CONTAINS THE WORD "Success" https://www.esp8266.com/viewtopic.php?f=34&t=4398
    // SAFARI (IOS) IS STUPID, G-ZIPPED FILES CAN'T END IN .GZ https://github.com/homieiot/homie-esp8266/issues/476 this is fixed by the webserver serve static function.
    // SAFARI (IOS) there is a 128KB limit to the size of the HTML. The HTML can reference external resources/images that bring the total over 128KB
    // SAFARI (IOS) popup browser has some severe limitations (javascript disabled, cookies disabled)

    // Required
    server.on("/connecttest.txt", [](AsyncWebServerRequest *request)
              { request->redirect("http://logout.net"); }); // windows 11 captive portal workaround
    server.on("/wpad.dat", [](AsyncWebServerRequest *request)
              { request->send(404); }); // Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

    // Background responses: Probably not all are Required, but some are. Others might speed things up?
    // A Tier (commonly used by modern systems)
    server.on("/generate_204", [](AsyncWebServerRequest *request)
              { request->redirect(LocalIPURL); }); // android captive portal redirect
    server.on("/redirect", [](AsyncWebServerRequest *request)
              { request->redirect(LocalIPURL); }); // microsoft redirect
    server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request)
              { request->redirect(LocalIPURL); }); // apple call home
    server.on("/canonical.html", [](AsyncWebServerRequest *request)
              { request->redirect(LocalIPURL); }); // firefox captive portal call home
    server.on("/success.txt", [](AsyncWebServerRequest *request)
              { request->send(200); }); // firefox captive portal call home
    server.on("/ncsi.txt", [](AsyncWebServerRequest *request)
              { request->redirect(LocalIPURL); }); // windows call home

    // return 404 to webpage icon
    server.on("/favicon.ico", [](AsyncWebServerRequest *request)
              { request->send(404); }); // webpage icon

    // Serve index.html
    server.on("/", HTTP_ANY, [&](AsyncWebServerRequest *request)
              {
		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", INDEX_HTML, INDEX_HTML_LEN);
		response->addHeader("Content-Encoding", "gzip");
    	request->send(response); });

    // Servce logo.svg
    server.on("/logo.svg", HTTP_ANY, [&](AsyncWebServerRequest *request)
              {
		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", LOGO_SVG, LOGO_SVG_LEN);
		response->addHeader("Content-Encoding", "gzip");
		response->addHeader("Content-Type", "image/svg+xml");
    	request->send(response); });

    server.on("/soft-reset", HTTP_ANY, [&](AsyncWebServerRequest *request)
              {
		resetSettings();
		if (_resetcallback != NULL) {
			_resetcallback(); // @CALLBACK
		}
		request->send(200); });

    auto scanGET = server.on("/scan", HTTP_GET, [&](AsyncWebServerRequest *request)
                             {
		String json = "[";
		int n = WiFi.scanComplete();
		if (n == WIFI_SCAN_FAILED) {
			WiFi.scanNetworks(true);
			return request->send(202);
		} else if(n == WIFI_SCAN_RUNNING){
			return request->send(202);
		} else {
			// Data structure to store the highest RSSI for each SSID
            // Warning: DO NOT USE true on this function in an async context!
            std::vector<Network> uniqueNetworks = _wifiConnector.getScannedUniqueNetworks(false);
            std::vector<Network> combinedNetworks =
                _credentialStore.annotateNetworksWithSavedStatus(uniqueNetworks);

            // Generate JSON response
            size_t size = 0;
            for (const auto &network : combinedNetworks)
            {
                String ssid = network.ssid;
				String rssi = String(network.rssi);

				// Escape invalid characters
				ssid.replace("\\","\\\\");
				ssid.replace("\"","\\\"");
				json+= "{";
				json+= "\"name\":\""+ssid+"\",";
				json+= "\"rssi\":\""+rssi+"\",";
				json+= "\"open\":"+String(network.open == WIFI_AUTH_OPEN ? "true,": "false,");
                json+= "\"saved\":"+String(network.saved ? "true": "false");
				json+= "}";

				size += 1;

				if (size != combinedNetworks.size())
				{
					json+= ",";
				}
            }

            WiFi.scanDelete();
			Serial.println(json);

			if (WiFi.scanComplete() == -2){
				WiFi.scanNetworks(true);
			}
		}
		json += "]";
		request->send(200, "application/json", json);
		json = String(); });

    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/connect", [&](AsyncWebServerRequest *request, JsonVariant &json)
                                                                           {
		JsonObject data = json.as<JsonObject>();
		String ssid = data["ssid"];
		String pswd = data["pswd"];
        String api_server = data["server"];
		_ssid = ssid;
		_password = pswd;
        _api_server = api_server;
		String mac = WiFi.macAddress();
		String message =  "{\"ssid\":\"" + _ssid +"\",\"mac\":\"" + mac +"\"}";
		request->send(200, "application/json", message); });

    server.addHandler(handler);

    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->redirect(LocalIPURL); });
}

bool WifiCaptive::startPortal()
{
    _dnsServer = new DNSServer();
    _server = new AsyncWebServer(80);

    // Set the WiFi mode to access point and station
    WiFi.mode(WIFI_MODE_AP);

    // Define the subnet mask for the WiFi network
    const IPAddress subnetMask(255, 255, 255, 0);
    const IPAddress localIP(4, 3, 2, 1);
    const IPAddress gatewayIP(4, 3, 2, 1);

    WiFi.disconnect();
    delay(50);

    // Configure the soft access point with a specific IP and subnet mask
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    delay(50);

    // Start the soft access point with the given ssid, password, channel, max number of clients
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, 0, MAX_CLIENTS);
    delay(50);

    // Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
    esp_wifi_stop();
    esp_wifi_deinit();
    wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
    my_config.ampdu_rx_enable = false;
    esp_wifi_init(&my_config);
    esp_wifi_start();
    vTaskDelay(100 / portTICK_PERIOD_MS); // Add a small delay

    // configure DSN and WEB server
    setUpDNSServer(*_dnsServer, localIP);
    setUpWebserver(*_server, localIP);

    // begin serving
    _server->begin();

    // start async network scan
    WiFi.scanNetworks(true);

    _credentialStore.readCredentials();

    bool succesfullyConnected = false;
    // wait until SSID is provided
    while (1)
    {
        _dnsServer->processNextRequest();

        if (_ssid == "")
        {
            delay(DNS_INTERVAL);
        }
        else
        {
            bool res = _wifiConnector.connect(_ssid, _password) == WL_CONNECTED;
            if (res)
            {
                _credentialStore.saveWifiCredentials(_ssid, _password);
                _credentialStore.saveApiServer(_api_server);
                succesfullyConnected = true;
                break;
            }
            else
            {
                _ssid = "";
                _password = "";

                WiFi.disconnect();
                WiFi.enableSTA(false);
                break;
            }
        }
    }

    // SSID provided, stop server
    WiFi.scanDelete();
    WiFi.softAPdisconnect(true);
    delay(1000);

    auto status = WiFi.status();
    if (status != WL_CONNECTED)
    {
        Log_info("Not connected after AP disconnect");
        WiFi.mode(WIFI_STA);
        WiFi.begin(_ssid.c_str(), _password.c_str());
        _wifiConnector.waitForConnectResult();
    }

    // stop dsn
    _dnsServer->stop();
    delete _dnsServer;
    _dnsServer = nullptr;

    // stop server
    _server->end();
    delete _server;
    _server = nullptr;

    return succesfullyConnected;
}

void WifiCaptive::resetSettings()
{
    _credentialStore.clearSavedWifiCredentials();
    _credentialStore.clearSavedApiUrl();

    WiFi.disconnect(true, true);
}

void WifiCaptive::setResetSettingsCallback(std::function<void()> func)
{
    _resetcallback = func;
}

bool WifiCaptive::isSaved()
{
    _credentialStore.readCredentials();
    return _credentialStore.hasCredentials();
}

bool WifiCaptive::tryConnectWithRetries(const String &ssid, const String &password)
{
    for (int attempt = 0; attempt < WIFI_CONNECTION_ATTEMPTS; attempt++)
    {
        Log_info("Attempt %d to connect to %s", attempt + 1, ssid.c_str());
        _wifiConnector.connect(ssid, password);

        // Check if connected
        if (WiFi.status() == WL_CONNECTED)
        {
            Log_info("Connected to %s", ssid.c_str());
            return true;
        }
        WiFi.disconnect();

        // Exponential backoff: 2s, 4s, 8s delays between attempts
        if (attempt < WIFI_CONNECTION_ATTEMPTS - 1)
        {
            uint32_t backoff_delay = 2000 * (1 << attempt); // 2^attempt * 2000ms
            Log_info("Connection failed, waiting %d ms before retry...", backoff_delay);
            delay(backoff_delay);
        }
    }
    return false;
}

bool WifiCaptive::autoConnect()
{
    Log_info("Trying to autoconnect to wifi...");
    _credentialStore.readCredentials();

    // if last used network is available, try to connect to it
    if (_credentialStore.hasLastUsedCredential())
    {
        WifiCreds lastUsed = _credentialStore.getLastUsedCredential();
        Log_info("Trying to connect to last used %s...", lastUsed.ssid.c_str());
        WiFi.setSleep(0);
        WiFi.setMinSecurity(WIFI_AUTH_OPEN);
        WiFi.mode(WIFI_STA);

        if (tryConnectWithRetries(lastUsed.ssid, lastUsed.pswd))
        {
            return true;
        }
    }

    Log_info("Last used network unavailable, scanning for known networks...");
    std::vector<Network> scanResults = _wifiConnector.getScannedUniqueNetworks(true);
    std::vector<WifiCreds> sortedNetworks = _credentialStore.getPrioritizedCredentials(scanResults);

    WiFi.mode(WIFI_STA);
    WifiCreds lastUsed = _credentialStore.getLastUsedCredential();
    for (auto &network : sortedNetworks)
    {
        if (network.ssid == "" || (network.ssid == lastUsed.ssid && network.pswd == lastUsed.pswd))
        {
            continue;
        }

        Log_info("Trying to connect to saved network %s...", network.ssid.c_str());

        if (tryConnectWithRetries(network.ssid, network.pswd))
        {
            _credentialStore.saveLastUsedSsid(network.ssid);
            return true;
        }
    }

    Log_info("Failed to connect to any network");
    return false;
}

WifiCaptive WifiCaptivePortal;
