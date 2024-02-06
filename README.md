# TRMNL Firmware

[insert something helpful]

**Web Server Endpoints**

following Wifi connection, swap Mac Address for API Key and Friendly ID (which get saved on device).

```curl
GET `/api/setup`

headers = {
  'ID' => 'XX:XX:XX:XX'
}

response example (success):
{ "status" => 200, "api_key" => "2r--SahjsAKCFksVcped2Q", friendly_id: "917F0B", image_url: '/images/setup/setup-logo.png' }

response example (fail, device with this Mac Address not found)
{ "status" => 404, "api_key" => nil, friendly_id: nil }
```
request for image / display content

```curl
GET `/api/display`

headers = {
  'ID' => 'XX:XX:XX:XX',
  'Access-Token' => '2r--SahjsAKCFksVcped2Q',
  'Refresh-Rate' => '1800' 
  'Battery-Voltage' => '4100' # not currently in use by web server
  'FW-Version' => '0.1.3' 
}

response example (success, device found with this access token):
{
  "image_url"=>"/images/sample_screens/shopify_orders_black.bmp",
  "firmware_url"=>nil,
  "refresh_rate"=>"1800"
}

response example (success, device found AND needs firmware update):
{
 "image_url"=>"/images/sample_screens/close_crm_stats.bmp",
 "firmware_url"=>"/some-firmware-endpoint-here",
 "refresh_rate"=>"1800"
}

response example (fail, device not found for this access token):
{"image_url"=>nil, "firmware_url"=>nil, "refresh_rate"=>nil}

if 'FW-Version' header and web server `Device::FIRMWARE_VERSION` do not match, server will respond with endpoint from which to download new Firmware.
```

**Power consumption**

The image displays the amount of power consumed during a work cycle that involves downloading and displaying images.

![Image Alt text](/pics/Simple_cycle.jpg "Optional title"))

The image displays the amount of power consumed while in sleep mode

![Image Alt text](/pics/Sleep_cycle.jpg "Optional title"))

The image displays the amount of power consumed during a work cycle that involves link pinging, new firmware downloading and OTA.